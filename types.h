#include <stdint.h>

#define NB_CAR_NL                   2// \r\n
#define LG_LIGNE                  512
#define LG_FILE_NAME              254
#define LUH_LG_FILE_NAME        65535
#define LG_HEADER_FILE            255
#define LG_PART_NUMBER            255
#define LUH_LG_PART_NUMBER      65535
#define LG_STATUS_DESCRIPTION     255
#define LG_PN_DESCRIPTION         255
#define LG_LOAD_RATIO               3
#define LG_USER_DATA              255
#define LG_FILE_NAME_LENGTH         3
#define LG_PROTOCOL_VERSION         2
#define LG_LRU_ID               65535
#define LG_CRC                      4

#define ACCESS_MODE_READ            4

// Constantes pour ecriture binaire
#define PROTOCOLE_VERSION	"A1"

typedef struct {
	long			FileLength;
	char 			ProtocolVersion[LG_PROTOCOL_VERSION+1];
	uint16_t			OpStatusCode;
	uint8_t			StatusLength;
	char			StatusDescription[LG_STATUS_DESCRIPTION+1];
} LCI_LUI;

typedef struct {
	char	PNLength;
	char	PN[LG_PART_NUMBER+1];
	char	DescLength;
	char	Desc[LG_PN_DESCRIPTION+1];
} PN;
typedef struct {
	long			FileLength;
	char 			ProtocolVersion[LG_PROTOCOL_VERSION+1];
	unsigned short	NbPN;
	PN		*ptabPN;
} LCL;

typedef struct {
	long			FileLength;
	char 			ProtocolVersion[LG_PROTOCOL_VERSION+1];
	uint16_t			Counter;
	uint16_t			OpStatusCode;
	uint16_t			ExceptionTimer;
	uint16_t			EstimatedTime;
	uint8_t			StatusLength;
	char			StatusDescription[LG_STATUS_DESCRIPTION+1];
} LCS;

typedef struct {
	char	HFLength;
	char	HF[LG_HEADER_FILE+1];
	char	PNLength;
	char	PN[LG_PART_NUMBER+1];
	char	LoadRatio[LG_LOAD_RATIO+1];
	uint16_t	LoadStatus;
	char	DescLength;
	char	Desc[LG_PN_DESCRIPTION+1];
} HF;
typedef struct {
	long			FileLength;
	char 			ProtocolVersion[LG_PROTOCOL_VERSION+1];
	uint16_t			OpStatusCode;
	uint8_t			StatusLength;
	char			StatusDescription[LG_STATUS_DESCRIPTION+1];
	uint16_t			Counter;
	uint16_t			EstimatedTime;
	char			LoadListRatio[LG_LOAD_RATIO+1];
	unsigned short	NbHF;
	HF		*ptabHF;
} LUS;

typedef struct {
	char			FileNameLength;
	char			FileName[LG_FILE_NAME];
	uint16_t			FileStatus;
	uint8_t			StatusLength;
	char			StatusDescription[LG_STATUS_DESCRIPTION+1];
}FILE_LNS;
typedef struct {
	long			FileLength;
	char 			ProtocolVersion[LG_PROTOCOL_VERSION+1];
	uint16_t			OpStatusCode;
	uint8_t			StatusLength;
	char			StatusDescription[LG_STATUS_DESCRIPTION+1];
	uint16_t			Counter;
	uint16_t			EstimatedTime;
	char			LoadListRatio[LG_LOAD_RATIO+1];
	uint16_t			NbFiles;
	FILE_LNS	*ptabF_LNS;
}LNS;

typedef struct {
	char			FileNameLength;
	char			FileName[LG_FILE_NAME+1];
}FN;
typedef struct {
	long			FileLength;
	char			ProtocolVersion[LG_PROTOCOL_VERSION+1];
	uint16_t			NbFiles;
	uint8_t			UserDataLength;
	char			UserData[LG_USER_DATA+1];
	FN		*ptabFN;
}LNR;

typedef struct {
	long			FileLength;
	char			ProtocolVersion[LG_PROTOCOL_VERSION+1];
	uint16_t			OpStatusCode;
	uint8_t			StatusLength;
	char			StatusDescription[LG_STATUS_DESCRIPTION+1];
}LND;

typedef struct{
	char hfn[LG_FILE_NAME+1];
	char hfpn[LG_PART_NUMBER+1];
}TXR;

typedef struct{
	uint8_t  FileLength;
	char     FileName[LG_FILE_NAME+1];
	uint8_t  PartNumberLength;
	char     PartNumber[LG_PART_NUMBER+1];
}LUR_FILE;
typedef struct{
	uint32_t FileLength;
	char     ProtocolVersion[LG_PROTOCOL_VERSION+1];
	uint16_t NbHF;
	LUR_FILE*ptabHFile;
}LUR;

typedef char LRU_ID[LG_LRU_ID+1]; 
typedef struct{
	char     dfn[LUH_LG_FILE_NAME+1];
	char     dfpn[LG_PART_NUMBER+1];
	char     dfcrc[LG_CRC+1];
}DATAFILE;
typedef struct{
	char     sfn[LUH_LG_FILE_NAME+1];
	char     sfpn[LG_PART_NUMBER+1];
	char     sfcrc[LG_CRC+1];
}SUPPFILE;
typedef struct{
	char     load_pn[LG_PART_NUMBER];
	LRU_ID  *ptab_lru;int nb_lru;
	DATAFILE*ptab_df ;int nb_df;
	SUPPFILE*ptab_sf ;int nb_sf;
	char     userdef[LUH_LG_FILE_NAME+1];
}TXH;

typedef struct{
	uint16_t Length;
	char     LRUID[LG_LRU_ID+1];
}LRUID;
typedef struct{
	uint16_t pNextDF;
	uint16_t DFLength;
	char     DF[LUH_LG_FILE_NAME];
	uint16_t PNLength;
	char     PN[LG_PART_NUMBER];
	int32_t  Length;
	uint16_t CRC;
}DF;
typedef struct{
	uint16_t pNextSF;
	uint16_t SFLength;
	char     SF[LUH_LG_FILE_NAME];
	uint16_t PNLength;
	char     PN[LG_PART_NUMBER];
	int32_t  Length;
	uint16_t CRC;
}SF;
typedef struct{
	uint32_t FileLength;
	uint16_t FormatVersion;
	uint32_t pPsPN;
	uint32_t pLRUID;
	uint32_t pNbDF;
	uint32_t pNbSF;
	uint32_t pUDD;
	uint16_t PartNumberLength;
	char     PartNumber[LG_PART_NUMBER+1];
	uint16_t LruIdNb;
	LRUID*   ptabLRU;
	uint16_t NbDataFile;
	DF*      ptabDF;
	uint16_t NbSuppFile;
	SF*      ptabSF;
	uint32_t TailleUDD;
	char*    puser_define;
	uint16_t HeaderFileCRC;
	uint32_t LoadCRC;
}LUH;
