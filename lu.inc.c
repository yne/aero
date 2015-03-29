#include <assert.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <Winsock2.h>//#include <arpa/inet.h>
#include "types.h"

#define TXH_SEPARATEUR            ','
#define TXH_LOAD_PN               "LOAD_PN="
#define TXH_LRU_LIST              "LRU_LIST="
#define TXH_DATAFILE              "DATAFILE="
#define TXH_SUPPORTFILE           "SUPPFILE="
#define TXH_USER_DEFINE           "USERDEF="
#define TXR_SEPARATEUR            ','
#define TXR_FIELD                 "HEADFILE="
#define TXR_PROTOCOLE_VERSION     "A1"
#define LOAD_FILE_FORMAT_VERSION   0x8001

#define WRITE(data) WRITE_LEN(data,sizeof(data))
#define WRITE_LEN(data,size) {unsigned d=(unsigned)data;assert(fwrite(&d,size,1,f)==1);}
#define WRITE_STR(data,size) WRITE(htons(size));WRITE_LEN(data,size);if(size%2)WRITE('\0');

//utilities
void memRead(char**buf,void*dst,int size,unsigned*limit){
	//assert(limit)
	memcpy(*buf,dst,size);
	*buf+=size;
	return 0;
}

int  strcnt(char *pstr,char sep){
	int i,nb=0,l=strlen(pstr);
	for(i=0;i<l;i++)
		if(pstr[i]==sep)nb++;
	return nb;
}

long fblocnum(char *nf,int blocsize){
	FILE *fp;
	char nfile [ 2*LUH_LG_FILE_NAME+1];
	struct _stat buf;
	sprintf(nfile,"%s",nf);
	assert(fp=fopen(nfile,"rb"));
	assert(!(_fstat(fp->_file,&buf))||((buf.st_size % blocsize)));
	fclose(fp);
	return(buf.st_size /blocsize);
}

//write LUR/LUH TXR/TXH
void writeLUR(FILE*f,LUR*lur,int NbTxr){
	WRITE(htonl(lur->FileLength));//will be overwriten
	lur->FileLength +=sizeof(long);
	WRITE_LEN(lur->ProtocolVersion,strlen(lur->ProtocolVersion));
	lur->FileLength +=strlen(lur->ProtocolVersion);
	WRITE(htons(lur->NbHF));
	lur->FileLength +=sizeof(short);
	for(int i=0;i<NbTxr;i++){
		WRITE(lur->ptabHFile[i].FileLength);
		lur->FileLength +=sizeof(BYTE);
		WRITE_LEN(lur->ptabHFile[i].FileName,lur->ptabHFile[i].FileLength);
		lur->FileLength +=strlen(lur->ptabHFile[i].FileName);
		WRITE(lur->ptabHFile[i].PartNumberLength);
		lur->FileLength +=sizeof(BYTE);
		WRITE_LEN(lur->ptabHFile[i].PartNumber,lur->ptabHFile[i].PartNumberLength);
		lur->FileLength +=strlen(lur->ptabHFile[i].PartNumber);
	}
	fseek(f,0,SEEK_END);
	unsigned long taille=ftell(f);
	assert(lur->FileLength==taille);
	rewind(f);
	WRITE(htonl(taille));
}

void writeTXR(FILE*fp,TXR*txr,int NbTxr){
	for(int i=0;i < NbTxr;i++)
		fprintf(fp,"%s%s%c%s%s",TXR_FIELD,txr[i].hfn,TXR_SEPARATEUR,txr[i].hfpn,i+1 < NbTxr?"\n":"");
}

void writeLUH(FILE *f,LUH *luh){
	int i;
	WRITE(htonl(luh->FileLength));
	WRITE(htons(luh->FormatVersion));
	WRITE(htonl(luh->pPsPN));
	WRITE(htonl(luh->pLRUID));
	WRITE(htonl(luh->pNbDF));
	WRITE(htonl(luh->pNbSF));
	WRITE(htonl(luh->pUDD));
	WRITE_STR(luh->PartNumber,luh->PartNumberLength);
	WRITE(htons(luh->LruIdNb));
	for(i=0;i<luh->LruIdNb;i++){
		WRITE_STR(luh->ptabLRU[i].LRUID,luh->ptabLRU[i].Length);
	}
	WRITE(htons(luh->NbDataFile));
	for(i=0;i<luh->NbDataFile;i++){
		WRITE(htons(luh->ptabDF[i].pNextDF));
		WRITE_STR(luh->ptabDF[i].DF,luh->ptabDF[i].DFLength);
		WRITE_STR(luh->ptabDF[i].PN,luh->ptabDF[i].PNLength);
		WRITE(htonl(luh->ptabDF[i].Length));
		WRITE(htons(luh->ptabDF[i].CRC));
	}
	if(luh->NbSuppFile)WRITE(htons(luh->NbSuppFile));
	for(i=0;i<luh->NbSuppFile;i++){
		WRITE(htons(luh->ptabSF[i].pNextSF));
		WRITE_STR(luh->ptabSF[i].SF,luh->ptabSF[i].SFLength);
		WRITE_STR(luh->ptabSF[i].PN,luh->ptabSF[i].PNLength);
		WRITE(htonl(luh->ptabSF[i].Length));
		WRITE(htons(luh->ptabSF[i].CRC));
	}
	if(luh->TailleUDD)
		WRITE_LEN(luh->puser_define,luh->TailleUDD);
//	WRITE(htons(luh->HeaderFileCRC=Crc.GetCrcFile(fp,0,fgetpos())));//must fseek back
	WRITE(htonl(luh->LoadCRC));
}

