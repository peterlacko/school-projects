/*
 * Autor: 		Peter Lacko, xlacko06
 * Datum:		14.3.2015
 * Soubor:		ahed.h
 * Komentar:	Hlavickovy subor k ahed.h
 */ 

#ifndef __KKO_AHED_H__
#define __KKO_AHED_H__

#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <iostream>
#include <string>
#include <map>
#include <queue>
#include <stack>

#define AHEDOK 0
#define AHEDFail -1
#define ZERO -2
#define BYTE_SIZE 8
#define BUF_SIZE 32

using namespace std;

/* Datovy typ zaznamu o (de)kodovani */
typedef struct{
	/* velikost nekodovaneho retezce */
	int64_t uncodedSize;
	/* velikost kodovaneho retezce */
	int64_t codedSize;
} tAHED;

typedef struct Code Code;
struct Code {
	int32_t c;
	uint32_t len;
};

typedef struct Node Node;
struct Node{
	// znak a frekvence uzlu
	int32_t sym;
	uint32_t freq;
	// kod uzlu vyuzivany pri jeho update
	Code code;
	// ukazatele na leveho/praveho potomka a predchozi uzol
	Node *left;
	Node *right;
	Node *parent;
	// pomocna premenna vyuzita pri rescalovani
	uint8_t visited;
};


/* Nazev:
 *   AHEDEncoding
 * Cinnost:
 *   Funkce koduje vstupni soubor do vystupniho souboru a porizuje zaznam o kodovani.
 * Parametry:
 *   ahed - zaznam o kodovani
 *   inputFile - vstupni soubor (nekodovany)
 *   outputFile - vystupní soubor (kodovany)
 * Navratova hodnota: 
 *    0 - kodovani probehlo v poradku
 *    -1 - pøi kodovani nastala chyba
 */
int AHEDEncoding(tAHED *ahed, FILE *inputFile, FILE *outputFile);


/* Nazev:
 *   AHEDDecoding
 * Cinnost:
 *   Funkce dekoduje vstupni soubor do vystupniho souboru a porizuje zaznam o dekodovani.
 * Parametry:
 *   ahed - zaznam o dekodovani
 *   inputFile - vstupni soubor (kodovany)
 *   outputFile - vystupní soubor (nekodovany)
 * Navratova hodnota: 
 *    0 - dekodovani probehlo v poradku
 *    -1 - pøi dekodovani nastala chyba
 */
int AHEDDecoding(tAHED *ahed, FILE *inputFile, FILE *outputFile);

#endif

