/*
 * =====================================================================================
 *
 *       Filename:  rdtserver.h
 *
 *    Description:  hlavickovy subor k rdtserver.c
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
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <vector>

#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

#include <libxml/parser.h>
#include <libxml/tree.h>

#include "b64_pton.c"

const int DATA_SIZE = 256;
const int B64_DATA_SIZE = 396; // cca 1.4*DATA_SIZE
const int MAX_PACKET_LEN = 512;	// maximalna delka paketu
const int MAX_CWND_SIZE = 10;

bool Stop = false;

struct cwndT {
    uint32_t begin;
    uint32_t end;
};

enum ReturnValues {
    WRONG_PARAMS=1,
    INTERNAL_ERROR,
    CONNECTION_FAILED,
};