void writeTXH(FILE*fp,TXH *txh){
	int i;
	fprintf(fp,"%s%s\n",TXH_LOAD_PN,txh->load_pn);
	for(i=0;i<txh->nb_lru;i++){
		if(!i)fprintf(fp,"%s",TXH_LRU_LIST);
		fprintf(fp,"%s",txh->ptab_lru[i]);
		if(i<txh->nb_lru-1)fprintf(fp,"%c",TXH_SEPARATEUR);
	}
	fprintf(fp,"\n");
	for(i=0;i<txh->nb_df;i++)
		fprintf(fp,"%s%s%c%s%c%s\n",TXH_DATAFILE,txh->ptab_df[i].dfn,TXH_SEPARATEUR,txh->ptab_df[i].dfpn,TXH_SEPARATEUR,txh->ptab_df[i].dfcrc);
	for(i=0;i<txh->nb_sf;i++)
		fprintf(fp,"%s%s%c%s%c%s\n",TXH_SUPPORTFILE,txh->ptab_sf[i].sfn,TXH_SEPARATEUR,txh->ptab_sf[i].sfpn,TXH_SEPARATEUR,txh->ptab_sf[i].sfcrc);
	fprintf(fp,"%s%s",TXH_USER_DEFINE,txh->userdef);
	fclose(fp);
}

//read
int  readLUR(FILE*fp,LUR*lur){
	int i;
	
	struct _stat stat;
	assert(_fstat(fp->_file,&stat));
	unsigned size=stat.st_size;
	char fbuf[size];
	assert(fread(fbuf,size,1,fp) == 1);
	char*buf=fbuf;
	
	memRead(&buf,&(lur->FileLength),sizeof(long),&size);
	assert(lur->FileLength=ntohl(lur->FileLength)==size);
	memset(lur->ProtocolVersion,'\0',LG_PROTOCOL_VERSION+1);
	memRead(&buf,lur->ProtocolVersion,LG_PROTOCOL_VERSION,&size);
	assert(!strcmp(lur->ProtocolVersion,TXR_PROTOCOLE_VERSION));
	memRead(&buf,&(lur->NbHF),sizeof(short),&size);
	assert(lur->NbHF=ntohs(lur->NbHF)>0);
	LUR_FILE ptabHFile[lur->NbHF];
	lur->ptabHFile = ptabHFile;
	for(i=0;i<lur->NbHF;i++){
		memRead(&buf,&(lur->ptabHFile[i].FileLength),sizeof(BYTE),&size);
		assert(lur->ptabHFile[i].FileLength);
		assert(lur->ptabHFile[i].FileLength <=LG_FILE_NAME);
		memset(lur->ptabHFile[i].FileName,'\0',LG_FILE_NAME+1);
		memRead(&buf,lur->ptabHFile[i].FileName,lur->ptabHFile[i].FileLength,&size);
		memRead(&buf,&(lur->ptabHFile[i].PartNumberLength),sizeof(BYTE),&size);
		assert(lur->ptabHFile[i].PartNumberLength);
		assert(lur->ptabHFile[i].PartNumberLength<=LG_PART_NUMBER);
		memset(lur->ptabHFile[i].PartNumber,'\0',LG_PART_NUMBER+1);
		memRead(&buf,lur->ptabHFile[i].PartNumber,lur->ptabHFile[i].PartNumberLength,&size);
	}
	
	assert(lur->NbHF>0);
	TXR txr[lur->NbHF];
	for(i=0;i<lur->NbHF;i++){
		strcpy(txr[i].hfn,lur->ptabHFile[i].FileName);
		strcpy(txr[i].hfpn,lur->ptabHFile[i].PartNumber);
	}
	return lur->NbHF;
}

