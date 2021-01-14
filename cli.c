#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "main.c"

typedef enum {false, true} bool;

bool isMove=0;
//char* moveList[18]={"U","U2","U'","D","D2","D'","R","R2","R'","L","L2","L'","F","F2","F'","B","B2","B'"};

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
		"      88       Y8,   _,dP       `8b,     88       Yb,`Yba,,____    ,88,,____,dP\n"
		"      88       `Y88888P\"         `Y8888888\"        Y8  `\"Y888888888888888888P\"\n\n" 
		"                        Here's A Replacement Cube Solver\n\n\n\n");
}

void info() {
	printf("\n\tHARCS v2.0pre : 01-08-2020 : Matt DiPalma : USA\n\n");
}

void cursor() {
	printf("\n [HARCS]> ");
}

void execute(char* buffer, struct CUBE* basecube, struct METHOD* method, struct MOVES* moves) {

	if (strcmp(buffer,"info")==0)
		info();
	else if (strcmp(buffer,"clear")==0)
		clear();
	else if (strcmp(buffer,"benchmark")==0){
		benchmarkMoves();
		printf("\n          ============================================================\n");
		for (int i=1;i<6;i++){
			printf("\n\tUnmasked UDRLFB hashtable to depth %d:",i);
			benchmarkHashTable(1,i);
		}
	}
	else if (isMove){
		for (int i=0; i<18; i++){
			if (strcmp(buffer,moveList[i])==0){
				applyMove(basecube,i+1);
				char new[999];
				strcpy(new,moves->list);
				strcat(new,moveList[i]);
				strcat(new," ");
				strcpy(moves->list,new);
			}
		}
		if (strcmp(buffer,"#")==0){
			isMove=0;
			printf("\n\tSequence applied.\n");
		}
	}
	else if (strcmp(buffer,"apply")==0)
		isMove=1;
	else if (strcmp(buffer,"revert")==0){
		revertCube(basecube);
		printf("\n\tCube reverted to solved.\n");
	}
	else if (strcmp(buffer,"state")==0)
		printCube(basecube);
	else if (strcmp(buffer,"help")==0)
		printf("\n\tSee accompanying README.\n");
	else if (strcmp(buffer,"exit")==0)
		exit(0);
	else if (strcmp(buffer,"petrus")==0){	
		struct STEP *s3x2x2=(struct STEP*)malloc(sizeof(struct STEP)); // probably need to malloc all this to keep it alive	
		struct STEP *sEO=(struct STEP*)malloc(sizeof(struct STEP)); // probably need to malloc all this to keep it alive	
		struct STEP *sF2L=(struct STEP*)malloc(sizeof(struct STEP)); // probably need to malloc all this to keep it alive	
		initMethod(method,"Petrus",s3x2x2); 
		initStep(s3x2x2,"3x2x2",1,5,5,0x0000f00ff0ff00c3,0x00003c03c030f3ff,sEO);
		initStep(sEO,"EO",1,5,5,0x0000f00ff0ff00c3,0x00003c03ffffffff,sF2L);
		initStep(sF2L,"F2L",1,5,5,0x0000ffffffff00ff,0x00003fffffffffff,NULL);
	}
	else if (strcmp(buffer,"solve")==0){	
		struct STEP *step=method->first;
		while (step!=NULL)
			{
				printf("\n\tSolving %s:\n",step->name);
				solveStep(step,moves,10);

				/*char new[999];
				strcpy(new,moves->list);
				strcat(new,moveList[i]);
				strcat(new," ");
				moves->list=new;*/
	
				step=step->next;
			}
	}
	else
		printf("\n\tCommand not recognized.\n");
}

void getInput(struct CUBE* basecube, struct METHOD* method, struct MOVES* moves) {
	char command[500];
	cursor();
	fgets(command,500,stdin);
	command[strlen(command)-1]='\0';

	char* buffer=strtok(command," ");
	while (buffer!=NULL) {
		execute(buffer,basecube,method,moves);
		buffer=strtok(NULL," ");
	}

}

int main() {

	struct CUBE *basecube=(struct CUBE*)malloc(sizeof(struct CUBE));
	revertCube(basecube);
	struct METHOD *method=(struct METHOD*)malloc(sizeof(struct METHOD));
	struct MOVES *moves=(struct MOVES*)malloc(sizeof(struct MOVES));

	clear();
	splash();

	while (1) {
		getInput(basecube,method,moves);
	}

	return 0;
}
