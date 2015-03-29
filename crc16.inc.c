void crcInitTable(unsigned short*table,int nbCell){
	unsigned short i,j;
	for(i=0;i<nbCell;i++)
		for(table[i]=i<<8,j=0;j<8;j++)
			table[i]=(table[i]<<1)^((table[i]&0x8000)?0x1021:0);
}
unsigned short crcHashFile(FILE*f,unsigned short*table){
	unsigned char data,nb_inv=2;
	unsigned short crc=0;
	while(!feof(f) && fread(&data,sizeof(data),1,f)){
		if(nb_inv)data=~data,nb_inv--;
		crc=table[crc>>8] ^ (crc<<8) ^ data;
	}
	return crc;
}