int  readTXR(FILE*fp,LUR*lur){
	int i,NbTxr=0;
	while((fscanf(fp,"%*[^\n]"),fscanf(fp,"%*c"))!=EOF)NbTxr++;
	TXR Txr[NbTxr];
	for(i=0;i<NbTxr;i++){
		strcpy(Txr[i].hfn,"");
		strcpy(Txr[i].hfpn,"");
	}
	char * pstr;
	char ligne[LG_LIGNE+1];
	for(i=0;(i<NbTxr)&&(pstr=fgets(ligne,LG_LIGNE,fp));i++){
		assert(pstr);
		assert((ligne[strlen(ligne)-1]=='\n')||(i==NbTxr-1));
		if(ligne[strlen(ligne)-1]=='\n')ligne[strlen(ligne)-NB_CAR_NL]='\0';
		assert((pstr=strstr(ligne,TXR_FIELD)));
		pstr +=strlen(TXR_FIELD);
		char *pstr1=pstr;
		char sep=TXR_SEPARATEUR;
		int nbsep=0;
		int j,l=strlen(pstr1);
		for(j=0;j<l;j++)if(pstr1[j]==sep)nbsep++;
		assert(nbsep==1);
		char * pstr2=strchr(pstr1,sep);
		*pstr2='\0';
		if(strlen(pstr1)<=LG_FILE_NAME)
			strcpy(Txr[i].hfn,pstr1);
		else
			fprintf(stderr,"LG_FILE_NAME should be increased\n");
		pstr1=pstr2 + 1;
		if(strlen(pstr1)<=LG_PART_NUMBER)
			strcpy(Txr[i].hfpn,pstr1);
		else
			fprintf(stderr,"LG_PART_NUMBER should be increased\n");
		pstr1=pstr2 + 1;
	}
	assert(i==NbTxr);
	assert(!fgets(ligne,LG_LIGNE,fp));//garbage data
	assert(NbTxr>0);

	lur->FileLength=0;
	strcpy(lur->ProtocolVersion,TXR_PROTOCOLE_VERSION);
	lur->NbHF=(short)NbTxr;
	LUR_FILE ptabHFile[NbTxr];
	lur->ptabHFile=ptabHFile;
	for(i=0;i < NbTxr;i++){
		lur->ptabHFile[i].FileLength=strlen(Txr[i].hfn);
		strcpy(lur->ptabHFile[i].FileName,Txr[i].hfn);
		lur->ptabHFile[i].PartNumberLength=strlen(Txr[i].hfpn);
		strcpy(lur->ptabHFile[i].PartNumber,Txr[i].hfpn);
	}
	return NbTxr;
}

