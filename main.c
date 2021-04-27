#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

int nThreads=8;

typedef enum {false, true} bool;

unsigned char cw[]={0,2,3,1};
unsigned char acw[]={0,3,1,2};

char* moveList[18]={"U","U2","U'","D","D2","D'","R","R2","R'","L","L2","L'","F","F2","F'","B","B2","B'"};

struct CUBE {
	uint64_t EPCO,CPEOCN;
};

struct MOVES {
	char list[999];
};

typedef struct entry_t {
	uint64_t EPCO,CPEOCN,sequence;
	struct entry_t *next;
} entry_t;

typedef struct {
	entry_t **entries;
} ht_t;

struct STEP {
	char movegroup;	
	char tableDepth;
	char searchDepth;
	char Nbits;
	ht_t *table;
	char name[32];
	uint64_t EPCOmask,CPEOCNmask;
	int min, max;
	float mean;	
	struct histogram *hist;
	struct STEP *next;
	struct STEP *listnext;
};

struct histogram {
	int table[21];
};

struct state {
	struct CUBE *cube;
	struct solution *solution;
};

struct METHOD {
	char name[32];
	struct STEP *first;
};

struct solution {
	uint64_t sol1,sol2;
	struct solution *next;
};

struct parallelOut {
	struct solution *solutions;
	int quantity;
};

struct parallelArgs {
	uint64_t *candidates;
	struct CUBE* cube;
	struct STEP* step;
	struct MOVES* moves;
	unsigned char Nbits;
	int start;
	int end;
};

struct parallelAnalyzeArgs {
	uint64_t *candidates;
	int nCandidates;
	struct CUBE* cube;
	struct STEP* step;
	struct MOVES* moves;
	unsigned char Nbits;
	struct state** states;
	int start;
	int end;
};

int gettimeofday(struct timeval *restrict tp, void *restrict tzp);

long power(char base, char exponent) {
	long out=1;
	for (int i=0; i<exponent; i++) {
		out=out*base;	
	}
	return out;
}

void binaryOut(uint64_t number) {
	if (number) {
		binaryOut(number>>1);
		putc((number&1)?'1':'0',stdout);
	}
}

void printCube(struct CUBE* cube) {
	printf("\n\tEP:");
	for (int i=0; i<12; ++i)
		printf(" %lx",0xf&(cube->EPCO>>(60-4*i)));
	printf("\n\tEO:");
	for (int i=0; i<12; ++i)
		printf(" %lx",0x3&(cube->CPEOCN>>(28-2*i)));
	printf("\n\tCP:");
	for (int i=0; i<8; ++i)
		printf(" %lx",0xf&(cube->CPEOCN>>(58-4*i)));
	printf("\n\tCO:");
	for (int i=0; i<8; ++i)
		printf(" %lx",0x3&(cube->EPCO>>(14-2*i)));
	printf("\n\tCN:");
	for (int i=0; i<2; ++i)
		printf(" %lx",0x7&(cube->CPEOCN>>(3-3*i)));
	printf("\n");
}

void revertCube(struct CUBE* cube) {
	//0001 0010 0011 0100 0101 0110 0111 1000 1001 1010 1011 1100 11 11 11 11 11 11 11 11 
	cube->EPCO=0b0001001000110100010101100111100010011010101111001111111111111111;
	//0001 0010 0011 0100 0101 0110 0111 1000 11 11 11 11 11 11 11 11 11 11 11 11 001 010
	cube->CPEOCN=0b00010010001101000101011001111000111111111111111111111111001010;
}

void randomizeCube(struct CUBE* cube) {	// fisher-yates shuffle

	int s,i;
	bool parity=0;

	for(i=0;i<11;i++){
		s=(rand()%(11-i+1))+i;
		if (s!=i) {
			cube->EPCO=(cube->EPCO&(~(15ULL<<(60-4*s)))&(~(15ULL<<(60-4*i))))|((cube->EPCO&(15ULL<<(60-4*i)))>>(4*(s-i)))|((cube->EPCO&(15ULL<<(60-4*s)))<<(4*(s-i)));
			parity=!parity;
		}
	}

	for(i=0;i<7;i++){
		s=(rand()%(7-i+1))+i;
		if (s!=i) {
			cube->CPEOCN=(cube->CPEOCN&(~(15ULL<<(58-4*s)))&(~(15ULL<<(58-4*i))))|((cube->CPEOCN&(15ULL<<(58-4*i)))>>(4*(s-i)))|((cube->CPEOCN&(15ULL<<(58-4*s)))<<(4*(s-i)));
			parity=!parity;
		}
	}

	// do an extra cycle in case of parity - unlike the sheeple that just skip the last cycle and break randomness
	while (parity) {
		i=(rand()%(12));
		s=(rand()%(11-i+1))+i;
		if (s!=i) {
			cube->EPCO=(cube->EPCO&(~(15ULL<<(60-4*s)))&(~(15ULL<<(60-4*i))))|((cube->EPCO&(15ULL<<(60-4*i)))>>(4*(s-i)))|((cube->EPCO&(15ULL<<(60-4*s)))<<(4*(s-i)));
			parity=!parity;
		}
	}

	bool flips=0;
	for (i=0;i<12;i++){
		if ((i==11) && !flips) {
			break;
		}
		if ((rand()%(2)) || ((i==11) && flips)){
			cube->CPEOCN^=(2ULL<<(28-2*i));
			flips=!flips;
		}
	}
	
	int spins=0;

	for (i=0;i<7;i++){
		s=(rand()%3)+1;
		cube->EPCO=(cube->EPCO&(~(3ULL<<(14-2*i))))|(s<<(14-2*i));	
		spins+=s;
	}
	if (spins%3){
		cube->EPCO=(cube->EPCO&(~3ULL))|(3-spins%3);	
	}

}

void applyMask(struct CUBE* cube, uint64_t EPCOmask, uint64_t CPEOCNmask) {
	cube->EPCO&=EPCOmask;
	cube->CPEOCN&=CPEOCNmask;
}

void applyMaskScrambled(struct CUBE* cube, uint64_t EPCOmask, uint64_t CPEOCNmask) {
	struct CUBE *cube0=(struct CUBE*)malloc(sizeof(struct CUBE));
	struct CUBE *cubeS=(struct CUBE*)malloc(sizeof(struct CUBE));
	revertCube(cube0);
	revertCube(cubeS);
	cube0->EPCO&=EPCOmask;
	cube0->CPEOCN&=CPEOCNmask;
	bool inMask=0;

	uint64_t EO=0;
	uint64_t CO=0;
	uint64_t CN=0;

	for (int i=0;i<2;i++){ // CN
		if (CPEOCNmask>>(3-3*i)&7ULL){ 
			for (int j=0; j<2; j++){ 
				if ((((cube0->CPEOCN)&(7ULL<<(3-3*i)))>>(3-3*i))==(((cube->CPEOCN)&(7ULL<<(3-3*j)))>>(3-3*j))){
					CN+=((cube->CPEOCN>>(3-3*j))&7ULL)<<(3-3*j);
					break;
				}
			}
		}
	}

	for (int i=0;i<12;i++){ // EO
		if (CPEOCNmask>>(29-2*i)&1ULL){ // if we care about the orientation for the ith edge
			for (int j=0; j<12; j++){ // check if the ith solved edges permutation is in the cube state (permutation) (it will be, we just need to know where) 
				if ((((cubeS->EPCO)&(15ULL<<(60-4*i)))>>(60-4*i))==(((cube->EPCO)&(15ULL<<(60-4*j)))>>(60-4*j))){
					EO+=((cube->CPEOCN>>(28-2*j))&3ULL)<<(28-2*j);
					break;
				}
			}
		}
	}

	for (int i=0;i<8;i++){ // CO
		if (EPCOmask>>(14-2*i)&3ULL){ 
			for (int j=0; j<8; j++){ 
				if ((((cubeS->CPEOCN)&(15ULL<<(58-4*i)))>>(58-4*i))==(((cube->CPEOCN)&(15ULL<<(58-4*j)))>>(58-4*j))){
					CO+=((cube->EPCO>>(14-2*j))&3ULL)<<(14-2*j);
					break;
				}
			}
		}
	}

	for (int i=0;i<12;i++){ // EP
		inMask=0;
		for (int j=0; j<12; j++){
			if ((((cube0->EPCO)&(15ULL<<(60-4*j)))>>(60-4*j))==(((cube->EPCO)&(15ULL<<(60-4*i)))>>(60-4*i))){
				inMask=1;
				break;
			}
		}
		if (inMask==0){
			cube->EPCO=cube->EPCO&(~(15ULL<<(60-4*i)));
		}
	}

	for (int i=0;i<8;i++){ // CP
		inMask=0;
		for (int j=0; j<8; j++){
			if ((((cube0->CPEOCN)&(15ULL<<(58-4*j)))>>(58-4*j))==(((cube->CPEOCN)&(15ULL<<(58-4*i)))>>(58-4*i))){
				inMask=1;
				break;
			}
		}
		if (inMask==0){
			cube->CPEOCN=cube->CPEOCN&(~(15ULL<<(58-4*i)));
		}
	}

	cube->CPEOCN=(cube->CPEOCN&(~((1ULL<<30)-1)))+EO+CN;
	cube->EPCO=(cube->EPCO&(~((1ULL<<16)-1)))+CO;

	free(cube0);
}

