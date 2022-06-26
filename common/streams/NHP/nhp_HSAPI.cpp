#if defined(_WIN32) // Not ported yet

#include "nhp_HSAPI.h"
#include "VS_NHP_Types.h"
#include "../VS_StreamsDefinitions.h"
#include "../../acs/VS_AcsDefinitions.h"
#include "std-generic/cpplib/VS_Container.h"
#include "../../std/cpplib/VS_MemoryLeak.h"
#include "std/cpplib/md5.h"

#include <time.h>

#define VS_NHP_CURRENT_VERSION	1;

bool NHPSyn(const unsigned long timeout,
			SYN_TYPE* syndata/*[out]*/,
			int* err/*[out]*/)
{
	syndata->RND = NHPRand();
	NHPRandchar(syndata->Message,MESSAGE_SIZE);
	if(!NHPFillFields(timeout,syndata,err))
		return false;
	*err = 0;
	return true;
}

bool NHPTransformSYN(const SYN_TYPE* syn_data/*[in]*/,
					 SYN_ACK_TYPE* syn_ack_buf/*[out]*/,
					 int* err/*[out]*/)
{
	if(!NHPGenSeq(syn_data,&syn_ack_buf->SEQ,err))
		return false;
    if(!NHPGetHash(syn_data->Message,syn_ack_buf->synhash,HASH_SIZE,err))
		return false;
	if(!NHPFillFields(syn_ack_buf,err))
		return false;
	return true;
}

bool NHPAck(//const SYN_ACK_TYPE* syn_ack_data/*[in]*/,
			const char Ep[]/*[in]*/,
			const char ConnectedEp[]/*[in]*/,
			const char ConfName[]/*[in]*/,
			unsigned long IP/*[in]*/,
			unsigned short port/*[in]*/,
			ACK_TYPE* ack_buf/*[out]*/,
			int* err)
{//¬вести проверку длинны имен
	if(!err)
		return false;
	if(!ack_buf)
	{
		*err = 1;
		return false;
	}
	ack_buf->IP = IP;
	ack_buf->port = port;
	ack_buf->ack_buf.length = 0;
	ack_buf->ack_buf.buf = 0;
	if(!NHPSetContainerData(ACK_CLIENT_ENDPOINT,Ep,(void**)&ack_buf->ack_buf.buf,&ack_buf->ack_buf.length,err))
		return false;
	if(!NHPSetContainerData(ACK_CONNECTED_CLIENT_ENDPOINT,ConnectedEp,(void**)&ack_buf->ack_buf.buf,&ack_buf->ack_buf.length,err))
		return false;
	return NHPSetContainerData(ACK_CONFERENCE_NAME_ENDPOINT,ConfName,(void**)&ack_buf->ack_buf.buf,&ack_buf->ack_buf.length,err);
}

bool NHPGetAckInfo(const ACK_TYPE	*ackdata,
				   char				*Ep,
				   char				*ConnectedEp,
				   char				*ConfName,
				   unsigned long	*IP,
				   unsigned short	*port,
				   int				*err)
{
	unsigned long len;
	if(!err)
		return false;
	if((!IP)||(!ackdata)||(!port))
	{
		*err = 1;
		return false;
	}
	len = VS_ACS_MAX_SIZE_ENDPOINT_NAME+1;
	if(!NHPGetContainerData(ACK_CLIENT_ENDPOINT,ackdata->ack_buf.buf,ackdata->ack_buf.length,Ep,&len,err))
		return false;
	len = VS_ACS_MAX_SIZE_ENDPOINT_NAME+1;
	if(!NHPGetContainerData(ACK_CONNECTED_CLIENT_ENDPOINT,ackdata->ack_buf.buf,ackdata->ack_buf.length,ConnectedEp,&len,err))
		return false;
	len = VS_STREAMS_MAX_SIZE_CONFERENCE_NAME+1;
	if(!NHPGetContainerData(ACK_CONFERENCE_NAME_ENDPOINT,ackdata->ack_buf.buf,ackdata->ack_buf.length,ConfName,&len,err))
		return false;
	*IP = ackdata->IP;
	*port = ackdata->port;
	*err = 0;
	return true;
}

bool NHPGetPushInfo(const PUSH_TYPE	*pushdata,
					const char		*pushbuf,
					unsigned long	*ip,
					unsigned short	*port,
					unsigned long	*local_ip,
					unsigned short	*local_port,
					int				*err)
{
	if(!err)
		return false;
	if((!ip) || (!port) || (!local_ip) || (!local_port))
	{
		*err = 1;
		return false;
	}
	VS_NHP_ConnectionData	info;
	VS_NHP_ConnectionData	info_local;
	unsigned long			len = sizeof(info);

	if(!NHPGetContainerData(PUSH_CONNECT_INFORMATION,(void*)pushbuf,pushdata->length,&info,&len,err))
		return false;

	if(!NHPGetContainerData(PUSH_LOCAL_CONNECT_INFORMATION,(void*)pushbuf,pushdata->length,&info_local,&len,err))
		return false;

	*ip = info.ip;
	*port = info.port;
	*local_ip = info_local.ip;
	*local_port = info_local.port;

	return true;
}

