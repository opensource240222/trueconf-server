#include "VS_SymmetricCrypt.h"

#include "SecureLib/OpenSSLCompat/tc_evp.h"
#include "SecureLib/OpenSSLTypesWrapImpl.h"

#include <openssl/rand.h>
#include <cstring>
#include <vector>


VS_SymmetricKey::VS_SymmetricKey():m_key(0), m_keylen(0)
{}

VS_SymmetricKey::~VS_SymmetricKey()
{
	if((m_keylen)&&(m_key))
		delete [] m_key;
}

bool VS_SymmetricKey::Init()
{
	return true;
}
bool VS_SymmetricKey::GenerateKey(const uint32_t length, unsigned char *keybuf) const
{
	if((!length)||(length > VS_MAX_KEY_LENGTH))
		return false;
	RAND_bytes(keybuf, length);

	return true;
}

bool VS_SymmetricKey::CheckKeyStrength() const
{
	return true;
}

uint32_t VS_SymmetricKey::GetKeyLength() const
{
	return m_keylen;
}

bool VS_SymmetricKey::SetKey(const uint32_t length, const unsigned char *key)
{
	if((!length)||(!key))
		return false;
	ClearKey();
	m_key = new unsigned char [length];
	m_keylen = length;
	memcpy(m_key, key, length);
	return true;
}
bool VS_SymmetricKey::GetKey(const uint32_t buflen, unsigned char *keybuf, uint32_t *keylen) const
{
	if(buflen<m_keylen)
		return false;
	memcpy(keybuf, m_key, m_keylen);
	*keylen = m_keylen;
	return true;
}

void VS_SymmetricKey::ClearKey()
{
	if(m_keylen)
	{
		delete [] m_key;
		m_key = 0;
		m_keylen = 0;
	}
}

VS_SymmetricCrypt & VS_SymmetricCrypt::operator =(const VS_SymmetricCrypt &src)
{
	if (&src == this)
		return *this;
	if (!Init(src.GetAlg(), src.GetCipherMode()))
		return *this;
	uint32_t key_sz(GetKeyLen());
	uint32_t sz(key_sz);
	std::vector<unsigned char> key;
	key.resize(key_sz);
	if (src.GetKey(sz, &key[0], &key_sz))
		SetKey(key_sz, &key[0]);
	return *this;
}

VS_SymmetricCrypt::VS_SymmetricCrypt()
	: m_encrCTX(nullptr), m_decrCTX(nullptr), m_Alg(alg_sym_NONE), m_CipherMode(mode_NONE), m_state(st_none)
{}
VS_SymmetricCrypt::~VS_SymmetricCrypt()
{
	if(m_encrCTX.get())
	{
		EVP_CIPHER_CTX_cleanup(m_encrCTX.get());
		EVP_CIPHER_CTX_free(m_encrCTX.get());
	}
	if(m_decrCTX.get())
	{
		EVP_CIPHER_CTX_cleanup(m_decrCTX.get());
		EVP_CIPHER_CTX_free(m_decrCTX.get());
	}
}