void applyMove(struct CUBE* cube, char move) {
	switch(move) {
		case 1: // U
			cube->EPCO=(((0xfUL<<48)&cube->EPCO)<<12)|
				(((0xfffUL<<52)&cube->EPCO)>>4)|
				(((0x3UL<<8)&cube->EPCO)<<6)|
				(((0xfcUL<<8)&cube->EPCO)>>2)|
				(0xffffffff00ff&cube->EPCO);
			cube->CPEOCN=(((0xfUL<<46)&cube->CPEOCN)<<12)|
				(((0xfffUL<<50)&cube->CPEOCN)>>4)|
				(((0x3UL<<22)&cube->CPEOCN)<<6)|
				(((0xfcUL<<22)&cube->CPEOCN)>>2)|
				(0x3fffc03fffff&cube->CPEOCN);
			break;
		case 2: // U2
			cube->EPCO=(((0xffUL<<48)&cube->EPCO)<<8)|
				(((0xffUL<<56)&cube->EPCO)>>8)|
				(((0xfUL<<8)&cube->EPCO)<<4)|
				(((0xfUL<<12)&cube->EPCO)>>4)|
				(0xffffffff00ff&cube->EPCO);	
			cube->CPEOCN=(((0xffUL<<46)&cube->CPEOCN)<<8)|
				(((0xffUL<<54)&cube->CPEOCN)>>8)|
				(((0xfUL<<22)&cube->CPEOCN)<<4)|
				(((0xfUL<<26)&cube->CPEOCN)>>4)|
				(0x3fffc03fffff&cube->CPEOCN);
			break;
		case 3: // U'
			cube->EPCO=(((0xfffUL<<48)&cube->EPCO)<<4)|
				(((0xfUL<<60)&cube->EPCO)>>12)|
				(((0x3fUL<<8)&cube->EPCO)<<2)|
				(((0xcUL<<12)&cube->EPCO)>>6)|
				(0xffffffff00ff&cube->EPCO);
			cube->CPEOCN=(((0xfffUL<<46)&cube->CPEOCN)<<4)|
				(((0xfUL<<58)&cube->CPEOCN)>>12)|
				(((0x3fUL<<22)&cube->CPEOCN)<<2)|
				(((0x3UL<<28)&cube->CPEOCN)>>6)|
				(0x3fffc03fffff&cube->CPEOCN);
			break;
		case 4: // D 
			cube->EPCO=(((0xfffUL<<16)&cube->EPCO)<<4)|
				(((0xfUL<<28)&cube->EPCO)>>12)|
				(((0x3fUL)&cube->EPCO)<<2)|
				(((0xcUL<<4)&cube->EPCO)>>6)|
				(0xffffffff0000ff00&cube->EPCO);
			cube->CPEOCN=(((0xfffUL<<30)&cube->CPEOCN)<<4)|
				(((0xfUL<<42)&cube->CPEOCN)>>12)|
				(((0x3fUL<<6)&cube->CPEOCN)<<2)|
				(((0x3UL<<12)&cube->CPEOCN)>>6)|
				(0x3fffc0003fffc03f&cube->CPEOCN);
			break;
		case 5: // D2
			cube->EPCO=(((0xffUL<<16)&cube->EPCO)<<8)|
				(((0xffUL<<24)&cube->EPCO)>>8)|
				(((0xfUL)&cube->EPCO)<<4)|
				(((0xfUL<<4)&cube->EPCO)>>4)|
				(0xffffffff0000ff00&cube->EPCO);	
			cube->CPEOCN=(((0xffUL<<30)&cube->CPEOCN)<<8)|
				(((0xffUL<<38)&cube->CPEOCN)>>8)|
				(((0xfUL<<6)&cube->CPEOCN)<<4)|
				(((0xfUL<<10)&cube->CPEOCN)>>4)|
				(0x3fffc0003fffc03f&cube->CPEOCN);
			break;
		case 6: // D'
			cube->EPCO=(((0xfUL<<16)&cube->EPCO)<<12)|
				(((0xfffUL<<20)&cube->EPCO)>>4)|
				(((0x3UL)&cube->EPCO)<<6)|
				(((0xfcUL)&cube->EPCO)>>2)|
				(0xffffffff0000ff00&cube->EPCO);
			cube->CPEOCN=(((0xfUL<<30)&cube->CPEOCN)<<12)|
				(((0xfffUL<<34)&cube->CPEOCN)>>4)|
				(((0x3UL<<6)&cube->CPEOCN)<<6)|
				(((0xfcUL<<6)&cube->CPEOCN)>>2)|
				(0x3fffc0003fffc03f&cube->CPEOCN);
			break;
		case 7: // R
			cube->EPCO=(((0xfUL<<36)&cube->EPCO)<<20)|
				(((0xfUL<<56)&cube->EPCO)>>16)|
				(((0xfUL<<40)&cube->EPCO)>>16)|
				(((0XfUL<<24)&cube->EPCO)<<12)|
				((cw[(((0x3UL<<10)&cube->EPCO)>>10)]))<<12|
				((acw[(((0x3UL<<12)&cube->EPCO)>>12)]))<<4|
				((cw[(((0x3UL<<4)&cube->EPCO)>>4)]))<<2|
				((acw[(((0x3UL<<2)&cube->EPCO)>>2)]))<<10|
				(0xf0fff00ff0ffc3c3&cube->EPCO);
			cube->CPEOCN=(((0xfUL<<50)&cube->CPEOCN)<<4)|
				(((0xfUL<<54)&cube->CPEOCN)>>16)|
				(((0xfUL<<38)&cube->CPEOCN)>>4)|
				(((0xfUL<<34)&cube->CPEOCN)<<16)|
				(((0x3UL<<16)&cube->CPEOCN)<<10)|
				(((0x3UL<<26)&cube->CPEOCN)>>8)|
				(((0x3UL<<18)&cube->CPEOCN)>>8)|
				(((0x3UL<<10)&cube->CPEOCN)<<6)|
				(0x3c03fc03f3f0f3ff&cube->CPEOCN);
			break;
		case 8: // R2 
			cube->EPCO=(((0xfUL<<36)&cube->EPCO)<<4)|
				(((0xfUL<<56)&cube->EPCO)>>32)|
				(((0xfUL<<40)&cube->EPCO)>>4)|
				(((0XfUL<<24)&cube->EPCO)<<32)|
				(((0x3UL<<10)&cube->EPCO)>>6)|
				(((0x3UL<<12)&cube->EPCO)>>10)|
				(((0x3UL<<4)&cube->EPCO)<<6)|
				(((0x3UL<<2)&cube->EPCO)<<10)|
				(0xf0fff00ff0ffc3c3&cube->EPCO);
			cube->CPEOCN=(((0xfUL<<50)&cube->CPEOCN)>>12)|
				(((0xfUL<<54)&cube->CPEOCN)>>20)|
				(((0xfUL<<38)&cube->CPEOCN)<<12)|
				(((0xfUL<<34)&cube->CPEOCN)<<20)|
				(((0x3UL<<16)&cube->CPEOCN)<<2)|
				(((0x3UL<<26)&cube->CPEOCN)>>16)|
				(((0x3UL<<18)&cube->CPEOCN)>>2)|
				(((0x3UL<<10)&cube->CPEOCN)<<16)|
				(0x3c03fc03f3f0f3ff&cube->CPEOCN);
			break;
		case 9: // R' 
			cube->EPCO=(((0xfUL<<36)&cube->EPCO)>>12)|
				(((0xfUL<<56)&cube->EPCO)>>20)|
				(((0xfUL<<40)&cube->EPCO)<<16)|
				(((0XfUL<<24)&cube->EPCO)<<16)|
				((cw[(((0x3UL<<10)&cube->EPCO)>>10)]))<<2|
				((acw[(((0x3UL<<12)&cube->EPCO)>>12)]))<<10|
				((cw[(((0x3UL<<4)&cube->EPCO)>>4)]))<<12|
				((acw[(((0x3UL<<2)&cube->EPCO)>>2)]))<<4|
				(0xf0fff00ff0ffc3c3&cube->EPCO);
			cube->CPEOCN=(((0xfUL<<50)&cube->CPEOCN)>>16)|
				(((0xfUL<<54)&cube->CPEOCN)>>4)|
				(((0xfUL<<38)&cube->CPEOCN)<<16)|
				(((0xfUL<<34)&cube->CPEOCN)<<4)|
				(((0x3UL<<16)&cube->CPEOCN)>>6)|
				(((0x3UL<<26)&cube->CPEOCN)>>10)|
				(((0x3UL<<18)&cube->CPEOCN)<<8)|
				(((0x3UL<<10)&cube->CPEOCN)<<8)|
				(0x3c03fc03f3f0f3ff&cube->CPEOCN);
			break;
		case 10: // L 
			cube->EPCO=(((0xfUL<<48)&cube->EPCO)>>16)|
				(((0xfUL<<32)&cube->EPCO)>>16)|
				(((0xfUL<<16)&cube->EPCO)<<28)|
				(((0XfUL<<44)&cube->EPCO)<<4)|
				((cw[(((0x3UL<<14)&cube->EPCO)>>14)]))<<8|
				((acw[(((0x3UL<<8)&cube->EPCO)>>8)]))|
				((cw[(((0x3UL)&cube->EPCO))]))<<6|
				((acw[(((0x3UL<<6)&cube->EPCO)>>6)]))<<14|
				(0xfff00ff0fff03c3c&cube->EPCO);
			cube->CPEOCN=(((0xfUL<<58)&cube->CPEOCN)>>12)|
				(((0xfUL<<46)&cube->CPEOCN)>>16)|
				(((0xfUL<<30)&cube->CPEOCN)<<12)|
				(((0xfUL<<42)&cube->CPEOCN)<<16)|
				(((0x3UL<<22)&cube->CPEOCN)>>8)|
				(((0x3UL<<14)&cube->CPEOCN)>>8)|
				(((0x3UL<<6)&cube->CPEOCN)<<14)|
				(((0x3UL<<20)&cube->CPEOCN)<<2)|
				(0x03fc03fc3f0f3f3f&cube->CPEOCN);
			break;
		case 11: // L2
			cube->EPCO=(((0xfUL<<48)&cube->EPCO)>>32)|
				(((0xfUL<<32)&cube->EPCO)<<12)|
				(((0xfUL<<16)&cube->EPCO)<<32)|
				(((0XfUL<<44)&cube->EPCO)>>12)|
				(((0x3UL<<14)&cube->EPCO)>>14)|
				(((0x3UL<<8)&cube->EPCO)>>2)|
				(((0x3UL)&cube->EPCO)<<14)|
				(((0x3UL<<6)&cube->EPCO)<<2)|
				(0xfff00ff0fff03c3c&cube->EPCO);
			cube->CPEOCN=(((0xfUL<<58)&cube->CPEOCN)>>28)|
				(((0xfUL<<46)&cube->CPEOCN)>>4)|
				(((0xfUL<<30)&cube->CPEOCN)<<28)|
				(((0xfUL<<42)&cube->CPEOCN)<<4)|
				(((0x3UL<<22)&cube->CPEOCN)>>16)|
				(((0x3UL<<14)&cube->CPEOCN)<<6)|
				(((0x3UL<<6)&cube->CPEOCN)<<16)|
				(((0x3UL<<20)&cube->CPEOCN)>>6)|
				(0x03fc03fc3f0f3f3f&cube->CPEOCN);
			break;
		case 12: // L' 
			cube->EPCO=(((0xfUL<<48)&cube->EPCO)>>4)|
				(((0xfUL<<32)&cube->EPCO)<<16)|
				(((0xfUL<<16)&cube->EPCO)<<16)|
				(((0XfUL<<44)&cube->EPCO)>>28)|
				((cw[(((0x3UL<<14)&cube->EPCO)>>14)]))<<6|
				((acw[(((0x3UL<<8)&cube->EPCO)>>8)]))<<14|
				((cw[(((0x3UL)&cube->EPCO))]))<<8|
				((acw[(((0x3UL<<6)&cube->EPCO)>>6)]))|
				(0xfff00ff0fff03c3c&cube->EPCO);
			cube->CPEOCN=(((0xfUL<<58)&cube->CPEOCN)>>16)|
				(((0xfUL<<46)&cube->CPEOCN)<<12)|
				(((0xfUL<<30)&cube->CPEOCN)<<16)|
				(((0xfUL<<42)&cube->CPEOCN)>>12)|
				(((0x3UL<<22)&cube->CPEOCN)>>2)|
				(((0x3UL<<14)&cube->CPEOCN)<<8)|
				(((0x3UL<<6)&cube->CPEOCN)<<8)|
				(((0x3UL<<20)&cube->CPEOCN)>>14)|
				(0x03fc03fc3f0f3f3f&cube->CPEOCN);
			break;
		case 13: // F 
			cube->EPCO=(((0xfUL<<52)&cube->EPCO)>>16)|
				(((0xfUL<<36)&cube->EPCO)>>16)|
				(((0xfUL<<20)&cube->EPCO)<<12)|
				(((0XfUL<<32)&cube->EPCO)<<20)|
				((cw[(((0x3UL<<8)&cube->EPCO)>>8)]))<<10|
				((acw[(((0x3UL<<10)&cube->EPCO)>>10)]))<<2|
				((cw[(((0x3UL<<2)&cube->EPCO)>>2)]))|
				((acw[(((0x3UL)&cube->EPCO))]))<<8|
				(0xff0fff00ff0ff0f0&cube->EPCO);
			cube->CPEOCN=(((0xfUL<<46)&cube->CPEOCN)<<4)|
				(((0xfUL<<50)&cube->CPEOCN)>>16)|
				(((0xfUL<<34)&cube->CPEOCN)>>4)|
				(((0xfUL<<30)&cube->CPEOCN)<<16)|
				((((0x1UL<<24)&cube->CPEOCN)<<1)^((0x3UL<<24)&cube->CPEOCN))>>8|
				((((0x1UL<<16)&cube->CPEOCN)<<1)^((0x3UL<<16)&cube->CPEOCN))>>8|
				((((0x1UL<<8)&cube->CPEOCN)<<1)^((0x3UL<<8)&cube->CPEOCN))<<6|
				((((0x1UL<<14)&cube->CPEOCN)<<1)^((0x3UL<<14)&cube->CPEOCN))<<10|
				(0x3fc03fc03cfc3cff&cube->CPEOCN);
			break;
		case 14: // F2 
			cube->EPCO=(((0xfUL<<52)&cube->EPCO)>>32)|
				(((0xfUL<<36)&cube->EPCO)>>4)|
				(((0xfUL<<20)&cube->EPCO)<<32)|
				(((0XfUL<<32)&cube->EPCO)<<4)|
				(((0x3UL<<8)&cube->EPCO)>>6)|
				(((0x3UL<<10)&cube->EPCO)>>10)|
				(((0x3UL<<2)&cube->EPCO)<<6)|
				(((0x3UL)&cube->EPCO)<<10)|
				(0xff0fff00ff0ff0f0&cube->EPCO);
			cube->CPEOCN=(((0xfUL<<46)&cube->CPEOCN)>>12)|
				(((0xfUL<<50)&cube->CPEOCN)>>20)|
				(((0xfUL<<34)&cube->CPEOCN)<<12)|
				(((0xfUL<<30)&cube->CPEOCN)<<20)|
				(((0x3UL<<24)&cube->CPEOCN))>>16|
				(((0x3UL<<16)&cube->CPEOCN))>>2|
				(((0x3UL<<8)&cube->CPEOCN))<<16|
				(((0x3UL<<14)&cube->CPEOCN))<<2|
				(0x3fc03fc03cfc3cff&cube->CPEOCN);
			break;
		case 15: // F' 
			cube->EPCO=(((0xfUL<<52)&cube->EPCO)>>20)|
				(((0xfUL<<36)&cube->EPCO)<<16)|
				(((0xfUL<<20)&cube->EPCO)<<16)|
				(((0XfUL<<32)&cube->EPCO)>>12)|
				((cw[(((0x3UL<<8)&cube->EPCO)>>8)]))|
				((acw[(((0x3UL<<10)&cube->EPCO)>>10)]))<<8|
				((cw[(((0x3UL<<2)&cube->EPCO)>>2)]))<<10|
				((acw[(((0x3UL)&cube->EPCO))]))<<2|
				(0xff0fff00ff0ff0f0&cube->EPCO);
			cube->CPEOCN=(((0xfUL<<46)&cube->CPEOCN)>>16)|
				(((0xfUL<<50)&cube->CPEOCN)>>4)|
				(((0xfUL<<34)&cube->CPEOCN)<<16)|
				(((0xfUL<<30)&cube->CPEOCN)<<4)|
				((((0x1UL<<24)&cube->CPEOCN)<<1)^((0x3UL<<24)&cube->CPEOCN))>>10|
				((((0x1UL<<16)&cube->CPEOCN)<<1)^((0x3UL<<16)&cube->CPEOCN))<<8|
				((((0x1UL<<8)&cube->CPEOCN)<<1)^((0x3UL<<8)&cube->CPEOCN))<<8|
				((((0x1UL<<14)&cube->CPEOCN)<<1)^((0x3UL<<14)&cube->CPEOCN))>>6|
				(0x3fc03fc03cfc3cff&cube->CPEOCN);
			break;
		case 16: // B 
			cube->EPCO=(((0xfUL<<60)&cube->EPCO)>>16)|
				(((0xfUL<<44)&cube->EPCO)>>16)|
				(((0xfUL<<28)&cube->EPCO)<<12)|
				(((0XfUL<<40)&cube->EPCO)<<20)|
				((acw[(((0x3UL<<14)&cube->EPCO)>>14)]))<<6|
				((cw[(((0x3UL<<6)&cube->EPCO)>>6)]))<<4|
				((acw[(((0x3UL<<4)&cube->EPCO)>>4)]))<<12|
				((cw[(((0x3UL<<12)&cube->EPCO)>>12)]))<<14|
				(0x0fff00ff0fff0f0f&cube->EPCO);
			cube->CPEOCN=(((0xfUL<<58)&cube->CPEOCN)>>16)|
				(((0xfUL<<42)&cube->CPEOCN)>>4)|
				(((0xfUL<<38)&cube->CPEOCN)<<16)|
				(((0xfUL<<54)&cube->CPEOCN)<<4)|
				((((0x1UL<<28)&cube->CPEOCN)<<1)^((0x3UL<<28)&cube->CPEOCN))>>8|
				((((0x1UL<<20)&cube->CPEOCN)<<1)^((0x3UL<<20)&cube->CPEOCN))>>8|
				((((0x1UL<<12)&cube->CPEOCN)<<1)^((0x3UL<<12)&cube->CPEOCN))<<6|
				((((0x1UL<<18)&cube->CPEOCN)<<1)^((0x3UL<<18)&cube->CPEOCN))<<10|
				(0x003fc03fcfc3cfff&cube->CPEOCN);
			break;
		case 17: // B2 
			cube->EPCO=(((0xfUL<<60)&cube->EPCO)>>32)|
				(((0xfUL<<44)&cube->EPCO)>>4)|
				(((0xfUL<<28)&cube->EPCO)<<32)|
				(((0XfUL<<40)&cube->EPCO)<<4)|
				(((0x3UL<<14)&cube->EPCO)>>10)|
				(((0x3UL<<6)&cube->EPCO)<<6)|
				(((0x3UL<<4)&cube->EPCO)<<10)|
				(((0x3UL<<12)&cube->EPCO)>>6)|
				(0x0fff00ff0fff0f0f&cube->EPCO);
			cube->CPEOCN=(((0xfUL<<58)&cube->CPEOCN)>>20)|
				(((0xfUL<<42)&cube->CPEOCN)<<12)|
				(((0xfUL<<38)&cube->CPEOCN)<<20)|
				(((0xfUL<<54)&cube->CPEOCN)>>12)|
				(((0x3UL<<28)&cube->CPEOCN))>>16|
				(((0x3UL<<20)&cube->CPEOCN))>>2|
				(((0x3UL<<12)&cube->CPEOCN))<<16|
				(((0x3UL<<18)&cube->CPEOCN))<<2|
				(0x003fc03fcfc3cfff&cube->CPEOCN);
			break;
		case 18: // B' 
			cube->EPCO=(((0xfUL<<60)&cube->EPCO)>>20)|
				(((0xfUL<<44)&cube->EPCO)<<16)|
				(((0xfUL<<28)&cube->EPCO)<<16)|
				(((0XfUL<<40)&cube->EPCO)>>12)|
				((acw[(((0x3UL<<14)&cube->EPCO)>>14)]))<<12|
				((cw[(((0x3UL<<6)&cube->EPCO)>>6)]))<<14|
				((acw[(((0x3UL<<4)&cube->EPCO)>>4)]))<<6|
				((cw[(((0x3UL<<12)&cube->EPCO)>>12)]))<<4|
				(0x0fff00ff0fff0f0f&cube->EPCO);
			cube->CPEOCN=(((0xfUL<<58)&cube->CPEOCN)>>4)|
				(((0xfUL<<42)&cube->CPEOCN)<<16)|
				(((0xfUL<<38)&cube->CPEOCN)<<4)|
				(((0xfUL<<54)&cube->CPEOCN)>>16)|
				((((0x1UL<<28)&cube->CPEOCN)<<1)^((0x3UL<<28)&cube->CPEOCN))>>10|
				((((0x1UL<<20)&cube->CPEOCN)<<1)^((0x3UL<<20)&cube->CPEOCN))<<8|
				((((0x1UL<<12)&cube->CPEOCN)<<1)^((0x3UL<<12)&cube->CPEOCN))<<8|
				((((0x1UL<<18)&cube->CPEOCN)<<1)^((0x3UL<<18)&cube->CPEOCN))>>6|
				(0x003fc03fcfc3cfff&cube->CPEOCN);
			break;
	}
}

