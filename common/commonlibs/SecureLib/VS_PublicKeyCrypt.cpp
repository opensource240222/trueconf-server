#include "VS_PublicKeyCrypt.h"
#include "SecureLib/OpenSSLTypesWrapImpl.h"

#include "std-generic/cpplib/scope_exit.h"
#include <openssl/pem.h>
#include <openssl/rsa.h>

#include <cstring>
#include <memory>
#include <vector>

VS_PKey::VS_PKey():m_pkey(EVP_PKEY_new())
{
}
VS_PKey::~VS_PKey() = default;
VS_PKey& VS_PKey::operator=(VS_PKey&&) = default;
VS_PKey::VS_PKey(VS_PKey&&) = default;
VS_PublicKeyAlg VS_PKey::GetPKeyAlg() const
{
	if(EVP_PKEY_RSA == EVP_PKEY_type(EVP_PKEY_base_id(m_pkey.get().get())))
		return alg_pk_RSA;
	return alg_pk_NONE;
}

bool VS_PKey::GenerateKeys(const uint32_t keylen, const VS_PublicKeyAlg alg)
{
	RSA	*rsakey(0);
	if(!m_pkey.get())
		return false;
	switch(alg)
	{
	case alg_pk_RSA:
		do
		{
			if(rsakey)
				RSA_free(rsakey);
			rsakey = RSA_generate_key(keylen,RSA_F4,0,0);
			if(!rsakey)
				return false;
		}while(!RSA_check_key(rsakey));
		EVP_PKEY_assign(m_pkey.get().get(), EVP_PKEY_RSA, reinterpret_cast<char*>(rsakey));
		return true;
	default:
		return false;
	}
}

bool VS_PKey::CheckKeyStrength() const
{
	return false;
}

uint32_t VS_PKey::GetKeyLength() const
{
	return 0;
}

bool VS_PKey::SetPublicKey(const char *key_source, const VS_StorageType type)
{
	FILE	*fp(0);
	BIO		*bio(0);
	EVP_PKEY *newKey = EVP_PKEY_new();
	VS_SCOPE_EXIT{
		EVP_PKEY_free(newKey);
		if (fp)
			fclose(fp);
		if (bio)
			BIO_free(bio);
	};

	if(!key_source || strlen(key_source) > 65535)
		return false;
	switch(type)
	{
	case store_PEM_FILE:
		if(!(fp = fopen(key_source,"r"))||
		   !(PEM_read_PUBKEY(fp,&newKey,0,0)))
			return false;
		else
		{
			m_pkey.get().reset(newKey);
			newKey = nullptr;
			return true;
		}
	case store_PEM_BUF:
		if(!(bio = BIO_new(BIO_s_mem())) ||
		   (0>=BIO_write(bio,key_source,static_cast<int>(strlen(key_source) + 1)))||
		   (!PEM_read_bio_PUBKEY(bio,&newKey,0,0)))
			return false;
		else
		{
			m_pkey.get().reset(newKey);
			newKey = nullptr;
			return true;
		}
	case store_DER_FILE:
	case store_DER_BUF:
	case store_PLAIN_BUF:
	case store_BER_FILE:
	case store_BER_BUF:	break;

	}
	return false;
}

