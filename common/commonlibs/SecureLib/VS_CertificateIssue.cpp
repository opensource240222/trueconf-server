#include "VS_CertificateIssue.h"

#include "SecureLib/OpenSSLTypesWrapImpl.h"
#include "SecureLib/VS_CertMixins.h"

#include "std-generic/cpplib/scope_exit.h"
#include "std-generic/cpplib/TimeUtils.h"

#include <openssl/pem.h>

#include <cstring>
#include <ctime>
#include <utility>
#include <vector>

const EVP_MD * GetDigestByEnum(const VS_HashAlg &alg)
{
	switch (alg)
	{
	case alg_hsh_SHA1:
		return EVP_sha1();
	case alg_hsh_SHA256:
		return EVP_sha256();
	default:
		return nullptr;
	}
}

static inline bool JoinNames(X509_NAME *src, X509_NAME *dst)
{
	if (!src || !dst)
		return true;
	for (int i = 0; i < X509_NAME_entry_count(src); i++)
	{
		X509_NAME_delete_entry(dst, X509_NAME_get_index_by_OBJ(dst, X509_NAME_ENTRY_get_object(X509_NAME_get_entry(src, i)), -1));
		if (-1 == X509_NAME_add_entry(dst, X509_NAME_get_entry(src, i), -1, 0))
			return false;
	}
	return true;
}
bool VS_CertificateRequest::SetPKeys(VS_PKey *aPublicKey, VS_PKey *aPrivateKey)
{
	uint32_t sz;

	if((!aPublicKey)||(!aPrivateKey))
		return false;

	VS_PKey pub_key;
	sz = 0;
	aPublicKey->GetPublicKey(store_PEM_BUF, 0, &sz);
	if (sz)
	{
		auto pem_buf = std::make_unique<char[]>(sz);
		if (!aPublicKey->GetPublicKey(store_PEM_BUF, pem_buf.get(), &sz) || !pub_key.SetPublicKey(pem_buf.get(), store_PEM_BUF))
			return false;
	}
	else
		return false;

	VS_PKey priv_key;
	sz = 0;
	aPrivateKey->GetPrivateKey(store_PEM_BUF, 0, &sz);
	if (sz)
	{
		auto pem_buf = std::make_unique<char[]>(sz);
		if (!aPrivateKey->GetPrivateKey(store_PEM_BUF, pem_buf.get(), &sz) || !priv_key.SetPrivateKey(pem_buf.get(), store_PEM_BUF))
			return false;
	}
	else
		return false;

	m_PrivateKey = std::move(priv_key);
	m_PublicKey = std::move(pub_key);
	return true;
}

VS_CertificateRequest::VS_CertificateRequest():
	m_PublicKey(), m_PrivateKey(),
	m_req(X509_REQ_new()), m_subj(X509_NAME_new()),
	m_extlist(reinterpret_cast<STACK_OF(X509_EXTENSION) *>(sk_new_null())), m_IsSigned(false)
{}

VS_CertificateRequest::~VS_CertificateRequest() = default;
bool VS_CertificateRequest::SetEntry(const char *aName, string_view aValue)
{
	if(!aName || m_IsSigned)
		return false;
	X509_NAME_ENTRY* ent = X509_NAME_ENTRY_create_by_txt(
		0, aName, MBSTRING_UTF8, reinterpret_cast<const unsigned char*>(aValue.data()), aValue.length());
	VS_SCOPE_EXIT{if (ent) X509_NAME_ENTRY_free(ent);};
	if(!ent || -1 == X509_NAME_add_entry(m_subj.get().get(), ent, -1, 0))
		return false;
	else
		return true;
}