char *readableSequence(uint64_t sequence) {
	char *moves[18]={"U ","U2 ","U' ","D ","D2 ","D' ",
		"R ","R2 ","R' ","L ","L2 ","L' ",
		"F ","F2 ","F' ","B ","B2 ","B' "};
	int place=0;
	char *out=malloc(99);
	for (int i=0; i<=10; i++) {
		char buffer=((sequence&(63ULL<<(6*i)))>>(6*i));
		if (buffer) {
			for (int j=0;j<strlen(moves[buffer-1]);j++) {
				out[place++]=moves[buffer-1][j];
			}
		}
		else {
			out[place]='\0';
			break;
		}
	}
	return out;
}

void addLayers(char movegroup, uint64_t baseSequence, char depth, uint64_t *candidates, int *c) {
	const unsigned char axes[]={1,1,0,0,2,2};
	if (movegroup==1) { // {U,D,R,L,F,B}
		for (int64_t i=1; i<19; i++) {
			// currently makes sure no moves that ought to cancel U U2 and assures U never follows D, etc
			if (depth==0) {
				candidates[(*c)++]=baseSequence+(i<<(6*depth));
			}
			else if (((i-1)/3)!=((((baseSequence&(63UL<<(6*(depth-1))))>>(6*(depth-1)))-1)/3)) {
				if (!((i<((baseSequence&(63UL<<(6*(depth-1))))>>(6*(depth-1))))&(axes[(i-1)/3]==axes[(((baseSequence&(63UL<<(6*(depth-1))))>>(6*(depth-1)))-1)/3]))) {
					candidates[(*c)++]=baseSequence+(i<<(6*depth));
				}
			}
		} 
	}
	else if(movegroup==2) { // {U,R,F,B}
		// all these movegroups can be combined into one single thing, to implement later
		int64_t is[]={1,2,3,7,8,9,13,14,15,16,17,18};
		for (int64_t j=0; j<12; j++) {
			int64_t i=is[j];
			if (depth==0) {
				candidates[(*c)++]=baseSequence+(i<<(6*depth));
			}
			else if (((i-1)/3)!=((((baseSequence&(63UL<<(6*(depth-1))))>>(6*(depth-1)))-1)/3)) {
				if (!((i<((baseSequence&(63UL<<(6*(depth-1))))>>(6*(depth-1))))&(axes[(i-1)/3]==axes[(((baseSequence&(63UL<<(6*(depth-1))))>>(6*(depth-1)))-1)/3]))) {
					candidates[(*c)++]=baseSequence+(i<<(6*depth));
				}
			}
		}
	}
	else if(movegroup==3) { // {U,R}
		// all these movegroups can be combined into one single thing, to implement later
		int64_t is[]={1,2,3,7,8,9};
		for (int64_t j=0; j<6; j++) {
			int64_t i=is[j];
			if (depth==0) {
				candidates[(*c)++]=baseSequence+(i<<(6*depth));
			}
			else if (((i-1)/3)!=((((baseSequence&(63UL<<(6*(depth-1))))>>(6*(depth-1)))-1)/3)) {
				if (!((i<((baseSequence&(63UL<<(6*(depth-1))))>>(6*(depth-1))))&(axes[(i-1)/3]==axes[(((baseSequence&(63UL<<(6*(depth-1))))>>(6*(depth-1)))-1)/3]))) {
					candidates[(*c)++]=baseSequence+(i<<(6*depth));
				}
			}
		}
	}
}

