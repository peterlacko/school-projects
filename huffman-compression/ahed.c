/*
 * Autor: 		Peter Lacko, xlacko06
 * Datum: 		14.3.2015
 * Soubor:		ahed.c
 * Komentar:	Kniznica obsahujuca funkcie na kompresiu suborov
 * 				vyuzitim adaptivneho huffmanovho kodovania.
 */ 

#include "ahed.h"

map<int32_t, Code> symCodes;	// uchovava kody znakov pre rychly pristup
uint32_t buffer = 0;			// premenne na buffrovany zapis a citanie
uint32_t rwDone = 0;			// pocet precitanych/zapisanych bitov
int64_t uncodedSize = 0;	
int64_t codedSize = 0;
bool endOfFile = false;
uint32_t uint32Max = (uint32_t) pow(2, sizeof(uint32_t)*BYTE_SIZE);

/*
 * Pociatocna inicializacia uzlu
 */
void initNode(Node *n, int32_t c, uint32_t f, Node *l, Node *r, Node *p) {
	n->sym = c;
	n->freq = f;
	(n->code).c = 0; // spociatku prazdne 
	(n->code).len = 0;
	n->left = l;
	n->right = r;
	n->parent = p;
	n->visited = 0;
}

/*
 * vrati bitovu masku v podobe 0000000011111 a pod
 */
uint32_t getBitmask(int32_t len) {
	int32_t ret = 1;
	for (int32_t i = 1; i < len; i++) {
		ret = ret << 1;
		ret = ret | 1;
	}
	return ret;
}

const char *intToBinary(long int x) {
    static char b[BUF_SIZE+3];
    b[0] = '\0';
    long int z;
    for (z = (long int) pow(2, BUF_SIZE); z > 0; z = z >> 1)
    {
        strcat(b, ((x & z) == z) ? "1" : "0");
    }
    return b;
}

/*
 * Zapisuje obsah buffru po bytoch do suboru
 */
void mywrite(FILE *file) {
	codedSize += fwrite(&buffer, 1, (rwDone%BYTE_SIZE==0?
				rwDone/BYTE_SIZE:(rwDone/BYTE_SIZE)+1), file);
}

/*
 * Funkcia sluzi na buffrovany zapis kodu znaku do suboru
 */
void bufferedWrite(FILE *file, Code code) {
	// zapisem cely kod naraz
	if (code.len <= (BUF_SIZE - rwDone)) {
		uint32_t aux = (code.c << rwDone);
		buffer = buffer | aux;
		rwDone += code.len;
		if (rwDone == BUF_SIZE) {
			mywrite(file);
			rwDone = 0;
			buffer = 0;
		}
	}
	// v buffri nie je miesto, zapisem po castiach
	else {
		uint32_t aux = code.c << rwDone;
		uint32_t diff = (BUF_SIZE-rwDone);
		buffer = buffer | aux;
		rwDone = BUF_SIZE;
		mywrite(file);
		// a zbytok zapisem neskor
		rwDone = 0;
		buffer = 0;
		buffer = code.c >> diff;
		rwDone = code.len - diff;
	}
}

/*
 * Cita obsah suboru do buffru
 */
void myread(FILE *file) {
	codedSize+=fread(&buffer, 1, sizeof(uint32_t), file);
	int32_t c = getc(file);
	if (c != EOF) {
		ungetc((uint8_t) c, file);
	} else {
		endOfFile = true;
	}
}

/*
 * Buffrovane citanie znaku zo suboru. Precita sa bud cely byte
 * naraz (ak je v bufferi miesto), alebo v dvoch castiach.
 */
Code readBufferedByte(FILE *file) {
	if (rwDone == 0) {
		myread(file);
	}
	if (rwDone <= (BUF_SIZE - BYTE_SIZE)) {
		uint32_t mask = getBitmask(BYTE_SIZE);
		Code code = {.c = mask&buffer, .len=BYTE_SIZE};
		buffer = buffer >> BYTE_SIZE;
		rwDone += BYTE_SIZE;
		if (rwDone==BUF_SIZE) {
			rwDone = 0;
			buffer = 0;
		}
		return code;
	} else {
		uint32_t mask = getBitmask(BUF_SIZE-rwDone);
		Code code = {.c = mask&buffer, .len=BUF_SIZE-rwDone};
		myread(file);
		mask = getBitmask(BYTE_SIZE - code.len);
		uint32_t aux = mask & buffer;
		aux = aux << (code.len);
		code.c = code.c | aux;
		buffer = buffer >> (BYTE_SIZE - code.len);
		rwDone = BYTE_SIZE - code.len;
		code.len = BYTE_SIZE;
		if (rwDone==BUF_SIZE) {
			rwDone = 0;
			buffer = 0;
		}
		return code;
	}
}

/*
 * Cita subor bit po bite a sucasne prechadza strom a hlada prislusny
 * list so symbolom.
 */