bool VS_CertificateRequest::GetEntry(const char *aName, char *aValue, uint32_t &aSz)
{
	int				nid(0);
	int				index(-1);
	ASN1_STRING	*data(0);
	X509_NAME_ENTRY	*ent(0);
	nid = OBJ_txt2nid(aName);
	if(NID_undef == nid)
		return false;
	if((-1==(index = X509_NAME_get_index_by_NID(m_subj.get().get(), nid, -1)))||
		(!(ent = X509_NAME_get_entry(m_subj.get().get(), index)))||
		(!(data = X509_NAME_ENTRY_get_data(ent))))
		return false;
	if(strlen(reinterpret_cast<char*>(ASN1_STRING_data(data))) + 1 > aSz)
	{
		aSz = static_cast<uint32_t>(strlen(reinterpret_cast<char*>(ASN1_STRING_data(data)))) + 1;
		return false;
	}
	memcpy(aValue, ASN1_STRING_data(data), strlen(reinterpret_cast<char*>(ASN1_STRING_data(data)))+1);
	return true;
}
bool VS_CertificateRequest::SetExtension(const char *aName, const char *aValue)
{
	if((m_IsSigned)||(!aName)||(!aValue)||
		(strlen(aName)>65535)||(strlen(aValue)>65535))
		return false;

	X509_EXTENSION	*ext(0);
	char *Name = new char[strlen(aName) + 1];
	char * Value = new char[strlen(aValue) + 1];

	memcpy(Name, aName, strlen(aName) + 1);
	memcpy(Value, aValue, strlen(aValue) + 1);
	ext = X509V3_EXT_conf(0, 0, Name, Value);
	delete [] Name;
	delete [] Value;
	if(!ext)
		return false;
	VS_SCOPE_EXIT{X509_EXTENSION_free(ext);};
	STACK_OF(X509_EXTENSION)* temp = m_extlist.get().release();
	VS_SCOPE_EXIT{m_extlist.get().reset(temp);};
	if (!X509v3_add_ext(&temp, ext, -1))
		return false;
	return true;

}
bool VS_CertificateRequest::SignRequest()
{
	bool res(true);
	char	*pem_buf(0);
	uint32_t	sz(0);
	BIO				*bio(0);
	EVP_PKEY *pub_key	= EVP_PKEY_new();
	EVP_PKEY *priv_key	= EVP_PKEY_new();

	m_PublicKey.GetPublicKey(store_PEM_BUF, 0, &sz);
	if(!sz)
		res = false;
	else
	{
		pem_buf = new char[sz];
		if(!m_PublicKey.GetPublicKey(store_PEM_BUF, pem_buf, &sz))
			res = false;
		else
		{
			if(!(bio = BIO_new(BIO_s_mem())) ||
				(0>=BIO_write(bio, pem_buf, static_cast<int>(sz)))||
				(!PEM_read_bio_PUBKEY(bio, &pub_key, 0, 0)))
				res = false;
			else
				res = true;
			if(bio)
				BIO_free(bio);
			bio = 0;
		}
		delete [] pem_buf;
		pem_buf = 0;
		sz = 0;
	}
	if(res)
	{
		m_PrivateKey.GetPrivateKey(store_PEM_BUF, 0, &sz);
		if(!sz)
			res = false;
		else
		{
			pem_buf = new char[sz];
			if(!m_PrivateKey.GetPrivateKey(store_PEM_BUF, pem_buf, &sz))
				res = false;
			else
			{
				if(!(bio = BIO_new(BIO_s_mem())) ||
					0 >= BIO_write(bio, pem_buf, static_cast<int>(sz)) ||
					!(PEM_read_bio_PrivateKey(bio, &priv_key, 0, 0)))
					res = false;
				else
					res = true;
				if(bio)
					BIO_free(bio);
				bio = 0;
			}
			delete [] pem_buf;
			pem_buf = 0;
			sz = 0;
		}
	}
	if(res)
	{
		const EVP_MD	*digest(0);
		X509_REQ_set_pubkey(m_req.get().get(), pub_key);
		if(1!=X509_REQ_set_subject_name(m_req.get().get(), m_subj.get().get()))
			res = false;
		else
		{
			if(sk_X509_EXTENSION_num(m_extlist.get().get())>0)
			{
				if(!X509_REQ_add_extensions(m_req.get().get(), m_extlist.get().get()))
					res = false;
			}
			if(res)
			{
				if (EVP_PKEY_type(EVP_PKEY_id(priv_key)) == EVP_PKEY_RSA)
					digest = EVP_sha256();
				else
					res = false;
			}
		}
		if(res)
		{
			if(!X509_REQ_sign(m_req.get().get(), priv_key, digest))
				res = false;
			else
			{
				m_IsSigned = true;
				res = true;
			}
		}

	}
	EVP_PKEY_free(pub_key);
	EVP_PKEY_free(priv_key);
	return res;
}
bool VS_CertificateRequest::SaveTo(char *aDest, uint32_t &aSz, const VS_StorageType aType)
{
	if(!m_IsSigned)
		return false;
	FILE *fp(0);
	BIO	*bio(0);
	bool res(true);
	switch(aType)
	{
	case store_PEM_BUF:
		if(!(bio = BIO_new(BIO_s_mem()))||
			(!PEM_write_bio_X509_REQ(bio, m_req.get().get())))
			res = false;
		else
		{
			BUF_MEM *pmem(0);
			BIO_get_mem_ptr(bio, &pmem);
			auto sz = pmem->length;
			if(sz > aSz)
			{
				aSz = sz;
				res = false;
			}
			else if(0>=BIO_read(bio, aDest, sz))
				res = false;
			else
				res = true;
		}
		BIO_free(bio);
		break;
	case store_PEM_FILE:
		if(!(fp = fopen(aDest, "wb")))
			res = false;
		else
		{
			if(1!=PEM_write_X509_REQ(fp, m_req.get().get()))
				res = false;
			else
				res = true;
		}
		if(fp)
			fclose(fp);
		break;
	default:
		res = false;
	}
	return res;
}