void applySequence(uint64_t sequence, struct CUBE* cube) {
	for (int m=0; m<10; m++) {
		if ((sequence&(63<<(6*m)))>>(6*m)) {
			applyMove(cube,(sequence&(63<<(6*m)))>>(6*m));
		}
		else {
			break;
		}
	}
}

void applyMoves(struct MOVES* moves, struct CUBE* cube) {
	char tmp[999];
	strcpy(tmp,moves->list);	

	char* buffer=strtok(tmp," ");
	while (buffer!=NULL){
		for (int i=0; i<18; i++){
			if (strcmp(buffer,moveList[i])==0){
				applyMove(cube,i+1);
			}
		}
		buffer=strtok(NULL," ");
	}
}

unsigned int hash(char Nbits, struct CUBE* cube) { // fibonnaci hash
	uint64_t mask=0;
	for (int i=0; i<Nbits; i++) {
		mask+=(1<<i); // or just do 2^N-1
	}
	uint64_t key=11400714819323198485ULL*(cube->EPCO+cube->CPEOCN);
	return key>>(64-Nbits);
}

char countMoves(uint64_t *sequence) {
	char out=0;
	for (uint64_t i=0; i<10; i++) {
		if ((63ULL<<(6*i))&(*sequence)) {
			out++;
		}	
		else {
			break;
		}
	}
	return out;
}

