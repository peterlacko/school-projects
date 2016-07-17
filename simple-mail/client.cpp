/*
 * =====================================================================================
 *
 *       Filename:  client.cpp
 *
 *    Description:  program "client" pro projekt #2 kurzu IPK
 *
 *        Version:  1.0
 *        Created:  02.03.2012 22:01:19
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Peter Lacko (xlacko06), xlacko06@stud.fit.vutbr.cz
 *        Company:  
 *
 * =====================================================================================
 */

#include "client.h"

// kontrola uzivateskaho mena
int checkUsername (const char *username) {
   int strLen = strlen(username);
   if (strLen == 0)
      return 1;
   for (int i = 0; i < strLen; i++) {
      if (username[i] >= 'a' && username[i] <= 'z')
	 continue;
      if (username[i] >= '0' && username[i] <= '9')
	 continue;
      if (username[i] >= 'A' && username[i] <= 'Z')
	 continue;
      return 1;
   }
   return 0;
}

// spracovni parametru
int handleArgs (const int argc, const char *argv[], connectionInfo & server) {
   if (argc < 4 || argc > 5) {
      return 1;
   }
   if (strlen(argv[2]) != 2) {
      return 1;
   }
   if (checkUsername(argv[3]) != 0) {
      return 1;
   }

   if (argc == 4) {
      if (!strcmp(argv[2], "-r"))
	 server.requestType = requestTypes[0];
      else
	 return 1;
   }
   if (argc == 5) {
      if (!strcmp(argv[2], "-s")) {
	 server.requestType = requestTypes[1];
	 server.message = argv[4];
      }
      else if (!strcmp(argv[2], "-d")) {
	 server.requestType = requestTypes[2];
	 for (unsigned i = 0; i < strlen(argv[4]); i++) {
	    if (!isdigit(argv[4][i]))
	       return 1;
	 }
	 server.messageIndex = argv[4];
      }
      else
	 return 1;
   }
   server.username = argv[3];

   // ziskani adresy a cisla portu
   std::string host = argv[1];
   regmatch_t strMatch[3];
   regex_t uriRe;
   int status;
   std::string uriPattern = "^([^/?#:]+):([0-9]+)$";
   if (regcomp(&uriRe, uriPattern.c_str(), REG_EXTENDED) != 0) {
      return 1;
   }
   status = regexec(&uriRe, host.c_str(), 3, strMatch, 0);
   if (status == 0) {
      if (strMatch[0].rm_so == -1 || strMatch[1].rm_so == -1)
	 return 1;
      server.hostname = host.substr(strMatch[1].rm_so, strMatch[1].rm_eo-strMatch[1].rm_so);
      std::string portNumber = host.substr(strMatch[2].rm_so, strMatch[2].rm_eo-strMatch[2].rm_so);
      server.port = atoi(portNumber.c_str());
   }
   return 0;
}

int main (int argc, char *argv[]) {
   connectionInfo server = {};
   if (handleArgs(static_cast<const int>(argc),(const char**) argv, server) == 1) {
      std::cout << "pouziti:\nclient HOST:PORT [-r UZIVATEL] [-s PRIJEMCA \"sprava\"] [-d UZIVATEL CISLO_SPRAVY]\n";
      std::cout << "   -r citanie sprav\n";
      std::cout << "   -s odoslanie spravy\n";
      std::cout << "   -d odstranenie spravy daneho uzivatela\n\n";
      exit (WRONG_PARAMS);
   }
   int sockNumber;
   if((sockNumber = socket( PF_INET, SOCK_STREAM, 0)) < 0) {
      std::cerr <<"error: cannot create socket\n";
      exit (INTERNAL_ERROR);
   }
   //hostent *host = new hostent;
   hostent *host = gethostbyname(server.hostname.c_str());
   if (host == NULL) {
      std::cerr << "error: cannot resolve hostname\n";
      exit (CONNECTION_FAILED);
   }

   sockaddr_in servSock;
   servSock.sin_family = AF_INET;
   servSock.sin_port = htons(server.port);
   memcpy(&(servSock.sin_addr), host->h_addr_list[0], host->h_length);

   // pripojenie k serveru
   if ( connect(sockNumber, reinterpret_cast<sockaddr*>(&servSock), sizeof(servSock)) != 0) {
      std::cerr << "error: cannot connect to server\n";
      exit (CONNECTION_FAILED);
   }
   // povinny typ poziadavku, uzivateske meno + sprava alebo index spravy
   std::string request = server.requestType + " " + server.username + " " + server.message
      + server.messageIndex + "\n";
   if (write( sockNumber, request.c_str(), request.length()) == -1) {
      std::cerr << "error: cannot write to socket\n";
      exit (CONNECTION_FAILED);
   }

   std::string reply;
   char replyBuff[BUFFER_SIZE];
   unsigned replySize = 0;
   while ((replySize = read(sockNumber, replyBuff, sizeof(replyBuff))) > 0) {
      reply.append(replyBuff, replySize);
   }
   close (sockNumber);

   if (reply[reply.size()-1] != '\n') {
      std::cout << "error: message transmitting failed\n";
      exit (CONNECTION_FAILED);
   }
   if (reply == "\n\n")
      exit (EXIT_SUCCESS);
   if ((server.requestType == requestTypes[0]) )
      std::cout << reply;
   else
      std::cerr << reply;
   exit (EXIT_SUCCESS);
}
