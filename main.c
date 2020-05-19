#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
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
		printf(" %x",0xf&(cube->EPCO>>(60-4*i)));
	printf("\n\tEO:");
	for (int i=0; i<12; ++i)
		printf(" %x",0x3&(cube->CPEOCN>>(28-2*i)));
	printf("\n\tCP:");
	for (int i=0; i<8; ++i)
		printf(" %x",0xf&(cube->CPEOCN>>(58-4*i)));
	printf("\n\tCO:");
	for (int i=0; i<8; ++i)
		printf(" %x",0x3&(cube->EPCO>>(14-2*i)));
	printf("\n\tCN:");
	for (int i=0; i<2; ++i)
		printf(" %x",0x7&(cube->CPEOCN>>(3-3*i)));
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
		case 0: // U
			cube->EPCO=(((0xfUL<<48)&cube->EPCO)<<12)|(((0xfffUL<<52)&cube->EPCO)>>4)|(((0x3UL<<8)&cube->EPCO)<<6)|(((0xfcUL<<8)&cube->EPCO)>>2)|(0xffffffff00ff&cube->EPCO);
			cube->CPEOCN=(((0xfUL<<46)&cube->CPEOCN)<<12)|(((0xfffUL<<50)&cube->CPEOCN)>>4)|(((0x3UL<<22)&cube->CPEOCN)<<6)|(((0xfcUL<<22)&cube->CPEOCN)>>2)|(0x3fffc03fffff&cube->CPEOCN);
			break;
		case 1: // U2
			cube->EPCO=(((0xffUL<<48)&cube->EPCO)<<8)|(((0xffUL<<56)&cube->EPCO)>>8)|(((0xfUL<<8)&cube->EPCO)<<4)|(((0xfUL<<12)&cube->EPCO)>>4)|(0xffffffff00ff&cube->EPCO);	
			cube->CPEOCN=(((0xffUL<<46)&cube->CPEOCN)<<8)|(((0xffUL<<54)&cube->CPEOCN)>>8)|(((0xfUL<<22)&cube->CPEOCN)<<4)|(((0xfUL<<26)&cube->CPEOCN)>>4)|(0x3fffc03fffff&cube->CPEOCN);
			break;
		case 2: // U'
			cube->EPCO=(((0xfffUL<<48)&cube->EPCO)<<4)|(((0xfUL<<60)&cube->EPCO)>>12)|(((0x3fUL<<8)&cube->EPCO)<<2)|(((0xcUL<<12)&cube->EPCO)>>6)|(0xffffffff00ff&cube->EPCO);
			cube->CPEOCN=(((0xfffUL<<46)&cube->CPEOCN)<<4)|(((0xfUL<<58)&cube->CPEOCN)>>12)|(((0x3fUL<<22)&cube->CPEOCN)<<2)|(((0x3UL<<28)&cube->CPEOCN)>>6)|(0x3fffc03fffff&cube->CPEOCN);
			break;
		case 3: // D 
			cube->EPCO=(((0xfffUL<<16)&cube->EPCO)<<4)|(((0xfUL<<28)&cube->EPCO)>>12)|(((0x3fUL)&cube->EPCO)<<2)|(((0xcUL<<4)&cube->EPCO)>>6)|(0xffffffff0000ff00&cube->EPCO);
			cube->CPEOCN=(((0xfffUL<<30)&cube->CPEOCN)<<4)|(((0xfUL<<42)&cube->CPEOCN)>>12)|(((0x3fUL<<6)&cube->CPEOCN)<<2)|(((0x3UL<<12)&cube->CPEOCN)>>6)|(0x3fffc0003fffc03f&cube->CPEOCN);
			break;
		case 4: // D2
			cube->EPCO=(((0xffUL<<16)&cube->EPCO)<<8)|(((0xffUL<<24)&cube->EPCO)>>8)|(((0xfUL)&cube->EPCO)<<4)|(((0xfUL<<4)&cube->EPCO)>>4)|(0xffffffff0000ff00&cube->EPCO);	
			cube->CPEOCN=(((0xffUL<<30)&cube->CPEOCN)<<8)|(((0xffUL<<38)&cube->CPEOCN)>>8)|(((0xfUL<<6)&cube->CPEOCN)<<4)|(((0xfUL<<10)&cube->CPEOCN)>>4)|(0x3fffc0003fffc03f&cube->CPEOCN);
			break;
		case 5: // D'
			cube->EPCO=(((0xfUL<<16)&cube->EPCO)<<12)|(((0xfffUL<<20)&cube->EPCO)>>4)|(((0x3UL)&cube->EPCO)<<6)|(((0xfcUL)&cube->EPCO)>>2)|(0xffffffff0000ff00&cube->EPCO);
			cube->CPEOCN=(((0xfUL<<30)&cube->CPEOCN)<<12)|(((0xfffUL<<34)&cube->CPEOCN)>>4)|(((0x3UL<<6)&cube->CPEOCN)<<6)|(((0xfcUL<<6)&cube->CPEOCN)>>2)|(0x3fffc0003fffc03f&cube->CPEOCN);
			break;
		case 6: // R 
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
		case 7: // R2 
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
		case 8: // R' 
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
	
	
	
	//0001 0010 0011 0100 0101 0110 0111 1000 1001 1010 1011 1100 11 11 11 11 11 11 11 11 
	
	
	
	
	
	}
}

void benchmark(long quantity) {

	struct CUBE cube;
	revertCube(&cube);

	clock_t start=clock(), diff;
	for (int i=0; i<quantity/6; ++i) {
		applyMove(&cube,0);
		applyMove(&cube,1);
		applyMove(&cube,2);
		applyMove(&cube,3);
		applyMove(&cube,4);
		applyMove(&cube,5);
	}
	diff=clock()-start;
	int msec=diff*1000/CLOCKS_PER_SEC;
	printf("\t%d [Ux/Dx] moves per second per thread\n",quantity/msec*1000);

	start=clock();
	for (int i=0; i<quantity/3; ++i) {
		applyMove(&cube,6);
		applyMove(&cube,7);
		applyMove(&cube,8);
	}
	diff=clock()-start;
	msec=diff*1000/CLOCKS_PER_SEC;
	printf("\t%d [Rx] moves per second per thread\n",quantity/msec*1000);

	printCube(&cube);

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
	
	benchmark(1e9);

	//testMove(6);

	return 0;
}