bool VS_PKey::SetPrivateKey(const char *key_source,const VS_StorageType type, const char *pass)
{
	FILE	*fp(0);
	BIO		*bio(0);
	EVP_PKEY *newKey = EVP_PKEY_new();
	VS_SCOPE_EXIT{
		EVP_PKEY_free(newKey);
		if (fp)
			fclose(fp);
		if (bio)
			BIO_free(bio);
	};

//	SetPrivateKey is used kind of often, but pass is passed very rarely so we don't need to care.
//	Assuming pass is at least a default value (which is '\0')
	std::vector<char> passCopy;
	if (pass)
		passCopy.assign(pass, pass + strlen(pass) + 1);// including '\0'
	else
		passCopy.assign({'\0'});

	if(!key_source || strlen(key_source) > 65535)
		return false;
	switch(type)
	{
	case store_PEM_FILE:
		if(!(fp = fopen(key_source,"rb")) ||
		   !(PEM_read_PrivateKey(fp,&newKey,0,passCopy.data())))
			return false;
		else
		{
			m_pkey.get().reset(newKey);
			newKey = nullptr;
			return true;
		}
	case store_PEM_BUF:
		if(!(bio = BIO_new(BIO_s_mem()))||
			(0>=BIO_write(bio,key_source,static_cast<int>(strlen(key_source) + 1)))||
		   !(PEM_read_bio_PrivateKey(bio,&newKey,0,passCopy.data())))
			return false;
		else
		{
			m_pkey.get().reset(newKey);
			newKey = nullptr;
			return true;
		}
	case store_DER_FILE:
	case store_DER_BUF:
	case store_PLAIN_BUF:
	case store_BER_FILE:
	case store_BER_BUF:	break;
	}
	return false;
}
template<>
EVP_PKEY *VS_PKey::GetKeyContext()// returns non-const value since everybody goes for const-cast anyway
{
	return m_pkey.get().get();
}
bool VS_PKey::WritePublicKey(const char *file_name, const VS_StorageType format)
{
	FILE	*fp(0);
	if(!m_pkey.get())
		return false;
	VS_SCOPE_EXIT{if (fp) fclose(fp);};
	switch(format)
	{
	case store_PEM_FILE:
		if(!(fp = fopen(file_name,"r"))||
		   !PEM_write_PUBKEY(fp,m_pkey.get().get()))
			return false;
		else
			return true;
		case store_PEM_BUF:
	case store_DER_FILE:
	case store_DER_BUF:
	case store_PLAIN_BUF:
	case store_BER_FILE:
	case store_BER_BUF:	break;

	}
	return false;
}
bool VS_PKey::GetPublicKey(const VS_StorageType format, char *buf,uint32_t *buflen)
{
	BIO	*bio(0);
	BUF_MEM *pmem(0);
	uint32_t sz(0);
	uint32_t	tmp_sz(0);
	VS_SCOPE_EXIT{if (bio) BIO_free(bio);};
	if(!m_pkey.get())
		return false;
	switch(format)
	{
	case store_PEM_BUF:
		if(!(bio = BIO_new(BIO_s_mem())) ||
			(!PEM_write_bio_PUBKEY(bio,m_pkey.get().get())))
			return false;
		BIO_get_mem_ptr(bio,&pmem);
		sz = pmem->length;
		tmp_sz = sz + 1; // for zero terminate
		if(tmp_sz > *buflen)
		{
			*buflen = tmp_sz;
			return false;
		}
		if(0 >= BIO_read(bio,buf,sz))
			return false;
		buf[tmp_sz - 1] = 0; //zero terminate
		return true;
	default:
		return false;
	}
}
bool VS_PKey::GetPrivateKey(const VS_StorageType format, char *buf, uint32_t *buflen)
{
	BIO	*bio(0);
	BUF_MEM *pmem(0);
	uint32_t sz(0);
	uint32_t tmp_sz(0);
	VS_SCOPE_EXIT{if (bio) BIO_free(bio);};
	if(!m_pkey.get())
		return false;
	switch(format)
	{
	case store_PEM_BUF:
		if(!(bio = BIO_new(BIO_s_mem()))||
			(!PEM_write_bio_PrivateKey(bio,m_pkey.get().get(),0,0,0,0,0)))
			return false;
		BIO_get_mem_ptr(bio,&pmem);
		sz = pmem->length;
		tmp_sz = sz + 1; // + 1 for zero terminate
		if(tmp_sz > *buflen)
		{
			*buflen = tmp_sz;
			return false;
		}

		if(0>=BIO_read(bio,buf,sz))
			return false;
		buf[tmp_sz - 1] = 0; // zero terminate
		return true;
	default:
		return false;
	}
}
bool VS_PKey::WritePrivateKey(char *dist, uint32_t &sz, const VS_StorageType format, const VS_SymmetricAlg enc,
										  const VS_SymmetricCipherMode mode, const char *pass, const int pass_len)
{
	FILE	*fp(0);
	BIO		*bio(0);
	BUF_MEM *pmem(0);
	const EVP_CIPHER *cip(0);
	if(!m_pkey.get())
		return false;
	switch(enc)
	{
	case alg_sym_NONE:
		cip = 0;
		break;
	case alg_sym_AES128:
	case alg_sym_AES256:
		if(mode_ECB == mode)		cip = EVP_aes_256_ecb();
		else if(mode_CBC == mode)	cip = EVP_aes_256_cbc();
		else
			return false;
		break;
	default:
		return false;
	}
	bool bRes(false);

	std::vector<unsigned char> passCopy(pass, pass + pass_len);
	if (pass)
		passCopy.assign(pass, pass + strlen(pass) + 1);// including '\0'
	else
		passCopy.assign({'\0'});

	switch(format)
	{
	case store_PEM_FILE:
		fp = fopen(dist,"w");
		if(!fp)
			return false;
		if(PEM_write_PrivateKey(fp, m_pkey.get().get(), cip, passCopy.data(), passCopy.size(), 0, 0))
			bRes = true;
		else
			bRes = false;
		fclose(fp);
		break;
	case store_PEM_BUF:
		if((!(bio = BIO_new(BIO_s_mem())))||
			(!PEM_write_bio_PrivateKey(bio, m_pkey.get().get(), cip, passCopy.data(), passCopy.size(), 0, 0)))
			bRes = false;
		else
		{
			BIO_get_mem_ptr(bio,&pmem);
			auto bio_sz = pmem->length;
			uint32_t	tmp_sz = bio_sz + 1;//zero terminate
			if(tmp_sz > sz)
			{
				bRes = false;
				sz = tmp_sz;
			}
			else
			{
				BIO_read(bio,dist,bio_sz);
				dist[tmp_sz - 1] = 0;
				bRes = true;
			}
			if(bio)
				BIO_free(bio);
		}
		break;
	default:
		bRes = false;
	}
	return bRes;
}

