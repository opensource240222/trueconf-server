#if defined(_WIN32) // Not ported yet

#include "VS_TransportLib.h"
#include "../VS_TransportDefinitions.h"
#include "SecureLib/VS_SecureConstants.h"
#include "SecureLib/VS_Sign.h"
#include "../../std/cpplib/VS_Utils.h"
#include "../../std/cpplib/VS_RegistryKey.h"
#include "../../std/cpplib/VS_RegistryConst.h"

#include <stdlib.h>
#include <string.h>

const char VS_Transport_PrimaryField[net::HandshakeHeader::primary_field_size] = { '_','V','S','_','T','R','A','N','S','P','O','R','T','_',0 };

const unsigned char VS_SSL_SUPPORT_BITMASK			 = 0x10;

const unsigned long VS_TRANSPORT_TIMELIFE_PING		 = 5000;
const unsigned long VS_TRANSPORT_TIMELIFE_CONNECT	 = 30000;
const unsigned long VS_TRANSPORT_TIMELIFE_DISCONNECT = 30000;

const char *VS_TRANSPORT_MANAGING_TYPE_NAME_VALUE	 = "Type";
const char *VS_TRANSPORT_MANAGING_TYPE_NAME_GATES	 = "Gates";
const char *VS_TRANSPORT_MANAGING_VERSION_NAME_VALUE = "Version";

const char *VS_TypeNameValue	= VS_TRANSPORT_MANAGING_TYPE_NAME_VALUE;
const char *VS_GatesNameValue	= VS_TRANSPORT_MANAGING_TYPE_NAME_GATES;
const char *VS_VersionNameValue	= VS_TRANSPORT_MANAGING_VERSION_NAME_VALUE;

// use onlu for reply to old architecture clients
net::HandshakeHeader* VS_FormTransportReplyHandshake___OLDARCH()
{
	const char *key = "0";
	const unsigned char type = 0;
	const unsigned char resultCode = 2;
	const unsigned short maxConnSilenceMs = 0;
	const unsigned char fatalSilenceCoef = 0;
	const unsigned char hops = 0;
	bool IsSSLSupport = true;

	const unsigned long   key_length = (!key || !*key) ? 0 : (unsigned long)strlen( key );
	if (key_length > VS_TRANSPORT_MAX_SIZE_KEY)		return 0;
	const unsigned long   body_length = 5 + (!key_length ? 0 : (1 + key_length + 1 + 1));
	const size_t sz = sizeof(net::HandshakeHeader) + body_length;
	unsigned char   *buffer = (unsigned char *)malloc( sz );
	if (!buffer)	return 0;			memset( (void *)buffer, 0, sz );
	auto* hs = reinterpret_cast<net::HandshakeHeader*>(buffer);
	strcpy(hs->primary_field, VS_Transport_PrimaryField);
	hs->version = VS_CURRENT_TRANSPORT_VERSION;
	if(IsSSLSupport)
		hs->version |= VS_SSL_SUPPORT_BITMASK;
	hs->body_length = body_length - 1;
	buffer += sizeof(net::HandshakeHeader);
	*buffer = type;									++buffer;
	*buffer = resultCode;							++buffer;
	*(unsigned short *)buffer = maxConnSilenceMs;	buffer += 2;
	*buffer = fatalSilenceCoef;						++buffer;
	if (key_length)
	{	*buffer = (unsigned char)key_length;		++buffer;
		strcpy( (char *)buffer, key );				buffer += key_length + 1;
		*buffer = hops;								++buffer;	}
	net::UpdateHandshakeChecksums(*hs);
	return hs;
}