bool NHPPush(const ACK_TYPE			*ack_data/*[in]*/,
			 const unsigned long	IP /*[in]*/,
			 const unsigned short	port/*[in]*/,
			 const unsigned long	local_IP/*[in]*/,
			 const unsigned short	local_port/*[in]*/,
			 const unsigned char	master/*[in]*/,
			 PUSH_TYPE				*push_buf/*[out]*/,
			 char					**buf/*[in,out]*/,
			 unsigned long			*len/*[in,out]*/,
			 int					*err)
{
	if(!err)
		return false;
	VS_NHP_ConnectionData info;
	VS_NHP_ConnectionData info_local;
	char message[MESSAGE_SIZE];
	memset(message,0,MESSAGE_SIZE);
	if((!ack_data)||(!push_buf)||(!len))
	{
		*err = 1;
		return false;
	}
	if(ack_data->ack_buf.length<MESSAGE_SIZE)
		memcpy(message,ack_data->ack_buf.buf,ack_data->ack_buf.length);
	else
		memcpy(message,ack_data->ack_buf.buf,MESSAGE_SIZE);
	if(!NHPGetHash(message,push_buf->hash,HASH_SIZE,err))
		return false;
	push_buf->master = master;
	info.ip =IP;
	info.port = port;
	info_local.ip = local_IP;
	info_local.port = local_port;
	//////info.local_ip = local_IP;////////ack_data->IP;//local
	/////info.local_port = local_port;///////ack_data->port;//local
	if(!NHPSetContainerData(PUSH_CONNECT_INFORMATION,(const void*)&info,sizeof(info),(void**)buf,len,err))
		return false;
	if(!NHPSetContainerData(PUSH_LOCAL_CONNECT_INFORMATION,(const void*)&info_local,sizeof(info_local),(void **)buf,len,err))
		return false;
	push_buf->length = *len;
	return true;
}

bool NHPSynAckIsValid(const SYN_ACK_TYPE* syn_ack_data, const SYN_TYPE* syn_data)
{
	SYN_ACK_TYPE	syn_ack_data4cmp;
	int				err;
	if(!NHPTransformSYN(syn_data,&syn_ack_data4cmp,&err))
		return false;
	int res = memcmp(syn_ack_data,&syn_ack_data4cmp,sizeof(SYN_ACK_TYPE));
	return !res;
}
bool NHPPushIsValid(const PUSH_TYPE* push_data, const ACK_TYPE* ack_data)
{
	char	ValidHash[HASH_SIZE];
	char	message[MESSAGE_SIZE];
	int		err;
	if((!push_data)||(!ack_data))
		return false;
	memset(message,0,MESSAGE_SIZE);
	if(ack_data->ack_buf.length<MESSAGE_SIZE)
		memcpy(message,ack_data->ack_buf.buf,ack_data->ack_buf.length);
	else
		memcpy(message,ack_data->ack_buf.buf,MESSAGE_SIZE);
	if(!NHPGetHash(message,ValidHash,HASH_SIZE,&err))
		return false;
	if(!memcmp(ValidHash,push_data->hash,HASH_SIZE))
		return true;
	else
		return false;
}
///////////////////////////////////////////////////////////////////////

bool NHPFillFields(const unsigned long timeout,SYN_TYPE* syndata,int *err)
{
	if(!syndata)
	{
		*err = 1;
		return false;
	}
	syndata->field1 = timeout;
	syndata->field2 = 0;
	syndata->field3 = VS_NHP_CURRENT_VERSION;
	*err = 0;
	return true;
}
bool NHPFillFields(SYN_ACK_TYPE* syndata,int *err)
{
	if(!syndata)
	{
		*err = 1;
		return false;
	}
	syndata->field1 = 0;
	syndata->field2 = 0;
	syndata->field3 = VS_NHP_CURRENT_VERSION;
	*err = 0;
	return true;
}
int NHPRand()
{
static bool nhp_sranded = false;
	if(!nhp_sranded)
		srand((unsigned)time(NULL));
	nhp_sranded = true;
	return rand();
}
void NHPRandchar(char* chBuf, unsigned int Length)
{
	int tmp;
	unsigned int step = sizeof(unsigned int);
	unsigned int uCounter = Length;
	char *Pos = chBuf;
	do{
		tmp = rand();
		if(uCounter>=step)
			memcpy(Pos,&tmp,step);
		else
			memcpy(Pos,&tmp,uCounter);
		Pos+=step;
		uCounter-=step;
	}while(uCounter>0);

}

