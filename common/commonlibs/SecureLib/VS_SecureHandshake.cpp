#include "VS_SecureHandshake.h"
#include "VS_SymmetricCrypt.h"
#include "VS_PublicKeyCrypt.h"
#include "VS_Certificate.h"
#include "VS_SecureConstants.h"
#include "VS_Sign.h"
#include "std-generic/cpplib/VS_Container.h"
#include "std-generic/clib/strcasecmp.h"
#include "std-generic/debuglog/VS_Debug.h"

#include <openssl/engine.h>
#include <random>

#ifdef _DEBUG
#include <fstream>
#include <iostream>
#endif

#define DEBUG_CURRENT_MODULE VS_DM_LOGS

/*const char * VS_PEM_CACERT1 = "-----BEGIN CERTIFICATE-----\n\
MIIDiTCCAnGgAwIBAgIJANAtAP54ioqHMA0GCSqGSIb3DQEBBAUAMHsxEzARBgNV\n\
BAMTCkV4YW1wbGUgQ0ExDzANBgNVBAgTBk1vc2tvdzELMAkGA1UEBhMCUnUxHzAd\n\
BgkqhkiG9w0BCQEWEGNhQGV4YW1wbGVjYS5vcmcxJTAjBgNVBAoTHFJvb3QgQ2Vy\n\
dGlmaWNhdGlvbiBBdXRob3JpdHkwHhcNMDYwMTE2MTYxNzQwWhcNMDYwMjE1MTYx\n\
NzQwWjB7MRMwEQYDVQQDEwpFeGFtcGxlIENBMQ8wDQYDVQQIEwZNb3Nrb3cxCzAJ\n\
BgNVBAYTAlJ1MR8wHQYJKoZIhvcNAQkBFhBjYUBleGFtcGxlY2Eub3JnMSUwIwYD\n\
VQQKExxSb290IENlcnRpZmljYXRpb24gQXV0aG9yaXR5MIIBIjANBgkqhkiG9w0B\n\
AQEFAAOCAQ8AMIIBCgKCAQEAsfQdjnPx+RIYPWvJlPL5zXtMob6HIeO5pIskMnGj\n\
uj03XXmMvkaUpYwqHwjbPPkCja++pTb2eYM19nCrwxYv1z1COcMEv3dbf2A6UESt\n\
a2Pl3PldUus5OinHUBCR2jzgB/gcDkoAeneuEVFshK3z42k2NYu5240U6uscxzmu\n\
8X+mCtUUJYwFgeMX0T5l5koWI/9SdlAvoieBwHZvJpCp2p6WMF60ObYUm1G/PHIw\n\
8FJM+o7PajaFevy6JzQtyeLOpElISX0vbkwLiYE58Cl7Eu/DKkPkdYfcVUm8CAdd\n\
SM+/x22eabXjzqgSAgntsUVrIf+p9gos4tZspmGjP5y1owIDAQABoxAwDjAMBgNV\n\
HRMEBTADAQH/MA0GCSqGSIb3DQEBBAUAA4IBAQBnWej4To066vDhdJP7ALmJViJ3\n\
vIffZJMOCJxaQcvLQmP9RMLXahD8zEX9CTUHNU7wn2mKWAL9AW5yYAq1K+3TcVw5\n\
fzt7lXzzJde3WFcTifcMQfRY+YpQyF5Bja89wYk1iRn54dV1083i8/Wa04hGpwOV\n\
/ZT1a++apFtuCh+ksuwUm/0klXq1FIopt2CpE/6JEsHnWiU4ZFG8WIVSwDB5mZOF\n\
+5EcAtrTPbqjQj2C8Sh+CDw8B/gRi7FF4weEAlB+DZJSN/gGcYPE9hJv2MFVa4pH\n\
4vK4GFr1oaiQE8P6XnTU/035zIctZ4jnsGFPXITwSCc6FNUPtMu+rwkeSI/k\n\
-----END CERTIFICATE-----\n\
";*/


char *VS_SecureHandshake::m_RegistryRoot = 0;
bool VS_SecureHandshake_Implementation::m_ImprovedSecurity = false;


VS_SecureHandshake_Implementation::VS_SecureHandshake_Implementation():m_ErrorCode(e_err_NotInit)
{}
void VS_SecureHandshake_Implementation::SetErrorCode(const VS_SecureHandshakeError err_code)
{
	m_ErrorCode = err_code;
}
bool VS_SecureHandshake_Implementation::Init(const VS_SecureHandshakeType type)
{
	SetErrorCode(e_err_NoError);
	return InitAct(type);
}
void VS_SecureHandshake_Implementation::StopHandshake()
{
	SetErrorCode(e_err_NotInit);
	StopHandshakeAct();
}
void VS_SecureHandshake_Implementation::SetImprovedSecurityState(bool state)
{
	m_ImprovedSecurity = state;
}
VS_SecureHandshake::VS_SecureHandshake(void) : imp(0), m_Version(0xffffffff)
{}

VS_SecureHandshake::VS_SecureHandshake(const uint32_t Version, const enum VS_SecureHandshakeType type) : imp(nullptr), m_Version(0xffffffff)

{
	Init(Version, type);
}

VS_SecureHandshake::~VS_SecureHandshake(void)
{
	if(imp)
		delete imp;
}

void VS_SecureHandshake::SetRegistryRoot(const char *root)
{
	if(!m_RegistryRoot)
	{
		if((!root)||(strlen(root)>=255))
			return;
		else
		{
			m_RegistryRoot = new char[strlen(root) + 1];
			strcpy(m_RegistryRoot, root);
		}
	}
}
bool VS_SecureHandshake::GetRegistryRoot(char *out, unsigned int &buflen)
{
	if(!m_RegistryRoot)
		return false;
	else
	{
		if(buflen<strlen(m_RegistryRoot) + 1)
		{
			buflen = static_cast<unsigned int>(strlen(m_RegistryRoot)) + 1;
			return false;
		}
		else
		{
			memcpy(out, m_RegistryRoot, strlen(m_RegistryRoot) + 1);
			return true;
		}
	}
}
void VS_SecureHandshake::SetImprovedSecurityState(bool state)
{
	VS_SecureHandshake_Implementation::SetImprovedSecurityState(state);
}
bool VS_SecureHandshake::Init(const uint32_t Version, const enum VS_SecureHandshakeType type)
{
	if(m_Version!=0xffffffff)
		StopHandshake();
	switch(Version)
	{
	case 0:
		imp = new VS_EmptySecureHandshake_Impl;
		if(imp->Init(type))
		{
			m_Version = Version;
			return true;
		}
		else
		{
			delete imp;
			return false;
		}
	case 1:
		imp = new VS_SecureHandshakeVer1_Impl;
		if(imp->Init(type))
		{
			m_Version = Version;
			return true;
		}
		else
		{
			delete imp;
			return false;
		}
	case 2:
		imp = new VS_SecureHandshakeVer2_Impl;
		if(imp->Init(type))
		{
			m_Version = Version;
			return true;
		}
		else
		{
			delete imp;
			return false;
		}
	default:
		return false;
	}

}
void VS_SecureHandshake::StopHandshake()
{
	if(imp)
	{
		imp->StopHandshake();
		delete imp;
		imp = 0;
	}
	m_Version = 0xffffffff;
}

