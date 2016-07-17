/*
 * =====================================================================================
 *
 *       Filename:  server.h
 *
 *    Description:  hlavickovy subor k server.cpp
 *
 *        Version:  1.0
 *        Created:  06.03.2012 09:12:55
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Peter Lacko (xlacko06), xlacko06@stud.fit.vutbr.cz
 *        Company:  
 *
 * =====================================================================================
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <iterator>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <regex.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/signal.h>
#include <signal.h>
#include <sys/time.h>

#include <sys/resource.h>
#include <sys/wait.h>
#include <pthread.h>


// datovy subor
std::ofstream file;

// pthread mutex
pthread_mutex_t dataMutex;

// z nudze cnost, globalny vektor je nutny koli zapisu buffrov pri zachyteni SIGTERmu
std::vector<std::string> fileCopy;

const char *const filename = "data";
const int BuffSize = 100;

// navratove hodnoty
enum ReturnValues {
	WRONG_PARAMS=11,
	INTERNAL_ERROR,
	CONNECTION_FAILED,
};

// obslhuha signalov SIGINT, SIGTERM
static void *sigThreadCatch (void *arg);
// spracovanie argumentov prikazoveho riadku
int handleArgs (int argc, char **argv);
// obsluha jedneho spojenia s klientom realizovana multithreadingom
void *handleConnection( void *data);
