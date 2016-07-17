/*
 * prvocisla.c
 * Rie¹enie IJC-DU1, príklad b) 21.3.2011
 * Autor: Peter Lacko, FIT
 * Prelo¾ené: GCC 4.5.2
 */

#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<limits.h>
#include"error.h"

#define N 90000000L
#define PRINT 20
#define LONG_S (sizeof(unsigned long)*CHAR_BIT)

// vytvori pole typu unsigned long prislusnej velkosti,
// inicializuje ho na nulu
#define BitArray(name,size) unsigned long name[(size/(LONG_S)) +1] = {0}

#ifdef USE_INLINE	// prelozi sa pri definovani symbolu USE_INLINE

// nastavi bit "index" na stav "vyraz"
inline void SetBit(unsigned long name[],int index,int vyraz) {
    if(((index)<0)||(index)>N-1) 
	Error("Index %ld mimo rozsah 0..%ld", (long)index, (long)N-1);
    (vyraz) ? (name[index/(LONG_S)] |= (1 << (LONG_S-1-(index%(LONG_S))))):(name[index/(LONG_S)] &= ~(1 << (LONG_S-1-((index%(LONG_S))))));
}

// ziska hodnotu bitu na pozicii "index"
inline int GetBit(unsigned long name[], int index) {
    if(((index)<0)||(index)>=N)
	Error("Index %ld mimo rozsah 0..%ld", (long)index, (long)N-1);
    return ((name[index/(LONG_S)] & (1 << (LONG_S-1-(index%(LONG_S)))))==0 ? 0:1);
}

#else

#define SetBit(name,index,vyraz) \
    do {\
	if(((index)<0)||(index)>N-1) Error("Index %ld mimo rozsah 0..%ld", (long)index, (long)N-1);\
	(vyraz) ? (name[index/(LONG_S)] |= (1 << (LONG_S-1-(index%(LONG_S))))):(name[index/(LONG_S)] &= ~(1 << (LONG_S-1-((index%(LONG_S))))));\
    } while(0)

#define GetBit(name,index) (((index)<0)||(index)>=N) ?  Error("Index %ld mimo rozsah 0..%ld", (long)index, (long)N-1),0:\
						  (name[(index)/(LONG_S)] & (1 << (LONG_S-1-((index)%(LONG_S)))))

#endif

int main(void)
{
    int sN = sqrt(N);
    BitArray(pole,N);
    long primes[PRINT] = {0};		// pole na ulozenie prvocisel urcenych na tisk
    for(int i = 2; i <= sN; i++) {	// "skrta" cisla az do sqrt N
	while(i < N && GetBit(pole,i)!=0)	// preskoci uz "vyskrtane" bity
	    i++;
	for(int n = i+i; n < N; n+=i) 	// nasobky i nastavi na '1'
	    SetBit(pole,n,1);
    }
    long j = N-1;
    for(int i = PRINT-1; i >=0; i--) {	// prechadza pole od N-1, prvocisla uklada do "primes"
	for(; (GetBit(pole,j)) != 0; j--)
	    ;
	primes[i]=j;
	j--;
    }
    for(int i = 0; i < PRINT; i++)	// tisk prvocisel
	printf("%lu\n", primes[i]);
    return EXIT_SUCCESS;
}