VS_SecureHandshakeState VS_SecureHandshake::Next()
{
	return imp ? imp->Next() : secure_st_Error;
}
bool VS_SecureHandshake::PreparePacket(void **buf, uint32_t *size)
{
	return imp ? imp->PreparePacket(buf, size) : false;
}
void VS_SecureHandshake::FreePacket(void ** buf)
{
	if(imp)
		imp->FreePacketBuf(buf);
}
bool VS_SecureHandshake::ProcessPacket(const void *buf, const uint32_t size)
{
	return imp ? imp->ProcessPacket(buf, size) : false;
}

VS_SymmetricCrypt *VS_SecureHandshake::GetReadSymmetricCrypt()
{
	return imp ? imp->GetReadSymmetricCrypt() : 0;
}
VS_SymmetricCrypt *VS_SecureHandshake::GetWriteSymmetricCrypt()
{
	return imp ? imp->GetWriteSymmetricCrypt() : 0;
}
bool VS_SecureHandshake::SetPrivateKey(VS_PKey *pkey)
{
	return imp ? imp->SetPrivateKey(pkey) : false;
}

uint32_t VS_SecureHandshake::GetVersion() const
{
	return m_Version;

}
VS_SecureHandshakeError VS_SecureHandshake::GetHandshakeErrorCode()
{
	return !imp ? e_err_NoError : imp->m_ErrorCode;
}

std::shared_ptr<VS_Certificate> VS_SecureHandshake::CloneCertificate() const
{
    return imp ? imp->CloneCertificate() : nullptr;
}

bool VS_SecureHandshake::GetCertificate(char *buf, uint32_t &sz)
{
	return imp ? imp->GetCertificate(buf, sz) : false;
}
void VS_SecureHandshake::ReleaseSymmetricCrypt(VS_SymmetricCrypt **crypt)
{
	delete *crypt;
	*crypt = 0;
}
VS_EmptySecureHandshake_Impl::VS_EmptySecureHandshake_Impl()
{
}

VS_EmptySecureHandshake_Impl::~VS_EmptySecureHandshake_Impl()
{
}

bool VS_EmptySecureHandshake_Impl::InitAct(const enum VS_SecureHandshakeType type)
{
	if((type!= handshake_type_Client)&&(type != handshake_type_Server))
		return false;
	else
		return true;
}
void VS_EmptySecureHandshake_Impl::StopHandshakeAct()
{}

VS_SecureHandshakeState VS_EmptySecureHandshake_Impl::Next()
{
	return secure_st_Finish;
}

bool VS_EmptySecureHandshake_Impl::PreparePacket(void **, uint32_t *)
{
	return false;
}
void VS_EmptySecureHandshake_Impl::FreePacketBuf(void **) const
{
}
bool VS_EmptySecureHandshake_Impl::ProcessPacket(const void *, const uint32_t )
{
	return false;
}
VS_SymmetricCrypt * VS_EmptySecureHandshake_Impl()
{
	return 0;
}
std::shared_ptr<VS_Certificate> VS_EmptySecureHandshake_Impl::CloneCertificate() const {
    return std::shared_ptr<VS_Certificate>();
}
bool VS_EmptySecureHandshake_Impl::GetCertificate(char *, uint32_t &)
{
	return false;
}
void VS_EmptySecureHandshake_Impl::ReleaseSymmetricCrypt(VS_SymmetricCrypt *crypt) const
{
	if(crypt)
		delete crypt;
}
VS_SymmetricCrypt * VS_EmptySecureHandshake_Impl::GetReadSymmetricCrypt()
{
	return 0;
}
VS_SymmetricCrypt *VS_EmptySecureHandshake_Impl::GetWriteSymmetricCrypt()
{
	return 0;
}

bool VS_EmptySecureHandshake_Impl::SetPrivateKey(VS_PKey *)
{
	return true;
}

VS_SecureHandshakeVer1_Impl::VS_SecureHandshakeVer1_Impl() :
				m_type(handshake_type_None),
				m_GostSupported(
					(!!ENGINE_by_id("gost") && !!ENGINE_set_default_ciphers(ENGINE_by_id("gost")))
					|| (ERR_clear_error(), false)
				),
				m_state(st_none), m_somedata(0), m_size_somedata(0), m_private_key_buf(0),
				m_cert_pem_buf(0), m_cert_buf_len(0)
{
}

VS_SecureHandshakeVer1_Impl::~VS_SecureHandshakeVer1_Impl()
{
	if(m_size_somedata)
		delete [] m_somedata;
	if(m_private_key_buf)
		delete [] m_private_key_buf;
	if(m_cert_pem_buf)
		delete [] m_cert_pem_buf;

}
void VS_SecureHandshakeVer1_Impl::PrepareClientErrorMessage(const VS_SecureHandshakeError err_code)
{
	if(m_size_somedata)
	{
		delete [] m_somedata;
		m_somedata = 0;
		m_size_somedata = 0;
	}
	m_somedata = new char[sizeof(uint32_t) + sizeof(VS_SecureHandshakeError)];
	m_size_somedata = sizeof(uint32_t) + sizeof(VS_SecureHandshakeError);
	m_state = st_send_err | st_init;
	(*reinterpret_cast<uint32_t*>(m_somedata)) = m_size_somedata - sizeof(uint32_t);
	memcpy(m_somedata + sizeof(uint32_t), &err_code, sizeof(VS_SecureHandshakeError));
}
bool VS_SecureHandshakeVer1_Impl::IsServerHandshake() const
{
	return (m_type == handshake_type_Server);
}
bool VS_SecureHandshakeVer1_Impl::InitAct(const enum VS_SecureHandshakeType type)
{
	if((m_state&st_init)||(handshake_type_None == type))
		return false;
	m_type = type;
	m_state = st_init;
	m_state|= st_start;
	return true;
}
void VS_SecureHandshakeVer1_Impl::StopHandshakeAct()
{
	m_state = st_none;
	m_type = handshake_type_None;
	if(m_size_somedata)
		delete [] m_somedata;
	m_size_somedata = 0;
	m_somedata = 0;
	if(m_cert_pem_buf)
		delete [] m_cert_pem_buf;
	m_cert_pem_buf = 0;
	m_cert_buf_len = 0;
}

VS_SecureHandshakeState VS_SecureHandshakeVer1_Impl::Next()
{
	VS_SecureHandshakeState Result(secure_st_Error);
	if(!(m_state&st_init))
		return secure_st_Error;
	if(IsServerHandshake())
	{
		switch(m_state)
		{
		case st_start|st_init:
			//послать файл, переключить st
			Result = secure_st_SendCert;
			m_state = st_send_cert | st_init;
			break;
		case st_send_cert | st_init:
			//послать пакет данных, st переключить только в процедуре подготовке буфера
			Result = secure_st_SendPacket;
			break;
		case st_send_alg | st_init:
		case st_get_packet_size | st_init:
			//получить пакет, st поменять в процедуре подготовке буфера
			Result = secure_st_GetPacket;
			break;
		case st_finish | st_init:
			// Final
			Result = secure_st_Finish;

			break;
		default:
			Result = secure_st_Error;
		}
	}
	else
	{
		switch(m_state)
		{
		case st_start | st_init:
		case st_get_cert_size | st_init:
		case st_get_cert | st_init:
		case st_get_packet_size | st_init:
			//получть пакет
			Result = secure_st_GetPacket;
			break;
		case st_get_packet | st_init:
		case st_send_err | st_init:
			Result = secure_st_SendPacket;
			break;
		case st_send_result | st_init: //???
		case st_finish | st_init:
			Result = secure_st_Finish;
			break;
		default:
			Result = secure_st_Error;
		}
	}
	return Result;
}