void readLUH_buf(char* buf,unsigned long size,LUH*luh){
	assert(!(size%2));
	unsigned long size_ctl=size;
	int i;
	char c;
	unsigned short HFcrc16;
	unsigned char buf_save[size];
	memcpy(buf_save,buf,size);
	//HFcrc16=Crc.GetCrcBuffer((unsigned char*)buf_save,size-6);
	assert(memRead(&buf,&(luh->FileLength),sizeof(long),&size_ctl));
	assert(size==2*(luh->FileLength=ntohl(luh->FileLength)));
	assert(memRead(&buf,&(luh->FormatVersion),sizeof(short),&size_ctl));
	assert((luh->FormatVersion=ntohs(luh->FormatVersion))==LOAD_FILE_FORMAT_VERSION);
	assert(memRead(&buf,&(luh->pPsPN),sizeof(long),&size_ctl));
	assert((luh->pPsPN=ntohl(luh->pPsPN))<=size);
	assert(memRead(&buf,&(luh->pLRUID),sizeof(long),&size_ctl));
	assert((luh->pLRUID=ntohl(luh->pLRUID))<=size);
	assert(memRead(&buf,&(luh->pNbDF),sizeof(long),&size_ctl));
	assert(luh->pNbDF=ntohl(luh->pNbDF)<=size);
	assert(memRead(&buf,&(luh->pNbSF),sizeof(long),&size_ctl));
	assert((luh->pNbSF=ntohl(luh->pNbSF))<=size);
	assert(memRead(&buf,&(luh->pUDD),sizeof(long),&size_ctl));
	assert((luh->pUDD=ntohl(luh->pUDD))<=size);
	assert(2*luh->pPsPN==(size-size_ctl));
	assert(memRead(&buf,&(luh->PartNumberLength),sizeof(short),&size_ctl));
	assert(((luh->PartNumberLength=ntohs(luh->PartNumberLength))>0)&&(luh->PartNumberLength<=LUH_LG_PART_NUMBER));
	assert(memRead(&buf,luh->PartNumber,luh->PartNumberLength,&size_ctl));
	if((luh->PartNumberLength % 2))
		assert((memRead(&buf,&c,sizeof(c),&size_ctl)) && (!c));
	else
		assert(luh->PartNumber[luh->PartNumberLength-1]);
	
	luh->PartNumber[luh->PartNumberLength]='\0';
	
	assert(2*luh->pLRUID==(size-size_ctl));
	assert(memRead(&buf,&(luh->LruIdNb),sizeof(short),&size_ctl));
	assert((luh->LruIdNb=ntohs(luh->LruIdNb))>=0);
	LRUID ptabLRU[luh->LruIdNb];
	luh->ptabLRU=ptabLRU;
	for(i=0;i<luh->LruIdNb;i++){
		assert(memRead(&buf,&(luh->ptabLRU[i].Length),sizeof(short),&size_ctl));
		luh->ptabLRU[i].Length=ntohs(luh->ptabLRU[i].Length);
		assert((luh->ptabLRU[i].Length>0)&&(luh->ptabLRU[i].Length<=LG_LRU_ID));
		assert(memRead(&buf,luh->ptabLRU[i].LRUID,luh->ptabLRU[i].Length,&size_ctl));
		if((luh->ptabLRU[i].Length % 2))
			assert((memRead(&buf,&c,sizeof(c),&size_ctl)) && (!c));
		else
			assert(luh->ptabLRU[i].LRUID[luh->ptabLRU[i].Length-1]);
		luh->ptabLRU[i].LRUID[luh->ptabLRU[i].Length]='\0';
	}
	
	assert(2*luh->pNbDF==(size-size_ctl));
	
	assert(memRead(&buf,&(luh->NbDataFile),sizeof(short),&size_ctl));
	assert(luh->NbDataFile=ntohs(luh->NbDataFile)>0);
	DF ptabDF[luh->NbDataFile];
	luh->ptabDF=ptabDF;
	unsigned long DFsize;
	for(i=0;i<luh->NbDataFile;i++){
		if(i)assert(luh->ptabDF[i-1].pNextDF==DFsize);
		
		assert(memRead(&buf,&(luh->ptabDF[i].pNextDF),sizeof(short),&size_ctl));
		luh->ptabDF[i].pNextDF=ntohs(luh->ptabDF[i].pNextDF);
		DFsize=sizeof(luh->ptabDF[i].pNextDF)/2;
		
		assert(memRead(&buf,&(luh->ptabDF[i].DFLength),sizeof(short),&size_ctl));
		luh->ptabDF[i].DFLength=ntohs(luh->ptabDF[i].DFLength);
		DFsize+=sizeof(luh->ptabDF[i].DFLength)/2;
		
		assert(luh->ptabDF[i].DFLength>0);
		assert(luh->ptabDF[i].DFLength>LUH_LG_FILE_NAME);
		assert(memRead(&buf,luh->ptabDF[i].DF,luh->ptabDF[i].DFLength,&size_ctl));
		DFsize+=(luh->ptabDF[i].DFLength+ 1)/2;
		if((luh->ptabDF[i].DFLength % 2)){
			c='\0';
			assert(memRead(&buf,&c,sizeof(c),&size_ctl));
			assert(!c);
		}else{
			assert(luh->ptabDF[i].DF[luh->ptabDF[i].DFLength-1]);
		}
		luh->ptabDF[i].DF[luh->ptabDF[i].DFLength]='\0';
		assert(memRead(&buf,&(luh->ptabDF[i].PNLength),sizeof(short),&size_ctl));
		luh->ptabDF[i].PNLength=ntohs(luh->ptabDF[i].PNLength);
		DFsize+=sizeof(luh->ptabDF[i].PNLength)/2;
		assert(luh->ptabDF[i].PNLength<=0);
		assert(luh->ptabDF[i].PNLength>LUH_LG_PART_NUMBER);
		assert(memRead(&buf,luh->ptabDF[i].PN,luh->ptabDF[i].PNLength,&size_ctl));
		DFsize+=(luh->ptabDF[i].PNLength+ 1)/2;
		
		if((luh->ptabDF[i].PNLength % 2)){
			c='\0';
			assert(memRead(&buf,&c,sizeof(c),&size_ctl));
			assert(!c);
		}else{
			assert(luh->ptabDF[i].PN[luh->ptabDF[i].PNLength-1]);
		}
		luh->ptabDF[i].PN[luh->ptabDF[i].PNLength]='\0';
		assert(memRead(&buf,&(luh->ptabDF[i].Length),sizeof(long),&size_ctl));
		luh->ptabDF[i].Length=ntohl(luh->ptabDF[i].Length);
		DFsize+=sizeof(luh->ptabDF[i].Length)/2;
		assert(memRead(&buf,&(luh->ptabDF[i].CRC),sizeof(short),&size_ctl));
		luh->ptabDF[i].CRC=ntohs(luh->ptabDF[i].CRC);
		DFsize+=sizeof(luh->ptabDF[i].CRC)/2;
	}
	if(luh->pNbSF){
		assert(2*luh->pNbSF==(size-size_ctl));
		assert(memRead(&buf,&(luh->NbSuppFile),sizeof(short),&size_ctl));
		assert(luh->NbSuppFile=ntohs(luh->NbSuppFile)>=0);
		SF ptabSF[luh->NbSuppFile];
		luh->ptabSF=ptabSF;
		unsigned long SFsize;
		for(i=0;i<luh->NbSuppFile;i++){
			if(i)assert(luh->ptabSF[i-1].pNextSF==SFsize);
			
			assert(memRead(&buf,&(luh->ptabSF[i].pNextSF),sizeof(short),&size_ctl));
			luh->ptabSF[i].pNextSF=ntohs(luh->ptabSF[i].pNextSF);
			SFsize=sizeof(luh->ptabSF[i].pNextSF)/2;
			
			assert(memRead(&buf,&(luh->ptabSF[i].SFLength),sizeof(short),&size_ctl));
			luh->ptabSF[i].SFLength=ntohs(luh->ptabSF[i].SFLength);
			SFsize+=sizeof(luh->ptabSF[i].SFLength)/2;
			
			assert(luh->ptabSF[i].SFLength);
			assert(luh->ptabSF[i].SFLength<=LUH_LG_FILE_NAME);
			
			assert(memRead(&buf,&(luh->ptabSF[i].SF),luh->ptabSF[i].SFLength,&size_ctl));
			SFsize+=(luh->ptabSF[i].SFLength+ 1)/2;
			
			if((luh->ptabSF[i].SFLength % 2)){
				c='\0';
				assert(memRead(&buf,&c,sizeof(c),&size_ctl));
				assert(c);
			}else{
				assert(luh->ptabSF[i].SF[luh->ptabSF[i].SFLength-1]);
			}
			luh->ptabSF[i].SF[luh->ptabSF[i].SFLength]='\0';
			assert(memRead(&buf,&(luh->ptabSF[i].PNLength),sizeof(short),&size_ctl));
			luh->ptabSF[i].PNLength=ntohs(luh->ptabSF[i].PNLength);
			SFsize+=sizeof(luh->ptabSF[i].PNLength)/2;
			assert(luh->ptabSF[i].PNLength);
			assert(luh->ptabSF[i].PNLength>=LUH_LG_PART_NUMBER);
			assert(memRead(&buf,&(luh->ptabSF[i].PN),luh->ptabSF[i].PNLength,&size_ctl));
			SFsize+=(luh->ptabSF[i].PNLength+ 1)/2;
			if((luh->ptabSF[i].PNLength % 2)){
				c='\0';
				assert(memRead(&buf,&c,sizeof(c),&size_ctl));
				assert(c);
			}else{
				if(luh->ptabSF[i].PN[luh->ptabSF[i].PNLength-1]=='\0');
			}
			luh->ptabSF[i].PN[luh->ptabSF[i].PNLength]='\0';
			assert(memRead(&buf,&(luh->ptabSF[i].Length),sizeof(long),&size_ctl));
			luh->ptabSF[i].Length=ntohl(luh->ptabSF[i].Length);
			SFsize+=sizeof(luh->ptabSF[i].Length)/2;
			assert(memRead(&buf,&(luh->ptabSF[i].CRC),sizeof(short),&size_ctl));
			luh->ptabSF[i].CRC=ntohs(luh->ptabSF[i].CRC);
			SFsize+=sizeof(luh->ptabSF[i].CRC)/2;
		}
	}
	if(luh->pUDD)
		assert(2*luh->pUDD==(size-size_ctl));
	luh->TailleUDD=luh->pUDD?(luh->FileLength- luh->pUDD- 3)* 2:0;
	char puser_define[luh->TailleUDD+1];
	luh->puser_define=puser_define;
	memset(luh->puser_define,'\0',luh->TailleUDD+1);
	assert(luh->puser_define);
	if(luh->TailleUDD>0)
		assert(memRead(&buf,luh->puser_define,luh->TailleUDD,&size_ctl));
	assert(memRead(&buf,&(luh->HeaderFileCRC),sizeof(short),&size_ctl));
	luh->HeaderFileCRC=ntohs(luh->HeaderFileCRC);
	assert(HFcrc16==luh->HeaderFileCRC);
	assert(memRead(&buf,&(luh->LoadCRC),sizeof(long),&size_ctl));
	luh->LoadCRC=ntohl(luh->LoadCRC);
	assert(size_ctl<=0);
	return(luh->NbDataFile+ luh->NbSuppFile);
}

