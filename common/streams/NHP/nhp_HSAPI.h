#pragma once
#include "stdlib.h"
#include "memory.h"


#define MESSAGE_SIZE	32
#define	HASH_SIZE		32
#define	END_POINT_SIZE	256

#pragma pack( 1 )

typedef struct _SYN_TYPE
{
	long	RND;
	char	Message[MESSAGE_SIZE];
	long	field1;//time_out
	long	field2;//Счетчик
	long	field3;//Версия
} SYN_TYPE;

typedef struct _SYN_ACK_TYPE
{
	long	SEQ;
	char	synhash[HASH_SIZE];//Не хеш, а что-то другое
	long	field1;
	long	field2;//Счетчик
	long	field3;//Версия
} SYN_ACK_TYPE;

typedef struct _ACK_BUF
{
	unsigned long	length;
	unsigned char	*buf;
} ACK_BUF;

typedef struct _ACK_TYPE
{
	unsigned long	IP;
	unsigned short	port;
	ACK_BUF			ack_buf;
} ACK_TYPE;

typedef struct _PUSH_TYPE
{
  char hash[HASH_SIZE];//hash от ACK'а
  unsigned char master;
  long length;
} PUSH_TYPE; //+ ConnectInformation NatIP и NatPort

#pragma pack( )

/////////////////////////////////////////////////////////////

#define ACK_CLIENT_ENDPOINT "ACE"
#define ACK_CONNECTED_CLIENT_ENDPOINT "ACCE"
#define ACK_CONFERENCE_NAME_ENDPOINT "ACNE"

#define PUSH_CONNECT_INFORMATION "PCI"
#define PUSH_LOCAL_CONNECT_INFORMATION "PCI_LOCAL"


/////////////////////////////////////////////////////////////
bool NHPSyn(const unsigned long timeout,
			SYN_TYPE* syndata/*[out]*/,
			int* err/*[out]*/);

bool NHPTransformSYN(const SYN_TYPE* syn_data/*[in]*/,
					 SYN_ACK_TYPE* syn_ack_buf/*[out]*/,
					 int* err/*[out]*/);

bool NHPAck(//const SYN_ACK_TYPE* syn_ack_data/*[in]*/,
			const char Ep[]/*[in]*/,
			const char ConnectedEp[]/*[in]*/,
			const char ConfName[]/*[in]*/,
			unsigned long IP/*[in]*/,
			unsigned short port/*[in]*/,
			ACK_TYPE* ack_buf/*[out]*/,
			int* err);
bool NHPSynAckIsValid(const SYN_ACK_TYPE* syn_ack_data, const SYN_TYPE* syn_data);
bool NHPPushIsValid(const PUSH_TYPE* push_data, const ACK_TYPE* ack_data);
bool NHPPush(const ACK_TYPE			*ack_data/*[in]*/,//ACK клиента для которого формируется PUSH
			 const unsigned long	IP /*[in]*/,
			 const unsigned short	port/*[in]*/,
			 const unsigned long	local_IP/*[in]*/,
			 const unsigned short	local_port/*[in]*/,
			 const unsigned char	master/*[in]*/,
			 PUSH_TYPE				*push_buf/*[out]*/,
			 char					**buf/*out*/,
			 unsigned long			*len/*out*/,
			 int					*err
			 );
bool NHPGetAckInfo(const ACK_TYPE	*ackdata,
				   char				*Ep,
				   char				*ConnectedEp,
				   char				*ConfName,
				   unsigned long	*IP,
				   unsigned short	*port,
				   int				*err);
bool NHPGetPushInfo(const PUSH_TYPE	*pushdata,
					const char		*pushbuf,
					unsigned long	*ip,
					unsigned short	*port,
					unsigned long	*local_ip,
					unsigned short	*local_port,
					int				*err);
int NHPGetCurrentVersion();
int NHPGetSynVersion(const SYN_TYPE *syn_pack);
int NHPGetSynAckVersion(const SYN_ACK_TYPE	*syn_ack_pack);
///////////////////////////////////////////////////////////
int NHPRand();
void NHPRandchar(char* chBuf, unsigned int Length);
bool NHPFillFields(const unsigned long timeout,SYN_TYPE* syndata, int *err);
bool NHPFillFields(SYN_ACK_TYPE* synackdata, int *err);
bool NHPGenSeq(const SYN_TYPE* syndata, long* SEQ, int *err);
bool NHPGetHash(const char Message[MESSAGE_SIZE], char* HASH,unsigned long hashsize,int *err);
bool NHPSetAckMessageData(const SYN_ACK_TYPE* syn_ack_data,ACK_TYPE* ack_buf,int* err);
bool NHPSetContainerData(const char *ParamName,const char *Value, void **buf, unsigned long* len, int* err);
bool NHPSetContainerData(const char *ParamName,const void *Value, const unsigned long ValSize, void **buf, unsigned long* len, int* err);
bool NHPGetContainerData(const char *ParamName,void *buf, unsigned long buflen,char *retbuf,unsigned long *retbuflen,int *err);
bool NHPGetContainerData(const char *ParamName,void *buf, unsigned long buflen,void *retbuf,unsigned long *retbuflen,int *err);
void NHPBufFree(char** buf);
void NHPBufFree(ACK_BUF* ack_buf);
void NHPPrepareBuf(char **buf,unsigned long size);