bool VS_SecureHandshakeVer1_Impl::PreparePacket(void **buf, uint32_t *size)
{
	bool			bRes(false);
	uint32_t	new_size(0);
	uint32_t	count_items(0);
	char			*Pos;

	if(!(m_state&st_init))
	{
		SetErrorCode(e_err_NotInit);
		return false;
	}
	if(*buf)
	{
		SetErrorCode(e_err_ArgNotValid);
		return false;
	}
	if(IsServerHandshake())
	{
		switch(m_state)
		{
		case st_send_cert | st_init:
			//подготовить посылаемые алгоритмы
			count_items = VS_COUNT_SYMMETRIC_ALG;
			if (m_ImprovedSecurity)
			{
				++count_items;
				if (m_GostSupported)
					++count_items;
			}
			new_size =  sizeof(count_items) + sizeof(VS_SymmetricAlg)*count_items;
			*buf = new char[new_size];
			Pos = static_cast<char*>(*buf);
			*(reinterpret_cast<uint32_t*>(Pos)) = count_items;				Pos+= sizeof(count_items);
			*(reinterpret_cast<uint32_t*>(Pos)) = alg_sym_NONE;				Pos+= sizeof(VS_SymmetricAlg);

			if (m_ImprovedSecurity)
			{
				if (m_GostSupported)
				{
					*(reinterpret_cast<uint32_t*>(Pos)) = alg_sym_GOST;		Pos+= sizeof(VS_SymmetricAlg);
				}

				*(reinterpret_cast<uint32_t*>(Pos)) = alg_sym_AES256;		Pos+= sizeof(VS_SymmetricAlg);
			}
			*(reinterpret_cast<uint32_t*>(Pos)) = alg_sym_AES128;			Pos+= sizeof(VS_SymmetricAlg);
			*size = new_size;
			bRes = true;
			m_state = st_send_alg | st_init;
			break;
		case st_send_alg | st_init:
			*buf = new char[sizeof(uint32_t)];
			*size = sizeof(uint32_t);
			m_state = st_get_packet_size | st_init;
			bRes = true;
			break;
		case st_get_packet_size | st_init:
			//подготовить буффер для для полученного size
			if(sizeof(uint32_t)!=m_size_somedata)
			{
				SetErrorCode(e_err_BufSizeExpectedValue);
				bRes = false;
				dstream3 << "Prepare of buffer for client packet data failed!!! (PreparePacket() size of received data failed. Developers Error!)";
			}
			else
			{
				*buf = new char[*(reinterpret_cast<uint32_t*>(m_somedata))];
				*size = *(reinterpret_cast<uint32_t*>(m_somedata));
				m_state = st_get_packet | st_init;
				bRes = true;
			}
			break;
		default:
			SetErrorCode(e_err_UnknownError);
			bRes =false;
			dstream3 << "Prepare of buffer failed!!! (PreparePacket() Unknown state.)";
		}
	}
	else
	{
		switch(m_state)
		{
		case st_start | st_init:
			//получить размер сертификата подготовить буффер в 4 байта
			*buf = new char[sizeof(uint32_t)];
			*size = sizeof(uint32_t);
			m_state = st_get_cert_size | st_init;
			bRes = true;
			break;
		case st_get_cert_size | st_init:
			// получить сам сертификат
			if(sizeof(uint32_t)!=m_size_somedata)
			{
				SetErrorCode(e_err_UnknownError);
				bRes = false;
			}
			else
			{
				*buf = new char[*(reinterpret_cast<uint32_t*>(m_somedata))];
				*size = *(reinterpret_cast<uint32_t*>(m_somedata));
				m_state = st_get_cert | st_init;
				bRes = true;
			}
			break;
		case st_get_cert | st_init:
			// получить кол-во поддерживаемых алгоритмов
			*buf = new char[sizeof(uint32_t)];
			*size = sizeof(uint32_t);
			m_state = st_get_packet_size | st_init;
			bRes = true;
			break;
		case st_get_packet_size | st_init:
			// получить получить массив алгоритмов
			if(sizeof(uint32_t)!=m_size_somedata)
			{
				SetErrorCode(e_err_UnknownError);
				bRes = false;
			}
			else
			{
				*buf = new char[*(reinterpret_cast<uint32_t*>(m_somedata)) * sizeof(VS_SymmetricAlg)];
				*size = *(reinterpret_cast<uint32_t*>(m_somedata)) * sizeof(VS_SymmetricAlg);
				m_state = st_get_packet | st_init;
				bRes = true;
			}
			break;
		case st_get_packet | st_init:
			// подготовить буффер с ключами для отправки
			if(m_size_somedata<=(sizeof(VS_SymmetricCrypt*)*2))
			{
				SetErrorCode(e_err_BufSizeExpectedValue);
				bRes = false;
			}
			else
			{
				*buf = new char[m_size_somedata - sizeof(VS_ReadWriteCrypt)];
				*size = m_size_somedata - sizeof(VS_ReadWriteCrypt);
				memcpy(*buf, (m_somedata) + sizeof(VS_ReadWriteCrypt), m_size_somedata-sizeof(VS_ReadWriteCrypt));
				memset((m_somedata) + sizeof(VS_ReadWriteCrypt), 0, m_size_somedata-sizeof(VS_ReadWriteCrypt));
				bRes = true;
				//m_state = st_send_result | st_init;
				m_state = st_finish | st_init;
			}
			break;
		case st_send_err | st_init:
			*buf = new char[m_size_somedata];
			memcpy(*buf, m_somedata, m_size_somedata);
			m_state = st_handshake_err | st_init;
			*size = m_size_somedata;
			bRes = true;
			break;
		default:
			SetErrorCode(e_err_UnknownError);
			bRes = false;
		}
	}
	return bRes;
}
void VS_SecureHandshakeVer1_Impl::FreePacketBuf(void **buf) const
{
	delete[] reinterpret_cast<char*>(*buf);
	*buf = 0;
}
bool VS_SecureHandshakeVer1_Impl::ProcessPacket(const void *buf, const uint32_t size)
{
	bool bRes = false;

	if(!(m_state&st_init))
	{

		dstream3 << "Process received packet failed. (ProcessPacket () Class is not init. Developers error.)";
		return false;
	}
	if(IsServerHandshake())
	{
		switch(m_state)
		{
		case st_get_packet_size | st_init:
			bRes = ServerProcPacketSize(buf, size);
			break;
		case st_get_packet | st_init:
			//Надо подготовить VS_Encrypt бла-бла
			bRes = ServerProcPacket(buf, size);
			break;
		default:
			bRes = false;
		}
	}
	else
	{
		switch(m_state)
		{
		case st_get_cert_size | st_init:
			bRes = ClientProcCertSize(buf, size);
			break;
		case st_get_cert | st_init:
			bRes = ClientProcCert(buf, size);
			break;
		case st_get_packet_size | st_init:
			bRes = ClientProcPacketSize(buf, size);
			break;
		case st_get_packet | st_init:
			//получили список алгоритмов, надо выбрать алгоритм и сгенерить для него ключи, также сформировать структуры
			bRes = ClientProcPacket(buf, size);
			break;
		default:
			SetErrorCode(e_err_UnknownError);
			bRes = false;
		}
	}
	return bRes;
}