bool VS_SymmetricCrypt::Init(const VS_SymmetricAlg alg, const VS_SymmetricCipherMode mode)
{
	bool bRes(false);
	if(!m_key.Init())
		return false;
	if(m_encrCTX.get())
	{
		EVP_CIPHER_CTX_cleanup(m_encrCTX.get());
		EVP_CIPHER_CTX_free(m_encrCTX.get());
	}
	if(m_decrCTX.get())
	{
		EVP_CIPHER_CTX_cleanup(m_decrCTX.get());
		EVP_CIPHER_CTX_free(m_decrCTX.get());
	}
	m_encrCTX.get() = EVP_CIPHER_CTX_new();
	m_decrCTX.get() = EVP_CIPHER_CTX_new();
	EVP_CIPHER_CTX_init(m_encrCTX.get());
	EVP_CIPHER_CTX_init(m_decrCTX.get());
	switch(alg)
	{
	case alg_sym_AES128:
		if (mode == mode_ECB)
		{
			EVP_CIPHER_CTX_reset(m_encrCTX.get());
			EVP_CIPHER_CTX_reset(m_decrCTX.get());
			if((!EVP_EncryptInit_ex(m_encrCTX.get(), EVP_aes_128_ecb(), 0, 0, 0))||
				(!EVP_DecryptInit_ex(m_decrCTX.get(), EVP_aes_128_ecb(), 0, 0, 0)))
				bRes = false;
			else
				bRes = true;
		}
		else if (mode == mode_CBC)
		{
			EVP_CIPHER_CTX_reset(m_encrCTX.get());
			EVP_CIPHER_CTX_reset(m_decrCTX.get());
			if((!EVP_EncryptInit_ex(m_encrCTX.get(), EVP_aes_128_cbc(), 0, 0, 0))||
				(!EVP_DecryptInit_ex(m_decrCTX.get(), EVP_aes_128_cbc(), 0, 0, 0)))
				bRes = false;
			else
				bRes = true;
		}
		else
			bRes = false;

		if(bRes)
		{
			m_Alg = alg;
			m_CipherMode = mode;
		}
		break;
	case alg_sym_AES256:
		if (mode == mode_ECB)
		{
			EVP_CIPHER_CTX_reset(m_encrCTX.get());
			EVP_CIPHER_CTX_reset(m_decrCTX.get());
			if((!EVP_EncryptInit_ex(m_encrCTX.get(), EVP_aes_256_ecb(), 0, 0, 0))||
				(!EVP_DecryptInit_ex(m_decrCTX.get(), EVP_aes_256_ecb(), 0, 0, 0)))
				bRes = false;
			else
				bRes = true;
		}
		else if (mode == mode_CBC)
		{
			EVP_CIPHER_CTX_reset(m_encrCTX.get());
			EVP_CIPHER_CTX_reset(m_decrCTX.get());
			if((!EVP_EncryptInit_ex(m_encrCTX.get(), EVP_aes_256_cbc(), 0, 0, 0))||
				(!EVP_DecryptInit_ex(m_decrCTX.get(), EVP_aes_256_cbc(), 0, 0, 0)))
				bRes = false;
			else
				bRes = true;
		}
		else
			bRes = false;

		if(bRes)
		{
			m_Alg = alg;
			m_CipherMode = mode;
		}
		break;
	case alg_sym_GOST:
	{
		if (mode != mode_CFB)
			break;

		const EVP_CIPHER *cipher = EVP_get_cipherbyname("gost89");
		EVP_CIPHER_CTX_reset(m_encrCTX.get());
		EVP_CIPHER_CTX_reset(m_decrCTX.get());
		if (!EVP_EncryptInit_ex(m_encrCTX.get(), cipher, 0, 0, 0) ||
			!EVP_DecryptInit_ex(m_decrCTX.get(), cipher, 0, 0, 0))
		{
			bRes = false;
		}
		else
			bRes = true;

		if (bRes)
		{
			m_Alg = alg;
			m_CipherMode = mode;
		}
	}
		break;
	default:
		break;
	}
	if(!bRes)
	{
		EVP_CIPHER_CTX_cleanup(m_encrCTX.get());
		EVP_CIPHER_CTX_cleanup(m_decrCTX.get());
		EVP_CIPHER_CTX_free(m_encrCTX.get());
		EVP_CIPHER_CTX_free(m_decrCTX.get());
		m_encrCTX.get() = m_decrCTX.get() = 0;
		m_state = st_none;
		m_CipherMode = mode_NONE;
		m_Alg = alg_sym_NONE;
	}
	else
		m_state = st_ctx_init;
	return bRes;
}
bool VS_SymmetricCrypt::GenerateKey(const uint32_t len, unsigned char *buf) const
{
	return m_key.GenerateKey(len, buf);
}
bool VS_SymmetricCrypt::SetKey(const uint32_t len, const unsigned char *buf)
{
	if(!(m_state&st_ctx_init))
		return false;
	if(!m_key.SetKey(len, buf))
		return false;
	if((!EVP_CIPHER_CTX_set_key_length(m_encrCTX.get(), len))||
		(!EVP_CIPHER_CTX_set_key_length(m_decrCTX.get(), len)))
	{
		m_key.ClearKey();
		return false;
	}
	if((!EVP_DecryptInit_ex(m_decrCTX.get(), 0, 0, buf, 0))||
		(!EVP_EncryptInit_ex(m_encrCTX.get(), 0, 0, buf, 0)))
	{
		m_key.ClearKey();
		return false;
	}
	return true;
}
bool VS_SymmetricCrypt::GetKey(const uint32_t buflen, unsigned char *keybuf, uint32_t *keylen) const
{
	if(!(m_state&st_ctx_init))
		return false;
	return m_key.GetKey(buflen, keybuf, keylen);
}
bool VS_SymmetricCrypt::ChekKeyStrength() const
{
	if(!(m_state&st_ctx_init))
		return false;
	return m_key.CheckKeyStrength();
}
uint32_t VS_SymmetricCrypt::GetBlockSize() const
{
	if(!(m_state&st_ctx_init))
		return false;
	return EVP_CIPHER_CTX_block_size(m_encrCTX.get());
}
uint32_t VS_SymmetricCrypt::GetKeyLen() const
{
	if(!(m_state&st_ctx_init))
		return 0;
	return m_key.GetKeyLength();
}
VS_SymmetricAlg VS_SymmetricCrypt::GetAlg() const
{
	return m_Alg;
}
VS_SymmetricCipherMode VS_SymmetricCrypt::GetCipherMode() const
{
	return m_CipherMode;
}
bool VS_SymmetricCrypt::Encrypt(const unsigned char*data, const uint32_t data_len,
											unsigned char* encr_buf, uint32_t *buf_len) const
{
	if(!(m_state&st_ctx_init))
	{
		*buf_len = 0;
		return false;
	}
	int blocksize = EVP_CIPHER_CTX_block_size(m_encrCTX.get());
	const bool streamMode = blocksize == 1;
	uint32_t min_size = data_len/blocksize;
	if (!streamMode)
	{
		++min_size;
		min_size *= blocksize;
	}
	if(*buf_len < min_size)
	{
		*buf_len = min_size;
		return false;
	}
	int tmpsize(0);
	if(!EVP_EncryptUpdate(m_encrCTX.get(), encr_buf, &tmpsize, data, data_len))
	{
		*buf_len = 0;
		EVP_EncryptInit_ex(m_encrCTX.get(), 0, 0, 0, 0);
		return false;
	}
	*buf_len = tmpsize;
	if(!EVP_EncryptFinal(m_encrCTX.get(), &encr_buf[*buf_len], &tmpsize))
	{
		*buf_len = 0;
		EVP_EncryptInit_ex(m_encrCTX.get(), 0, 0, 0, 0);
		return false;
	}
	*buf_len+=tmpsize;
	EVP_EncryptInit_ex(m_encrCTX.get(), 0, 0, 0, 0);
	return true;
}
bool VS_SymmetricCrypt::Decrypt(const unsigned char *encr_buf, const uint32_t buf_len,
											unsigned char *decr_data, uint32_t *decr_len) const
{
	if(!(m_state&st_ctx_init))
	{
		*decr_len = 0;
		return false;
	}
	if(*decr_len < buf_len)
	{
		*decr_len = buf_len + /*EVP_MAX_BLOCK_LENGTH*/64;
		return false;
	}
	int tmpsize(0);
	if(!decr_data || !EVP_DecryptUpdate(m_decrCTX.get(), decr_data, &tmpsize, encr_buf, buf_len))
	{
		*decr_len = 0;
		EVP_DecryptInit_ex(m_decrCTX.get(), 0, 0, 0, 0);
		return false;
	}
	*decr_len = tmpsize;
	int result = EVP_DecryptFinal(m_decrCTX.get(), &decr_data[*decr_len], &tmpsize);
	*decr_len += tmpsize;
//	If EVP_DecryptFinal() failed, we assume that there is more data to decrypt, so context doesn't get re-inited
	if (result)
		EVP_DecryptInit_ex(m_decrCTX.get(), 0, 0, 0, 0);
	return true;
}
template<>
EVP_CIPHER_CTX* VS_SymmetricCrypt::GetEncryptCtx()
{
	return m_encrCTX.get();
}
template<>
EVP_CIPHER_CTX* VS_SymmetricCrypt::GetDecryptCtx()
{
	return m_decrCTX.get();
}