bool NHPGenSeq(const SYN_TYPE* syndata,long* SEQ,int *err)
{
	if(!err)
		return false;
	if((!SEQ)||(!syndata))
	{
		*err = 1;
		return false;
	}
	*SEQ = syndata->RND + 1;
	*err = 0;
	return true;
}

bool NHPGetHash(const char Message[MESSAGE_SIZE], char* HASH,unsigned long hashsize,int *err)
{
	//EVP_MD_CTX	ctx;
	unsigned int len = 16;
	if(!err)
		return false;
	if(!HASH)
	{
		*err = 1;
		return false;
	}

	MD5 md5;
	md5.Update(Message, MESSAGE_SIZE);
	md5.Final();
	unsigned char hash[16];
	md5.GetBytes(hash);
	memcpy(HASH,hash,len>hashsize?hashsize:len);
	memset(&HASH[len],0,hashsize-len);
	*err = 0;
	return true;
}
bool NHPSetAckMessageData(const SYN_ACK_TYPE* syn_ack_data,ACK_TYPE* ack_buf,int* err)
{
	if(!err)
		return false;
	if((!syn_ack_data)||(!ack_buf))
	{
		*err = 1;
		return false;
	}
	ack_buf->ack_buf.buf = (unsigned char*)malloc(HASH_SIZE);//new unsigned char[HASH_SIZE];
	ack_buf->ack_buf.length = HASH_SIZE;
	memcpy(ack_buf->ack_buf.buf,syn_ack_data->synhash,HASH_SIZE);
	*err = 0;
	return true;
}
bool NHPSetContainerData(const char *ParamName,const char *Value,void **buf, unsigned long* len, int* err)
{
	if(!err)
		return false;
	VS_Container	cont;
	void			*vbuf(0);
	size_t			vbuflen(0);
	if(*len)
	{
		if(!cont.Deserialize(*buf,*len))
		{
			*err = 2;
			return false;
		}
		free(*buf);//delete [] *buf;
	}
	if(!cont.AddValue(ParamName,Value))
	{
		*err = 2;
		return false;
	}
	if(!cont.SerializeAlloc(vbuf,vbuflen))
	{
		*err = 2;
		return false;
	}
	*buf = vbuf;
	*len = vbuflen;
	return true;
}

bool NHPGetContainerData(const char *ParamName,void *buf, unsigned long buflen,char *retbuf,unsigned long *retbuflen,int *err)
{
	VS_Container cont;
	size_t retbuflen_tmp;
	if((!cont.Deserialize(buf,buflen))	||
		(!cont.GetValue(ParamName,retbuf,retbuflen_tmp)))
	{
		*err = 2;
		return false;
	}
	*retbuflen = retbuflen_tmp;
	*err = 0;
	return true;
}
bool NHPGetContainerData(const char *ParamName,void *buf, unsigned long buflen,void *retbuf,unsigned long *retbuflen,int *err)
{
	if(!err)
		return false;
	VS_Container cont;
	size_t retbuflen_tmp;
	if((!cont.Deserialize(buf,buflen))	||
		(!cont.GetValue(ParamName,retbuf,retbuflen_tmp)))
	{
		*err = 2;
		return false;
	}
	*retbuflen = retbuflen_tmp;
	*err = 0;
	return true;
}

bool NHPSetContainerData(const char *ParamName,const void *Value, const unsigned long ValSize, void **buf, unsigned long* len, int* err)
{
	VS_Container	cont;
	void			*vbuf(0);
	size_t			vbuflen(0);
	if(*len)
	{
		if(!cont.Deserialize(*buf,*len))
		{
			*err = 2;
			return false;
		}
		free(*buf);//delete [] *buf;
	}
	if(!cont.AddValue(ParamName,Value, ValSize))
	{
		*err = 2;
		return false;
	}
	if(!cont.SerializeAlloc(vbuf,vbuflen))
	{
		*err = 2;
		return false;
	}
	*buf = vbuf;
	*len = vbuflen;
	return true;
}
void NHPBufFree(char **buf)
{
	free(*buf);//delete [] *buf;
	*buf=0;
}
void NHPBufFree(ACK_BUF* ack_buf)
{
	if(!ack_buf)
		return;
	NHPBufFree((char**)&ack_buf->buf);
	ack_buf->length = 0;
}
void NHPPrepareBuf(char **buf,unsigned long size)
{
	if(!size)
	{
		*buf = 0;
		return;
	}
	*buf = (char*)malloc(size);//new char[size];
}

int NHPGetCurrentVersion()
{
	return VS_NHP_CURRENT_VERSION;
}
int NHPGetSynVersion(const SYN_TYPE *syn_pack)
{
	if(!syn_pack)
		return -1;
	else
		return syn_pack->field3;
}
int NHPGetSynAckVersion(const SYN_ACK_TYPE	*syn_ack_pack)
{
	if(!syn_ack_pack)
		return -1;
	else
		return syn_ack_pack->field3;
}

#endif
