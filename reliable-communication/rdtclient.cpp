/*
 * =====================================================================================
 *
 *       Filename:  rdtclient.c
 *
 *    Description:  program rdtclient do projektu #3 kurzu IPK
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

#include "rdtclient.h"

/*-----------------------------------------------------------------------------
 *  funkce na odchyceni ukoncovacich signalu
 *  nastavi globalni promennou "Done" na true a posilani dat se ukonci
 *-----------------------------------------------------------------------------*/
void termCatcher(int n) {
    Done = true;
}


/*-----------------------------------------------------------------------------
 *  odchyceni SIGALRM
 *  znova posle cely obsah okna a spusti casovac s vacsim timeoutom
 *-----------------------------------------------------------------------------*/
void alarmCatcher(int n) {
    resendCwnd();
    ++timeouts;
    if (timeouts == 5) {
	timeouts = 0;
	mytimer.it_value.tv_usec += 500000;
    }
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
 * vrati poradie potvrdeneho paketu
 * @param odpoved v xml
 * @param dlzka odpovede
 */
long getOrder( char *ack, int ack_len) {
    xmlChar *acked_order_str;
    xmlDocPtr tree;
    xmlNodePtr root;
    tree = xmlReadMemory(ack, ack_len, NULL, NULL, 0);
    if (tree == NULL)
	std::cerr << "Error while parsing xml (wrong packet?)\n";

    root = xmlDocGetRootElement(tree);
    if (root == NULL)
	std::cerr << "Error obtaining root (missing root element?)\n";

    root = root->xmlChildrenNode;
    while (root != NULL) {
	if ((!xmlStrcmp(root->name, (const xmlChar *)"header"))) {
	    parseStory (tree, root, "order", &acked_order_str);
	}
	root = root->next;
    }
    // ziskanie cisla zo stringu

    long ret_value = atol((char*) acked_order_str);
    xmlFreeDoc(tree);
    xmlFree(acked_order_str);

    return ret_value;    
}

/*
 * funkce vytvori packet vo formate xml
 * @param reference na vysledny paket
 * @param data odoslane paketom
 * @param dlzka dat v pakete
 * @param poradie vytvaraneho paketu
 */
void createPacket (std::string & packet, 
	const char *b64_data, 
	int b64_bytes_read, 
	int order) {
    char b64_bytes_read_str[4];
    char order_str[11];
    sprintf(order_str, "%d", order);
    sprintf(b64_bytes_read_str, "%d", b64_bytes_read);
    packet = "<?xml version=\"1.0\"?>\n<rdt-segment id=\"xlacko06\">\n<header>\n<order>";
    packet.append(order_str, strlen(order_str));
    packet  += "</order><len>";
    packet.append(b64_bytes_read_str, strlen(b64_bytes_read_str));
    packet += "</len>\n</header>\n<data>";
    packet.append(b64_data,b64_bytes_read);
    packet += "</data>\n</rdt-segment>\n";
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
 *  znova odoslanie obsahu celeho okna
 *-----------------------------------------------------------------------------*/
void resendCwnd() {
    std::vector<std::string>::iterator iter;
    for (iter = buffer.begin(); iter != buffer.end(); iter++) {
	// odoslani dat
	udt_send (sockNumber, remote_addr, d_port, (*iter).c_str(), (*iter).length());
    }
    setitimer(ITIMER_REAL, &mytimer, NULL);
}

/*-----------------------------------------------------------------------------
 *  hlavna funkcia
 *-----------------------------------------------------------------------------*/
int main(int argc, char **argv) {
    int s_port;
    if ( HandleArgs(argc, argv, &s_port, &d_port) != 0 ) {
	std::cout << "HELP!!\n";
	return WRONG_PARAMS;
    }

    uint8_t data[DATA_SIZE]; 		// nactena data
    char b64_data[B64_DATA_SIZE]; // data kodovane do b64
    int bytes_read = -1;	// pocet nactenych bytov ze suboru
    int b64_bytes_read = -1;	// pocet znakov vratenych 
    std::string packet;

    // promenne sluziace pro prijaty paket
    char ack[MAX_ACK_LEN];
    long acked_order;	// poradie potvrdeneho paketu
    int ack_len;	// delka prijateho potvrdenia

    // sliding window
    cwndT cwnd;
    cwnd.begin = 1;
    cwnd.end = 5;

    // pripojenie k serveru
    remote_addr = ntohl(inet_addr("127.0.0.1"));
    sockNumber = socket( AF_INET, SOCK_DGRAM, 0);		// socket
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

    uint32_t order = 1;	// urcuje poradi packetu vramci prenosu
    bool last = false;	// priznak posledneho paketu

    // odchyceni signalu sigterm, sigint
    signal(SIGINT, termCatcher);
    signal(SIGTERM, termCatcher);
    signal(SIGALRM, alarmCatcher);
    while (!Done) {
	if (order <= cwnd.end ) {
	    while (!last && order <= cwnd.end) {
		if ((bytes_read = fread (data, 1, DATA_SIZE, stdin))>0) {
		    // prekodovani dat do base64
		    b64_bytes_read = b64_ntop (data, bytes_read, b64_data, B64_DATA_SIZE);
		    // vytvoreni paketu
		    createPacket (packet, b64_data, b64_bytes_read, order);
		    // pridanie packetu do buffru
		    buffer.push_back(packet);
		    // odoslani dat
		    udt_send (sockNumber, remote_addr, d_port, packet.c_str(), packet.length());

		    /*-----------------------------------------------------------------------------
		     *  if timer not runnig
		     *   run timer
		     *-----------------------------------------------------------------------------*/
		    getitimer(ITIMER_REAL, &test_timer);
		    if (test_timer.it_value.tv_sec == 0 && test_timer.it_value.tv_usec == 0)
			setitimer(ITIMER_REAL, &mytimer, NULL);
		    // kontrola konca vstupu
		    if (feof(stdin))
			last = true;
		    else {
			++order;
		    }
		}
	    }
	}
	select (sockNumber+1, &readfds, NULL, NULL, NULL);
	// na socket bol doruceny packet
	if (!Done && FD_ISSET(sockNumber, &readfds))  {
	    // ziskanie odpovede
	    ack_len = udt_recv (sockNumber, ack, MAX_ACK_LEN, NULL, NULL);
	    // rozparsovanie odpovede - iba cislo paketu
	    acked_order = getOrder (ack, ack_len);
	    if (acked_order >= cwnd.begin) {

		while (cwnd.begin <= acked_order) {
		    buffer.erase(buffer.begin());
		    ++cwnd.begin;
		    ++cwnd.end;
		}
		/*-----------------------------------------------------------------------------
		 *  if not (last && acked_order == order) && buffer not empty
		 *  run timer
		 *-----------------------------------------------------------------------------*/
		if ( !last && !buffer.empty()) {
		    setitimer(ITIMER_REAL, &mytimer, NULL);
		}
	    }
	    if (last && (acked_order == order)) {
		Done = true;
	    }
	}
	FD_ZERO(&readfds);
	FD_SET(sockNumber, &readfds);
    } // elihw
    // odoslanie ukoncujuceho packetu

    timeval myTimer;
    int final_len = 0;
    myTimer.tv_usec = -500000;
    createPacket (packet, "FIN", 0, order);
    do {
	udt_send (sockNumber, remote_addr, d_port, packet.c_str(), packet.length());
	myTimer.tv_sec = 1;
	// po kazdom neuspechu sa timeout zvacsi o 0.5 sec, max vsak na 3 sec
	myTimer.tv_usec += 500000;
	if (myTimer.tv_usec >= 2000000) {
	    std::cerr << "Data transfer failed.\n";
	    exit(TRANSFER_FAILED);
	}
	select (sockNumber+1, &readfds, NULL, NULL, &myTimer);
	final_len = udt_recv (sockNumber, ack, MAX_ACK_LEN, NULL, NULL);
    } while (!final_len);

} // end main