bool VS_SecureHandshakeVer1_Impl::ServerProcPacketSize(const void *buf, const uint32_t size)
{
	if((m_size_somedata)||(size!=sizeof(uint32_t)))
	{
		SetErrorCode(e_err_BufSizeExpectedValue);
		dstream3 << "Process received packet size failed. (ServerProcPacketSize () size =" << size
			<< "; expected value = 4)\n";
		return false;
	}
	else
	{
		if(*(static_cast<const uint32_t*>(buf))>65536)
		{
			SetErrorCode(e_err_BufSizeExpectedValue);
			dstream3 << "Process received packet size failed. (ServerProcPacketSize () Very big size)";
			return false;
		}
		m_somedata = new char[sizeof(uint32_t)];
		m_size_somedata = size;
		memcpy(m_somedata, buf, size);

		return true;
	}
}
bool VS_SecureHandshakeVer1_Impl::ServerProcPacket(const void *buf, const uint32_t size)
{
//_CrtMemState s1, s2, s3;
	const size_t symmetricAlgSizeDefault = 4;
	bool bRes(true);
	const char *Pos = static_cast<const char*>(buf);
	if(size!=*(reinterpret_cast<uint32_t*>(m_somedata)))
	{
		SetErrorCode(e_err_BufSizeExpectedValue);
		dstream3 << "Process received Packet failed. (ServerProcPacket() size of received packet is not valid)";
		bRes = false;
	}
	else
	{
		VS_SymmetricAlg		alg(VS_SymmetricAlg::alg_sym_NONE);
		VS_SymmetricCrypt	*pReadSymmetricCrypt(0), *pWriteSymmetricCrypt(0);



		VS_PKeyCrypt		crypt;
		if(size == sizeof(VS_SecureHandshakeError))
			return ServerProcErrorMessage(buf, size);
			//клиент прислал ошибку, надо обработать ее и закончить хендшейк.
		if (!m_private_key_buf||
			!crypt.SetPrivateKey(m_private_key_buf, store_PEM_BUF))
		{
			SetErrorCode(e_err_UnknownError);
			bRes = false;
			dstream3 << "Process received Packet failed. (ServerProcPacket() Developers error)";
		}
		else
		{
			const unsigned char	*sesskey(0);
			uint32_t lensesskey(0);
			uint32_t encrlen(0);
			uint32_t decrlen(0);
			unsigned char iv[16];
			auto curBytePos = symmetricAlgSizeDefault + sizeof(uint32_t);
			if(size < curBytePos)
			{
				SetErrorCode(e_err_BufSizeExpectedValue);
				dstream3 << "Process received Packet failed. (ServerProcPacket() size of the packet less then minimum. #1";
				bRes = false;
			}
			if(bRes)
			{
				alg = *(reinterpret_cast<const VS_SymmetricAlg*>(Pos));         Pos += symmetricAlgSizeDefault;
				lensesskey = *(reinterpret_cast<const uint32_t*>(Pos));         Pos += sizeof(uint32_t);

				curBytePos += lensesskey;
				if(size < curBytePos)
				{
					SetErrorCode(e_err_BufSizeExpectedValue);
					dstream3 << "Process received Packet failed. (ServerProcPacket() size of the packet less then minimum. #2";
					bRes = false;
				}
			}
			if(bRes)
			{
				sesskey = reinterpret_cast<const unsigned char*>(Pos);          Pos += lensesskey;
				curBytePos += sizeof(iv);
				if(size < curBytePos)
				{
					SetErrorCode(e_err_BufSizeExpectedValue);
					dstream3 << "Process received Packet failed. (ServerProcPacket() size of the packet less then minimum. #3";
					bRes = false;
				}
			}
			if(bRes)
			{
				memcpy(iv, Pos, sizeof(iv));				Pos += sizeof(iv);
				encrlen = size - sizeof(VS_SymmetricAlg) - sizeof(uint32_t) -
						lensesskey - sizeof(iv);
				curBytePos += encrlen;
				if(size<curBytePos)
				{
					SetErrorCode(e_err_BufSizeExpectedValue);
					dstream3 << "Process received Packet failed. (ServerProcPacket() size of the packet less then minimum. #4";
					bRes = false;
				}
			}
			if(bRes)
			{
				unsigned char		*decrdata(0);
				auto encrbuf = reinterpret_cast<const unsigned char*>(Pos);
				crypt.Decrypt(encrbuf, encrlen, alg, iv, sesskey, lensesskey, decrdata, &decrlen);
				decrdata = new unsigned char [decrlen];
				if(!crypt.Decrypt(encrbuf, encrlen, alg, iv, sesskey, lensesskey, decrdata, &decrlen))
				{
					SetErrorCode(e_err_DecryptKeys);
					dstream3 << "Process received Packet failed. (ServerProcPacket() Decrypt clients packet failed";
					bRes = false;
				}
				else
				{
					uint32_t		ReadKeyLen(0);
					uint32_t		WriteKeyLen(0);
					const unsigned char		*ReadKey(0), *WriteKey(0);

					curBytePos = symmetricAlgSizeDefault + sizeof(uint32_t);
					if(curBytePos>decrlen)
					{
						SetErrorCode(e_err_BufSizeExpectedValue);
						dstream3 << "Process received Packet failed. (ServerProcPacket() size of the decrypt data less then minimum. #5";
						bRes = false;
					}
					if(bRes)
					{
						Pos = reinterpret_cast<char*>(decrdata);
						alg = *(reinterpret_cast<const VS_SymmetricAlg*>(Pos));   Pos += symmetricAlgSizeDefault;
						ReadKeyLen = *(reinterpret_cast<const uint32_t*>(Pos));   Pos += sizeof(uint32_t);
						curBytePos += ReadKeyLen + sizeof(uint32_t);
						if(curBytePos>decrlen)
						{
							SetErrorCode(e_err_BufSizeExpectedValue);
							dstream3 << "Process received Packet failed. (ServerProcPacket() size of the decrypt data less then minimum. #6";
							bRes = false;
						}
					}
					if(bRes)
					{
						ReadKey = reinterpret_cast<const unsigned char*>(Pos);			Pos +=ReadKeyLen;
						WriteKeyLen = *(reinterpret_cast<const uint32_t*>(Pos));	Pos += sizeof(uint32_t);
						curBytePos += WriteKeyLen;
						if(curBytePos>decrlen)
						{
							SetErrorCode(e_err_BufSizeExpectedValue);
							dstream3 << "Process received Packet failed. (ServerProcPacket() size of the decrypt data less then minimum. #7";
							bRes = false;
						}
					}
					if(bRes)
					{
						WriteKey = reinterpret_cast<const unsigned char*>(Pos);			Pos +=WriteKeyLen;
						pReadSymmetricCrypt = new VS_SymmetricCrypt;
						pWriteSymmetricCrypt = new VS_SymmetricCrypt;
						if((!pReadSymmetricCrypt->Init(alg, alg != alg_sym_GOST ? mode_ECB : mode_CFB))||
							(!pReadSymmetricCrypt->SetKey(ReadKeyLen, ReadKey))		||
							(!pWriteSymmetricCrypt->Init(alg, alg != alg_sym_GOST ? mode_ECB : mode_CFB))||
							(!pWriteSymmetricCrypt->SetKey(WriteKeyLen, WriteKey)))
						{
							SetErrorCode(e_err_UnknownError);
							delete pReadSymmetricCrypt;
							delete pWriteSymmetricCrypt;
							bRes = false;
							dstream3 << "Process received Packet failed. (ServerProcPacket() Set key material failed. Developers error";
						}
						else
						{

							if(m_size_somedata)
							{
								delete [] m_somedata;
								m_size_somedata = 0;
							}
							VS_ReadWriteCrypt pRWCrypt;
							pRWCrypt.pReadCrypt = pReadSymmetricCrypt;
							pRWCrypt.pWriteCrypt = pWriteSymmetricCrypt;
							m_somedata = new char[sizeof(VS_ReadWriteCrypt)];
							m_size_somedata = sizeof(VS_ReadWriteCrypt);
							*(reinterpret_cast<VS_ReadWriteCrypt*>(m_somedata)) = pRWCrypt;
							m_state = st_finish | st_init;
							bRes = true;
						}
					}
				}
				delete [] decrdata;
			}
		}
	}

	return bRes;
}

