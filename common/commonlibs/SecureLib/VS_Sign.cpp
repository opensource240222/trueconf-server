#include "VS_Sign.h"
#include "SecureLib/OpenSSLTypesWrapImpl.h"

#include "std-generic/cpplib/scope_exit.h"

#include <openssl/evp.h>
#include <openssl/pem.h>

#include <cstring>
#include <vector>
#include <memory>

VS_Sign::VS_Sign() : m_flag(0),m_pkey(nullptr) ,m_sign_arg(zeroSignArg)
{
}
VS_Sign::~VS_Sign() = default;
VS_Sign::VS_Sign(VS_Sign&&) = default;
VS_Sign& VS_Sign::operator=(VS_Sign&&) = default;

bool VS_Sign::Init(const VS_SignArg sign_arg)
{
	if(!VerifySignArg(sign_arg))
		return false;
	m_sign_arg = sign_arg;
	m_flag |= st_init_arg;
	return true;
}

bool VS_Sign::IsInitSignArg () const
{
	return m_flag&st_init_arg;
}

bool VS_Sign::IsSpecifiedPrivateKey () const
{
	return (m_flag&st_specify_private_key)>0;
}

bool VS_Sign::IsSpecifiedPublicKey () const
{
	return (m_flag&st_specify_public_key)>0;
}

bool VS_Sign::SetPublicKey(const char* KeyStorage, const uint32_t, const int StorageType)
{
	FILE *fp;
	BIO		*bio(0);
	bool bRes = true;
	EVP_PKEY* newKey;
	VS_SCOPE_EXIT{if (bio) BIO_free(bio);};
	if((IsSpecifiedPublicKey())||
		(IsSpecifiedPrivateKey()))
		return false;
	switch(StorageType)
	{
	case store_PEM_FILE:
		fp = fopen(KeyStorage,"r");
		if(!fp)
			bRes = false;
		else
		{
			m_pkey.get().reset(PEM_read_PUBKEY(fp,0,0,0));
			fclose(fp);
			if(!m_pkey.get())
				bRes = false;
			else
			{
				m_flag |= st_specify_public_key;
				bRes = true;
			}
		}
		break;
	case store_PEM_BUF:
		if(!(bio = BIO_new(BIO_s_mem())) ||
		   (0>=BIO_write(bio,KeyStorage,static_cast<int>((strlen(KeyStorage) + 1))))||
		   (!(newKey = PEM_read_bio_PUBKEY(bio,0,0,0))))
			bRes = false;
		else
		{
			bRes = true;
			m_pkey.get().reset(newKey);
			if(!m_pkey.get())
				bRes = false;
			else
			{
				m_flag |= st_specify_public_key;
				bRes = true;
			}
		}
		break;
	case store_DER_FILE:
		bRes = false;
		break;
	case store_DER_BUF:
		bRes = false;
		break;
	case store_PLAIN_BUF:
		bRes = false;
		break;
	case store_BER_FILE:
		bRes = false;
		break;
	case store_BER_BUF:
		bRes = false;
		break;
	default:
		bRes = false;
	}
	return bRes;
}

bool VS_Sign::SetPrivateKey(const char *KeyStorage, const int StorageType,const char *pass)
{
	bool bRes(true);
	FILE* fp;
	BIO		*bio(0);
	EVP_PKEY* newKey;
	VS_SCOPE_EXIT{if (bio) BIO_free(bio);};
	if((IsSpecifiedPublicKey())||
		(IsSpecifiedPrivateKey()))
		return false;
	std::vector<char> passCopy;
	if (pass)
		passCopy.assign(pass, pass + strlen(pass) + 1);// including '\0'
	switch(StorageType)
	{
	case store_PEM_FILE:
		fp = fopen(KeyStorage,"r");
		if(!fp)
			bRes = false;
		else
		{
			if(pass && pass[0])
				m_pkey.get().reset(PEM_read_PrivateKey(fp, 0, 0, passCopy.data()));
			else
				m_pkey.get().reset(PEM_read_PrivateKey(fp, 0, 0, 0));
			if(m_pkey.get())
			{
				m_flag |= st_specify_private_key;
				bRes = true;
			}
			else
				bRes = false;
		}
		break;
	case store_PEM_BUF:
		if(!(bio = BIO_new(BIO_s_mem()))||
			(0>=BIO_write(bio,KeyStorage,static_cast<int>((strlen(KeyStorage) + 1))))||
		   !(newKey = PEM_read_bio_PrivateKey(bio, 0, 0, passCopy.data())))
			bRes = false;
		else
		{
			m_pkey.get().reset(newKey);
			if(m_pkey.get())
			{
				m_flag |= st_specify_private_key;
				bRes = true;
			}
			else
				bRes = false;

		}
		break;
	case store_DER_FILE:
		bRes = false;
		break;
	case store_DER_BUF:
		bRes = false;
		break;
	case store_PLAIN_BUF:
		bRes = false;
		break;
	case store_BER_FILE:
		bRes = false;
		break;
	case store_BER_BUF:
		bRes = false;
		break;
	default:
		bRes = false;
	}
	return bRes;
}