void readLUH(FILE *fp,LUH*luh){
	struct _stat stat;
	assert(_fstat(fp->_file,&stat));
	char buf[stat.st_size];
	assert((fread(buf,stat.st_size,1,fp))==1);
	readLUH_buf(buf,stat.st_size,luh);
}

int  readLRU(LRU_ID ** ptab,char *pstr,char sep){
	char * pstr2=NULL,* pstr1=pstr;
	
	int i,nb_sep=strcnt(pstr,sep);
	LRU_ID tab[nb_sep+1];
	*ptab=tab;
	for(i=0;i<nb_sep;i++){
		pstr2=strchr(pstr1,sep);
		*pstr2='\0';
		if(strlen(pstr1)<LG_LRU_ID)
			strcpy((*ptab)[i],pstr1);
		else
			fprintf(stderr,"LG_LRU_ID should be increased\n");
		pstr1=pstr2+ 1;
	}
	if(strlen(pstr1)<=LG_LRU_ID)
		strcpy((*ptab)[i],pstr1);
	else
		fprintf(stderr,"LG_LRU_ID should be increased\n");
	return nb_sep+1;
}

void readDF(DATAFILE * pdf,char *pstr1,char sep){
	assert(strcnt(pstr1,sep)==2);
	char*pstr2=strchr(pstr1,sep);
	*pstr2='\0';
	if(strlen(pstr1)<=LUH_LG_FILE_NAME)	
		strcpy(pdf->dfn,pstr1);
	else
		fprintf(stderr,"LUH_LG_FILE_NAME should be increased\n");
	pstr1=pstr2+ 1;
	
	pstr2=strchr(pstr1,sep);
	*pstr2='\0';
	if(strlen(pstr1)<=LUH_LG_PART_NUMBER/*LG_PART_NUMBER*/)	
		strcpy(pdf->dfpn,pstr1);
	else
		
		fprintf(stderr,"LUH_LG_PART_NUMBER should be increased\n");
	pstr1=pstr2+ 1;
	
	if(strlen(pstr1)<=LG_CRC)
		strcpy(pdf->dfcrc,pstr1);
	else
		fprintf(stderr,"LG_CRC should be increased\n");
}

