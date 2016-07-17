/*
 * =====================================================================================
 *
 *       Filename:  dns_stats.cpp
 *
 *    Description:  Proxy DNS generujici statistiky dotazu 
 *
 *        Version:  1.0
 *        Created:  28.09.2012 18:57:28
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  Peter Lacko, xlacko06@stud.fit.vutbr.cz
 *
 * =====================================================================================
 */
#include "dns_stats.hpp"

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handleArgs( int argc, char **argv)
 *  Description:  funkce spracuje parametry: informacie o statistikach ulozi do
 *  		  struktury typu DnsStatsInfo, informace o lokalhostu a dns servru do 
 *  		  LocalhostInfo
 * =====================================================================================
 */
int handleArgs( int argc, char **argv, 
	LocalhostInfo &localhost_info)
{
    for (int param = 1; param < argc; param++) {
	// statistiky podle typu
	if (strcmp(argv[param], "-type") == 0) {
	    if (dns_stats_info.type) {
		return -1;
	    }
	    else
		dns_stats_info.type = true;
	} 

	// statistiky podla zdrojov
	else if (strcmp(argv[param], "-source") == 0) {
	    if (dns_stats_info.source) {
		return -1;
	    }
	    else
		dns_stats_info.source = true;
	} 

	// statistiky podle dotazu
	else if (strcmp(argv[param], "-destination") == 0) {
	    if (dns_stats_info.destination) {
		return -1;
	    }
	    else
		dns_stats_info.destination = true;
	}

	// statistiky za posledni hodinu
	else if (strcmp(argv[param], "-hour") == 0) {
	    if (dns_stats_info.hour) {
		return -1;
	    }
	    else 
		dns_stats_info.hour = true;
	}

	// priemer statistik za hodinu
	else if (strcmp(argv[param], "-average") == 0) {
	    if (dns_stats_info.average) {
		return -1;
	    }
	    else
		dns_stats_info.average = true;
	}

//	// tcp spojeni
//	else if (strcmp(argv[param], "-t") == 0) {
//	    if (localhost_info.tcpflag) {
//		return -1;
//	    }
//	    else
//		localhost_info.tcpflag = true;
//	}

	// naslouchaci adresa
	else if (strcmp(argv[param], "-l") == 0) {
	    if (!localhost_info.listening_address.empty() || (param == argc-1)) {
		return -1;
	    }
	    else
		localhost_info.listening_address = argv[++param];
	}

	// naslouchaci port
	else if (strcmp(argv[param], "-p") == 0) {
	    if (!localhost_info.listening_port.empty() || (param == argc-1)) {
		return -1;
	    }
	    else
		localhost_info.listening_port = argv[++param];
	}

	// adresy dns servru, maximalne MAXNS (pravdepodobne 3)
	else if (strcmp(argv[param], "-s") == 0) {
	    if ((localhost_info.dns_server_address.size() > MAXNS) || (param == argc-1)) {
		return -1;
	    }
	    else
		localhost_info.dns_server_address.push_back(argv[++param]);
	}

	// defaultni vetev = chybne  parametre
	else {
	    return -1;
	}
    }
    if (localhost_info.dns_server_address.empty()) {
	return -1;
    }

    // adresy,domenove mena serverov -> adresy
    hostent *hptr;
    std::vector<std::string>::iterator addr_it;
    for (addr_it = localhost_info.dns_server_address.begin(); 
	    addr_it != localhost_info.dns_server_address.end();
	    addr_it++) {
	if ((hptr = gethostbyname((*addr_it).c_str())) == NULL) {
	    handleError(RECOVER, "nespravne zadane domenove meno");
	    return -1;
	}
	else {
	    if ((*addr_it) == std::string(hptr->h_name)) {
		(*addr_it) = hptr->h_name;
	    }
	    else {
		(*addr_it) = hptr->h_addr_list[0];
	    }
	}
    }
    return 0;
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handleError
 *  Description:  Obsluhuje chybove stavy programu: vypise informaciu o chybe na stderr
 *  		  a ukonci program se specifickym navratovym kodem
 * =====================================================================================
 */
inline void handleError( int err_value, const char *msg) 
{
    std::cerr << "error: " << msg << std::endl;
    if (err_value != RECOVER)
	exit( err_value);
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  portStr2Int
 *  Description:  provadi cislo portu z retezce na integer
 * =====================================================================================
 */
int portStr2Int(std::string &port_str)
{
    char *ch = new char;
    int port_int = strtol(port_str.c_str(), &ch, 0);
    if ((*ch == '\0') && (port_int > 0)) {
	return port_int;
    }
    return 0;
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  termHandler
 *  Description:  funkce slouzi na obsluhu signalu SIGTERM - korektne ukonci spojeni
 * =====================================================================================
 */
void termHandler(int n) {
    loop_stop = true;
    if (loop_waiting) {
	siglongjmp(jump, 1);
    }
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  sigusrHandler
 *  Description:  obsluzi prijem signalu SIGUSR1
 * =====================================================================================
 */
void sigusrHandler(int n) {
    printStats();
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  getRuntime
 *  Description:  cas behy programu v sekundach 
 *  		  rozmeni na hodiny, minuty a sekundy
 * =====================================================================================
 */
void getRuntime(tm &t_runtime) {
    int runtime = time(NULL) - start_time;
    t_runtime.tm_sec = runtime%60;
    t_runtime.tm_hour = runtime/3600;
    t_runtime.tm_min = (runtime/60)%60;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  phActualizeQuer
 *  Description:  aktualizace poctu dotazu za hodinu
 * =====================================================================================
 */
void phActualizeQuer()
{
    int i = 0;
    for (ph_stats_it = ph_stats.begin();  // odstran zaznamy starsi jak 1 hod
	    (ph_stats_it != ph_stats.end()) && ((time(NULL) - (*ph_stats_it).timestamp) > SECS_PER_HOUR);
	    ph_stats_it++) {

	if (dns_stats_info.destination) { 				
	    ((*((*ph_stats_it).ph_dest_it)).second--);	  // uprav pocet stejnych dotazu
	    if ((*((*ph_stats_it).ph_dest_it)).second == 0)   // ak dane domainname nebolo vyhladane
		ph_dest.erase((*ph_stats_it).ph_dest_it);     // za posled. hodinu ani raz-odstranit
	}

	if (dns_stats_info.source) {	
	    ((*(*ph_stats_it).ph_src_it).second--);		// uprav pocet dotazu od stejne osoby		
	    if ((*(*ph_stats_it).ph_src_it).second == 0)	// ak sa klient za poslednu hodinu neozval
		ph_src.erase((*ph_stats_it).ph_src_it);	// ani raz, zmazeme ho
	}

	if (dns_stats_info.type) {
	    ph_type_values[(*ph_stats_it).type]--;		// znizenie poctu jednotlivych
	}							// typov dotazov

	i++;

	ph_q_rcvd--;
    }
    for (int j = 0; j < i; j++)
	ph_stats.erase(ph_stats.begin());			// odstraneni zaznamu zo zoznamu
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  phActualizeResp
 *  Description:  aktualizace poctu odpovedi za hodinu
 * =====================================================================================
 */
void phActualizeResp()
{
    int i = 0;
    for (resp_timestamp_it = resp_timestamp.begin();
	    (resp_timestamp_it != resp_timestamp.end()) 
	    && ((unsigned)(time(NULL) - (*resp_timestamp_it)) > (unsigned)SECS_PER_HOUR);
	    resp_timestamp_it++) {
	i++;
    }
    for (int j = 0; j < i; j++) 
	resp_timestamp.erase(resp_timestamp.begin());
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  phActualize
 *  Description:  aktualizuje statistiky za posledni hodinu
 * =====================================================================================
 */
void phActualize(void)
{
    // aktualizace dotazu
    phActualizeQuer();

    // aktualizace odpovedi
    phActualizeResp();
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  printLastHour
 *  Description:  zobrazi statistiky ziskane za posledni hodinu
 * =====================================================================================
 */
void printLastHour(void)
{
    phActualize();
    std::cout << "\n>>>>>>>>>>>> Per last hour statistics >>>>>>>>>>>>\n";
    if (dns_stats_info.source) {
	std::cout << "\nclient : number of queries\n";
	std::cout << "----------------------------------\n";
	for (ph_src_it = ph_src.begin(); ph_src_it != ph_src.end(); ph_src_it++)
	    std::cout << (*ph_src_it).first << " : " << (*ph_src_it).second << std::endl;
    }

    if (dns_stats_info.destination) {
	std::cout << "\ndomainname : number of queries\n";
	std::cout << "----------------------------------\n";
	for (ph_dest_it = ph_dest.begin(); ph_dest_it != ph_dest.end(); ph_dest_it++)
	    std::cout << (*ph_dest_it).first << " : " << (*ph_dest_it).second << std::endl;
    }

    if (dns_stats_info.type) {
	std::cout << "\ntype : number of queries\n";
	std::cout << "----------------------------------\n";
	for(int i = 0; i < TYPE_NAMES_SIZE; i++) {
	    if (ph_type_values[i] != 0)
		std::cout << type_names[i] << " : " << ph_type_values[i] << std::endl;
	}
    }

    std::cout << "\nQueries per last hour: " << ph_q_rcvd << std::endl;
    std::cout << "Responses per past hour: " << resp_timestamp.size() << std::endl;
    // std::cout << "\n<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n";
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  printHourAverage
 *  Description:  vytiskne priemer statistik ziskanych za hodinu
 * =====================================================================================
 */
void printHourAverage(void)
{
    std::cout << "\n>>>>>>>>>>>>>>>> Average per hour >>>>>>>>>>>>>>>>\n";
    float runtime = float(time(NULL)-start_time) / float(SECS_PER_HOUR);
    if (dns_stats_info.source) {
	std::cout << "\nclient : number of queries\n";
	for (st_src_it = st_src.begin(); st_src_it != st_src.end(); st_src_it++)
	    std::cout << (*st_src_it).first << " : " 
		<< float((*st_src_it).second) / runtime << std::endl;
    }

    if (dns_stats_info.destination) {
	std::cout << "\ndomainname : number of queries\n";
	for (st_dest_it = st_dest.begin(); st_dest_it != st_dest.end(); st_dest_it++)
	    std::cout << (*st_dest_it).first << " : " 
		<< float((*st_dest_it).second) / runtime << std::endl;
    }

    if (dns_stats_info.type) {
	std::cout << "\ntype : number of queries\n";
	for(int i = 0; i < TYPE_NAMES_SIZE; i++) {
	    if (type_values[i] != 0)
		std::cout << type_names[i] << " : " 
		    << float(type_values[i]) / runtime << std::endl;
	}
    }
    std::cout << "\nAverage queries per hour: " << float(q_rcvd) / runtime << std::endl;
    std::cout << "Average responses per hour: " << float(r_rcvd) / runtime << std::endl;
    //std::cout << "\n<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n";
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  printOverallStats
 *  Description:  vypis statistik ziskanych za celu dobu behu programu
 * =====================================================================================
 */
void printOverallStats(void)
{
    std::cout << "\n>>>>>>>>>>>>>>> Overall statistics >>>>>>>>>>>>>>>\n";
    if (dns_stats_info.source) {
	std::cout << "\nclient : number of queries\n";
	std::cout << "----------------------------------\n";
	for (st_src_it = st_src.begin(); st_src_it != st_src.end(); st_src_it++)
	    std::cout << (*st_src_it).first << " : " << (*st_src_it).second << std::endl;
    }

    if (dns_stats_info.destination) {
	std::cout << "\ndomainname : number of queries\n";
	std::cout << "----------------------------------\n";
	for (st_dest_it = st_dest.begin(); st_dest_it != st_dest.end(); st_dest_it++)
	    std::cout << (*st_dest_it).first << " : " << (*st_dest_it).second << std::endl;
    }

    if (dns_stats_info.type) {
	std::cout << "\ntype : number of queries\n";
	std::cout << "----------------------------------\n";
	for(int i = 0; i < TYPE_NAMES_SIZE; i++) {
	    if (type_values[i] != 0)
		std::cout << type_names[i] << " : " << type_values[i] << std::endl;
	}
    }

    std::cout << "\nTotal queries: " << q_rcvd << std::endl;
    std::cout << "Total responses: " << r_rcvd << std::endl;
    std::cout << "--------------------------------------------------\n";
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  printStats
 *  Description:  vypise prislusne statistiky na stdout
 * =====================================================================================
 */
void printStats(void)
{
    std::cout << "\n";

    // vypis doby behu programu
    tm t_runtime;
    getRuntime(t_runtime);
    std::cout.fill('0');
    std::cout << "Runtime: "; 
    std::cout << std::setw(2) << t_runtime.tm_hour << ":" 
	<< std::setw(2) << t_runtime.tm_min << ":" 
	<< std::setw(2) << t_runtime.tm_sec << std::endl;

    // vypis statistik za poslednu hodinu
    if (dns_stats_info.hour) {
	printLastHour();
    }

    // vypis priemernych hodnot za hodinu
    if (dns_stats_info.average) {
	printHourAverage();
    }

    // vypis statistik za celu dobu behu programu
    printOverallStats();

    std::cout << "\n\n";
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  parseQuery
 *  Description:  napasuje DNS query od klienta a aktualizuje statistiky
 * =====================================================================================
 */
int parseQuery(const u_char *query, int query_len, in_addr &addr) 
{
    ns_msg handle;
    ns_rr rr;
    char addr_str[16];
    entry newEntry;
    memset(&newEntry, 0, sizeof(newEntry));

    if (-1 == ns_initparse((u_char*) query, query_len, &handle))
	return -1;

    if (-1 == ns_parserr(&handle, ns_s_qd, 0, &rr))
	return -1;

    // statistika podla dotazovaneho domainname
    if (dns_stats_info.destination) {
	st_dest[rr.name]++;
    }

    // podla zdroja (kto sa pytal)
    if (dns_stats_info.source) {
	inet_ntop(AF_INET, &addr, addr_str, 16);
	st_src[addr_str]++;
    }

    // podla typu dotazu
    if (dns_stats_info.type) {
	type_values[rr.type]++;
    }

    // statistiky za poslednu hodinu
    //
    // je vhodne vytvorit novy zaznam predtym, ako sa zmazu stare, aby sa zbytocne nemazali
    // a nasledne nevytvarali polozky v kontajneroch ph_dest a ph_src
    if (dns_stats_info.hour) 
    {
	newEntry.timestamp = time(NULL); 	// pridanie casovej znacky do noveho zaznamu
	ph_q_rcvd++;	// inkrementace poctu prijatych dotazu
	if (dns_stats_info.destination) {	// pridame/inkrementujeme domainname do zaznam
	    ph_dest[rr.name]++;
	    newEntry.ph_dest_it = ph_dest.find(rr.name);
	}

	if (dns_stats_info.source) {		// pridame/inkrementujeme informaci o dotazujucom sa klientovi
	    ph_src[addr_str]++;
	    newEntry.ph_src_it = ph_src.find(addr_str);
	}

	if (dns_stats_info.type) {		// pridame informaciu o type zaznamu
	    ph_type_values[rr.type]++;
	    newEntry.type = rr.type;
	}

	ph_stats.push_back(newEntry);

	// aktualizuj statistiky za posledni hodinu
	phActualizeQuer();

    }

    return 0;
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  udpConnection
 *  Description:  obsluhuje nespojovou komunikaci client-proxy-DNS server
 * =====================================================================================
 */
void udpConnection(int &sockfd,	// naslouchani
	sockaddr_in &dns_server_addr, int &sockfd2, 	// vzdaleny server
	std::vector<std::string> &dns_servers)			// zoznam DNS adries 
{
    // nastaveni vychoziho DNS servra
    inet_pton(AF_INET, dns_servers.front().c_str(), &dns_server_addr.sin_addr);

    ssize_t dns_packet_len; // delka prijateho dotazu
    char dns_packet_str[MAX_DNS_LEN]; // prijaty dotaz
    sockaddr_in client_addr;
    socklen_t client_addr_size = sizeof(client_addr);

    int bytes_sent; // pocet bytov odoslanych na dns server
    int selvalue; 	// navratova hodnota selectu
    socklen_t dns_server_addr_size = sizeof(dns_server_addr); // velikost struktury

    // select
    fd_set client_query_fds;
    fd_set server_response_fds;

    FD_ZERO(&client_query_fds);
    FD_SET(sockfd, &client_query_fds);

    // timeout na odpoved od servra
    timeval timeout;

    loop_stop = false; 
    unsigned dns_changed;
    sigsetjmp(jump, 1); // ak pri prijati signalu ukonceni program "stoji" na funkci select
    			// ve funkci obsluhy signalu sa loop_stop nastavi na true
			// a skonci sa pred tento cyklus = funkce se ukonci
    while (!loop_stop) {
	bzero(dns_packet_str, MAX_DNS_LEN);
	bzero(&client_addr, client_addr_size); // vynulovanie buffru a struktury

	FD_ZERO(&server_response_fds);
	FD_SET(sockfd2, &server_response_fds);

	timeout.tv_sec = 5;
	timeout.tv_usec = 0;

	dns_changed = 0;
	loop_waiting = true;
	selvalue = select(sockfd+1, &client_query_fds, NULL, NULL, NULL);
	if (FD_ISSET(sockfd, &client_query_fds) && selvalue > 0) {
	    FD_ZERO(&client_query_fds);
	    FD_SET(sockfd, &client_query_fds);

	    dns_packet_len = recvfrom(sockfd, dns_packet_str, MAX_DNS_LEN, 0, // dns query recieved
		    reinterpret_cast<sockaddr*>(&client_addr), &client_addr_size);
	    loop_waiting = false;
	    q_rcvd++;

	    if ( -1 == parseQuery((u_char*) dns_packet_str, dns_packet_len, client_addr.sin_addr))
		handleError(RECOVER, "cannot parse query");

	    /*-----------------------------------------------------------------------------
	     *  obsluha jedneho klienta
	     *-----------------------------------------------------------------------------*/
send_again:
	    // poslani dotazu na DNS server
	    bytes_sent = sendto(sockfd2, dns_packet_str, dns_packet_len, 0,
		    reinterpret_cast<sockaddr*>(&dns_server_addr), sizeof(dns_server_addr));
	    if (bytes_sent != dns_packet_len)
		handleError(RECOVER, "sendto(): cannot send query");

	    // prijeti odpovedi z DNS servru
	    bzero(dns_packet_str, MAX_DNS_LEN);


	    do {  // bol zachytany signal
		FD_ZERO(&server_response_fds);
		FD_SET(sockfd2, &server_response_fds);
		timeout.tv_sec = 3;
		timeout.tv_usec = 0;
		selvalue = select(sockfd2+1, &server_response_fds, NULL, NULL, &timeout);
	    } while (selvalue == -1 && errno==EINTR);

	    // prijem odpovede ze servru
	    if (FD_ISSET(sockfd2, &server_response_fds) && selvalue > 0) {
		dns_packet_len = recvfrom(sockfd2, dns_packet_str, MAX_DNS_LEN, 0, 
			reinterpret_cast<sockaddr*>(&dns_server_addr), &dns_server_addr_size);
		r_rcvd++;

		// aktualizace prijatych odpovedi za hodinu
		if (dns_stats_info.hour) {
		    resp_timestamp.push_back(time(NULL));
		    phActualizeResp();
		}

		bytes_sent = sendto(sockfd, dns_packet_str, dns_packet_len, 0,
			reinterpret_cast<sockaddr*>(&client_addr), sizeof(client_addr));
		if (bytes_sent != dns_packet_len)
		    handleError(RECOVER, "sendto(): cannot send query");
	    }
	    else { // vyprsal timeout -> zmena DNS
		if (dns_servers.size() > 1) {
		    dns_servers.push_back(dns_servers.front());
		    dns_servers.erase(dns_servers.begin());
		    inet_pton(AF_INET, dns_servers.front().c_str(), &dns_server_addr.sin_addr);
		    std::cerr << "Timeout vyprsel, novy DNS server: " << dns_servers.front() << std::endl;
		    if (++dns_changed < dns_servers.size())
			goto send_again;
		}
	    }
	}
	else {
	    FD_ZERO(&client_query_fds);
	    FD_SET(sockfd, &client_query_fds);
	}
    }
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description:  hlavni funkce programu
 * =====================================================================================
 */
int main(int argc, char **argv) 
{
    LocalhostInfo localhost_info;
    localhost_info.tcpflag = false;
    if (argc == 1) {
	std::cout << help_message;	
	exit(EXIT_SUCCESS);
    }
    else if ((handleArgs( argc, argv, localhost_info)) != 0) {
	handleError( WRONG_PARAMS, "Nespravne zadane parametre, spustite \"dns_stat\" pre napovedu.");
    }

    // port number from string to int 
    short port;
    if (localhost_info.listening_port.empty()) {
	port = 53;
    } 
    else {
	port = portStr2Int(localhost_info.listening_port);
	if (!port)
	    handleError( WRONG_PARAMS, "Nespravne zadane parametre, spustite \"dns_stat\" pre napovedu.");
    }

    // "naslouchaci" socket
    sockaddr_in proxy_addr; // PARAMETER
    bzero(&proxy_addr, sizeof(proxy_addr));
    proxy_addr.sin_family = AF_INET;
    proxy_addr.sin_port = htons(port);
    if (localhost_info.listening_address.empty())
	proxy_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    else {
	if (inet_pton(AF_INET, localhost_info.listening_address.c_str(), &proxy_addr.sin_addr) != 1) {
	// if ((proxy_addr.sin_addr.s_addr = inet_addr(localhost_info.listening_address.c_str()))==INADDR_NONE) {
	    handleError( WRONG_PARAMS, "Nespravne zadane parametre, spustite \"dns_stat\" pre napovedu.");
	}
    }

    signal (SIGUSR1, sigusrHandler);
    // Signal (SIGCHLD, sigchldHandler);
    signal (SIGTERM, termHandler); 
    signal (SIGINT, termHandler);


    int sockfd = socket(AF_INET, SOCK_DGRAM, 0); // PARAMETER
    if (bind( sockfd, reinterpret_cast<sockaddr*>(&proxy_addr), sizeof(proxy_addr)) != 0) {
	close(sockfd);
	handleError(SYSTEM_ERROR, "bind() failed");
    }

    int sockfd2 = socket(AF_INET, 
	    (localhost_info.tcpflag ? SOCK_STREAM : SOCK_DGRAM), 0); // socket pro komunikaci s dns servrom

    sockaddr_in dns_server_addr; // struktura pro komunikaci s dns servrom
    bzero(&dns_server_addr, sizeof(dns_server_addr));
    dns_server_addr.sin_family = AF_INET;
    dns_server_addr.sin_port = htons(DNS_PORT);


    // inicializace startu programu
    start_time = time(NULL);

    // alokacia a vynulovanie pamate pre dlhodobe, i "hodinove" zaznamy o type dotazu
    if (dns_stats_info.type) {
	type_values = new short[TYPE_NAMES_SIZE];
	memset(type_values, 0, TYPE_NAMES_SIZE * sizeof(short));
	if (dns_stats_info.hour) {
	    ph_type_values = new short[TYPE_NAMES_SIZE];
	    memset(ph_type_values, 0, TYPE_NAMES_SIZE * sizeof(short));
	}
    }

    // zahajeni provozu servru
    udpConnection(sockfd, dns_server_addr, sockfd2, localhost_info.dns_server_address);

    printStats();
    close(sockfd);
    close(sockfd2);
    delete[] type_values;
    delete[] ph_type_values;
}