ht_t *htCreate(int c) {
	ht_t *hashtable = malloc(sizeof(ht_t)); 
	hashtable->entries=malloc(sizeof(entry_t*)*c);
	for (int i=0; i<c; i++) {
		hashtable->entries[i]=NULL;
	}
	return hashtable;
}

entry_t *htPair(uint64_t *EPCO, uint64_t *CPEOCN, uint64_t *sequence) {
	entry_t *entry=malloc(sizeof(entry_t)*1);
	entry->EPCO=*(uint64_t*)malloc(sizeof(uint64_t)*1);
	entry->CPEOCN=*(uint64_t*)malloc(sizeof(uint64_t)*1);
	entry->sequence=*(int*)malloc(sizeof(uint64_t)*1);

	entry->EPCO=*EPCO;
	entry->CPEOCN=*CPEOCN;
	entry->sequence=*sequence;
	entry->next=NULL;

	return entry;
}

void htInsert(ht_t *hashtable, unsigned int key, uint64_t *EPCO, uint64_t *CPEOCN, uint64_t *sequence, int *collisions) {
	entry_t *entry=hashtable->entries[key];
	if (entry==NULL) {
		hashtable->entries[key]=htPair(EPCO,CPEOCN,sequence);
		return;
	}

	(*collisions)++;

	entry_t *prev;

	while (entry!=NULL) {
	if ((entry->EPCO==*EPCO)&(entry->CPEOCN==*CPEOCN)) { // if exact collision (same exact cube)
			if (countMoves(sequence)>countMoves(&entry->sequence)) { // if its more moves than what's there, break
				return;
			}
		}
		prev=entry;
		entry=prev->next;
	}

	prev->next=htPair(EPCO,CPEOCN,sequence);
}

uint64_t *htRetrieve(ht_t *hashtable, unsigned int key, uint64_t *EPCO, uint64_t *CPEOCN) {

	entry_t *entry=hashtable->entries[key];

	if (entry==NULL) {
		return NULL;
	}
	
	while (entry!=NULL) {
		if ((entry->EPCO==*EPCO)&(entry->CPEOCN==*CPEOCN)) {
			return &entry->sequence;
		}
		entry=entry->next;
	}
	return NULL;
}

long estimateSequenceCount(char movegroup, char depth) {
	const unsigned char movesInGroup[]={18,12,6};
	long out=1;
	long lastLayer=1;
	for (int d=0; d<depth; d++) {
		lastLayer=lastLayer*(movesInGroup[movegroup-1]-(d>1)*3);
		out+=lastLayer;
	}
	return out;
}

ht_t *createTable(char movegroup, char depth, uint64_t EPCOmask, uint64_t CPEOCNmask, struct STEP * step) {

	// generate candidate move sequences
	uint64_t *candidates;
	candidates = (uint64_t*)malloc(estimateSequenceCount(movegroup,depth)*sizeof(uint64_t));
	int c=1;
	int start=0;
	int end=1;
	for (char i=0; i<depth; i++) {
		for (int s=start; s<end; s++) {
			addLayers(movegroup,candidates[s],i,candidates,&c);
		}
		start=end;
		end=c;
	}
		
	candidates=(uint64_t*)realloc(candidates,c*sizeof(uint64_t));
	
	unsigned char tmp=0;
	while (power(2,tmp)<=c) {
		tmp++;
	}
	const unsigned char Nbits=tmp; // THIS CAN BE CHANGED! more memory for less collisions by increasing Nbits
	if (step!=NULL){
		step->Nbits=Nbits;
	}

	ht_t *ht = htCreate(power(2,Nbits));
	
	struct CUBE cube;

	int collisions=0;

	for (int i=0; i<c; i++) {
		revertCube(&cube);
		applyMask(&cube,EPCOmask,CPEOCNmask);
		applySequence(candidates[i],&cube);
		unsigned int key=hash(Nbits,&cube);
		htInsert(ht,key,&cube.EPCO,&cube.CPEOCN,&candidates[i],&collisions);
	//	printf("\n%s",readableSequence(candidates[i]));
	}
	
	printf("\n\t\t%d collisions for %d states in %ld buckets in ",collisions,c,power(2,Nbits));
	free(candidates);

	return ht;
}

void initMethod(struct METHOD* method, char name[], struct STEP* first) {
	strcpy(method->name,name);
	method->first=first;
}

void initStep(struct STEP* step, char name[], char movegroup, char tableDepth, char searchDepth, uint64_t EPCOmask, uint64_t CPEOCNmask, struct STEP* next) {
	long tstart, tend;
	struct timeval timecheck;
	gettimeofday(&timecheck,NULL);
	tstart=(long)timecheck.tv_sec*1000+(long)timecheck.tv_usec/1000;

	strcpy(step->name,name);
	step->movegroup=movegroup;
	step->tableDepth=tableDepth;
	step->searchDepth=searchDepth;
	step->EPCOmask=EPCOmask;
	step->CPEOCNmask=CPEOCNmask;
	step->next=next;
	
	printf("\n\tTabulating %s:",name);
	step->table=createTable(movegroup,tableDepth,EPCOmask,CPEOCNmask,step);
	gettimeofday(&timecheck,NULL);
	tend=(long)timecheck.tv_sec*1000+(long)timecheck.tv_usec/1000;
	printf("%ld ms\n",(tend-tstart));
}

uint64_t reverseSequence(uint64_t sequence) {
	uint64_t out=0;
	int place=0;
	uint64_t temp=0;
	for (int m=10; m>=0; m--) {
		if ((sequence&(63ULL<<(6*m)))>>(6*m)) {
			temp=(sequence&(63ULL<<(6*m)))>>(6*m);
			out|=(3*((temp-1)/3)+4-(((temp-1)%3)+1))<<(6*place++);
		}
	}
	return out;
}

// no need for sol1 and sol2. can do just one combined sol, yes?
// actually we do need to separate 1 and 2 because 64 bits isnt enough for both
void insertSolution(struct solution* solutionsList, uint64_t sol1, uint64_t sol2) {
	if (solutionsList->sol1==0&&solutionsList->sol2==0) {
		solutionsList->sol1=sol1;
		solutionsList->sol2=sol2;
		solutionsList->next=NULL;
		return;
	}

	struct solution* newSol=NULL;
	newSol=(struct solution*)malloc(sizeof(struct solution));
	newSol->sol1=sol1;
	newSol->sol2=sol2;
	newSol->next=NULL;
	struct solution* head=solutionsList;
	
	while(head->next!=NULL)
		head=head->next;

	head->next=newSol;
	return;
}

