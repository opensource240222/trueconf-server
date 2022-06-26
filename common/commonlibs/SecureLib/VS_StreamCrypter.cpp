#include "VS_StreamCrypter.h"
#include "VS_SymmetricCrypt.h"

#include <cstring>
#include <stdio.h>

VS_StreamCrypter::VS_StreamCrypter() :m_crypter(nullptr), m_isInit(false)
{
}

VS_StreamCrypter::~VS_StreamCrypter()
{
	Free();
}

VS_StreamCrypter::VS_StreamCrypter(VS_StreamCrypter&& other):m_crypter(nullptr), m_isInit(false){
	m_crypter = other.m_crypter;
	other.m_crypter = nullptr;

	m_isInit = other.m_isInit;
	other.m_isInit = false;
}

VS_StreamCrypter& VS_StreamCrypter::operator=(VS_StreamCrypter&& other) {
    if (this != &other) {
        Free();

        m_crypter = other.m_crypter;
        other.m_crypter = nullptr;

        m_isInit = other.m_isInit;
        other.m_isInit = false;
    }

	return *this;
}

bool VS_StreamCrypter::Init(const unsigned char *key, uint32_t key_len)
{
	if(!key||!key_len)
		return false;
	Free();
	m_crypter = new VS_SymmetricCrypt;
	VS_SymmetricAlg alg = key_len == 16 ? alg_sym_AES128 : alg_sym_AES256;
	if(!m_crypter->Init(alg,mode_ECB) || !m_crypter->SetKey(key_len,key))
	{
		Free();
		return false;
	}
	else
	{
		m_isInit = true;
		return true;
	}
}

bool VS_StreamCrypter::Init(const char *key)
{
	if (!key || strlen(key) < 32)
		return false;
	const char *c = key;
	unsigned char buff[16];
	for (int i = 0; i<16; i++) {
		unsigned int val = 0;
		sscanf(c+2*i, "%02X", &val);
		buff[i] = val;
	}
	return Init(buff, 16);
}


void VS_StreamCrypter::Free()
{
	if(m_crypter)
	{
		delete m_crypter;
		m_crypter = 0;
	}
	m_isInit = false;
}

unsigned long VS_StreamCrypter::GetBlockSize() const
{
    return (m_isInit && m_crypter) ? m_crypter->GetBlockSize() : 0;
}

bool VS_StreamCrypter::Encrypt(const unsigned char *buf, uint32_t buf_sz, unsigned char *encr_buf, uint32_t *encr_buf_sz)
{
	if(!m_isInit || !m_crypter)
		return false;
	return m_crypter->Encrypt(buf,buf_sz,encr_buf,encr_buf_sz);
}

bool VS_StreamCrypter::Decrypt(const unsigned char *buf, uint32_t buf_sz, unsigned char *decr_buf, uint32_t *decr_buf_sz)
{
	if(!m_isInit || !m_crypter)
		return false;
	return m_crypter->Decrypt(buf,buf_sz,decr_buf,decr_buf_sz);
}