Node *readBufferedNode(Node *root, FILE *file) {
	if (rwDone == 0) {
		myread(file);
	}
	Node *currNode = root;
	while (currNode->sym == -1) {
		if (rwDone == BUF_SIZE) {
			rwDone = 0;
			buffer = 0;
			myread(file);
		}
		if ((buffer & 1) == 0) {
			currNode = currNode->left;
		} else {
			currNode = currNode->right;
		}
		buffer = buffer >> 1;
		rwDone++;
	}
	if (rwDone==BUF_SIZE) {
		rwDone = 0;
		buffer = 0;
	}
	return currNode;
}

/*
 * Vrati kod pozadovaneho symbolu
 */
Code treeGetCodeSymbol(int32_t sym) {
	return symCodes[sym];
}

/*
 * Vrati uzol s code danym ako parameter.
 */
Node *treeGetNodeCode(Node *root, Code code) {
	Node *n = root;
	int32_t aux = code.c;
	for (int32_t i = 0; i < code.len; i++) {
		if ((code.c & (1<<i)) == 0) {
			n = n->left;
		} else {
			n = n->right;
		}
	}
	return n;
}

/*
 * Vrati symbol s danym kodom v strome
 */
int32_t treeGetSymbolCode(Node *root, Code code) {
	return treeGetNodeCode(root, code)->sym;
}

/*
 * Vrati odkaz na uzol obsahujuci symbol sym
 */
Node *treeGetNodeSymbol(Node *root, int32_t sym) {
	Code code = symCodes[sym];
	if (code.len == 0) {
		return root;
	}
	return treeGetNodeCode(root, code);
}

/*
 * Kontroluje ci sa zadany symbol nachadza v strome
 */
bool treeIsSymIn(int32_t symbol) {
	if (symCodes.find(symbol) == symCodes.end()) {
		return false;
	}
	return true;
}

/*
 * Prechadza strom od korena a vrati uzol s rovnakym alebo nizsim ohodnotenim
 * "napravo hore" od uzla currNode, alebo NULL
 */
Node *treeGetExchangeNode(Node *root, Node *currNode) {
	queue<Node*> q;
	q.push(root);
	while (!q.empty()) {
		Node *n = q.front();
		q.pop();
		if (n == currNode) {
			return NULL;
		}
		if (n->freq == currNode->freq && n != currNode->parent) {
			return n;
		}
		if (n->right != NULL) {
			q.push(n->right);
			q.push(n->left);
		}
	}
	return NULL;
}

/*
 * Vymeni zadane uzly v strome  
 */
void treeExchangeNodes(Node *n1, Node *n2) {
	// najprv sa vymenia odkazy z potomkov na rodicov, potom z rodicov na potomkov
	Node *p1 = n1->parent;
	Node *p2 = n2->parent;
	n1->parent = p2;
	n2->parent = p1;
	if (p1->left == n1) {
		p1->left = n2;
	} else {
		p1->right = n2;
	}
	if (p2->left == n2) {
		p2->left = n1;
	} else {
		p2->right = n1;
	}
}

/*
 * Aktualizuje kody jednotlivych znakov v symCodes
 */
void treeUpdateCodes(Node *root) {
	queue<Node*> q;
	q.push(root);
	while(!q.empty()) {
		Node *n = q.front();
		q.pop();
		// sme v liste
		if (n->sym != -1) {
			symCodes[n->sym] = n->code;
		} else { // sme vo vnutornom uzle
			(n->left->code).c = (n->code).c;
			(n->left->code).len = (n->code).len +1;
			(n->right->code).c = (n->code).c | (1 << (n->code).len);
			(n->right->code).len = (n->code).len +1;
			q.push(n->left);
			q.push(n->right);
		}
	}
}

/*
 * V pripade pretecenia unsigned int, uprav frekvencie uzlov na polovicu.
 * Vyuziva dva zasobniky na prechod, vracia najlavejsi uzol najvyssej urovne.
 */
Node *treeRescale(Node *root) {
	stack<Node*> nodeStack;
	nodeStack.push(root);
	Node *n = root;
	Node *top;
	do {
		if (nodeStack.top()->sym == -1) {
			if (nodeStack.top()->visited == 2) {
				nodeStack.top()->freq =
					nodeStack.top()->left->freq +nodeStack.top()->right->freq;
				nodeStack.top()->parent->visited++;
				nodeStack.top()->visited = 0;
				nodeStack.pop();
			} else {
				if (nodeStack.top()->left->code.len > n->code.len) {
					if (nodeStack.top()->left->sym != -2) {
						n = nodeStack.top()->left;
					} else {
						n = nodeStack.top()->right;
					}
				}
				top = nodeStack.top();
				nodeStack.push(top->right);
				nodeStack.push(top->left);
			}
		} else { // sme v liste
			nodeStack.top()->freq = nodeStack.top()->freq%2==0?
				nodeStack.top()->freq/2:(nodeStack.top()->freq/2)+1;
			nodeStack.top()->parent->visited++;
			nodeStack.pop();
		}
		if ((int64_t) nodeStack.top() == (int64_t) root) {
		}
	} while ((int64_t) nodeStack.top() != (int64_t) root);
	root->freq = root->left->freq+root->right->freq;
	root->visited = 0;
	return n;
}

