#include <signal.h>
#include <stdio.h>
#include "UpLoad.c"
#include "DownLoad.c"
#include "Information.c"

void sigint(int sig){
	exit(0);
}
int usage(int argc,char**argv){
	return printf("615ALoader\n\
Usage :\n\
	%s Upload\n\
	%s Download\n\
	%s Information\n",argv[0],argv[0],argv[0]);
}
int main(int argc, char* argv[]){
	signal(SIGINT,sigint);
	if(argc<=1)return usage(argc,argv);
	switch(toupper(argv[1][0])){
		case 'U':return Upload(argc,argv);
		case 'D':return Download(argc,argv);
		case 'I':return Information(argc,argv);
		default :break;
	}
	return usage(argc,argv);
}
