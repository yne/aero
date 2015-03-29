#include <stdio.h>
#include "crc16.inc.c"

int main(int argc,char**argv){
	FILE*f;//=stdin;
	unsigned short table[256];
	crcInitTable(table,(sizeof(table)/sizeof(table[0])));

	if(argc>1 && !(f=fopen(argv[1],"rb")))
		return fprintf(stderr,"Unable to open %s\n",argv[1]);
	printf("%04X\n",crcHashFile(f,table));
	fclose(f);
	
	return 0;
}