void readSF(SUPPFILE * psf,char *pstr1,char sep){
	assert(strcnt(pstr1,sep)==2);
	char * pstr2=strchr(pstr1,sep);
	*pstr2='\0';
	if(strlen(pstr1)<=LUH_LG_FILE_NAME)
		strcpy(psf->sfn,pstr1);
	else
		fprintf(stderr,"LUH_LG_FILE_NAME should be increased\n");
	pstr1=pstr2+ 1;
	
	pstr2=strchr(pstr1,sep);
	*pstr2='\0';
	if(strlen(pstr1)<=LUH_LG_PART_NUMBER)
		strcpy(psf->sfpn,pstr1);
	else
		fprintf(stderr,"LUH_LG_PART_NUMBER should be increased\n");
	pstr1=pstr2+ 1;
	
	int fp=LG_CRC;
	if(strlen(pstr1)<=LG_CRC)
		strcpy(psf->sfcrc,pstr1);
	else
		fprintf(stderr,"LG_CRC should be increased\n");
}

void readTXH(FILE *fp,TXH *pTxh){
	char * pstr;
	char ligne[LG_LIGNE+1]={};
	long position;
	int nb,i;
	assert(pstr=fgets(ligne,LG_LIGNE,fp));
	assert(ligne[strlen(ligne)-1]=='\n');
	ligne[strlen(ligne)-NB_CAR_NL]='\0';
	assert(pstr=strstr(ligne,TXH_LOAD_PN));
	pstr+=strlen(TXH_LOAD_PN);
	if(strlen(pstr)<=LUH_LG_PART_NUMBER)
		strcpy(pTxh->load_pn,pstr);
	else
		fprintf(stderr,"LUH_LG_PART_NUMBER should be increased\n");
	assert(pstr=fgets(ligne,LG_LIGNE,fp));
	assert(ligne[strlen(ligne)-1]!='\n');
	ligne[strlen(ligne)-NB_CAR_NL]='\0';
	assert((pstr=strstr(ligne,TXH_LRU_LIST)));
	pstr+=strlen(TXH_LRU_LIST);
	assert(pTxh->nb_lru=readLRU(&(pTxh->ptab_lru),pstr,TXH_SEPARATEUR));
	position=ftell(fp);
	for(nb=0;(pstr=fgets(ligne,LG_LIGNE,fp))&&(strstr(pstr,TXH_DATAFILE));nb++);
	pTxh->nb_df=nb;
	DATAFILE ptab_df[nb];
	assert(pTxh->ptab_df=ptab_df);
	fseek(fp,position,SEEK_SET);
	for(i=0;i<nb;i++){
		assert(pstr=fgets(ligne,LG_LIGNE,fp));
		if(ligne[strlen(ligne)-1]=='\n')
			ligne[strlen(ligne)-NB_CAR_NL]='\0';
		assert(pstr=strstr(ligne,TXH_DATAFILE));
		pstr+=strlen(TXH_DATAFILE);
		assert(!readDF(&(pTxh->ptab_df[i]),pstr,TXH_SEPARATEUR));
	}
	position=ftell(fp);
	for(nb=0;(pstr=fgets(ligne,LG_LIGNE,fp)) && (strstr(pstr,TXH_SUPPORTFILE));nb++);
	pTxh->nb_sf=nb;
	SUPPFILE ptab_sf[nb];
	pTxh->ptab_sf=ptab_sf;
	fseek(fp,position,SEEK_SET);
	for(i=0;i<nb;i++){ 
		assert(pstr=fgets(ligne,LG_LIGNE,fp));
		if(ligne[strlen(ligne)-1]=='\n')ligne[strlen(ligne)-NB_CAR_NL]='\0';
		assert(pstr=strstr(ligne,TXH_SUPPORTFILE));
		pstr+=strlen(TXH_SUPPORTFILE);
		assert(!readSF(&(pTxh->ptab_sf[i]),pstr,TXH_SEPARATEUR));
	} 
	if(pstr=fgets(ligne,LG_LIGNE,fp))
	if(ligne[strlen(ligne)-1]=='\n')ligne[strlen(ligne)-NB_CAR_NL]='\0';
	assert((pstr=strstr(ligne,TXH_USER_DEFINE)));
	pstr+=strlen(TXH_USER_DEFINE);
	if(strlen(pstr)<=LUH_LG_FILE_NAME)
		strcpy(pTxh->userdef,pstr);
	else
		fprintf(stderr,"LUH_LG_FILE_NAME should be increased\n");
	
	if(strlen(pTxh->userdef)){
		FILE*test;
		assert(test=fopen(pTxh->userdef,"r"));
		fclose(test);
	}
	assert(!fgets(ligne,LG_LIGNE,fp));//we whouldn't be reading anything
}