void reduceSequence(struct solution* solution) {
	
	uint64_t working=solution->sol1;

	bool allgood=0;
	int length=0;
	uint64_t last, this;
	
	while (!allgood) {
		allgood=1;
		working=solution->sol1;
		
		last=working&(63ULL);
		
		for (int m=1; m<=10; m++) { // sol1
			this=(working&(63ULL<<(6*m)))>>(6*m);
			if (!this) {
				working=solution->sol2;
				length=m-1;
				break; // maybe have to add something here to carry on to next section
			}
			else if (this<=18) { // this alg only works for UDRLFB, add variant for the others
				if ((((last-1)/3)==((this-1)/3+1))&&(((last-1)/6)==((this-1)/6)))	{
					solution->sol1=((solution->sol1)&(~(4095ULL<<(6*(m-1)))))|(this<<(6*(m-1)))|(last<<(6*m));
					allgood=0;	
				}
				else if (((this-1)/3)==((last-1)/3)) {
					if (((this-1)%3+(last-1)%3)==2) { // cancellation
						solution->sol1=(solution->sol1&((1<<(6*(m-1)))-1))|((solution->sol1&(((1ULL<<63)-1)<<(6*(m+1))))>>12);
						allgood=0;	
					}
					else { // combine moves
						solution->sol1=(solution->sol1&((1<<(6*(m-1)))-1))|((solution->sol1&(((1ULL<<63)-1)<<(6*(m+1))))>>6)|(((((this-1)%3+1)+((last-1)%3+1))%4+((this-1)/3)*3)<<(6*m));
						allgood=0;	
					}
				}
			}
			last=this;
		}

		// between sol1 and sol2
		this=working&(63ULL);
		if (this<=18) { // this alg only works for UDRLFB, add variant for the others
			if ((((last-1)/3)==((this-1)/3+1))&&(((last-1)/6)==((this-1)/6)))	{
				allgood=0;	
				solution->sol1=((solution->sol1)&(~(63ULL<<(6*length))))|(this<<(6*length));	
				solution->sol2=(((solution->sol2)>>6)<<6)+last;	
			}
			else if (((this-1)/3)==((last-1)/3)) {
				if (((this-1)%3+(last-1)%3)==2) { // cancellation
					allgood=0;	
					solution->sol1=(solution->sol1)&(~(63ULL<<(6*length)));	
					solution->sol2=((solution->sol2)>>6);	
				}
				else { // combine moves
					allgood=0;	
					solution->sol1=((solution->sol1)&(~(63ULL<<(6*length))))|(((((this-1)%3+1)+((last-1)%3+1))%4+((this-1)/3)*3)<<(6*length));
					solution->sol2=((solution->sol2)>>6);	
				}
			}
		}

		last=this;



		for (int m=1; m<=10; m++) { // sol2
			this=(working&(63ULL<<(6*m)))>>(6*m);
			if (!this) {
				break; // maybe have to add something here to carry on to next section
			}
			else if (this<=18) { // this alg only works for UDRLFB, add variant for the others
				if ((((last-1)/3)==((this-1)/3+1))&&(((last-1)/6)==((this-1)/6)))	{
					allgood=0;	
					solution->sol2=((solution->sol2)&(~(4095ULL<<(6*(m-1)))))|(this<<(6*(m-1)))|(last<<(6*m));	
				}
				else if (((this-1)/3)==((last-1)/3)) {
					if (((this-1)%3+(last-1)%3)==2) { // cancellation
						allgood=0;	
						solution->sol2=(solution->sol2&((1<<(6*(m-1)))-1))|((solution->sol2&(((1ULL<<63)-1)<<(6*(m+1))))>>12);
					}
					else { // combine moves
						allgood=0;	
						solution->sol2=(solution->sol2&((1<<(6*(m-1)))-1))|((solution->sol2&(((1ULL<<63)-1)<<(6*(m+1))))>>6)|(((((this-1)%3+1)+((last-1)%3+1))%4+((this-1)/3)*3)<<(6*m));
					}
				}
			}
			last=this;
		}
	}
	return;
}

// add something to clean out the duplicates

void sortSolutions(struct solution* solutionsList, struct solution* output) {
	struct solution* iterator=solutionsList;

	int lengths[22]={0};

	while(iterator!=NULL) { 
		reduceSequence(iterator);
		lengths[countMoves(&iterator->sol1)+countMoves(&iterator->sol2)]++;
		//printf("first %d: %s%s\n",countMoves(&iterator->sol1)+countMoves(&iterator->sol2),readableSequence(iterator->sol1),readableSequence(iterator->sol2));
		iterator=iterator->next;
	}

	char min=22;
	char max=0;

	for (int i=0;i<22;i++){ // find minimum and maximum movecounts in list
		if (lengths[i]&&i<min)
				min=i;
		else if (lengths[i]&&i>max)
				max=i;
	}
	
	// create new list
	struct solution* orderedList=(struct solution*)malloc(sizeof(struct solution));
	iterator=solutionsList;

	// loop thru all sequences in list and pull out all at minimum movecount, and add to new list
	// repeat loop above up to maximum movecount
	for (int i=min;i<=max;i++) {
		iterator=solutionsList;
		while(iterator!=NULL) { 
			if ((countMoves(&iterator->sol1)+countMoves(&iterator->sol2))==i) {
				insertSolution(orderedList,iterator->sol1,iterator->sol2);
			}
			iterator=iterator->next;
		}
	}
	iterator=orderedList;
	while (iterator!=NULL) {
		insertSolution(output,iterator->sol1,iterator->sol2);
		iterator=iterator->next;
	}
}


bool isSame(uint64_t sol1a, uint64_t sol2a, uint64_t sol1b, uint64_t sol2b) {
	uint64_t comparingFrom=sol1a;
	uint64_t comparingTo=sol1b;
	bool switchedA=0;
	bool switchedB=0;
	int mA=0;
	int mB=0;
	while (1) {	
		if ((comparingFrom&(63ULL<<(6*mA)))==0){
			if (switchedA) {
				return 1;
			}
			else {
				switchedA=1;
				comparingFrom=sol2a;
				mA=0;
				continue;
			}
		}
		if ((comparingTo&(63ULL<<(6*mB)))==0) {
			if (switchedB) {
				return 1;
			}
			else {
				switchedB=1;
				comparingTo=sol2b;
				mB=0;
				continue;
			}
		}
		if (((comparingFrom&(63ULL<<(6*mA)))>>(6*mA))!=((comparingTo&(63ULL<<(6*mB)))>>(6*mB))) {
			return 0;
		}
		mA++;
		mB++;
	}
}

int removeDuplicates(struct solution* solutionsList, int quantity) {
	struct solution* iterator=solutionsList;
	struct solution* iterator2=NULL;
	struct solution* previous=NULL;
	int numSolutions=0;
	if (iterator->next==NULL)
			return 0;
	iterator2=iterator->next;
	while(iterator!=NULL) { // go one by one thru list
		iterator2=iterator->next;
		previous=iterator;
		while(iterator2!=NULL) { // check if there are duplicates in the list
			if (isSame(iterator->sol1,iterator->sol2,iterator2->sol1,iterator2->sol2)) {
				previous->next=iterator2->next;
			}
			previous=iterator2;
			iterator2=iterator2->next;
		}
		iterator=iterator->next;
	}

	iterator=solutionsList;
	while (iterator!=NULL) {
		numSolutions++;
		iterator=iterator->next;
	}

	iterator=solutionsList;
	for (int q=0;q<=quantity;q++) {
		iterator=iterator->next;
	}

	return numSolutions;
}


void printSolutions(struct solution* solutionsList) {
	struct solution* iterator=solutionsList;
	while(iterator!=NULL) { 
		printf("%s%s\n",readableSequence(iterator->sol1),readableSequence(iterator->sol2));
		iterator=iterator->next;
	}
	return;
}

void combineSolutionLists(struct solution* solutionsList, struct solution* BigSolutionsList) {
	struct solution* iterator=solutionsList;
	while(iterator!=NULL) { 
		insertSolution(BigSolutionsList, iterator->sol1, iterator->sol2);
		iterator=iterator->next;
	}
	return;
}