bool VS_PKeyCrypt::SetPublicKey(const char *key_source, const VS_StorageType type)
{
	bool bRes = key.SetPublicKey(key_source, type);
	if(bRes)
		bRes = (alg_pk_RSA == key.GetPKeyAlg());
	return bRes;
}
bool VS_PKeyCrypt::SetPublicKey(VS_PKey *pkey)
{
	char *key_buf(0);
	uint32_t buf_len(0);
	bool bRes;
	if((!pkey)||(alg_pk_RSA != pkey->GetPKeyAlg()))
		return false;

	pkey->GetPublicKey(store_PEM_BUF,key_buf,&buf_len);
	if(!buf_len)
		return false;
	key_buf = new char[buf_len];
	bRes = pkey->GetPublicKey(store_PEM_BUF,key_buf,&buf_len);
	bRes = bRes ? key.SetPublicKey(key_buf,store_PEM_BUF) : false;
	delete [] key_buf;
	return bRes;
}

bool VS_PKeyCrypt::SetPrivateKey(const char *key_source, const VS_StorageType type)
{
	bool bRes = key.SetPrivateKey(key_source, type);
	return (bRes && (alg_pk_RSA == key.GetPKeyAlg()));
}

bool VS_PKeyCrypt::SetPrivateKey(VS_PKey *pkey)
{
	char *key_buf(0);
	uint32_t buf_len(0);
	bool bRes;
	if((!pkey)|(alg_pk_RSA != pkey->GetPKeyAlg()))
		return false;
	pkey->GetPrivateKey(store_PEM_BUF, key_buf, &buf_len);
	if(!buf_len)
		return false;
	key_buf = new char[buf_len];
	bRes = pkey->GetPrivateKey(store_PEM_BUF,key_buf,&buf_len) ? key.SetPrivateKey(key_buf,store_PEM_BUF) : false;
	delete [] key_buf;
	return bRes;
}

