#include<stdio.h>
#include<stdlib.h>
#include<string.h>

void benchmarkMoves(void);


void clear() {
	for (int i=0; i<100; i++)
			printf("\n");
}

void splash() {
printf(	"  ,ggg,        gg            ,ggg,   ,ggggggggggg,        ,gggg,        ,gg,   \n"
		" dP\"\"Y8b       88           dP\"\"8I  dP\"\"\"88\"\"\"\"\"\"Y8,    ,88\"\"\"Y8b,     i8\"\"8i  \n"
		" Yb, `88       88          dP   88  Yb,  88      `8b   d8\"     `Y8     `8,,8'  \n"
		"  `\"  88       88         dP    88   `\"  88      ,8P  d8'   8b  d8      `88'   \n"
		"      88aaaaaaa88        ,8'    88       88aaaad8P\"  ,8I    \"Y88P'      dP\"8,  \n"
		"      88\"\"\"\"\"\"\"88        d88888888       88\"\"\"\"Yb,   I8'               dP' `8a\n" 
		"      88       88       ,8\"     88       88     \"8b  d8               dP'   `Yb\n"
		"      88       88      ,8P      Y8       88      `8i Y8,            ,dP'     I8\n"
		"      88       Y8,   _,dP       `8b,     88       Yb,`Yba,,_____   ,88,,____,dP\n"
		"      88       `Y88888P\"         `Y8888888\"        Y8  `\"Y888888888888888888P\"\n\n" 
		"                        Here's A Replacement Cube Solver\n\n\n\n\n");
}

void info() {
	printf("\n\t HARCS v1.0 : 01-01-2020 : Matt DiPalma : USA\n\n");
}

void cursor() {
	printf(" [HARCS]> ");
}

void execute(char* buffer) {

	if (strcmp(buffer,"info")==0)
		info();
	else if (strcmp(buffer,"clear")==0)
		clear();
	else if (strcmp(buffer,"benchmark")==0)
		benchmarkMoves();
	else if (strcmp(buffer,"exit")==0)
		exit(0);

}


void getInput() {
	char command[500];
	cursor();
	fgets(command,500,stdin);
	command[strlen(command)-1]='\0';

	char* buffer=strtok(command," ");
	while (buffer!=NULL) {
		execute(buffer);
		buffer=strtok(NULL," ");
	}

}

int main() {

	clear();
	splash();

	while (1) {
		getInput();
	}

	return 0;
}