void *parallelSearch(void *args) {
	struct parallelArgs *pArgs=(struct parallelArgs*) args;
	struct CUBE cube0;
	revertCube(&cube0);
	uint64_t *out=NULL;	
	struct solution* solutions=NULL;
	solutions=(struct solution*)malloc(sizeof(struct solution));
	int quantity=0;

	for (int i=(pArgs->start); i<(pArgs->end); i++) {
		cube0.EPCO=pArgs->cube->EPCO;
		cube0.CPEOCN=pArgs->cube->CPEOCN;

		applySequence(pArgs->candidates[i], &cube0);
		unsigned int key=hash(pArgs->Nbits,&cube0);
		out=htRetrieve(pArgs->step->table,key,&cube0.EPCO,&cube0.CPEOCN);
		if (out)
		{
			insertSolution(solutions,pArgs->candidates[i],reverseSequence(*out));
			quantity++;
		}
	}
	free(pArgs);
	struct parallelOut* pout=NULL;
	pout=(struct parallelOut*)malloc(sizeof(struct parallelOut));
	pout->solutions=solutions;
	pout->quantity=quantity;
	pthread_exit(pout);
	//pthread_exit(solutions);
}

void solveStep(struct STEP* step, struct CUBE* cube0, int quantity) {
	
	long tstart, tend;
	struct timeval timecheck;
	gettimeofday(&timecheck,NULL);
	tstart=(long)timecheck.tv_sec*1000+(long)timecheck.tv_usec/1000;


	struct CUBE *cubecopy=(struct CUBE*)malloc(sizeof(struct CUBE));
	cubecopy->EPCO=cube0->EPCO;
	cubecopy->CPEOCN=cube0->CPEOCN;

	applyMaskScrambled(cubecopy,step->EPCOmask,step->CPEOCNmask);

	// generate candidate move sequences
	uint64_t *candidates;
	candidates = (uint64_t*)malloc(estimateSequenceCount(step->movegroup,step->searchDepth)*sizeof(uint64_t));
	int c=1;
	int start=0;
	int end=1;
	for (char i=0; i<(step->searchDepth); i++) {
		for (int s=start; s<end; s++) {
			addLayers(step->movegroup,candidates[s],i,candidates,&c);
		}
		start=end;
		end=c;
	}
	
	candidates=(uint64_t*)realloc(candidates,c*sizeof(uint64_t));

	unsigned char tmp=0;
	while (power(2,tmp)<=c) {
				tmp++;
	}
	const unsigned char Nbits=tmp; // THIS CAN BE CHANGED! more memory for less collisions by increasing Nbits
	
	// divide them up into groups for each thread
	
	pthread_t threads[nThreads];
	for (int i=0; i<(nThreads); i++) {
		struct parallelArgs *args=(struct parallelArgs *)malloc(sizeof(struct parallelArgs));
		args->candidates=candidates;
		args->start=i*c/nThreads;
		args->end=(i+1)*c/nThreads;
		args->cube=cubecopy;
		args->step=step;
		args->Nbits=step->Nbits;
		//args->moves=moves;
		pthread_create(&threads[i], NULL, parallelSearch,(void *)args);
		//free(args);
	}
	
	struct solution* allSolutions=NULL;
	allSolutions=(struct solution*)malloc(sizeof(struct solution));
	
	for (int i=0; i<(nThreads); i++) {
		struct parallelOut* pout=NULL;
		pout=(struct parallelOut*)malloc(sizeof(struct parallelOut));
		pthread_join(threads[i],(void**)&pout);
		if (pout->quantity)
			combineSolutionLists(pout->solutions,allSolutions);
	}

	struct solution* resultSolutions=NULL;
	resultSolutions=(struct solution*)malloc(sizeof(struct solution));
	
	sortSolutions(allSolutions,resultSolutions);
	
	int numSolutions=removeDuplicates(resultSolutions,quantity);

	gettimeofday(&timecheck,NULL);
	tend=(long)timecheck.tv_sec*1000+(long)timecheck.tv_usec/1000;
	
	printf("\n\t\t%d unique solutions found & sorted in %ld ms, %d reported.\n",numSolutions,(tend-tstart),quantity);
	
	return;
}

void *parallelAnalyze(void *args) {
	struct parallelAnalyzeArgs *pArgs=(struct parallelAnalyzeArgs*) args;
	struct CUBE cube0;
	struct CUBE cube1;
	uint64_t *out=NULL;	
	struct solution* solutions=NULL;
	solutions=(struct solution*)malloc(sizeof(struct solution));
	// loop over certain portion of the cube states
	for (int i=(pArgs->start); i<(pArgs->end); i++) {
		cube0.EPCO=pArgs->states[i]->cube->EPCO;
		cube0.CPEOCN=pArgs->states[i]->cube->CPEOCN;
		applyMaskScrambled(&cube0,pArgs->step->EPCOmask,pArgs->step->CPEOCNmask);
	//	 loop over candidates looking for solution
		for (int j=0; j<pArgs->nCandidates; j++) {
			cube1.EPCO=cube0.EPCO;
			cube1.CPEOCN=cube0.CPEOCN;
			applySequence(pArgs->candidates[j], &cube1);
			unsigned int key=hash(pArgs->Nbits,&cube1);
			out=htRetrieve(pArgs->step->table,key,&cube1.EPCO,&cube1.CPEOCN);
			if (out) { // solution found
				struct solution* solutions=(struct solution*)calloc(1,sizeof(struct solution));
				insertSolution(solutions,pArgs->candidates[j],reverseSequence(*out)); // can get rid of this and put it in the following 2
				pArgs->states[i]->solution->sol1=solutions->sol1;
				pArgs->states[i]->solution->sol2=solutions->sol2;
				break;
			}
		}
	}
	free(pArgs);
	pthread_exit(NULL);
}

void plotHistogram(struct STEP * step) {
	printf("\n\t%s Histogram:",step->name);
	int max=0;
	int width=50;
	for (int i=0; i<=21; i++) {
		if (step->hist->table[i]>max) {
			max=step->hist->table[i];
		}
	}
	for (int i=0; i<=21; i++) {
		if (step->hist->table[i]) {
			printf("\n\t%d\t: ",i);
			for (int j=0; j<((float)(width*step->hist->table[i]))/max; j++){
				printf("=");
			}
			printf("  (%d)",step->hist->table[i]);
		}
	}
	printf("\n");
}