uint32_t VS_PKeyCrypt::GetKeyLength()
{
	return key.GetKeyLength();
}
bool VS_PKeyCrypt::Encrypt(const unsigned char *data, const uint32_t data_len,
									   const VS_SymmetricAlg sym_alg,
									   unsigned char *encr_buf, uint32_t *buf_len,
									   unsigned char iv[16], unsigned char *sym_key, uint32_t *sym_key_len)
{
	EVP_CIPHER_CTX		*ctx(0);
	const EVP_CIPHER	*type(0);
	EVP_PKEY			*pkey(0);
	int					ekl(0);
	memset(iv,0,16);

	if(alg_pk_RSA != key.GetPKeyAlg())
		return false;
	switch(sym_alg)
	{
	case alg_sym_AES128:
	case alg_sym_AES256:
		type = EVP_aes_256_cbc();
		break;
	default:
		return false;
	}
	if(!(pkey = key.GetKeyContext<decltype(pkey)>()) ||
		!(ekl = EVP_PKEY_size(pkey)))
		return false;
	if(ekl > static_cast<int>(*sym_key_len))
	{
		*sym_key_len = static_cast<uint32_t>(ekl);
		return false;
	}
	if(!sym_key)
	{
		return false;
	}

	ctx = EVP_CIPHER_CTX_new();
	EVP_CIPHER_CTX_init(ctx);
	if(!(EVP_SealInit(ctx,type,&sym_key,&ekl,iv,&pkey,1)))
	{
		EVP_CIPHER_CTX_cleanup(ctx);
		EVP_CIPHER_CTX_free(ctx);
		return false;
	}
	int blocksize = EVP_CIPHER_CTX_block_size(ctx);
	uint32_t min_size = data_len / blocksize;
	++min_size *= blocksize;
	if(min_size > *buf_len)
	{
		*buf_len = min_size;
		EVP_CIPHER_CTX_cleanup(ctx);
		EVP_CIPHER_CTX_free(ctx);
		return false;
	}
	if(!encr_buf)
	{
		EVP_CIPHER_CTX_cleanup(ctx);
		EVP_CIPHER_CTX_free(ctx);
		return false;
	}
	int tmp(0);
	EVP_EncryptUpdate(ctx,encr_buf,&tmp,data,data_len);
	*buf_len = tmp;
	EVP_SealFinal(ctx,&encr_buf[tmp],&tmp);
	*buf_len += tmp;
	EVP_CIPHER_CTX_cleanup(ctx);
	EVP_CIPHER_CTX_free(ctx);
	return true;
}
bool VS_PKeyCrypt::Decrypt(const unsigned char *encr_buf, const uint32_t buf_len,
									   const VS_SymmetricAlg sym_alg, const unsigned char iv[16],
									   const unsigned char *encr_key,const uint32_t key_len,
									   unsigned char *decr_data, uint32_t *decr_len)
{
	EVP_CIPHER_CTX		*ctx(0);
	const EVP_CIPHER	*type(0);
	EVP_PKEY			*pkey(0);
	if(alg_pk_RSA != key.GetPKeyAlg())
		return false;
	switch(sym_alg)
	{
	case alg_sym_AES128:
	case alg_sym_AES256:
		type = EVP_aes_256_cbc();
		break;
	default:
		return false;
	}
	if(buf_len > *decr_len)
	{
		*decr_len = buf_len + /*EVP_MAX_BLOCK_LENGTH*/64;
		return false;
	}

	pkey =key.GetKeyContext<decltype(pkey)>();
	ctx = EVP_CIPHER_CTX_new();
	EVP_CIPHER_CTX_init(ctx);
	if(0>=EVP_OpenInit(ctx,type,encr_key,key_len,iv,pkey))
	{
		EVP_CIPHER_CTX_cleanup(ctx);
		EVP_CIPHER_CTX_free(ctx);
		return false;
	}
	//EVP_OpenInit(&ctx,type,encr_key,key_len,iv,pkey);
	int tmp(0);
	EVP_DecryptUpdate(ctx,decr_data,&tmp,encr_buf, buf_len);
	*decr_len = tmp;
	EVP_OpenFinal(ctx,&decr_data[tmp],&tmp);
	*decr_len += tmp;
	EVP_CIPHER_CTX_cleanup(ctx);
	EVP_CIPHER_CTX_free(ctx);
	return true;
}