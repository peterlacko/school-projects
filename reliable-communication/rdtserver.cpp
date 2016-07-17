/*
 * =====================================================================================
 *
 *       Filename:  rdtserver.c
 *
 *    Description:  program rdtserver do projektu #3 kurzu IPK
 *
 *        Version:  1.0
 *        Created:  30.03.2012 23:21:44
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Peter Lacko (xlacko06), xlacko06@stud.fit.vutbr.cz
 *
 * =====================================================================================
 */

#include "rdtserver.h"

void termCatcher(int n) {
    Stop = true;
}

/*-----------------------------------------------------------------------------
 *  spracovanie argumentov
 *-----------------------------------------------------------------------------*/
int HandleArgs (int argc, char **argv, int *s_port, int *d_port) {
    int ch;
    while ((ch = getopt(argc, argv, "s:d:")) != -1) {
	switch (ch) {
	    case 's' : *s_port = atol(optarg);
		       break;
	    case 'd' : *d_port = atol(optarg);
		       break;
	    default:
		       return 1;
	}
    }
    return 0;
}


/*-----------------------------------------------------------------------------
 *  pomocna funkce: tisk elementov xml suboru
 *-----------------------------------------------------------------------------*/
    static void
print_element_names(xmlNodePtr a_node)
{
    xmlNode *cur_node = NULL;

    for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
	if (cur_node->type == XML_ELEMENT_NODE) {
	    printf("node type: Element, name: %s\n", cur_node->name);
	}
	print_element_names(cur_node->children);
    }
}


/*-----------------------------------------------------------------------------
 *  @param dokumentova struktura
 *  @param pointer na korenovy element
 *  @param hladany element
 *  @param miesto na ulozenie retazca
 *-----------------------------------------------------------------------------*/
void
parseStory (xmlDocPtr doc, xmlNodePtr cur, const char *wanted, xmlChar **key) {
    cur = cur->xmlChildrenNode;
    while (cur != NULL) {
	if ((!xmlStrcmp(cur->name, (const xmlChar *)wanted))) {
	    *key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
	}
	cur = cur->next;
    }
    return;
}

/*
 * vunkce vytvori odpoved ktora bude poslana klientovi
 * @param poradie paketu
 */
void create_ack (std::string & ack, uint32_t p_number) {
    char order_str[20];
    sprintf(order_str, "%lu", (unsigned long) p_number);
    ack = "<?xml version=\"1.0\"?>\n<rdt-segment id=\"xlacko06\">\n<header>\n<order>";
    ack.append(order_str, strlen(order_str));
    ack += "</order>\n</header>\n</rdt-segment>\n";
}

/*
 * naparsovani ziskane hlavicky
 * @param ziskany packet
 */
int parse_packet (const char *packet, 
	int packet_len, 
	char **order_str, 
	char **len_str, 
	char **b64_data) {
    xmlDocPtr tree;	// xml tree
    xmlNodePtr root;
    xmlNodePtr root_orig;

    // naparsovani ziskaneho paketu
    tree = xmlReadMemory(packet, packet_len, NULL, NULL, 0);
    if (tree == NULL)
	std::cerr << "Error while parsing xml (wrong packet?)\n";

    root = xmlDocGetRootElement(tree);
    if (root == NULL)
	std::cerr << "Error obtaining root (missing root element?)\n";

    // ziskanie dat z hlavicky
    parseStory(tree, root, "data", (xmlChar**) b64_data);

    // ziskanie poradia paketu z hlavicky
    root = root->xmlChildrenNode;
    root_orig = root;
    while (root != NULL) {
	if ((!xmlStrcmp(root->name, (const xmlChar *)"header"))){
	    parseStory (tree, root, "order", (xmlChar**) order_str);
	}
	root = root->next;
    }

    // ziskanie dlzky prenasanych dat z hlavicky
    // obnovenie povodneho rootu
    root = root_orig;
    while (root != NULL) {
	if ((!xmlStrcmp(root->name, (const xmlChar *)"header"))){
	    parseStory (tree, root, "len", (xmlChar**) len_str);
	}
	root = root->next;
    }
    xmlFreeDoc(tree);
    return 0;
}

static inline int udt_recv(int udt, 
	void *buff, 
	size_t nbytes, 
	in_addr_t *addr, 
	in_port_t *port)
{
    struct sockaddr_in sa;
    bzero(&sa, sizeof(sa));
    socklen_t salen = sizeof(sa);
    ssize_t nrecv = recvfrom(udt, buff, nbytes, MSG_DONTWAIT, (struct sockaddr *) &sa, &salen);
    if(addr != NULL) (*addr) = ntohl(sa.sin_addr.s_addr);
    if(port!=NULL) (*port) = ntohs(sa.sin_port);
    if (nrecv < 0 ) nrecv = 0;
    return nrecv;
}

