/*
 * Autor: 		Peter Lacko, xlacko06
 * Datum:		14.3.2015
 * Soubor:		main.c
 * Komentar:	Hlavny program vykonavajuci kompresiu.
 */ 
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include "ahed.h"


int main(int argc, char **argv)
{
	static const char help[] = "ahed performs compressing of 8 bit symbols \
using adaptive huffman coding. Usage:\n\
-h Print this help\n\
-c Encode file\n\
-x Decode file\n\
-i FILE\n\
\t Specify input file. If not given, read from stdin.\n\
-o FILE\n\
\t Specify output file. If not given, write to stdout.\n\
-l FILE\n\
\t Specify log file. If not given, log will be ignored.\n";
	int opt = 0;
	int c = 0;
	int x = 0;
	int h = 0;
	FILE *fr = NULL;
	FILE *fw = NULL;
	FILE *flog = NULL;
	// parameters handling
	while((opt=getopt(argc, argv, "i:o:l:cxh"))!=-1) {
		switch(opt) {
			case 'i':
				fr = fopen(optarg, "r");
				if (fr == NULL) {
					perror(NULL);
					if (fw != NULL) {
						fclose(fw);
					}
					if (flog != NULL) {
						fclose(flog);
					}
					exit(AHEDFail);
				}
				break;
			case 'o':
				fw = fopen(optarg, "w");
				if (fw == NULL) {
					perror(NULL);
					if (fr != NULL) {
						fclose(fr);
					}
					if (flog != NULL) {
						fclose(flog);
					}
					exit(AHEDFail);
				}
				break;
			case 'l':
				flog = fopen(optarg, "w");
				if (flog == NULL) {
					perror(NULL);
					if (fr != NULL) {
						fclose(fr);
					}
					if (fw != NULL) {
						fclose(fw);
					}
					exit(AHEDFail);
				}
				break;
			case 'c':
				c = 1;
				break;
			case 'x':
				x = 1;
				break;
			case 'h':
				printf(help);
				exit(AHEDOK);
		}
	}
	// checking parameters, descriptors
	if ((c == 1 && x == 1) || (c == 0 && x == 0)) {
		exit(AHEDFail);
	}
	if (fw == NULL) {
		fw = stdout;
	}
	if (fr == NULL) {
		fr = stdin;
	}

	tAHED *ahed = new tAHED;
	int ret = 0;
	if (c == 1) {
		ret = AHEDEncoding(ahed, fr, fw);
	} else if (x == 1) {
		ret = AHEDDecoding(ahed, fr, fw);
	}
	if (flog != NULL) {
		fprintf(flog, "login = xlacko06\nuncodedSize = %ld\ncodedSize = %ld\n",
				ahed->uncodedSize, ahed->codedSize);
		fclose(flog);
	}
	free (ahed);
	fclose(fr);
	fclose(fw);
	exit(ret);
}
