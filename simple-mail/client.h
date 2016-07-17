/*
 * =====================================================================================
 *
 *       Filename:  client.h
 *
 *    Description:  hlavickovy subor k client.cpp
 *
 *        Version:  1.0
 *        Created:  06.03.2012 09:10:36
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Peter Lacko (xlacko06), xlacko06@stud.fit.vutbr.cz
 *        Company:  
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

const char * requestTypes[] = {
   "READ",
   "WRITE",
   "DELETE",
};

const int BUFFER_SIZE = 1000;

enum ReturnValues {
	WRONG_PARAMS=11,
	INTERNAL_ERROR,
	CONNECTION_FAILED,
};

struct connectionInfo {
   std::string hostname;
   size_t port;
   std::string requestType;
   std::string username;
   std::string message;
   std::string messageIndex;
};

int handleArgs (const int argc, const char *argv[], connectionInfo & server);