void analyze(struct METHOD* method, int quantity) {

	long tstart, tend;
	struct timeval timecheck;
	gettimeofday(&timecheck,NULL);
	tstart=(long)timecheck.tv_sec*1000+(long)timecheck.tv_usec/1000;

	printf("\n\tAnalyzing method: %s\n",method->name);
	printf("\n\tSTEP\tPRE\tSEARCH\tPOST");
	printf("\n\t----------------------------");
	printf("\n\tINIT");
	
	//struct state **states=(struct state**)malloc(quantity*sizeof(struct state*)); // init array
	struct state **states=(struct state**)calloc(quantity,sizeof(struct state*)); // init array

	for (int i=0;i<quantity;i++){ // fill array with random states
		states[i]=malloc(sizeof(struct state));
		struct CUBE *cube=(struct CUBE*)malloc(sizeof(struct CUBE));
//		struct solution* solutions=(struct solution*)malloc(sizeof(struct solution));
		struct solution* solutions=(struct solution*)calloc(1,sizeof(struct solution));
		revertCube(cube);
		randomizeCube(cube);
		states[i]->cube=cube;
		states[i]->solution=solutions;
	}
	
	gettimeofday(&timecheck,NULL);
	tend=(long)timecheck.tv_sec*1000+(long)timecheck.tv_usec/1000;
	printf("\t%ld ms",(tend-tstart));
	
	struct STEP* step=method->first;

	// loop over steps
	while (step!=NULL){
		
		printf("\n\t%s",step->name);
	
		step->min=0;
		step->max=0;
		step->mean=0;
		
		struct histogram *hist=(struct histogram*)calloc(1,sizeof(struct histogram));
		step->hist=hist;

		gettimeofday(&timecheck,NULL);
		tstart=(long)timecheck.tv_sec*1000+(long)timecheck.tv_usec/1000;
	
		// generate candidate move sequences
		uint64_t *candidates;
		candidates = (uint64_t*)malloc(estimateSequenceCount(step->movegroup,step->searchDepth)*sizeof(uint64_t));
		int c=1;
		int start=0;
		int end=1;
		for (char i=0; i<(step->searchDepth); i++) {
			for (int s=start; s<end; s++) {
				addLayers(step->movegroup,candidates[s],i,candidates,&c);
			}
			start=end;
			end=c;
		}
	
		candidates=(uint64_t*)realloc(candidates,c*sizeof(uint64_t));
	
		unsigned char tmp=0;
		while (power(2,tmp)<=c) {
			tmp++;
		}
		char Nbits=tmp; // THIS CAN BE CHANGED! more memory for less collisions by increasing Nbits
		
		gettimeofday(&timecheck,NULL);
		tend=(long)timecheck.tv_sec*1000+(long)timecheck.tv_usec/1000;
		printf("\t%ld ms",(tend-tstart));
		tstart=(long)timecheck.tv_sec*1000+(long)timecheck.tv_usec/1000;

		// divide them up into groups for each thread
	
		pthread_t threads[nThreads];
		for (int i=0; i<(nThreads); i++) {
			struct parallelAnalyzeArgs *args=(struct parallelAnalyzeArgs *)malloc(sizeof(struct parallelAnalyzeArgs));
			args->candidates=candidates;
			args->nCandidates=c;
			args->start=i*quantity/nThreads;
			args->end=(i+1)*quantity/nThreads;
			args->states=states;
			args->step=step;
			args->Nbits=step->Nbits;
			pthread_create(&threads[i], NULL, parallelAnalyze,(void *)args);
			//free(args);
		}

		for (int i=0; i<(nThreads); i++) {
			pthread_join(threads[i],NULL);
		}
	
		gettimeofday(&timecheck,NULL);
		tend=(long)timecheck.tv_sec*1000+(long)timecheck.tv_usec/1000;
		printf("\t%ld ms",(tend-tstart));
		tstart=(long)timecheck.tv_sec*1000+(long)timecheck.tv_usec/1000;

		int min=99;
		int max=0;
		int sum=0;
		int movecount=0;

		for (int i=0;i<quantity;i++) {
			struct solution* sol=states[i]->solution;
			while (sol->next!=NULL){
				sol=sol->next;
			}
			applySequence(sol->sol1,states[i]->cube);
			applySequence(sol->sol2,states[i]->cube);
	
		//	printf("\n%s - %s",readableSequence(sol->sol1),readableSequence(sol->sol2));

		//	printCube(states[i]->cube);
		
			movecount=countMoves(&sol->sol1)+countMoves(&sol->sol2);
			if (movecount>max)
					max=movecount;
			if (movecount<min)
					min=movecount;
			sum+=movecount;
			step->hist->table[movecount]=step->hist->table[movecount]+1;
		}

		step->min=min;
		step->max=max;
		step->mean=((float)sum)/quantity;
		
		step=step->next;

		gettimeofday(&timecheck,NULL);
		tend=(long)timecheck.tv_sec*1000+(long)timecheck.tv_usec/1000;
		printf("\t%ld ms",(tend-tstart));
	}

	printf("\n\n\tSTEP\tMIN\tMEAN\tMAX");
	printf("\n\t----------------------------");
	step=method->first;
	// loop over steps to print results
	while (step!=NULL){
		printf("\n\t%s\t%d\t%3.4f\t%d",step->name,step->min,step->mean,step->max);	
		step=step->next;
	}

	printf("\n\n\t%d solutions found.\n",quantity);

	step=method->first;
	// loop over steps to print results
	while (step!=NULL){
		plotHistogram(step);
		step=step->next;
	}
}

int runBenchmark(long quantity, char move) {
	struct CUBE cube;
	revertCube(&cube);
        clock_t start=clock();
	for (int i=0; i<quantity/3; ++i) {
		applyMove(&cube,move);
		applyMove(&cube,move+1);
		applyMove(&cube,move+2);
	}
	clock_t diff=clock()-start;
	int msec=diff*1000/CLOCKS_PER_SEC;
	printCube(&cube);
	return quantity/msec*1000;
}

void benchmarkMoves() {
	printf("\t%d [Ux] moves per second per thread\n",runBenchmark(1e8,1));
	printf("\t%d [Dx] moves per second per thread\n",runBenchmark(1e8,4));
	printf("\t%d [Rx] moves per second per thread\n",runBenchmark(1e8,7));
	printf("\t%d [Lx] moves per second per thread\n",runBenchmark(1e8,10));
	printf("\t%d [Fx] moves per second per thread\n",runBenchmark(1e8,13));
	printf("\t%d [Bx] moves per second per thread\n",runBenchmark(1e8,16));
}

void benchmarkRandomize() {
	clock_t start=clock();
	for (int i=0; i<=1e6; i++) {
		struct CUBE cube;
		revertCube(&cube);
		randomizeCube(&cube);
	}
	clock_t end=clock();
	double time_used=((double) (end-start))/CLOCKS_PER_SEC;
	printf("\n\t1e6 random states generated in %f seconds\n",time_used);
}

void benchmarkHashTable(char movegroup, char depth) {
	clock_t start=clock();
	createTable(movegroup,depth,0xffffffffffffffff,0xffffffffffffffff,NULL);
	clock_t end=clock();
	double time_used=((double) (end-start))/CLOCKS_PER_SEC;
	printf("%f seconds\n",time_used);
}

uint64_t parseNumber(char * word){
	if ((strncmp(word,"0x",2)==0)){
		return (uint64_t) strtol(word+2,NULL,16);
	}	
	else if ((strncmp(word,"0b",2)==0)){
		return (uint64_t) strtol(word+2,NULL,2);
	}	
	else {
		return (uint64_t) strtol(word,NULL,10);
	}	
}

bool parseInputFile(char * file, struct METHOD * method){
	FILE* f;
	if ((f=fopen(file,"r"))){
		char v[1024];
		char w[1024];
		char methodName[1024];
		memcpy(methodName,file,strlen(file)-5);
		methodName[strlen(file)-6]='\0';
		bool firstStep=1;
		bool isAll=0;
		struct STEP *tempStep=(struct STEP*)calloc(1,sizeof(struct STEP));
		struct STEP *stepList=(struct STEP*)calloc(1,sizeof(struct STEP));
		while (fscanf(f,"%1023s",v)==1){
	//		printf("\n%s %s\n",v,w);
			if (strcmp(w,"step")==0){
				memset(tempStep,0,sizeof(struct STEP));
				strcpy(tempStep->name,v);
			}		
			if (strcmp(w,"movegroup")==0){
				tempStep->movegroup=parseNumber(v);
			}		
			if (strcmp(w,"tabledepth")==0){
				tempStep->tableDepth=parseNumber(v);
			}
			if (strcmp(w,"searchdepth")==0){
				tempStep->searchDepth=parseNumber(v);
			}	
			if (strcmp(w,"comask")==0){
				tempStep->EPCOmask=tempStep->EPCOmask|(parseNumber(v));
			}
			if (strcmp(w,"cpmask")==0){
				tempStep->CPEOCNmask=tempStep->CPEOCNmask|(parseNumber(v)<<30);
			}		
			if (strcmp(w,"eomask")==0){
				tempStep->CPEOCNmask=tempStep->CPEOCNmask|(parseNumber(v)<<6);
			}
			if (strcmp(w,"epmask")==0){
				tempStep->EPCOmask|=(parseNumber(v)<<16);
			}		
			if (strcmp(w,"cnmask")==0){
				tempStep->CPEOCNmask=tempStep->CPEOCNmask|(parseNumber(v));
			}		
			if (strcmp(v,"endstep")==0){
				
				struct STEP *step=(struct STEP*)calloc(1,sizeof(struct STEP));
				initStep(step,tempStep->name,tempStep->movegroup,tempStep->tableDepth,tempStep->searchDepth,tempStep->EPCOmask,tempStep->CPEOCNmask,NULL);

				struct STEP *stepIterator=stepList;
				while (stepIterator->listnext!=NULL){
					stepIterator=stepIterator->listnext;
				}
				stepIterator->listnext=step;

			}		
			if (strcmp(v,"all")==0){
				isAll=1;
			}
			else if (isAll){
			
				struct STEP *stepIterator=stepList;
				while (strcmp(stepIterator->name,v)){
					stepIterator=stepIterator->listnext;
				}
			
				if (firstStep){
					initMethod(method,methodName,stepIterator);
					firstStep=0;
				}
				else{
		
					struct STEP *stepIteratorOld=stepList;
					while (strcmp(stepIteratorOld->name,w)){
						stepIteratorOld=stepIteratorOld->listnext;
					}
					stepIteratorOld->next=stepIterator;
				}

				if (strcmp(v,"endall")==0){
					isAll=0;
				}
			}

			strcpy(w,v);
		}		
		fclose(f);
		return 1;
	}
	else {
		return 0;
	}
}
