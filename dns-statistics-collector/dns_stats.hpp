/*
 * =====================================================================================
 *
 *       Filename:  dns_stats.hpp
 *
 *    Description:  Hlavickovy soubor k dns_stats.cpp
 *
 *        Version:  1.0
 *        Created:  30.09.2012 22:45:32
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  Peter Lacko, xlacko06@stud.fit.vutbr.cz
 *   Organization:  
 *
 * =====================================================================================
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <iomanip>

#include <strings.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <setjmp.h>
#include <resolv.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <errno.h>

/*-----------------------------------------------------------------------------
 *  definice struktur
 *-----------------------------------------------------------------------------*/
// ktore statistiky sa budu zbierat
struct DnsStatsInfo {
    bool type; 		// zbiera statistiky podle typu
    bool source;	// statistiky podle zdroju
    bool destination;	// statistiky podle dotazu
    bool hour;		// statistiky za posledni hodinu
    bool average;	// priemerne hodnoty za poslednu hodinu
};

// informacie o localhostu a adresa DNS servra
struct LocalhostInfo {
    std::string listening_address;
    std::string listening_port;
    std::vector <std::string> dns_server_address;
    bool tcpflag;
};

// polozka zoznamu statistiky za poslednu hodinu
struct entry {
    unsigned timestamp;
    int type;
    std::map <std::string,int>::iterator ph_src_it;
    std::map <std::string,int>::iterator ph_dest_it;
};

/*-----------------------------------------------------------------------------
 * konstanty  
 *-----------------------------------------------------------------------------*/
enum ExitCodes {
    RECOVER = 0,
    WRONG_PARAMS,
    SYSTEM_ERROR
};

const int DNS_PORT = 53;
const int MAX_DNS_LEN = 500;
const int TYPE_NAMES_SIZE = 43;
const unsigned SECS_PER_HOUR = 3600;

// nazvy pozadavku a odpovedi prevzate z <arpa/nameser.h>
const char *type_names[] = {
    "Cookie",
    "Host address",
    "Authoritative server",
    "Mail destination",
    "Mail forwarder",
    "Canonical name",
    "Start of authority zone",
    "Mailbox domain name",
    "Mail group member",
    "Mail rename name",
    "Null resource record",
    "Well known service",
    "Domain name pointer",
    "Host information",
    "Mailbox information",
    "Mail routing information",
    "Text strings",
    "Responsible person",
    "AFS cell database",
    "X_25 calling address",
    "ISDN calling address",
    "Router",
    "NSAP address",
    "Reverse NSAP lookup (deprecated)",
    "Security signature",
    "Security key",
    "X.400 mail mapping",
    "Geographical position (withdrawn)",
    "Ip6 Address",
    "Location Information",
    "Next domain (security)",
    "Endpoint identifier",
    "Nimrod Locator",
    "Server Selection",
    "ATM Addres",
    "Naming Authority PoinTe",
    "Key Exchang",
    "Certification recor",
    "IPv6 address (deprecated, use ns_t_aaaa",
    "Non-terminal DNAME (for IPv6",
    "Kitchen sink (experimentatl",
    "EDNS0 option (meta-RR",
    "Address prefix list (RFC3123)"
};

const char *help_message =
"\ndns_stat - program sluzi na zobrazenie statistik dns dotazov (zaslanim signalu SIGUSR1)\n\
pouzitie: dns_stat [parametre] -s adresa_dns_servra\n\
    dns_stat (bez parametrov)\n\
        vypise tuto napovedu\n\n\
    -s ip/nazov\n\
        specifikuje ip adresy/nazvy DNS serverov, na ktore sa budu posielat dotazy,\n\
	je mozne zadat maximalne 3 parametre -s\n\n\
    -l ip\n\
        urcuje adresu, na ktorej bude program naslouchat, defaultne INADDR_ANY\n\n\
    -p port\n\
        port, na ktorom bude program naslouchat (defaultne 53)\n\n\
    -type\n\
        zobrazi statistiky podla typov dotazov\n\n\
    -source\n\
        statistiky podla zdrojov (kto sa kolkokrat pytal)\n\n\
    -destination\n\
        statistiky podla dotazov (kilkokrat sa na co pytali)\n\n\
    -hour\n\
        zobrazi statistiky za poslednu hodinu\n\n\
    -average\n\
        priemerne hodnoty za hodinu\n\n";
    


// globalne promenne
bool loop_stop;
bool loop_waiting;
sigjmp_buf jump;
time_t start_time;

DnsStatsInfo dns_stats_info = { false, false, false, false, false};

unsigned q_rcvd = 0;
unsigned r_rcvd = 0;
std::map <std::string,int> st_dest;
std::map <std::string,int>::iterator st_dest_it;
std::map <std::string,int> st_src;
std::map <std::string,int>::iterator st_src_it;
short *type_values; 	// dlhodobe zaznamy o type dotazu
short *ph_type_values;	// kratkodobe zaznamy

// statistiky za hodinu
std::map <std::string,int> ph_dest;
std::map <std::string,int>::iterator ph_dest_it;
std::map <std::string,int> ph_src;
std::map <std::string,int>::iterator ph_src_it;
std::vector <entry> ph_stats;
std::vector <entry>::iterator ph_stats_it;

unsigned ph_q_rcvd = 0;
std::vector<int> resp_timestamp;
std::vector<int>::iterator resp_timestamp_it;

/*-----------------------------------------------------------------------------
 *  deklarace funkci
 *-----------------------------------------------------------------------------*/
int handleArgs( int argc, char **argv, 
	LocalhostInfo &localhost_info);
inline void handleError( int err_value, const char *msg);
void sigusrHandler(int n);
void termHandler(int n);
int portStr2Int(std::string &port_str);
void phActualizeResp(void);
void phActualizeQuer(void);
void phActualize(void);
void printLastHour(void);
void printHourAverage(void);
void printOverallStats(void);
void printStats(void);
int parseQuery(const u_char *query, int query_len, in_addr &addr);
void udpConnection(int &sockfd,	// naslouchani
	sockaddr_in &dns_server_addr, int &sockfd2, 	// vzdaleny server
	std::vector<std::string> &dns_servers);			// zoznam DNS adries 
