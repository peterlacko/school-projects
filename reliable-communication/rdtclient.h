/*
 * =====================================================================================
 *
 *       Filename:  rdtclient.h
 *
 *    Description:  hlavickovy subor k rdtclient.c
 *
 *        Version:  1.0
 *        Created:  30.03.2012 23:23:29
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Peter Lacko (xlacko06), xlacko06@stud.fit.vutbr.cz
 *
 * =====================================================================================
 */

#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iterator>

#include <libxml/parser.h>
#include <libxml/tree.h>

#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <time.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include "b64_ntop.c"
//#include "b64_pton.c"

const int DATA_SIZE = 256;
const int B64_DATA_SIZE = 396; // cca 1.4*DATA_SIZE
const int MAX_ACK_LEN = 110;
const int CWND_SIZE = 5;

bool Done = false;	// indikuje ziskanie potvrzeni posledneho paketu
int sockNumber; // cislo socketu
in_addr_t remote_addr; // adresa vzdialeneho hosta (localhost)
int d_port; // destination port
// buffer pre odosielane pakety

itimerval mytimer;
itimerval test_timer;
mytimer.it_interval.tv_sec = 0;
mytimer.it_interval.tv_usec = 0;
mytimer.it_value.tv_sec = 1;
mytimer.it_value.tv_usec = 0;

std::vector<std::string> buffer;

int timeouts = 0;

struct cwndT {
    uint32_t begin;
    uint32_t end;
};

enum ReturnValues {
    WRONG_PARAMS=1,
    INTERNAL_ERROR,
    CONNECTION_FAILED,
    TRANSFER_FAILED,
};

void resendCwnd();
