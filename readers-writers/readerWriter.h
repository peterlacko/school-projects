/*
 * =====================================================================================
 *
 *       Filename:  readerWriter.h
 *
 *    Description:  hlavickovy subor k readerWrter.c
 *
 *        Version:  1.0
 *        Created:  05.04.2012 17:13:31
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Peter Lacko (xlacko06), xlacko06@stud.fit.vutbr.cz
 *        Company:  
 *
 * =====================================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>

#define LOCK(a) sem_wait(a)
#define UNLOCK(a) sem_post(a)

// pomocna hodnota ku klucu generovanemu funciou ftok()
const int FTOK_NUMBER = 111;


// v strukture su ulozene hodnoty zadane ako parametre programu
typedef struct {
    int W;
    int R;
    int C;
    int SW;
    int SR;
} params;

// deklaracie pouzitych funkcii
int to_digit(char *str);
int handle_args (int argc, char **argv, params *param);