bool VS_SecureHandshakeVer1_Impl::ServerProcErrorMessage(const void *buf, const uint32_t sz)
{
	if(sz!=sizeof(VS_SecureHandshakeError))
		return false;
	VS_SecureHandshakeError	err;
	memcpy(&err, buf, sizeof(err));
	switch(err)
	{
	case e_err_SymmetricCryptAlgIsNotSupport:
		dstream2 << "ClientMsg: Alg Support error";
		break;
	case e_err_VerificationFailed:
		dstream2 << "ClientMsg: Certificate Verification failed";
		break;
	case e_err_DataAreNotCertificate:
		dstream2 << "ClientMsg: Data is not certificate";
		break;
	default:
		dstream2 << "ClientMsg: Unknown Error";
		break;
	}
	return true;
}
bool VS_SecureHandshakeVer1_Impl::ClientProcCertSize(const void *buf, const uint32_t size)
{
	if((m_size_somedata)||(size!=sizeof(uint32_t)))
	{
		if(m_size_somedata)
		{
			PrepareClientErrorMessage(e_err_UnknownError);
			SetErrorCode(e_err_UnknownError);
		}
		else
		{
			PrepareClientErrorMessage(e_err_BufSizeExpectedValue);
			SetErrorCode(e_err_BufSizeExpectedValue);
		}
		return true;
	}
	else
	{
		m_somedata = new char[sizeof(uint32_t)];
		m_size_somedata = sizeof(uint32_t);
		memcpy(m_somedata, buf, size);//Размер данных
		return true;
	}
}
bool VS_SecureHandshakeVer1_Impl::ClientProcCert(const void *buf, const uint32_t size)
{
	VS_GET_PEM_CACERT
	bool bRes(false);
	if(size!=*(reinterpret_cast<uint32_t*>(m_somedata)))
	{
		PrepareClientErrorMessage(e_err_BufSizeExpectedValue);
		SetErrorCode(e_err_BufSizeExpectedValue);
		bRes = true;
	}
	else
	{
		if(m_size_somedata)
		{
			delete [] m_somedata;
			m_size_somedata = 0;
		}
		//1. Проверить сертификат
		VS_CertificateCheck	certcheck;
		VS_Certificate		cert;
		bool init_res(true), addres(false), setcertres(false), verifyres(false);

		VS_Container	certs_cnt;
		if(!certs_cnt.Deserialize(buf, size))
			init_res = false;
		certs_cnt.Reset();
		size_t sz(0);
		while(init_res && certs_cnt.Next())
		{
			if (!strcasecmp(certs_cnt.GetName(), "Certificate Chain"))
			{
				const char* cert_chain_buf = reinterpret_cast<const char*>(certs_cnt.GetBinValueRef(sz));
				certcheck.SetCertToChain(cert_chain_buf, sz, store_PEM_BUF);
			}
			else if (!strcasecmp(certs_cnt.GetName(), "Certificate"))
			{
				const char *cert_buf = reinterpret_cast<const char*>(certs_cnt.GetBinValueRef(sz));
				setcertres = certcheck.SetCert(cert_buf, sz, store_PEM_BUF);
			}
			sz = 0;
		}
		const char *cert_buf = reinterpret_cast<const char *>( certs_cnt.GetBinValueRef("Certificate", sz));
		if((!init_res) ||
			(!(addres = certcheck.SetCertToChain(PEM_CACERT, strlen(PEM_CACERT)+1, store_PEM_BUF)))||
			(!(setcertres/* = certcheck.SetCert((const char *)buf, store_PEM_BUF)*/))||
			(!(verifyres = certcheck.VerifyCert())))
		{
#ifdef _DEBUG
			size_t _sz = 0;
			const char *certChainBuf = reinterpret_cast<const char *>(certs_cnt.GetBinValueRef("Certificate Chain", _sz));
			if (sizes.find(_sz) == sizes.end())
			{
				std::ofstream failedCert(std::string("cert_") + std::to_string(_sz),
					std::ios_base::out |
					std::ios_base::trunc |
					std::ios_base::binary);
				if (!failedCert.is_open())
					std::cout << "REEEEE" << std::endl;
				failedCert.write(certChainBuf, _sz);
				failedCert.close();
				sizes.insert(_sz);
			}
#endif
			if((!init_res)||(!addres))
			{
				SetErrorCode(e_err_UnknownError);
				PrepareClientErrorMessage(e_err_UnknownError);
			}
			else if(!setcertres)
			{
				SetErrorCode(e_err_DataAreNotCertificate);
				PrepareClientErrorMessage(e_err_DataAreNotCertificate);
			}
			else if(!verifyres)
			{
				SetErrorCode(e_err_VerificationFailed);
				PrepareClientErrorMessage(e_err_VerificationFailed);
				if(m_cert_pem_buf)
					delete [] m_cert_pem_buf;

				size_t cert_sz = 0;
				const void *cert_buf = certs_cnt.GetBinValueRef("Certificate", cert_sz);
				m_cert_buf_len = cert_sz;

				m_cert_pem_buf = new char[m_cert_buf_len];
				memcpy(m_cert_pem_buf, cert_buf, m_cert_buf_len);
			}
			bRes = true;
		}
		else if ((!cert.SetCert(cert_buf, sz, store_PEM_BUF))||
				(!cert.GetCertPublicKey(&m_pub_key)))
		{
			SetErrorCode(e_err_UnknownError);
			PrepareClientErrorMessage(e_err_UnknownError);
			bRes = true;
		}
		else
		{
			if(m_cert_pem_buf)
				delete [] m_cert_pem_buf;

			size_t cert_sz = 0;
			const void *cert_buf = certs_cnt.GetBinValueRef("Certificate", cert_sz);
			m_cert_buf_len = cert_sz;

			m_cert_pem_buf = new char[m_cert_buf_len];
			memcpy(m_cert_pem_buf, cert_buf, m_cert_buf_len);
			bRes = true;
		}
	}
	return bRes;
}
bool VS_SecureHandshakeVer1_Impl::ClientProcPacketSize(const void *buf, const uint32_t size)
{
	if(size!=sizeof(uint32_t))
	{
		PrepareClientErrorMessage(e_err_BufSizeExpectedValue);
		SetErrorCode(e_err_BufSizeExpectedValue);
		return false;
	}
	else
	{
		if(m_size_somedata)
		{
			delete [] m_somedata;
			m_size_somedata = 0;
		}
		m_somedata = new char[sizeof(uint32_t)];
		m_size_somedata = sizeof(uint32_t);
		memcpy(m_somedata, buf, size);
		return true;
	}
}
bool VS_SecureHandshakeVer1_Impl::ClientProcPacket(const void *buf, const uint32_t size)
{
	bool bRes(true);
	if(size<sizeof(VS_SymmetricAlg))
	{
		PrepareClientErrorMessage(e_err_BufSizeExpectedValue);
		SetErrorCode(e_err_BufSizeExpectedValue);
		bRes = true;
	}
	else
	{
		uint32_t	ArrSize = *(reinterpret_cast<uint32_t*>(m_somedata));
		if((ArrSize*sizeof(VS_SymmetricAlg))!=size)
		{
			PrepareClientErrorMessage(e_err_BufSizeExpectedValue);
			SetErrorCode(e_err_BufSizeExpectedValue);
			bRes = true;
		}else
		{
			VS_SymmetricAlg* AlgArr(0);
			VS_SymmetricAlg  CurrAlg(alg_sym_NONE);
			AlgArr = new VS_SymmetricAlg[ArrSize];
			memcpy(AlgArr, buf, size);
			for(uint32_t i =0;i<ArrSize;i++)
			{
				if (m_GostSupported && AlgArr[i] == alg_sym_GOST)
				{
					CurrAlg = alg_sym_GOST;
					break;
				}
				else if (AlgArr[i] == alg_sym_AES256)
				{
					CurrAlg = alg_sym_AES256;
					if (!m_GostSupported)
						break;
				}
				else if(AlgArr[i] == alg_sym_AES128 && CurrAlg == alg_sym_NONE)
					CurrAlg = alg_sym_AES128;
			}
			delete [] AlgArr;

			if(CurrAlg != alg_sym_NONE)
			{
				VS_SymmetricCrypt* pWriteCrypt(0), *pReadCrypt(0);
				const uint32_t ReadKeyLen(CurrAlg == alg_sym_AES128 ? 16 : 32), WriteKeyLen(ReadKeyLen); //Длина ключа
				unsigned char* pReadKey(0), *pWriteKey(0);
				VS_ReadWriteCrypt RWCrypt;

				pWriteCrypt = new VS_SymmetricCrypt;
				pReadCrypt = new VS_SymmetricCrypt;
				RWCrypt.pReadCrypt = pReadCrypt;
				RWCrypt.pWriteCrypt = pWriteCrypt;
				pReadKey = new unsigned char[ReadKeyLen];
				pWriteKey = new unsigned char[WriteKeyLen];


				if((!pWriteCrypt->Init(CurrAlg, CurrAlg != alg_sym_GOST ? mode_ECB : mode_CFB))||
					(!pReadCrypt->Init(CurrAlg, CurrAlg != alg_sym_GOST ? mode_ECB : mode_CFB))||
					(!pWriteCrypt->GenerateKey(ReadKeyLen, pReadKey))		||
					(!pReadCrypt->GenerateKey(WriteKeyLen, pWriteKey))		||
					(!pWriteCrypt->SetKey(ReadKeyLen, pReadKey))				||
					(!pReadCrypt->SetKey(WriteKeyLen, pWriteKey)))
				{
					PrepareClientErrorMessage(e_err_UnknownError);
					SetErrorCode(e_err_UnknownError);
					bRes = true;
				}
				else
				{
					if(m_size_somedata)
					{
						delete [] m_somedata;
						m_size_somedata = 0;
					}
					//1. Получить длинну для зашифрованного ключа
					//2. получить размер зашифрованных данных

					VS_PKeyCrypt	crypt;
					if (!crypt.SetPublicKey(&m_pub_key))
					{
						PrepareClientErrorMessage(e_err_UnknownError);
						SetErrorCode(e_err_UnknownError);
						bRes = true;
					}
					else
					{
						unsigned char *sesskey(0);
						uint32_t lensesskey(0);
						unsigned char iv[16];
						unsigned char *encrbuf(0);
						uint32_t encrlen(0);
						auto datalen = sizeof(VS_SymmetricAlg) + sizeof(ReadKeyLen) +
								ReadKeyLen + sizeof(WriteKeyLen) + WriteKeyLen;
						auto data = new unsigned char[datalen];
						auto Pos = reinterpret_cast<char*>(data);
						*(reinterpret_cast<VS_SymmetricAlg*>(Pos)) = CurrAlg;				Pos += sizeof(VS_SymmetricAlg);
						*(reinterpret_cast<uint32_t*>(Pos)) = ReadKeyLen;			Pos += sizeof(ReadKeyLen);
						memcpy(Pos, pReadKey, ReadKeyLen);				Pos += ReadKeyLen;
						*(reinterpret_cast<uint32_t*>(Pos)) = WriteKeyLen;			Pos += sizeof(WriteKeyLen);
						memcpy(Pos, pWriteKey, WriteKeyLen);
						crypt.Encrypt(data, datalen, alg_sym_AES256, encrbuf, &encrlen,
										iv, sesskey, &lensesskey);
						sesskey = new unsigned char[lensesskey];
						crypt.Encrypt(data, datalen, alg_sym_AES256, encrbuf, &encrlen,
										iv, sesskey, &lensesskey);
						encrbuf = new unsigned char[encrlen];
						if(!crypt.Encrypt(data, datalen, alg_sym_AES256, encrbuf, &encrlen,
											iv, sesskey, &lensesskey))
						{
							PrepareClientErrorMessage(e_err_UnknownError);
							SetErrorCode(e_err_UnknownError);
							bRes = true;
						}
						else
						{
							uint32_t PackSize = sizeof(PackSize) + sizeof(VS_SymmetricAlg) + sizeof(lensesskey) +
													lensesskey + sizeof(iv) + encrlen;
							m_size_somedata = PackSize + sizeof(VS_ReadWriteCrypt);
							m_somedata = new char[m_size_somedata];
							Pos = m_somedata;
							*(reinterpret_cast<VS_ReadWriteCrypt*>(m_somedata)) = RWCrypt;			Pos += sizeof(VS_ReadWriteCrypt);
							*(reinterpret_cast<uint32_t *>(Pos)) = PackSize - sizeof(PackSize);	Pos += sizeof(uint32_t);
							*(reinterpret_cast<VS_SymmetricAlg*>(Pos)) = alg_sym_AES128;				Pos += sizeof(VS_SymmetricAlg); // alg type AES128 is for backward compatibility
							*(reinterpret_cast<uint32_t*>(Pos)) = lensesskey;					Pos += sizeof(lensesskey);
							memcpy(Pos, sesskey, lensesskey);							Pos += lensesskey;
							memcpy(Pos, iv, sizeof(iv));								Pos += sizeof(iv);
							memcpy(Pos, encrbuf, encrlen);
							bRes = true;
						}
						delete [] sesskey;
						delete [] data;
						delete [] encrbuf;

					}
				}
				delete [] pReadKey;
				delete [] pWriteKey;
			}
			else
			{
				PrepareClientErrorMessage(e_err_SymmetricCryptAlgIsNotSupport);
				SetErrorCode(e_err_SymmetricCryptAlgIsNotSupport);
				bRes = false;
			}
		}
	}
	return bRes;
}
VS_SymmetricCrypt* VS_SecureHandshakeVer1_Impl::GetReadSymmetricCrypt()
{
	VS_SymmetricCrypt *pRes(0);
	if((m_state!= (st_finish | st_init))||
	   (sizeof(VS_ReadWriteCrypt) > m_size_somedata))
	{
		dstream3 << "GetWriteSymmetricCrypt failed. ReadSymetricCrypt is not constructed";
		return 0;
	}
	VS_ReadWriteCrypt RWCrypt;
	RWCrypt = *(reinterpret_cast<VS_ReadWriteCrypt*>(m_somedata));
	pRes = RWCrypt.pReadCrypt;
	if(!RWCrypt.pWriteCrypt)
	{
		delete [] m_somedata;
		m_size_somedata = 0;
	}
	else
	{
		RWCrypt.pReadCrypt = 0;
		*(reinterpret_cast<VS_ReadWriteCrypt*>(m_somedata)) = RWCrypt;

	}
	if(!pRes)
		dstream3 << "GetReadSymmetricCrypt failed. Handshake ReadCrypt is empty. Developers error.";
	return pRes;
}