/**
	Для формирования нужен CID (при первом коннекте это 0, sid, hops,
*/
net::HandshakeHeader* VS_FormTransportHandshake(const char *cid,
								 const char *sid,
								const unsigned char hops, bool IsSSLSupport, bool tcpKeepAliveSupport)
{
	/**
		В месаге должен быть SID, CID
	*/
	VS_Sign					signer;
	VS_SignArg				signarg = {alg_pk_RSA,alg_hsh_SHA1};
	std::unique_ptr<char, free_deleter> PrivKeyBuf;
	bool signAbility(true);
	VS_RegistryKey key(false, CONFIGURATION_KEY);
	if(!signer.Init(signarg) || !key.GetValue(PrivKeyBuf, VS_REG_BINARY_VT, SRV_PRIVATE_KEY) ||
		!signer.SetPrivateKey(PrivKeyBuf.get(),store_PEM_BUF))
		signAbility = false;

	const static char   zeroStr[] = "";
	const char	*loc_cid = !cid ? zeroStr : cid;
	const char	*loc_sid = !sid ? zeroStr : sid;
	const unsigned long	cid_length = (const unsigned long)strlen( loc_cid ),
						sid_length = (const unsigned long)strlen( loc_sid );

	// для подписи сообщения,
	unsigned long randomData_sz = (hops == 0)||!signAbility? 0 : VS_RND_DATA_FOR_SIGN_SZ + 2; // 2 байта под длинну
	unsigned short sign_sz = (hops == 0)||!signAbility? 0 : VS_SIGN_SIZE + 2; // 2 байта под длинну

	unsigned char randData[VS_RND_DATA_FOR_SIGN_SZ] = {0};
	unsigned char sign[VS_SIGN_SIZE] = {0};

	if(cid_length > VS_ACS_MAX_SIZE_CID
		|| sid_length > VS_ACS_MAX_SIZE_SERVER_NAME)
		return 0;

	const unsigned long	body_length = 1 + cid_length + 2 + sid_length + 2 + randomData_sz + sign_sz + 1;

	const size_t sz = sizeof(net::HandshakeHeader) + body_length;
	unsigned char	*buffer = (unsigned char*)malloc(sz);

	if(!buffer)
		return 0;
	memset(buffer, 0, sz);

	auto* hs = reinterpret_cast<net::HandshakeHeader*>(buffer);
	strcpy(hs->primary_field, VS_Transport_PrimaryField);

	hs->version = VS_CURRENT_TRANSPORT_VERSION;

	if(IsSSLSupport)
		hs->version |= VS_SSL_SUPPORT_BITMASK;

	hs->body_length = body_length - 1;
	buffer += sizeof(net::HandshakeHeader);

	*buffer = (unsigned char) cid_length;	++buffer;
	strcpy( (char*) buffer, loc_cid);		buffer += cid_length + 1;
	*buffer = (unsigned char ) sid_length;	++buffer;
	strcpy( (char*) buffer, loc_sid);		buffer += sid_length + 1;
	*buffer = hops;							++buffer;
	if(0 != hops && signAbility)
	{

		VS_GenKeyByMD5(randData);
		*((unsigned short *)buffer) = VS_RND_DATA_FOR_SIGN_SZ; //размер рандомных данных
		buffer += sizeof(unsigned short);
		memcpy(buffer,randData,VS_RND_DATA_FOR_SIGN_SZ);
		buffer+=VS_RND_DATA_FOR_SIGN_SZ;
		uint32_t l_sign_sz = VS_SIGN_SIZE;
		if(!signer.SignData(randData,VS_RND_DATA_FOR_SIGN_SZ,buffer + 2,&l_sign_sz))
		{
			free(hs);
			return 0;
		}
		sign_sz = (unsigned short)l_sign_sz;
		*((unsigned short *)buffer) = sign_sz;
		buffer += sizeof(unsigned short) + l_sign_sz;
	}

	*buffer = (char)tcpKeepAliveSupport;
	++buffer;

	net::UpdateHandshakeChecksums(*hs);
	return hs;
}
bool VS_TransformTransportHandshake( net::HandshakeHeader *hs, char *&cid,
										char *&sid, unsigned char &hops,unsigned long &rnd_data_ln,
									const unsigned char *&rnd_data, unsigned long &sign_sz,const unsigned char *&sign,
									bool &tcpKeepAliveSupport )
{
	if (!hs || hs->head_cksum != net::GetHandshakeHeaderChecksum(*hs)
			|| hs->body_cksum != net::GetHandshakeBodyChecksum(*hs)
			|| strcmp(hs->primary_field, VS_Transport_PrimaryField)
			|| hs->version < 1)
			return false;

	const unsigned long body_length = hs->body_length + 1,
		min_body_legth = sizeof(unsigned char) + sizeof(unsigned char) + 1;

	if(body_length < min_body_legth)
		return false;
	unsigned char* buffer = &((unsigned char *)hs)[sizeof(net::HandshakeHeader)];

	unsigned long index = 0;

	char	*loc_cid = 0;		unsigned char loc_cid_length = 0;
	char	*loc_sid = 0;		unsigned char loc_sid_length = 0;
	unsigned char   loc_hops = 0;
	loc_cid_length = buffer[index];
	if(++index >= body_length)
		return false;
	loc_cid = (char*)&buffer[index];
	if((index += loc_cid_length) >= body_length)
		return false;
	if(buffer[index])
		return false;
	if(++index >=body_length)
		return false;
	loc_sid_length = buffer[index];
	if(++index >= body_length)
		return false;
	loc_sid = (char*)&buffer[index];
	if((index += loc_sid_length) >= body_length)
		return false;
	if(buffer[index])
		return false;
	if((++index) >= body_length)
		return false;

	loc_hops = buffer[index];
	cid = loc_cid;
	sid = loc_sid;
	hops = loc_hops;

	if((++index<body_length-1)&&0!=loc_hops)
	{
		unsigned short loc_rnd_data_ln = *(unsigned short*)&buffer[index];
		if((index += sizeof(loc_rnd_data_ln))>=body_length)
			return false;
		unsigned char *loc_rnd_data = (unsigned char*)&buffer[index];
		if((index += loc_rnd_data_ln)>=body_length - 1)
			return false;
		unsigned short loc_sign_ln = *(unsigned short*)&buffer[index];
		if((index += sizeof(loc_sign_ln)) >= body_length)
			return false;
		if(index + loc_sign_ln >body_length)
			return false;
		unsigned char *loc_sign = (unsigned char*)&buffer[index];
		index += loc_sign_ln;
		rnd_data_ln = loc_rnd_data_ln;
		rnd_data = loc_rnd_data;
		sign_sz = loc_sign_ln;
		sign = loc_sign;
	}

	tcpKeepAliveSupport = false;
	if (index < body_length)
	{
		tcpKeepAliveSupport = !!buffer[index++];
	}

	/*else if(0!=loc_hops)
		return false;*/

	return true;
}

