#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#define __GNUC__
#define _AIM_WINDOWS
#include "api_civ/Api429.h"
#include "api_civ/Ai429Com.h"
#define except(exp) if(print_err(exp))die(fprintf(stderr," %s:%i: %s\n",__FILE__,__LINE__,#exp))
#define COUNT(A) (sizeof(A)/sizeof(A[0]))
AiUInt8 fd = ~0,alive = 1;
char* mask_label[256][21+1];//21bits + no bit message
char mask_file[1024*1024];
char* Usage="USAGE :  \n\tapp.exe RxN SPD LABELS MASK ...\n"
            "Example :\n\tapp.exe Rx1 HS 271 377 m.csv    Rx2 LS *   Rx3 HS *\n";

int load_mask(const char* path){
	FILE*f=fopen(path,"r");
	if(!f)return fprintf(stderr,"unable to open %s\n",path);
	char* file_pos, *token_start = mask_file;
	size_t line=0, col=0, size=fread(mask_file,sizeof(char),sizeof(mask_file)-1,f);
	mask_file[size]=0;
	for(file_pos=mask_file;*file_pos;file_pos++){
		if(*file_pos==';' || *file_pos=='\n'){
			mask_label[(line+1)%COUNT(mask_label)][col%COUNT(mask_label[0])]=token_start;//1st line = 1st label
			token_start = file_pos+1;
		}
		if(*file_pos==';')col++;
		if(*file_pos=='\n')line++,col=0;
		if(*file_pos==';' || *file_pos=='\n' || *file_pos=='\r')
			*file_pos=0;
	}
	return fclose(f);
}
void die(int ret){
	if(fd!=~0)
		Api429Close(fd);
	Api429Exit();
	exit(ret);
}
AiUInt16 print_err(AiInt16 code){
	if(code)
		fprintf(stderr,Api429GetErrorDescription(code));
	return code;
}
void intHandler(int dummy) {
	alive = 0;
}
void printer_mask(AiUInt16 num, AiUInt8 channel,TY_API429_RCV_STACK_ENTRY entry){
	unsigned bit, label_num = entry.ldata&0xFF, data = (entry.ldata>>8)&0x1FFFFF;
	printf("Rx%i:%03o > %i %i%i %06X = ",channel,label_num,(entry.ldata>>31)&1,(entry.ldata>>30)&1,(entry.ldata>>29)&1,data);
	for(bit = 0;bit < 21; bit++)
		if((data&(1<<bit)) && mask_label[label_num][bit])
			printf("| %s",mask_label[label_num][bit]);
	if(!data && mask_label[label_num][21])
		printf("%s",mask_label[label_num][21]);
	printf("\n");
	fflush(stdout);
}
void printer_raw(AiUInt16 num, AiUInt8 channel,TY_API429_RCV_STACK_ENTRY entry){
	printf("%i\t%i\t%08X\t%03o\t%08X\t%i\t%i%i\n",num,channel,entry.ldata,
		(entry.ldata   )&0xFF,
		(entry.ldata>>8)&0x1FFFFF,
		(entry.ldata>>31)&1,
		(entry.ldata>>30)&1,
		(entry.ldata>>29)&1);
	fflush(stdout);
}
int main(int argc,char** argv){
	TY_API429_INI_INFO config = {};
	TY_API429_RESET_CMD reset = {.lcen = 0xffff,.mbuf_size = 0x100,.res_ctl = API429_RESET_ALL};
	TY_API429_RESET_ACK ack = {};
	TY_API429_RCV_TRG_MODE_CMD trg_mode = {.tmod = API429_TRG_ANY};
	TY_API429_RCV_CAP_MODE_CMD cap_mode = {};
	TY_API429_RCV_STACK_ENTRY pool[255];
	
	AiUInt16 pool_pos, pool_len, line_num=0;
	void (*printer)(AiUInt16 num, AiUInt8 chan,TY_API429_RCV_STACK_ENTRY entry)=printer_raw;
	if(argc<=1)return fprintf(stderr,Usage);
	except(Api429Init()<=0);
	except(Api429Open(0, "local", &fd));
	except(Api429CmdIni(fd, &config));
	except(Api429CmdReset(fd, &reset, &ack));
	AiUInt8 arg,i, chan[16], ch_len = 0;
	for(arg=1;arg<argc;arg++){
		if(arg>1 && !ch_len)return fprintf(stderr,Usage);
		if(!memcmp(argv[arg],"Rx",2)){
			chan[ch_len]=atoi(argv[arg]+2);
			except(Api429CmdRmIni(fd, chan[ch_len], API429_MON_MODE_LOC));
			except(Api429CmdRmTrgDef(fd, chan[ch_len], &trg_mode));
			except(Api429CmdRmCapDef(fd, chan[ch_len], &cap_mode));
			except(Api429CmdDefChnSpeed(fd, chan[ch_len],(arg+1<argc)&&(toupper(argv[++arg][0])=='L')?API429_LO_SPEED:API429_HI_SPEED));
			except(Api429CmdRxStart(fd, chan[ch_len]));
			ch_len++;
		}else if(argv[arg][0]=='*'){
			for(i=0;i<255;i++)
				except(Api429CmdRmLabCon (fd, chan[ch_len-1], i, 0, API429_ENA));
		}else if(isdigit(argv[arg][0])){
			except(Api429CmdRmLabCon (fd, chan[ch_len-1], strtol(argv[arg],NULL,8), 0, API429_ENA));
		}else if(isalpha(argv[arg][0])){
			load_mask(argv[arg]);
		}
	}
	signal(SIGINT, intHandler);
	while(alive){
		for(i=0;i<ch_len;i++){
			except(Api429CmdRmDataRead (fd, chan[i], sizeof(pool)/sizeof(pool[0]), &pool_len, pool));
			for(pool_pos = 0; pool_pos < pool_len ; pool_pos++)
				printer(line_num++,chan[i],pool[pool_pos]);
		}
	}
	for(i=0;i<ch_len;i++)
		except(Api429CmdRxHalt(fd, chan[i]));
	die(0);
	return 0;
}