bool VS_CertificateRequest::SetSubjectAltName(SubjectAltNameType type, string_view value)
{
	X509_EXTENSION *ext = MakeExtension(type, value);

	VS_SCOPE_EXIT{X509_EXTENSION_free(ext);};
	STACK_OF(X509_EXTENSION)* temp = m_extlist.get().release();
	VS_SCOPE_EXIT{m_extlist.get().reset(temp);};
	if (!X509v3_add_ext(&temp, ext, -1))
		return false;
	return true;
}

void VS_CertificateRequest::GetSubjectAltNames(SubjAltNameExtensionsSet &out)
{
	FetchExtensions(m_extlist.get().get(), out);
}

bool VS_CertAuthority::SetCAPrivateKey(VS_PKey *aPrivateKey)
{
	if(!aPrivateKey)
		return false;
	char *pem_buf(0);
	uint32_t	sz(0);
	bool			res(false);

	aPrivateKey->GetPrivateKey(store_PEM_BUF, pem_buf, &sz);
	if(!sz)
		return false;
	pem_buf = new char [sz];
	if(aPrivateKey->GetPrivateKey(store_PEM_BUF, pem_buf, &sz))
		res = SetCAPrivateKey(pem_buf, store_PEM_BUF, 0);
	else
		res = false;
	delete [] pem_buf;
	return res;
}

bool VS_CertAuthority::SetCertRequest(VS_CertificateRequest *aReq)
{
	if(!aReq)
		return false;
	char *pem_buf(0);
	uint32_t sz(0);
	bool	res(false);
	aReq->SaveTo(pem_buf, sz, store_PEM_BUF);
	if(!sz)
		return false;
	pem_buf = new char[sz];
	if(aReq->SaveTo(pem_buf, sz, store_PEM_BUF))
		res = SetCertRequest(pem_buf, sz, store_PEM_BUF);
	else
		res = false;
	delete [] pem_buf;
	return res;
}

VS_CertAuthority::VS_CertAuthority() :
	m_CAKey(), m_CACert(nullptr), m_cert(X509_new()), m_req(X509_REQ_new()),
	m_ctx(new X509V3_CTX), m_subj(X509_NAME_new()), m_IsVerifed(false), m_IsReqData(false),
	m_IsCAData(false),m_IsSigned(false)
{
}
VS_CertAuthority::~VS_CertAuthority() = default;

bool VS_CertAuthority::SetCertRequest(const char *aReqSource, const uint32_t aSz, const VS_StorageType aType)
{
	if((!aReqSource)||(!m_cert.get())||(!aSz)||(m_IsSigned))
		return false;
	bool	res(false);
	FILE	*fp(0);
	BIO		*bio(0);
	X509_REQ* newReq;
	switch(aType)
	{
	case store_PEM_FILE:
		if(!(fp = fopen(aReqSource, "rb")))
			res = false;
		else
		{
			m_req.get().reset(PEM_read_X509_REQ(fp, 0, 0, 0));
			fclose(fp);
			if(!m_req.get())
				res = false;
			else
				res = true;
		}
		break;
	case store_PEM_BUF:
		if(!(bio = BIO_new(BIO_s_mem()))||
			(0>=BIO_write(bio, aReqSource, static_cast<int>(aSz))) ||
			(!(newReq = PEM_read_bio_X509_REQ(bio, 0, 0, 0))))
			res = false;
		else
		{
			m_req.get().reset(newReq);
			res = true;
		}
		if(bio)
			BIO_free(bio);
		break;
	default:
		res = false;
	}
	return res;
}
bool VS_CertAuthority::SetCACertificate(const char *aSource, const uint32_t aSz, const VS_StorageType aType)
{
	FILE	*fp(0);
	BIO		*bio(0);
	bool	res(false);
	X509* newCACert;
	if((!aSource)||(!aSz)||(!m_cert.get())||(m_IsSigned))
		return false;
	switch(aType)
	{
	case store_PEM_FILE:
		if(!(fp = fopen(aSource, "rb"))||
		   (!(newCACert = PEM_read_X509(fp, 0, 0, 0))))
			res = false;
		else
		{
			m_CACert.get().reset(newCACert);
			res = true;
		}
		if(fp)
			fclose(fp);
	break;
	case store_PEM_BUF:
		if((!(bio = BIO_new(BIO_s_mem())))||
			(0>=BIO_write(bio, aSource, static_cast<int>(aSz)))||
		   (!(newCACert = PEM_read_bio_X509(bio, 0, 0, 0))))
			res = false;
		else
		{
			m_CACert.get().reset(newCACert);
			res = true;
		}
		if(bio)
			BIO_free(bio);
		break;
	default:
		res = false;
	}
	if(res)
		X509V3_set_ctx(m_ctx.get().get(), m_CACert.get().get(), m_cert.get().get(), 0, 0, 0);
	return res;
}