/*
 * Aktualizuje strom s korenom v root a to od uzla node
 */
void treeUpdate(Node *root, Node *node) {
	Node *currNode = node;
	while (currNode != root) {
		Node *n = treeGetExchangeNode(root, currNode);
		if (n != NULL) {
			treeExchangeNodes(n, currNode);
		}
		currNode->freq += 1;
		currNode = currNode->parent;
	}
	currNode->freq += 1;
	if (currNode->freq == uint32Max) {
		Node *l = treeRescale(root);
		treeUpdate(root, l);
	}
	treeUpdateCodes(root);
}

/*
 * Uvolni pamat zabranu stromom s korenom v root
 */
void treeDestroy(Node *root) {
	queue<Node*> q;
	q.push(root);
	while (!q.empty()) {
		Node *n = q.front();
		q.pop();
		if (n->left != NULL) {
			q.push(n->left);
		}
		if (n->right != NULL) {
			q.push(n->right);
		}
		delete n;
	}
}


/*
 * Vlozi novy uzol ZERO a "sym" do stromu s korenom v root
 */
Node *treeInsertNewNode(Node *root, int32_t sym) {
	Node *zero = treeGetNodeSymbol(root, ZERO); 
	// vytvorime novy ZERO a "sym" uzol a pridame symbol do symcodes
	zero->left = new Node;
	zero->right = new Node;
	zero->sym = -1;
	initNode(zero->left, ZERO, 0, NULL, NULL, zero);
	initNode(zero->right, sym, 1, NULL, NULL, zero);
	return zero;
}

/* Nazev:
 *   AHEDEncoding
 * Cinnost:
 *   Funkce koduje vstupni soubor do vystupniho souboru a porizuje zaznam o kodovani.
 * Parametry:
 *   ahed - zaznam o kodovani
 *   inputFile - vstupni soubor (nekodovany)
 *   outputFile - vystupní soubor (kodovany)
 * Navratova hodnota: 
 *    0 - kodovani probehlo v poradku
 *    -1 - pøi kodovani nastala chyba
 */
int AHEDEncoding(tAHED *ahed, FILE *inputFile, FILE *outputFile) {
	// vytvorí pociatocny uzol
	Node *root = new Node;
	initNode(root, ZERO, 0, NULL, NULL, NULL);
	int32_t sym = fgetc(inputFile);
	Code c;
	while(sym != EOF) {
		uncodedSize++;
		if (!treeIsSymIn(sym)) {
			c = treeGetCodeSymbol(ZERO);
			if (c.len != 0) {
				bufferedWrite(outputFile, c);
			}
			c.c = sym; c.len=BYTE_SIZE;
			bufferedWrite(outputFile, c);
			Node *n = treeInsertNewNode(root, sym);
			treeUpdate(root, n);
		} else {
			bufferedWrite(outputFile, treeGetCodeSymbol(sym));
			treeUpdate(root, treeGetNodeSymbol(root, sym));
		}
		sym = fgetc(inputFile);
	}
	bufferedWrite(outputFile, treeGetCodeSymbol(ZERO));
	mywrite(outputFile);
	ahed->uncodedSize = uncodedSize;
	ahed->codedSize = codedSize;
	treeDestroy(root);
	return AHEDOK;
}

/* Nazev:
 *   AHEDDecoding
 * Cinnost:
 *   Funkce dekoduje vstupni soubor do vystupniho souboru a porizuje zaznam o dekodovani.
 * Parametry:
 *   ahed - zaznam o dekodovani
 *   inputFile - vstupni soubor (kodovany)
 *   outputFile - vystupní soubor (nekodovany)
 * Navratova hodnota: 
 *    0 - dekodovani probehlo v poradku
 *    -1 - pri dekodovani nastala chyba
 */
int AHEDDecoding(tAHED *ahed, FILE *inputFile, FILE *outputFile)
{
	// vytvorim novy strom s korenom v root
	Code code = readBufferedByte(inputFile);
	fprintf(outputFile, "%c", (char) code.c);
	uncodedSize++;
	Node *root = new Node;
	initNode(root, ZERO, 0, NULL, NULL, NULL);
	Node *n = treeInsertNewNode(root, code.c);
	treeUpdate(root, n);
	int32_t sym;
	while (1) {
		n = readBufferedNode(root, inputFile);
		if (n->sym == ZERO) {
			if (endOfFile) {
				break;
			}
			code = readBufferedByte(inputFile);
			fprintf(outputFile, "%c", (char) code.c);
			uncodedSize++;
			n = treeInsertNewNode(root, code.c);
			treeUpdate(root, n);
		} else {
			fprintf(outputFile, "%c", (char) n->sym);
			uncodedSize++;
			treeUpdate(root, n);
		}
	}
	treeDestroy(root);
	ahed->uncodedSize = uncodedSize;
	ahed->codedSize = codedSize;
	if (ferror(inputFile)) {
		return AHEDFail;
	}
	return AHEDOK;
}