VS_SymmetricCrypt* VS_SecureHandshakeVer1_Impl::GetWriteSymmetricCrypt()
{
	VS_SymmetricCrypt *pRes(0);
	if((m_state!= (st_finish | st_init)) ||
       (sizeof(VS_SymmetricCrypt*)*2 > m_size_somedata))
	{
		dstream3 << "GetWriteSymmetricCrypt failed. WriteSymetricCrypt is not constructed";
		return 0;
	}

	VS_ReadWriteCrypt RWCrypt;
	RWCrypt = *(reinterpret_cast<VS_ReadWriteCrypt*>(m_somedata));
	pRes = RWCrypt.pWriteCrypt;
	if(!RWCrypt.pReadCrypt)
	{
		delete [] m_somedata;
		m_size_somedata = 0;
	}
	else
	{
		RWCrypt.pWriteCrypt = 0;
		*(reinterpret_cast<VS_ReadWriteCrypt*>(m_somedata)) = RWCrypt;
	}
	if(!pRes)
		dstream3 << "GetWriteSymmetricCrypt failed. Handshake WriteCrypt is empty. Developers error.";
	return pRes;
}
bool VS_SecureHandshakeVer1_Impl::SetPrivateKey(VS_PKey *pkey)
{
	uint32_t	sz(0);
	if(!pkey)
		return false;
	pkey->GetPrivateKey(store_PEM_BUF, 0, &sz);
	if(!sz)
		return false;
	if(m_private_key_buf)
	{
		delete [] m_private_key_buf;
		m_private_key_buf = 0;
	}
	m_private_key_buf = new char[sz];
	if(!pkey->GetPrivateKey(store_PEM_BUF, m_private_key_buf, &sz))
	{
		delete [] m_private_key_buf;
		m_private_key_buf = 0;
		return false;
	}
	return true;
}
std::shared_ptr<VS_Certificate> VS_SecureHandshakeVer1_Impl::CloneCertificate() const
{
    if (!m_cert_pem_buf || !m_cert_buf_len) {
		dstream3 << "No certificate: " << m_cert_pem_buf << m_cert_buf_len;
        return std::shared_ptr<VS_Certificate>();
    }

    auto cert = std::make_shared<VS_Certificate>();
    if (!cert->SetCert(m_cert_pem_buf, m_cert_buf_len, store_PEM_BUF)) {
		dstream3 << "Error in SetCert";
        cert.reset();
    }
    return cert;
}
bool VS_SecureHandshakeVer1_Impl::GetCertificate(char *buf, uint32_t &sz)
{
	bool res(false);
	if((!m_cert_pem_buf)||(!m_cert_buf_len))
		res = false;
	else if(m_cert_buf_len > sz)
	{
		sz = m_cert_buf_len;
		res = false;
	}
	else
	{
		memcpy(buf, m_cert_pem_buf, sz);
		sz = m_cert_buf_len;
		res = true;
	}
	return res;
}
void VS_SecureHandshakeVer1_Impl::ReleaseSymmetricCrypt(VS_SymmetricCrypt *crypt) const
{
	delete crypt;
}