bool VS_CertAuthority::SetCAPrivateKey(const char *aSource,  const VS_StorageType aType, const char *aPass)
{
	if((!aSource)||(!m_cert.get())||(m_IsSigned))
		return false;
	return m_CAKey.SetPrivateKey(aSource, aType, aPass);
}
bool VS_CertAuthority::VerifyRequestSign()
{
	EVP_PKEY	*pub_key(0);
	bool		res(false);
    if(!m_req.get())
		return false;
	if(!(pub_key = X509_REQ_get_pubkey(m_req.get().get())))
		return false;
	if(1!=X509_REQ_verify(m_req.get().get(), pub_key))
		res = false;
	else
	{
		res = true;
		m_IsVerifed = true;
	}
	EVP_PKEY_free(pub_key);
	return res;
}
bool VS_CertAuthority::SetExtension(const char *aName, const char *aValue)
{
	if((!aName)||(!aValue)||(!m_CACert.get())||(!m_cert.get())||(m_IsSigned))
		return false;
	X509_EXTENSION	*ext(0);
	char *Name = new char[strlen(aName) + 1];
	char * Value = new char[strlen(aValue) + 1];

	memcpy(Name, aName, strlen(aName) + 1);
	memcpy(Value, aValue, strlen(aValue) + 1);
	ext = X509V3_EXT_conf(0, m_ctx.get().get(), Name, Value);
	delete [] Name;
	delete [] Value;
	if(!ext)
		return false;
	VS_SCOPE_EXIT{
		if (ext)
			X509_EXTENSION_free(ext);
	};
	if(!X509_add_ext(m_cert.get().get(), ext, -1))
		return false;
	return true;
}
bool VS_CertAuthority::SetEntry(const char *aName, string_view aValue)
{

	X509_NAME_ENTRY		*ent(0);
	int					nid(0);
	bool				res(false);
	if(!aName || !m_CACert.get() || !m_cert.get() || m_IsSigned)
		return false;

	std::vector<unsigned char> aValueCopy(aValue.begin(), aValue.end());

	nid = OBJ_txt2nid(aName);
	if(NID_undef == nid)
		return false;
	if(!(ent = X509_NAME_ENTRY_create_by_NID(0, nid, MBSTRING_UTF8, aValueCopy.data(), aValueCopy.size())))
		res = false;
	else if(-1 == X509_NAME_add_entry(m_subj.get().get(), ent, -1, 0))
		res = false;
	else
		res = true;
	if(ent)
		X509_NAME_ENTRY_free(ent);
	return res;
}

bool VS_CertAuthority::SetSerialNumber(const int32_t aSerial)
{
	if((!m_cert.get())||(!m_IsVerifed)||(!m_req.get())||(aSerial<=0)||(m_IsSigned))
		return false;
	ASN1_INTEGER_set(X509_get_serialNumber(m_cert.get().get()), aSerial);
	return true;
}
bool VS_CertAuthority::SetSerialNumber(const char *, const uint32_t)
{
	return false;
}