static inline int udt_send(int udt, 
	in_addr_t addr, 
	in_port_t port, 
	const void *buff, 
	size_t nbytes) {
    struct sockaddr_in sa;
    bzero(&sa, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = htonl(addr);
    sa.sin_port = htons(port);
    ssize_t nsend = sendto(udt, buff, nbytes, 0, (const struct sockaddr *) &sa, sizeof(sa));
    return nsend == nbytes;
}


/*-----------------------------------------------------------------------------
 *  hlavna funkce
 *-----------------------------------------------------------------------------*/
int main(int argc, char **argv) {
    int s_port;
    int d_port;
    if ( HandleArgs(argc, argv, &s_port, &d_port) != 0 ) {
	std::cout << "HELP!!\n";
	return WRONG_PARAMS;
    }
    char packet[MAX_PACKET_LEN];
    int packet_len;
    xmlChar *b64_data;    	// data v base64 ziskane z packetu
    xmlChar *order_str;		// poradie paketu - retezec
    xmlChar *len_str;		// dlzka dat ziskanych z paketu
    uint8_t data[DATA_SIZE+1]; 	// data po prekodovani

    uint32_t real_b64data_len;	// skutocna delka prenesenych dat
    uint32_t exp_b64data_len;	// ocakavana delka prenesenych dat
    int data_len;	// dlzka dekodovanych dat
    uint32_t order;		// poradie paketu
    uint32_t expected = 1;	// ocakavany paket
    int len; 		// dlzka dat ziskanych z paketu

    // promenne pro odeslani odpovedi
    std::string ack;
    in_addr_t remote_addr = ntohl(inet_addr("127.0.0.1"));

    // pripojenie k serveru
    int sockNumber = socket( AF_INET, SOCK_DGRAM, 0);		// socket
    fcntl(sockNumber, F_SETFL, O_NONBLOCK);
    if (sockNumber  < 0) {
	std::cerr << "error: cannot create socket\n";
	exit (INTERNAL_ERROR);
    }

    // zjisteni adresy hosta (stale je to localhost) 
    sockaddr_in servSock;	// informace o vzdialenom servri
    bzero(&servSock, sizeof(servSock));
    servSock.sin_family = AF_INET;
    servSock.sin_addr.s_addr = htonl(0);
    servSock.sin_port = htons(s_port);
    if (bind (sockNumber, reinterpret_cast<sockaddr*>(&servSock), sizeof(servSock)) == -1) {
	std::cerr << "error: cannot bind to specified port\n";
	exit(INTERNAL_ERROR);
    }

    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(sockNumber, &readfds);

    signal(SIGINT, termCatcher);
    signal(SIGTERM, termCatcher);

    while (!Stop) {
	select(sockNumber+1, &readfds, NULL, NULL, NULL);
	if (!Stop && FD_ISSET(sockNumber, &readfds)) {
	    packet_len = udt_recv (sockNumber, packet, MAX_PACKET_LEN, NULL, NULL);
	    //std::cerr << "Packet dorazil\n";

	    // naparsovani ziskaneho paketu
	    parse_packet(packet, packet_len, (char**) &order_str, (char**) &len_str, (char**) &b64_data);
	    order = atol((char*)order_str);
	    exp_b64data_len = atol((char*)len_str);

	    // ak ocakavana delka dat nie je nulova, zistime ich delku
	    if (exp_b64data_len) real_b64data_len = strlen((char*) b64_data);

	    // nulova delka dat indikuje ukonceni spojeni
	    if (exp_b64data_len == 0) {
		create_ack (ack, order);
		udt_send (sockNumber, remote_addr, d_port, ack.c_str(), ack.length());

		// znovunastaveni hodnot
		expected = 1;
		xmlFree(b64_data);
		xmlFree(order_str);
		xmlFree(len_str);
		FD_ZERO(&readfds);
		FD_SET(sockNumber, &readfds);
		continue;
	    }
	    // odoslani odpovedi
	    if (order != expected) {
		create_ack (ack, expected-1);
		udt_send (sockNumber, remote_addr, d_port, ack.c_str(), ack.length());
		std::cerr << "ACK: " << ack << std::endl;
	    }
	    else {
		// vytvorenie a odoslanie odpovede
		create_ack (ack, expected);
		udt_send (sockNumber, remote_addr, d_port, ack.c_str(), ack.length());
		// dekodovani puvodnich znaku
		data_len = b64_pton ((char*) b64_data, data, DATA_SIZE+1);
		fwrite (data, data_len, 1, stdout);
		++expected;
	    }

	    // uvolnenie zabranych zdrojov
	    xmlFree(b64_data);
	    b64_data = NULL;
	    xmlFree(order_str);
	    xmlFree(len_str);
	    std::cout.flush();
	} // end if 

	// znovunastaveni descriptoru
	FD_ZERO(&readfds);
	FD_SET(sockNumber, &readfds);
    } // end while
}
