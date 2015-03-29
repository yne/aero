#include <stdio.h>
#include <string.h>
#include "types.h"
#include "lu.inc.c"

#define MAX(a,b) (((a)<(b))?(b):(a))
#define EXT(fname,ext) !memcmp(fname+MAX(strlen(fname)-4,0),ext,sizeof(ext))

//TXR/LUR=0-N lines of LUH description (fname+name)
//TXH/LUH=name+datafiles(fname+name+crc) + userdef

int main(int argc,char**argv){
	FILE*in=stdin,*out=stdout;//TODO
	TXR txr;LUR lur;
	TXH txh;LUH luh;
	
	if(argc<=2){
		return fprintf(stderr,"USAGE:\n"
			"\t%s in.[txt,txr] out.lur\n"
			"\t%s in.[txt,txh] out.luh\n"
			"\t%s in.luh out.[txt/txh]\n"
			"\t%s in.lur out.[txt/txr]\n"
			,argv[0],argv[0],argv[0],argv[0]);
	}else if(!(in=fopen(argv[1],"rb")) || !(out=fopen(argv[2],"wb+"))){
		return fprintf(stderr,"error while opening in/out file\n");
	}else if((EXT(argv[1],".txt")||EXT(argv[1],".txr")) && EXT(argv[2],".lur")){
		int nb=readTXR(in,&lur);
		writeLUR(out,&lur,nb);
	}else if((EXT(argv[1],".txt")||EXT(argv[1],".txh")) && EXT(argv[2],".luh")){
		readTXH(in,&txh);
		writeLUH(out,&luh);
	}else if(EXT(argv[1],".lur") && (EXT(argv[2],".txt")||EXT(argv[2],".txr"))){
		int nb=readLUR(in,&lur);
		writeTXR(out,&txr,nb);
	}else if(EXT(argv[1],".luh") && (EXT(argv[2],".txt")||EXT(argv[2],".txh"))){
		readLUH(in,&luh);
		writeTXH(out,&txh);
	}else{
		return fprintf(stderr,"none of the USAGE in-out file extension matched\n");
	}
}