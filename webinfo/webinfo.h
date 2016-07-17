/*
 * =====================================================================================
 *
 *       Filename:  webinifo.h
 *
 *    Description:  hlavickovy subor k webinfo.cpp 
 *
 *        Version:  1.0
 *        Created:  26.02.2012 23:40:26
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Peter Lacko (xlacko06), xlacko06@stud.fit.vutbr.cz
 *
 * =====================================================================================
 */

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

// struktura uchovava informacie o parametroch
struct paramsInfo {
   int lSize;
   int sId;
   int mModified;
   int tType;
};

// struktura uriInfo obsahuje rozparsovanu adresu
struct uriInfo {
   std::string domain;
   std::string path;
   std::string port;
};

// velkost buffru pre nacitanu spravu
const int BUFFER_SIZE = 1000;

// maximalny povoleny pocet presmerovani
const int MaxRedirections = 5;

// navratove kody funkce spracujucej odpoved servra
enum httpReturnValues { 
	HTTP_OK, 
	HTTP_REDIRECT, 
	HTTP_ERROR
};

// navratove kony programu
enum ReturnValues {
	WRONG_PARAMS=11, 
	WRONG_ADDRESS,
	SOCKET_ERROR, 
	CONNECTION_ERROR, 
	INTERNAL_ERROR 
};

int connectAndWrite (std::string & reply, const uriInfo uri, int & sockNumber);
int handleReply (std::string & reply, std::string & codeOrAddress, int & crlf);
int parseUri (const char * uriStr_c, uriInfo & uri);
int GetArg (int argc, char *argv[], paramsInfo & params);

