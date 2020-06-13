#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct CUBE {
	uint64_t EPCO,CPEOCN;
};

void binaryOut(uint64_t number) {
	if (number) {
		binaryOut(number>>1);
		putc((number&1)?'1':'0',stdout);
	}
}

void printCube(struct CUBE* cube) {
	printf("\tEP:");
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
				(3&((((((0x3UL<<10)&cube->EPCO)>>10)+1)>>2)+(((0x3UL<<10)&cube->EPCO)>>10)+1))<<12|
				(3&((((((0x3UL<<12)&cube->EPCO)>>12)+2)>>2)+(((0x3UL<<12)&cube->EPCO)>>12)+2))<<4|
				(3&((((((0x3UL<<4)&cube->EPCO)>>4)+1)>>2)+(((0x3UL<<4)&cube->EPCO)>>4)+1))<<2|
				(3&((((((0x3UL<<2)&cube->EPCO)>>2)+2)>>2)+(((0x3UL<<2)&cube->EPCO)>>2)+2))<<10|
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
				(3&((((((0x3UL<<10)&cube->EPCO)>>10)+1)>>2)+(((0x3UL<<10)&cube->EPCO)>>10)+1))<<2|
				(3&((((((0x3UL<<12)&cube->EPCO)>>12)+2)>>2)+(((0x3UL<<12)&cube->EPCO)>>12)+2))<<10|
				(3&((((((0x3UL<<4)&cube->EPCO)>>4)+1)>>2)+(((0x3UL<<4)&cube->EPCO)>>4)+1))<<12|
				(3&((((((0x3UL<<2)&cube->EPCO)>>2)+2)>>2)+(((0x3UL<<2)&cube->EPCO)>>2)+2))<<4|
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
				(3&((((((0x3UL<<14)&cube->EPCO)>>14)+1)>>2)+(((0x3UL<<14)&cube->EPCO)>>14)+1))<<8|
				(3&((((((0x3UL<<8)&cube->EPCO)>>8)+2)>>2)+(((0x3UL<<8)&cube->EPCO)>>8)+2))|
				(3&((((((0x3UL)&cube->EPCO))+1)>>2)+(((0x3UL)&cube->EPCO))+1))<<6|
				(3&((((((0x3UL<<6)&cube->EPCO)>>6)+2)>>2)+(((0x3UL<<6)&cube->EPCO)>>6)+2))<<14|
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
				(3&((((((0x3UL<<14)&cube->EPCO)>>14)+1)>>2)+(((0x3UL<<14)&cube->EPCO)>>14)+1))<<6|
				(3&((((((0x3UL<<8)&cube->EPCO)>>8)+2)>>2)+(((0x3UL<<8)&cube->EPCO)>>8)+2))<<14|
				(3&((((((0x3UL)&cube->EPCO))+1)>>2)+(((0x3UL)&cube->EPCO))+1))<<8|
				(3&((((((0x3UL<<6)&cube->EPCO)>>6)+2)>>2)+(((0x3UL<<6)&cube->EPCO)>>6)+2))|
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
				(3&((((((0x3UL<<8)&cube->EPCO)>>8)+1)>>2)+(((0x3UL<<8)&cube->EPCO)>>8)+1))<<10|
				(3&((((((0x3UL<<10)&cube->EPCO)>>10)+2)>>2)+(((0x3UL<<10)&cube->EPCO)>>10)+2))<<2|
				(3&((((((0x3UL<<2)&cube->EPCO)>>2)+1)>>2)+(((0x3UL<<2)&cube->EPCO)>>2)+1))|
				(3&((((((0x3UL)&cube->EPCO))+2)>>2)+(((0x3UL)&cube->EPCO))+2))<<8|
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
				(3&((((((0x3UL<<8)&cube->EPCO)>>8)+1)>>2)+(((0x3UL<<8)&cube->EPCO)>>8)+1))|
				(3&((((((0x3UL<<10)&cube->EPCO)>>10)+2)>>2)+(((0x3UL<<10)&cube->EPCO)>>10)+2))<<8|
				(3&((((((0x3UL<<2)&cube->EPCO)>>2)+1)>>2)+(((0x3UL<<2)&cube->EPCO)>>2)+1))<<10|
				(3&((((((0x3UL)&cube->EPCO))+2)>>2)+(((0x3UL)&cube->EPCO))+2))<<2|
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
				(3&((((((0x3UL<<14)&cube->EPCO)>>14)+2)>>2)+(((0x3UL<<14)&cube->EPCO)>>14)+2))<<6|
				(3&((((((0x3UL<<6)&cube->EPCO)>>6)+1)>>2)+(((0x3UL<<6)&cube->EPCO)>>6)+1))<<4|
				(3&((((((0x3UL<<4)&cube->EPCO)>>4)+2)>>2)+(((0x3UL<<4)&cube->EPCO)>>4)+2))<<12|
				(3&((((((0x3UL<<12)&cube->EPCO)>>12)+1)>>2)+(((0x3UL<<12)&cube->EPCO)>>12)+1))<<14|
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
				(3&((((((0x3UL<<14)&cube->EPCO)>>14)+2)>>2)+(((0x3UL<<14)&cube->EPCO)>>14)+2))<<12|
				(3&((((((0x3UL<<6)&cube->EPCO)>>6)+1)>>2)+(((0x3UL<<6)&cube->EPCO)>>6)+1))<<14|
				(3&((((((0x3UL<<4)&cube->EPCO)>>4)+2)>>2)+(((0x3UL<<4)&cube->EPCO)>>4)+2))<<6|
				(3&((((((0x3UL<<12)&cube->EPCO)>>12)+1)>>2)+(((0x3UL<<12)&cube->EPCO)>>12)+1))<<4|
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

void append(char *s, char c) {
	int l=strlen(s);
	s[l]=c;
	s[l+1]='\0';
}

char* readableSequence(int sequence) {
	char *moves[18]={"U","U2","U'","D","D2","D'","R","R2","R'","L","L2","L'","F","F2","F'","B","B2","B'"};

	char out[100];
	for (int i=0; i<10; i++) {
		char buffer=((sequence&(63<<(6*i)))>>(6*i));
		if (buffer) {
			append(out,*moves[buffer-1]);
		}
	}
	return out;
}

void addLayers(char movegroup, int baseSequence, char depth, int *candidates, int *c) {
	if (movegroup==1) {
		for (int i=1; i<19; i++) {
			// currently makes sure no moves that ought to cancel U U2
			// add functionality to assure U never follows D, etc
			if ((depth==0)|(((i-1)/3)!=((((baseSequence&(63<<(6*depth-1)))>>(6*depth-1))-1)/3))) {
				candidates[(*c)++]=baseSequence+(i<<(6*depth));
			}
		}
	}
}

void createSequences(char movegroup, char depth) {
	int candidates[1000]={0};
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

	for (int i=0; i<1000; i++) {
		printf("\t%d\t%s\n",i,readableSequence(candidates[i]));
		
		/*binaryOut(candidates[i]);
		printf("\n");*/
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

void benchmark() {

	printf("\t%d [Ux] moves per second per thread\n",runBenchmark(1e8,1));
	
	printf("\t%d [Dx] moves per second per thread\n",runBenchmark(1e8,4));
	
	printf("\t%d [Rx] moves per second per thread\n",runBenchmark(1e8,7));;
	
	printf("\t%d [Lx] moves per second per thread\n",runBenchmark(1e8,10));;
	
	printf("\t%d [Fx] moves per second per thread\n",runBenchmark(1e8,13));;
	
	printf("\t%d [Bx] moves per second per thread\n",runBenchmark(1e8,16));;
	
}

void testMove(char move) {

	struct CUBE cube;
	revertCube(&cube);
	printf("\n BEFORE:\n");
	printCube(&cube);

	applyMove(&cube,move);

	printf("\n AFTER:\n");
	printCube(&cube);

}

int main() {
	
//	benchmark();

//	testMove(16);

	createSequences(1,2);

	return 0;
}
