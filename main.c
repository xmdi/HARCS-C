#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

void binaryOut(uint64_t number) {
	if(number) {
			binaryOut(number>>1);
			putc((number&1)?'1':'0',stdout);
	}
}

void printBinary(uint64_t number) {
	printf("0b");
	binaryOut(number);
	printf("\n");
}

struct CUBE {
	uint64_t EPCO,CPEOCN;
};

void revertCube(struct CUBE* cube) {
	//0011 0111 1011 1111 0010 0110 1010 1110 0001 0101 1001 1101 11 11 11 11 11 11 11 11 
	cube->EPCO=0b0011011110111111001001101010111000010101100111011111111111111111; 
	//1011 0011 0001 1001 1111 0111 0101 1101 11 11 11 11 11 11 11 11 11 11 11 11 001 010
	cube->CPEOCN=0b10110011000110011111011101011101111111111111111111111111001010;
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
	}
}

void benchmark(int quantity) {

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

	printBinary(cube.EPCO);
	printBinary(cube.CPEOCN);

}


int main() {
	
	benchmark(1e9);

	return 0;
}