net::HandshakeHeader* VS_FormTransportReplyHandshake(const char *cid,
								const unsigned char resultCode,
								const unsigned short maxConnSilenceMs,
								const unsigned char fatalSilenceCoef, const unsigned char hops,
								const char *server_id,
								bool IsSSLSupport, bool tcpKeepAliveSupport)
{
	const unsigned long server_id_length = (server_id && *server_id)?(unsigned long)strlen(server_id):0;
	const unsigned long cid_length = (cid && *cid)?(unsigned long)strlen(cid):0;
	if (server_id_length==0 || server_id_length>VS_ACS_MAX_SIZE_ENDPOINT_NAME)
		return 0;
	if (cid_length==0 || cid_length>VS_ACS_MAX_SIZE_ENDPOINT_NAME)
		return 0;

	const unsigned long   body_length = 5 + server_id_length+ 2 + cid_length + 2 + 1;
	const size_t sz = sizeof(net::HandshakeHeader) + body_length;
	unsigned char   *buffer = (unsigned char *)malloc( sz );
	auto* hs = reinterpret_cast<net::HandshakeHeader*>(buffer);
	strcpy(hs->primary_field, VS_Transport_PrimaryField);
	hs->version = VS_CURRENT_TRANSPORT_VERSION;
	if(IsSSLSupport)
		hs->version |= VS_SSL_SUPPORT_BITMASK;
	hs->body_length = body_length - 1;
	buffer += sizeof(net::HandshakeHeader);
	*buffer = resultCode;							++buffer;
	*(unsigned short *)buffer = maxConnSilenceMs;	buffer += 2;
	*buffer = fatalSilenceCoef;						++buffer;
	*buffer = hops;									++buffer;
	*buffer = (unsigned char)server_id_length; ++buffer;
	strcpy((char*)buffer,server_id); buffer += server_id_length + 1;
	*buffer = (unsigned char)cid_length; ++buffer;
	strcpy((char*)buffer,cid); buffer += cid_length + 1;
	*buffer = (char)tcpKeepAliveSupport; ++buffer;

	net::UpdateHandshakeChecksums(*hs);
	return hs;
}