bool VS_Sign::VerifySignArg(const VS_SignArg sign_arg) const
{
	switch(sign_arg.PKtype)
	{
	case alg_pk_NONE:
		return false;
	case alg_pk_RSA:
		switch(sign_arg.HASHtype)
		{
		case alg_hsh_SHA1:
			return true;
		default:
			return false;
		}
	default:
		return false;
	}
}

uint32_t VS_Sign::GetSignSize() const
{
	if((m_flag&st_specify_public_key)||
		(m_flag&st_specify_private_key))
		return EVP_PKEY_size(m_pkey.get().get());
	else
		return 0;
}

bool VS_Sign::SignData(const unsigned char *data, uint32_t data_size,unsigned char *sign,uint32_t *sign_size)
{
	if((!IsSpecifiedPrivateKey())||
		(!IsInitSignArg()))
		return false;
	if((*sign_size < GetSignSize())||(*sign_size == 0))
	{
		*sign_size = GetSignSize();
		return false;
	}
	EVP_MD_CTX *ctx = EVP_MD_CTX_create();
	bool bRes(false);
	switch(m_sign_arg.HASHtype)
	{
	case alg_hsh_SHA1:
		if(!EVP_DigestInit(ctx,EVP_sha1()))
		{
			EVP_MD_CTX_destroy(ctx);
			return bRes;
		}
		break;
	case alg_hsh_SHA256:
		if (!EVP_DigestInit(ctx, EVP_sha256()))
		{
			EVP_MD_CTX_destroy(ctx);
			return bRes;
		}
		break;
	default:
		return bRes;
	}
	EVP_DigestUpdate(ctx,data,data_size);
	if(EVP_SignFinal(ctx,sign,static_cast<unsigned int*>(sign_size),m_pkey.get().get()))
		bRes = true;
	EVP_MD_CTX_destroy(ctx);
	return bRes;
}

int VS_Sign::VerifySign(const unsigned char *data, uint32_t data_size,const unsigned char *sign,const uint32_t sign_size)
{
	if((!IsSpecifiedPublicKey())||
		(!IsInitSignArg()))
		return false;
	EVP_MD_CTX *ctx = EVP_MD_CTX_create();
	int Res(-1);
	switch(m_sign_arg.HASHtype)
	{
	case alg_hsh_SHA1:
		if(!EVP_DigestInit(ctx,EVP_sha1()))
		{
			EVP_MD_CTX_destroy(ctx);
			return Res;
		}
		break;
	case alg_hsh_SHA256:
		if (!EVP_DigestInit(ctx, EVP_sha256()))
		{
			EVP_MD_CTX_destroy(ctx);
			return Res;
		}
		break;
	default:
		return Res;
	}
	Res = EVP_DigestUpdate(ctx,data,data_size);
	if(Res>0)
		Res = EVP_VerifyFinal(ctx,sign,sign_size,m_pkey.get().get());
	EVP_MD_CTX_destroy(ctx);
	return Res;
}

int VS_Sign::SetPassword(char* buf, int len, int, void *cb_arg)
{
	VS_PrivateKeyStorage *pKeyStorage = reinterpret_cast<VS_PrivateKeyStorage*>(cb_arg);
	memcpy(buf,pKeyStorage->pass,len>VS_PASS_SIZE?VS_PASS_SIZE : len);
	return 1;
}