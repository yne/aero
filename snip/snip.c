#include <stdio.h>
#include <sys/timeb.h>
#include <time.h>

void main(int argc,char**argv){
	char*addr=argc>=2?argv[1]:"10.31.255.255";
	char*port=argc>=3?argv[2]:"1001";
	
	WSADATA wsd;
	WSAStartup(MAKEWORD( 2, 0 ),&wsd)
	int sock=socket(AF_INET,SOCK_DGRAM,0);
	adr.sin_port=htons(atoi(port));
	adr_l.sin_port=htons(atoi(port));
	adr.sin_family=AF_INET;
	adr.sin_addr.s_addr=inet_addr(addr);
	adr_d.sin_addr.s_addr=inet_addr(addr);

	printf("Sending IRQ request\n");
	char* irq_buffer[8]={0,1,'I','D','E','N','T',0};
	sendto(sock,irq_buffer,sizeof(irq_buffer),0,(struct sockaddr*)&adr,sizeof(adr));

	printf("Waiting for IAN request\n");
	char* ian_buffer[100]={};
	int length=sizeof(adr);
	recvfrom(sock,(char*)ian_buffer,sizeof(ian_buffer),0,(struct sockaddr*)&adr,&length);
	
	printf("%s\n",inet_ntoa(adr.sin_addr));
}