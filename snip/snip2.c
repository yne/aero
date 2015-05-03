#include <stdio.h>
#include <sys/timeb.h>
#include <time.h>
#include <Winsock2.h>
#include <sys/types.h>
#include <WS2tcpip.h>
#include <sys/timeb.h>
#include <Windows.h>
#include <Mswsock.h>

void main(int argc,char**argv){
	char*addr=argc>=2?argv[1]:"10.31.255.255";
	char*port=argc>=3?argv[2]:"1001";//8000
	
	WSADATA wsd;
	WSAStartup(MAKEWORD(2,0),&wsd);
	//retreive my IP
	char name[50];
	gethostname ( name, sizeof(name));
	PHOSTENT hostinfo = gethostbyname(name);
	char *my_ip = inet_ntoa (*(in_addr *)*hostinfo->h_addr_list);
	
	SOCK sock=socket(AF_INET,SOCK_DGRAM,0);

	SOCKADDR_IN adr={};
	adr.sin_family=AF_INET;
	adr.sin_port=htons(atoi(port));
	adr.sin_addr.s_addr=inet_addr(my_ip);
	bind(sock, (SOCKADDR *)&adr, sizeof(adr))

	SOCKADDR_IN sto={};
	sto.sin_family=AF_INET;
	sto.sin_port=htons(atoi(port));
	sto.sin_addr.s_addr=inet_addr(my_ip);
	char* send_buffer[8]={0,1,'I','D','E','N','T',0};
	sendto(sock,send_buffer,sizeof(send_buffer),0,(SOCKADDR *)&sto,sizeof(sto))

	SOCKADDR_IN sin={};
	struct timeval*tv={tv_sec=3;tv_usec=0};
	fd_set readfs;
	FD_ZERO(&readfs);
	FD_SET(sock, &readfs);
	select(sock+1, &readfs, NULL, NULL, &tv);
	assert(FD_ISSET(sock,&readfs));
	
	printf("Waiting for IAN request\n");
	char* ian_buffer[100]={};
	recvfrom(sock,(char*)ian_buffer,sizeof(ian_buffer),0,(SOCKADDR*)&sin,&(sizeof(sin)));
	
	char remote_IP_address[16];
	strcpy(remote_IP_address,inet_ntoa(adr.sin_addr));

	printf("IAN request received from %s\n",remote_IP_address);
}