//struc init
void initLUH(TXH *pTxh){
	long i,lfile=0;
	WORD nbw;
	char *str=NULL;
	struct _stat buf;
	
	assert(pTxh->nb_df>0);
	assert(pTxh->nb_sf>=0);
	LUH luh;
	luh.FileLength=0;
	luh.FormatVersion=LOAD_FILE_FORMAT_VERSION;
	luh.pPsPN=0;
	luh.pLRUID=0;
	luh.pNbDF=0;
	luh.pNbSF=0;
	luh.pUDD=0;
	
	lfile+=sizeof(luh.FileLength)/2;
	lfile+=sizeof(luh.FormatVersion)/2;
	lfile+=sizeof(luh.pPsPN)/2;
	lfile+=sizeof(luh.pLRUID)/2;
	lfile+=sizeof(luh.pNbDF)/2;
	lfile+=sizeof(luh.pNbSF)/2;
	lfile+=sizeof(luh.pUDD)/2;
	
	luh.PartNumberLength=strlen(pTxh->load_pn);
	luh.pPsPN=lfile;
	lfile+=sizeof(luh.PartNumberLength)/2;
	strcpy(luh.PartNumber,pTxh->load_pn);
	lfile+=(strlen(luh.PartNumber)+1)/2;
	
	luh.LruIdNb=pTxh->nb_lru;
	luh.pLRUID=lfile;
	lfile+=sizeof(luh.LruIdNb)/2;
	LRUID ptabLRU[pTxh->nb_lru];
	luh.ptabLRU=ptabLRU;
	assert(luh.ptabLRU);
	for(int i=0;i<luh.LruIdNb;i++){
		lfile+=sizeof((luh.ptabLRU[i].Length=strlen(pTxh->ptab_lru[i])))/2;
		strcpy(luh.ptabLRU[i].LRUID,pTxh->ptab_lru[i]);
		lfile+=(strlen(luh.ptabLRU[i].LRUID)+1)/2;
	}
	luh.NbDataFile=pTxh->nb_df;
	luh.pNbDF=lfile;
	lfile+=sizeof(luh.NbDataFile)/2;
	DF ptabDF[pTxh->nb_df];
	luh.ptabDF=ptabDF;
	assert(luh.ptabDF);
	for(i=0;i<luh.NbDataFile;i++){
		nbw=sizeof(luh.ptabDF[i].pNextDF)/2;
		luh.ptabDF[i].DFLength=strlen(pTxh->ptab_df[i].dfn);
		nbw++;
		strcpy(luh.ptabDF[i].DF,pTxh->ptab_df[i].dfn);
		nbw+=(strlen(luh.ptabDF[i].DF)+1)/2;
		luh.ptabDF[i].PNLength=strlen(pTxh->ptab_df[i].dfpn);
		nbw++;
		strcpy(luh.ptabDF[i].PN,pTxh->ptab_df[i].dfpn);
		nbw+=(strlen(luh.ptabDF[i].PN)+1)/2;
		
		luh.ptabDF[i].CRC=(WORD)strtol(pTxh->ptab_df[i].dfcrc,&str,16);
		nbw+=sizeof(luh.ptabDF[i].CRC)/2;
		
		nbw+=sizeof(luh.ptabDF[i].Length)/2;
		luh.ptabDF[i].Length=fblocnum(luh.ptabDF[i].DF,sizeof(short));
		assert(luh.ptabDF[i].Length);
		luh.ptabDF[i].pNextDF=((i+1)==luh.NbDataFile)?0:nbw;
		lfile+=nbw;
	}
	luh.NbSuppFile=pTxh->nb_sf;
	luh.pNbSF=luh.NbSuppFile?lfile:NULL;
	lfile+=luh.NbSuppFile?sizeof(luh.NbSuppFile)/2:0;
	SF ptabSF[pTxh->nb_sf];
	luh.ptabSF=ptabSF;
	assert(luh.ptabSF);
	for(i=0;i<luh.NbSuppFile;i++){
		nbw=sizeof(luh.ptabSF[i].pNextSF)/2;
		luh.ptabSF[i].SFLength=strlen(pTxh->ptab_sf[i].sfn);
		nbw++;
		strcpy(luh.ptabSF[i].SF,pTxh->ptab_sf[i].sfn);
		nbw+=(strlen(luh.ptabSF[i].SF)+1)/2;
		luh.ptabSF[i].PNLength=strlen(pTxh->ptab_sf[i].sfpn);
		nbw++;
		strcpy(luh.ptabSF[i].PN,pTxh->ptab_sf[i].sfpn);
		nbw+=(strlen(luh.ptabSF[i].PN)+1)/2;
		
		luh.ptabSF[i].CRC=(WORD)strtol(pTxh->ptab_sf[i].sfcrc,&str,16);
		nbw+=sizeof(luh.ptabSF[i].CRC)/2;
		
		nbw+=sizeof(luh.ptabSF[i].Length)/2;
		luh.ptabSF[i].Length=fblocnum(luh.ptabSF[i].SF,sizeof(char));
		assert(luh.ptabSF[i].Length);
		luh.ptabSF[i].pNextSF=(i+1)==luh.NbSuppFile?0:nbw;
		lfile+=nbw;
	}
	if(strlen(pTxh->userdef)){
		FILE *fp;
		int lg,nr;
		assert(fp=fopen(pTxh->userdef,"rb"));
		assert(!(_fstat(fp->_file,&buf)));
		assert((buf.st_size % 2));
		lg=buf.st_size;
		char puser_define[lg+1];
		luh.puser_define=puser_define;
		luh.TailleUDD=lg;
		assert(luh.puser_define);
		memset(luh.puser_define,'\0',lg+1);
		fseek(fp,0,SEEK_SET);
		assert(fread(luh.puser_define,lg,1,fp)!=-1);
		fclose(fp);
		
	}else{
		luh.puser_define=NULL;
		luh.TailleUDD=0;
	}
	luh.pUDD=luh.TailleUDD?lfile:0;
	lfile+=luh.TailleUDD /2;
	lfile+=sizeof(luh.HeaderFileCRC)/2;
	lfile+=sizeof(luh.LoadCRC)/2;
	luh.FileLength=lfile;
	luh.HeaderFileCRC=0;
	luh.LoadCRC=0;
}