bool VS_SecureHandshakeVer2_Impl::ServerProcPacket(const void *buf, const uint32_t size)
{
	if(!m_cert_buf_len)
	{
		bool res = ClientProcCert(buf, size);
		if (res)
			m_state = st_send_alg | st_init; //получить размер пакета
		return res;
	}
	else
	{
		/**
			проверить подпись буфера с помощью сертификата
			буфер это контейнер где ->	buf_ver1
										sign(buf_ver1+m_rnd);
		*/
		char	*pub_key_buf(0);
		uint32_t pub_k_s(0);
		VS_Container	cnt;
		VS_Sign			verifier;
		VS_SignArg		signarg = {alg_pk_RSA, alg_hsh_SHA1};
		VS_Certificate	cert;
		VS_PKey			cert_pub_key;
		cert.SetCert(m_cert_pem_buf, m_cert_buf_len, store_PEM_BUF);
		cert.GetCertPublicKey(&cert_pub_key);

		cert_pub_key.GetPublicKey(store_PEM_BUF, pub_key_buf, &pub_k_s);

		if(!pub_k_s || !(*m_rnd) || !m_somedata ||
			(m_size_somedata != sizeof(uint32_t)))
		{
			SetErrorCode(e_err_UnknownError);
			return false;
		}
		pub_key_buf = new char[pub_k_s];
		if(!cert_pub_key.GetPublicKey(store_PEM_BUF, pub_key_buf, &pub_k_s))
		{
			delete [] pub_key_buf;
			SetErrorCode(e_err_UnknownError);
			return false;
		}
		if(!m_somedata || (m_size_somedata != sizeof(uint32_t)) ||
			!cnt.Deserialize(buf, size) || !verifier.Init(signarg) ||
			!verifier.SetPublicKey(pub_key_buf, pub_k_s, store_PEM_BUF))
		{
			delete [] pub_key_buf;
			SetErrorCode(e_err_UnknownError);
			return false;
		}
		delete [] pub_key_buf;

		size_t sz_v1(0);
		const void *buf_v1(0);
		const void *sign(0);
		size_t sign_sz(0);
		uint32_t signed_buf_sz(0);
		void *signed_buf(0);


		buf_v1 = cnt.GetBinValueRef("v1_buf", sz_v1);
		sign = cnt.GetBinValueRef("sign", sign_sz);
		if(!buf_v1 || !sign)
		{
			SetErrorCode(e_err_UnknownError);
			return false;
		}

		signed_buf = new char [signed_buf_sz = sz_v1 + strlen(m_rnd)];
		memcpy(signed_buf, buf_v1, sz_v1);
		memcpy(static_cast<char*>(signed_buf)+sz_v1, m_rnd, strlen(m_rnd));
		if(!verifier.VerifySign(static_cast<unsigned char*>(signed_buf), signed_buf_sz, static_cast<const unsigned char*>(sign), sign_sz))
		{
			SetErrorCode(e_err_VerificationFailed);
			delete[] reinterpret_cast<char*>(signed_buf);
			return false;
		}
		delete [] reinterpret_cast<char*>(signed_buf);
		*reinterpret_cast<uint32_t *>(m_somedata) = *static_cast<const uint32_t *>(buf_v1);
		return VS_SecureHandshakeVer1_Impl::ServerProcPacket(
			reinterpret_cast<const char*>(buf_v1) + sizeof(uint32_t), sz_v1 - sizeof(uint32_t));
	}
}
//bool VS_SecureHandshakeVer2_Impl::ClientProcPacket