bool VS_CertAuthority::SetExpirationTime(const uint32_t aSec)
{
	if((!m_cert.get())||(!m_req.get())||(m_IsSigned))
		return false;
	if (!X509_gmtime_adj(X509_getm_notBefore(m_cert.get().get()), 0))
		return false;
	else if (!X509_gmtime_adj(X509_getm_notAfter(m_cert.get().get()), aSec))
		return false;
	else
		return true;
}

bool VS_CertAuthority::SetExpirationTime(const std::chrono::system_clock::time_point notBefore, const std::chrono::system_clock::time_point notAfter)
{
	if ((!m_cert.get()) || (!m_req.get()) || (m_IsSigned))
		return false;
	ASN1_GENERALIZEDTIME	*notBeforeASN1(0), *notAfterASN1(0);

	notBeforeASN1 = X509_getm_notBefore(m_cert.get().get());
	notAfterASN1 = X509_getm_notAfter(m_cert.get().get());
	if ((1 != ASN1_GENERALIZEDTIME_set_string(notBeforeASN1, tu::TimeToString(notBefore, "%Y%m%d%H%M%SZ", true).c_str())) ||
	    (1 != ASN1_GENERALIZEDTIME_set_string(notAfterASN1,  tu::TimeToString(notAfter,  "%Y%m%d%H%M%SZ", true).c_str())))
		return false;
	return true;
}

bool VS_CertAuthority::IssueCertificate(char *aDest, uint32_t &aSz, const VS_StorageType aType, const VS_HashAlg &digest_alg)
{
	bool	res(true);
	if((!m_IsVerifed)||(!SetReqData())||(!SetCAData())||(!SignCert(digest_alg)))
		return false;
	else
	{
		BIO				*bio(0);
		FILE			*fp(0);
		switch(aType)
		{
		case store_PEM_BUF:
			if((!(bio = BIO_new(BIO_s_mem())))||
			   (1!=PEM_write_bio_X509(bio, m_cert.get().get())))
			   res = false;
			else
			{
				BUF_MEM *pmem(0);
				BIO_get_mem_ptr(bio, &pmem);
				auto sz = pmem->length;
				if(sz > aSz)
				{
					aSz = sz;
					res = false;
				}
				else
				{
					if(0>=BIO_read(bio, aDest, sz))
						res = false;
					else
						res = true;
				}
			}
			if(bio)
				BIO_free(bio);
			break;
		case store_PEM_FILE:
			if((!(fp=fopen(aDest, "wb")))||(1!=PEM_write_X509(fp, m_cert.get().get())))
				res = false;
			else
				res = true;
			if(fp)
				fclose(fp);
			break;
		default:
			res = false;
		}
	}
	return res;
}

bool VS_CertAuthority::SetReqData()
{
	if((!m_IsVerifed)||(!m_req.get())||(!m_cert.get()))
		return false;
	if(m_IsReqData)
		return true;
	STACK_OF(X509_EXTENSION)	*reqext(0);
	X509_NAME					*name(0);
	EVP_PKEY					*pub_key(0);
	bool						res(true);

	if ((!(name = X509_REQ_get_subject_name(m_req.get().get()) )) ||
		!JoinNames(m_subj.get().get(), name) ||
		(1!=X509_set_subject_name(m_cert.get().get(), name))		||
		(!(pub_key =X509_REQ_get_pubkey(m_req.get().get())))	||
		(1!=X509_set_pubkey(m_cert.get().get(), pub_key)))
		res = false;
	else
	{
		reqext = X509_REQ_get_extensions(m_req.get().get());
		for(int i = 0; i<sk_X509_EXTENSION_num(reqext);i++)
		{
			auto ext = X509v3_get_ext(reqext, i);
			if(!X509_add_ext(m_cert.get().get(), ext, -1))
			{
				res = false;
				break;
			}
		}
	}
	if(pub_key)
		EVP_PKEY_free(pub_key);
	if(reqext)
		sk_X509_EXTENSION_pop_free(reqext, reinterpret_cast<void (*)(X509_EXTENSION *)>(::X509_EXTENSION_free));
	if(res)
		m_IsReqData = true;
	else
		m_IsReqData = false;
	return res;
}
bool VS_CertAuthority::SetCAData()
{
	if((!m_IsVerifed)||(!m_req.get())||
		(!m_cert.get())||(!m_CACert.get()))
		return false;
	if(m_IsCAData)
		return true;
	X509_NAME	*name(0);
	bool res(true);
	if((!(name = X509_get_subject_name(m_CACert.get().get())))||
		(1!= X509_set_issuer_name(m_cert.get().get(), name)))
		res = false;
	if(res)
		m_IsCAData = true;
	else
		m_IsCAData = false;
	return res;
}