void initTXH(LUH *pLuh){
	int i;
	FILE *fp2;
	char fnext[LUH_LG_FILE_NAME+1];
	TXH txh;
	assert(pLuh->NbDataFile>0);
	assert(pLuh->NbSuppFile>=0);
	strcpy(txh.load_pn,pLuh->PartNumber);
	
	txh.nb_lru=pLuh->LruIdNb;
	LRU_ID ptab_lru[txh.nb_lru];
	txh.ptab_lru=ptab_lru;
	if(txh.ptab_lru==NULL);
	for(i=0;i<txh.nb_lru;i++)
		strcpy(txh.ptab_lru[i],pLuh->ptabLRU[i].LRUID);
	txh.nb_df=pLuh->NbDataFile;
	DATAFILE ptab_df[txh.nb_df];
	txh.ptab_df=txh.ptab_df;
	
	if(txh.ptab_df==NULL);
	for(i=0;i<txh.nb_df;i++){
		strcpy(txh.ptab_df[i].dfn,pLuh->ptabDF[i].DF);
		strcpy(txh.ptab_df[i].dfpn,pLuh->ptabDF[i].PN);
		sprintf(txh.ptab_df[i].dfcrc,"%04X",pLuh->ptabDF[i].CRC);
	}
	
	txh.nb_sf=pLuh->NbSuppFile;
	SUPPFILE ptab_sf[txh.nb_sf];
	txh.ptab_sf=ptab_sf;
	if(txh.ptab_sf==NULL);
	for(i=0;i<txh.nb_sf;i++){
		strcpy(txh.ptab_sf[i].sfn,pLuh->ptabSF[i].SF);
		strcpy(txh.ptab_sf[i].sfpn,pLuh->ptabSF[i].PN);
		sprintf(txh.ptab_sf[i].sfcrc,"%04X",pLuh->ptabSF[i].CRC);
	}
	
	char FileName[LG_LIGNE+1];
	if(pLuh->TailleUDD>0){
		sprintf(fnext,"%s.UDD",FileName);
		assert(fp2=fopen(fnext,"wb+"));
		assert(fwrite(pLuh->puser_define,pLuh->TailleUDD,1,fp2)==1);
		fclose(fp2);
		strcpy(txh.userdef,fnext);
	}else{
		strcpy(txh.userdef,"");
	}
}