VS_SecureHandshakeState	VS_SecureHandshakeVer2_Impl::Next()
{
	if(!(m_state&st_init))
		return secure_st_Error;
	if(IsServerHandshake())
		return VS_SecureHandshakeVer1_Impl::Next();
	else
	{
		switch(m_state)
		{
		case st_start|st_init:
			//послать сертификат, переключить st
			m_state = st_send_cert | st_init;
			return secure_st_SendCert;
			break;
		case st_send_cert | st_init:
			m_state = st_start|st_init;
			return VS_SecureHandshakeVer1_Impl::Next();
		default:
			return VS_SecureHandshakeVer1_Impl::Next();
		};
		/**
		сделать почти как у сервера
		1. Отправить сертификат
		2 далее, как у клиента
		*/
	}
}

bool VS_SecureHandshakeVer2_Impl::PreparePacket(void **buf, uint32_t *size)
{
	if(IsServerHandshake())
	{
		/**
			сгенерить рандомный ключ и прибавить его к буферу, ключ запомнить
		*/
		void *buf_ver1(0);
		uint32_t sz_ver1(0);
		bool b = m_state == (st_send_cert | st_init);
		if(VS_SecureHandshakeVer1_Impl::PreparePacket(b?&buf_ver1:buf, b?&sz_ver1:size))
		{
			if(b)
			{
				VS_Container cnt;
				memset(m_rnd, 0, 256);
//				Filling m_rnd with 32 random bytes
				std::random_device device;
				std::uniform_int_distribution<uint32_t> dist;
				for (unsigned int i = 0; i < 8; ++i)
					reinterpret_cast<uint32_t*>(m_rnd)[i] = dist(device);
				cnt.AddValue("v1_buf", buf_ver1, sz_ver1);
				cnt.AddValue("v2rnd", m_rnd);
				FreePacketBuf(&buf_ver1);
				sz_ver1 = 0;
				size_t sz_ver1_tmp;
				if(cnt.SerializeAlloc(buf_ver1, sz_ver1_tmp))
				{
					sz_ver1 = sz_ver1_tmp;
					*buf = new char[sz_ver1 + sizeof(uint32_t)];
					**(reinterpret_cast<uint32_t**>(buf)) = sz_ver1;
					memcpy(*reinterpret_cast<char**>(buf)+sizeof(uint32_t), buf_ver1, sz_ver1);
					*size = sz_ver1 + sizeof(uint32_t);
					free(buf_ver1);
					return true;
				}
				else
				{
					SetErrorCode(e_err_UnknownError);
					return false;
				}
			}
			return true;
		}
		else
			return false;
	}
	else
	{
		switch(m_state)
		{
		case st_get_packet_size | st_init:
			// получить получить массив алгоритмов
			if(sizeof(uint32_t)!=m_size_somedata)
			{
				SetErrorCode(e_err_UnknownError);
				return false;
			}
			else
			{
				*buf = new char[*(reinterpret_cast<uint32_t*>(m_somedata))];
				*size = *(reinterpret_cast<uint32_t*>(m_somedata));
				m_state = st_get_packet | st_init;
				return true;
			}
			break;
		case st_get_packet | st_init:
			{
				void *buf_v1(0);
				uint32_t sz_b_v1(0);
				void		*buf_for_sign(0);

				VS_Sign		signer;
				VS_SignArg	signarg = {alg_pk_RSA, alg_hsh_SHA1};
				unsigned char sign[VS_SIGN_SIZE] = {0};

				bool bRes(false);
				do
				{
					if(!m_private_key_buf || !signer.Init(signarg) ||
						!signer.SetPrivateKey(m_private_key_buf, store_PEM_BUF, 0) || !(*m_rnd))
					{
						SetErrorCode(e_err_UnknownError);
						break;
					}
					if(VS_SecureHandshakeVer1_Impl::PreparePacket(&buf_v1, &sz_b_v1))
					{
						uint32_t sign_buf_s(0);
						buf_for_sign = new char [sign_buf_s = sz_b_v1 + strlen(m_rnd)];
						memcpy(buf_for_sign, buf_v1, sz_b_v1);
						memcpy(static_cast<char*>(buf_for_sign)+sz_b_v1, m_rnd, strlen(m_rnd));
						uint32_t sign_sz = VS_SIGN_SIZE;
						if(!signer.SignData(static_cast<unsigned char*>(buf_for_sign), sign_buf_s, sign, &sign_sz))
						{
							SetErrorCode(e_err_UnknownError);
							break;
						}
						VS_Container cnt;
						cnt.AddValue("v1_buf", buf_v1, sz_b_v1);
						cnt.AddValue("sign", sign, sign_sz);
						void *buf_v2(0);
						size_t sz_v2(0);
						if(!cnt.SerializeAlloc(buf_v2, sz_v2))
						{
							SetErrorCode(e_err_UnknownError);
							break;
						}
						*buf = new char[sizeof(uint32_t) + sz_v2];
						*(reinterpret_cast<uint32_t*>(*buf)) = sz_v2;
						memcpy(static_cast<char*>(*buf)+sizeof(uint32_t), buf_v2, sz_v2);
						free(buf_v2);
						*size = sizeof(uint32_t) + sz_v2;
						bRes = true;
					}
				}
				while(false);
				if(buf_for_sign)
					delete [] reinterpret_cast<char*>(buf_for_sign);
				if(buf_v1)
					FreePacketBuf(&buf_v1);
				return bRes;
			}
			break;
		default:
			return VS_SecureHandshakeVer1_Impl::PreparePacket(buf, size);
		}
		/**
			правильно обработать пришедший пакет с алгоритмами и подписать rnd
		*/
	}
	//	return VS_SecureHandshakeVer1_Impl::PreparePacket(buf, size);
}

bool VS_SecureHandshakeVer2_Impl::ClientProcPacket(const void *buf, const uint32_t size)
{
	VS_Container	cnt;
	if(!cnt.Deserialize(buf, size)||!m_somedata || (m_size_somedata != sizeof(uint32_t)))
	{
		SetErrorCode(e_err_UnknownError);
		return false;
	}
	size_t sz_ver1(0);
	const void *buf_ver1 = cnt.GetBinValueRef("v1_buf", sz_ver1);
	const char* ch_rnd = cnt.GetStrValueRef("v2rnd");
	if(!buf_ver1 || !ch_rnd || strlen(ch_rnd)>256)
	{
		SetErrorCode(e_err_UnknownError);
		return false;
	}
	strcpy(m_rnd, ch_rnd);
	*reinterpret_cast<uint32_t*>(m_somedata) = *static_cast<const uint32_t *>(buf_ver1);
	return VS_SecureHandshakeVer1_Impl::ClientProcPacket(static_cast<const char*>(buf_ver1)+sizeof(uint32_t), sz_ver1-sizeof(uint32_t));
}