bool VS_CertAuthority::SignCert(const VS_HashAlg &digest_alg)
{
	if((!m_IsVerifed)||(!m_req.get())	||
		(!m_cert.get())||(!m_CACert.get())	||
		(!m_IsCAData)||(!m_IsReqData))
		return false;

	const EVP_MD	*digest(0);
	EVP_PKEY		*priv_key = EVP_PKEY_new();
	BIO				*bio(0);
	char			*pem_buf(0);
	uint32_t	sz(0);
	bool			res(true);
	if (alg_pk_RSA == m_CAKey.GetPKeyAlg())
		digest = GetDigestByEnum(digest_alg);//EVP_sha1();
	else
		res = false;
	m_CAKey.GetPrivateKey(store_PEM_BUF, pem_buf, &sz);
	if(!sz)
		res = false;
	else if(res)
	{
		pem_buf = new char [sz];
		if((!m_CAKey.GetPrivateKey(store_PEM_BUF, pem_buf, &sz))||
			(!(bio = BIO_new(BIO_s_mem())))||(0>=BIO_write(bio, pem_buf, static_cast<int>(sz)))||
			(!PEM_read_bio_PrivateKey(bio, &priv_key, 0, 0)))
			res = false;
		else
			res = true;
		delete [] pem_buf;
		sz = 0;
		if(bio)
			BIO_free(bio);
	}
	if(((res)&&(!X509_sign(m_cert.get().get(), priv_key, digest)))||(!res))
		res = false;
	else
	{
		m_IsSigned = true;
		res = true;
	}
	EVP_PKEY_free(priv_key);
	return res;
}
bool VS_CertAuthority::GetRequestEntry(const char *aName, char *aValue, uint32_t &aValSz)
{
	if(!m_req.get())
		return false;
	X509_NAME		*subj_name(0);
	int				nid(0), index(0);
	X509_NAME_ENTRY	*ent(0);
	ASN1_STRING		*data(0);

	if((NID_undef ==(nid = OBJ_txt2nid(aName)))||
	   (!(subj_name = X509_REQ_get_subject_name(m_req.get().get()) ))||
	   (-1==(index = X509_NAME_get_index_by_NID(subj_name, nid, -1)))||
	   (!(ent = X509_NAME_get_entry(subj_name, index)))||
	   (!(data = X509_NAME_ENTRY_get_data(ent))))
		return false;
	if(strlen(reinterpret_cast<char*>(ASN1_STRING_data(data))) + 1 > aValSz)
	{
		aValSz = static_cast<uint32_t>(strlen(reinterpret_cast<char*>(ASN1_STRING_data(data)))) + 1;
		return false;
	}
	memcpy(aValue, ASN1_STRING_data(data), strlen(reinterpret_cast<char*>(ASN1_STRING_data(data)))+1);
	return true;
}
bool VS_CertAuthority::GetRequestPublicKey(VS_PKey *out)
{
	BIO *bp(0);
	BUF_MEM *pmem(0);
	int len(0);
	char *buf(0);
	if(!m_req.get())
		return false;
	if(!out)
		return false;

	EVP_PKEY *pkey = X509_REQ_get_pubkey(m_req.get().get());
	if(!pkey)
		return false;
	if(!(bp = BIO_new(BIO_s_mem()))||
	    (!PEM_write_bio_PUBKEY(bp, pkey))||
		(!BIO_get_mem_ptr(bp, &pmem)))
	{
		EVP_PKEY_free(pkey);
		BIO_free(bp);
		return false;
	}
	len = pmem->length;
	uint32_t	tmp_len = len + 1;
	buf = new char[tmp_len];
	buf[tmp_len - 1] = 0;//null terminate
	bool bRes = false;
	if((-1 == BIO_read(bp, buf, len))||
	   (!out->SetPublicKey(buf, store_PEM_BUF)))
	   bRes = false;
	else
		bRes = true;
	delete [] buf;
	EVP_PKEY_free(pkey);
	BIO_free(bp);
	return bRes;
}

bool VS_CertAuthority::SetSubjectAltName(SubjectAltNameType type, string_view value)
{
	X509_EXTENSION *ext = MakeExtension(type, value);

	VS_SCOPE_EXIT{X509_EXTENSION_free(ext);};
	if (!X509_add_ext(m_cert.get().get(), ext, -1))
		return false;
	return true;
}