bool VS_TransformTransportReplyHandshake(net::HandshakeHeader* hs,
											unsigned char &resultCode,
											unsigned short &maxConnSilenceMs,
											unsigned char &fatalSilenceCoef,
											unsigned char &hops,
											char *&server_id,
											char *&cid,
											bool &tcpKeepAliveSupport
											)
{
	if (!hs || hs->head_cksum != net::GetHandshakeHeaderChecksum(*hs)
			|| hs->body_cksum != net::GetHandshakeBodyChecksum(*hs)
			|| strcmp(hs->primary_field, VS_Transport_PrimaryField)
			|| hs->version < 1)		return false;
	const unsigned long   body_length = hs->body_length + 1;
	if (body_length < 5)	return false;
	unsigned char* buffer = &((unsigned char *)hs)[sizeof(net::HandshakeHeader)];
	unsigned long   index = 0;	unsigned char   loc_resultCode;
	unsigned short   loc_maxConnSilenceMs;		unsigned char   loc_fatalSilenceCoef;
	unsigned char   loc_hops = 0;
	char   *loc_server_id = 0;
	unsigned long loc_server_id_length = 0;
	char   *loc_cid = 0;
	unsigned long loc_cid_length = 0;
	loc_resultCode = buffer[index];								++index;
	loc_maxConnSilenceMs = *(unsigned short *)&buffer[index];	index += 2;
	loc_fatalSilenceCoef = buffer[index];						if ((++index + 1) == body_length)	return false;
	if (index < body_length)
	{
		loc_hops = buffer[index]; ++index;
	}
	if (index < body_length)
	{
		loc_server_id_length = buffer[index];	++index	;
		loc_server_id = (char*)&buffer[index];
		if ((index +=loc_server_id_length) >=body_length)
			return false;
		if (buffer[index])	return false; ++index;
	}
	if (index < body_length)
	{
		loc_cid_length = buffer[index];	++index	;
		loc_cid = (char*)&buffer[index];
		if ((index +=loc_cid_length) >body_length)
			return false;
		if (buffer[index])	return false; ++index;
	}

	tcpKeepAliveSupport = false;
	if (index < body_length)
	{
		tcpKeepAliveSupport = !!buffer[index++];
	}

	resultCode = loc_resultCode;
	maxConnSilenceMs = loc_maxConnSilenceMs;
	fatalSilenceCoef = loc_fatalSilenceCoef;
	hops = loc_hops;
	///Для обратной совместимости. Пока толком не продумано,
	///будем поддерживать. В будущем, возможно надо будет
	///добавить return false;

	//ЗЫ: обратная соместимость скорее всего здесь не сработает - false вернется раньше
	if ( (hs->version & VS_MAX_TRANSPORT_VERSION) >= VS_NEWARCH_TRANSPORT_VERSION )
	{
		if (!loc_server_id || !loc_cid)
			return false;
		server_id = loc_server_id;
		cid = loc_cid;
	}
	return true;
}

#endif
