#include <signal.h>
#include <stdio.h>
#include "Upload.c"
#include "Download.c"
#include "Information.c"

typedef struct{
	char type[2],//LU
	int pos,//1
	char ip,//192.168.0.10
	int bsize;//blocs size [8..65464], 512
	int ftp_to;//FTP timeout, 30
	int ftp_it;//FTP retry, 1
	int dlp_to;//DLP timeout, 30
	int dlp_it;//DLP retry, 1
	int debug;//Debug level, [0..2], 0
	int tsize;//TFTP tsize negociation 0..1, 0
}Config;

int loadConfig(int argc, char* argv[]){

}
void sigint(int sig){
	exit(0);
}
int usage(char*name){
	printf("%s\nUsage :\n",name);
	UploadUsage(name);
	DownloadUsage(name);
	InformationUsage(name);
	return 0;
}
int main(int argc, char* argv[]){
	signal(SIGINT,sigint);
	if(argc>1){
		Conf conf=loadConfig(argc,argv)
		switch(toupper(argv[1][0])){
			case 'U':return Upload(&conf);
			case 'D':return Download(&conf);
			case 'I':return Information(&conf);
			default :break;
		}
	}
	return usage(argc>0?argv[0]:"ARINC615A");
}
