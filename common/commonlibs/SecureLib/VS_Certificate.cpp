#include "VS_Certificate.h"

#include "SecureLib/OpenSSLCompat/tc_asn1.h"
#include "SecureLib/OpenSSLCompat/tc_crypto.h"
#include "SecureLib/OpenSSLCompat/tc_x509.h"
#include "SecureLib/OpenSSLTypesWrapImpl.h"
#include "SecureLib/VS_CertMixins.h"
#include "SecureLib/VS_PublicKeyCrypt.h"

#include "std-generic/cpplib/scope_exit.h"

#include <openssl/pem.h>

#include <algorithm>
#include <cstring>
#include <string>

VS_Certificate::VS_Certificate()
	: cert(X509_new())
	{}

VS_Certificate::VS_Certificate(const VS_Certificate& old):
	cert(X509_dup(old.cert.get().get()))
	{}

VS_Certificate::~VS_Certificate() = default;
VS_Certificate& VS_Certificate::operator=(const VS_Certificate& old)
{
	cert.get().reset(X509_dup(old.cert.get().get()));
	return *this;
}

template<>
bool VS_Certificate::SetCert(X509* newCert)
{
	cert.get().reset(newCert);
	return true;
}

bool VS_Certificate::SetCert(const char *cert_source, const uint32_t sz, const VS_StorageType type)
{
	FILE *fp(0);
	BIO *bio(0);
	X509* newCert(0);
	switch(type)
	{
	case store_PEM_FILE:
		if(!(fp = fopen(cert_source, "r")) ||
		   (!PEM_read_X509(fp, &newCert, 0, 0)))
		{
			fclose(fp);
			return false;
		}
		fclose(fp);
		cert.get().reset(newCert);
		return true;
	case store_PEM_BUF:
		if(!(bio = BIO_new(BIO_s_mem()))||
		    (0>=BIO_write(bio, cert_source, sz))||
		    (!(newCert = PEM_read_bio_X509(bio, 0, 0, 0))))
		{
			BIO_free(bio);
			return false;
		}
		BIO_free(bio);
		cert.get().reset(newCert);
		return true;
	case store_DER_FILE:
	case store_DER_BUF:
	case store_PLAIN_BUF:
	case store_BER_FILE:
	case store_BER_BUF:	break;
	}
	return false;
}

bool VS_Certificate::GetCertificateData(std::string& serverEP_u8, std::string& country_u8, std::string& organization_u8,
                                        std::string& contactPerson_u8, std::string& email_u8) const
{
    if (!GetSubjectEntry("commonName", serverEP_u8)) serverEP_u8.clear();
    if (!GetSubjectEntry("countryName", country_u8)) country_u8.clear();
    if (!GetSubjectEntry("organizationName", organization_u8)) organization_u8.clear();
    if (!GetSubjectEntry("surname", contactPerson_u8)) contactPerson_u8.clear();
    if (!GetSubjectEntry("emailAddress", email_u8)) email_u8.clear();

    return true;
}

bool VS_Certificate::GetSubjectEntry(const char *aName, std::string& entry) const
{
	bool res(false);
	int nid(0);
	int index(-1);
	X509_NAME *subj(0);
	ASN1_STRING	*data(0);
	X509_NAME_ENTRY	*ent(0);

	nid = OBJ_txt2nid(aName);

	if(NID_undef == nid)
		return false;
	if(!(subj = X509_get_subject_name(cert.get().get())))
		return false;
	if((-1 == (index = X509_NAME_get_index_by_NID(subj, nid, -1)))||
		(!(ent = X509_NAME_get_entry(subj, index)))||
		(!(data = X509_NAME_ENTRY_get_data(ent))))
		res = false;
	else
	{

		unsigned char *tmp(0);
		uint32_t l;
		l = ASN1_STRING_to_UTF8(&tmp, data);
		if (l > 0)
		{
            entry.assign(reinterpret_cast<char *>(tmp), l);
			res = true;
		}
		CRYPTO_free(tmp, NULL, 0);
	}
	return res;
}

bool VS_Certificate::GetIssuerEntry(const char *aName, std::string& entry) const
{
	bool res(false);
	int nid(0);
	int index(-1);
	X509_NAME *subj(0);
	ASN1_STRING	*data(0);
	X509_NAME_ENTRY	*ent(0);

	nid = OBJ_txt2nid(aName);

	if(NID_undef == nid)
		return false;
	if(!(subj = X509_get_issuer_name(cert.get().get())))
		return false;
	if((-1 == (index = X509_NAME_get_index_by_NID(subj, nid, -1)))||
		(!(ent = X509_NAME_get_entry(subj, index)))||
		(!(data = X509_NAME_ENTRY_get_data(ent))))
		res = false;
	else
	{
        const auto len = ASN1_STRING_length( data );
		if (len > 0)
            entry.assign(reinterpret_cast<char *>(ASN1_STRING_data(data)), len);
		else
			entry.clear();
		res = true;
	}

	return res;
}

bool VS_Certificate::GetExtension(const char *aName, std::string& extStr) const
{
	ASN1_OBJECT * o = OBJ_txt2obj(aName, 0);
	if(!o)
		return false;
	VS_SCOPE_EXIT{ ASN1_OBJECT_free(o); };
	int idx = X509_get_ext_by_OBJ(cert.get().get(), o, -1);
	if(idx<0)
		return false;
		X509_EXTENSION *ext = X509_get_ext(cert.get().get(), idx);
	if(!ext)
		return false;
	auto worstStringDesignEver = X509_EXTENSION_get_data(ext);
	const unsigned char *data = ASN1_STRING_get0_data(worstStringDesignEver);
	auto len = ASN1_STRING_length(worstStringDesignEver);
	if (len > 2)
		extStr.assign(reinterpret_cast<const char*>(data) + 2, len - 2);
	else
		extStr.clear();
	return true;
}

bool VS_Certificate::GetExpirationTime(std::string& notBefore ,
												   std::string& notAfter, bool isFormating) const
{
	bool res(false);
	ASN1_UTCTIME	*notBeforeTime(0), *notAfterTime(0);

	if(isFormating)
	{
		BIO *notBeforeBio(0), *notAfterBio(0);
		BUF_MEM *pA(0), *pB(0);
		notBeforeBio = BIO_new(BIO_s_mem());
		notAfterBio = BIO_new(BIO_s_mem());
		if((!notBeforeBio)||(!notAfterBio))
			return false;
		if((!(notBeforeTime = X509_getm_notBefore(cert.get().get())))||
			(!(notAfterTime = X509_getm_notAfter(cert.get().get())))||
			(!ASN1_GENERALIZEDTIME_print(notBeforeBio, notBeforeTime))||
			(!ASN1_GENERALIZEDTIME_print(notAfterBio, notAfterTime))||
			(!BIO_get_mem_ptr(notBeforeBio, &pB))||
			(!BIO_get_mem_ptr(notAfterBio, &pA)))
			res = false;
		else
		{
			res = true;
			auto lenA = pA->length; auto lenB = pB->length;
			std::vector<char> data( std::max(lenA, lenB) );
			if (BIO_read(notBeforeBio, data.data(),lenB))
				notBefore.assign(data.data(), lenB);
			else
				res = false;
			if (BIO_read(notAfterBio, data.data(),lenA))
				notAfter.assign(data.data(), lenA);
			else
				res = false;
		}
		if(notBeforeBio)
			BIO_free(notBeforeBio);
		if(notAfterBio)
			BIO_free(notAfterBio);
	}
	else
	{
		if((!(notBeforeTime = X509_getm_notBefore(cert.get().get())))||
			(!(notAfterTime = X509_getm_notAfter(cert.get().get()))))
			res = false;
		else
		{
			uint32_t lenB = notBeforeTime->length;
			uint32_t lenA = notAfterTime->length;

			notBefore.assign(reinterpret_cast<const char*>(notBeforeTime->data), lenB - 1);
			notAfter.assign(reinterpret_cast<const char*>(notAfterTime->data), lenA - 1);

			res = true;
		}
	}
	return res;
}

int VS_Certificate::CheckExpirationTime() const
{
	int res(0);
	BIO *notBeforeBio(0), *notAfterBio(0);
	ASN1_UTCTIME	*notBeforeTime(0), *notAfterTime(0);
	notBeforeBio = BIO_new(BIO_s_mem());
	notAfterBio = BIO_new(BIO_s_mem());
	if((!notBeforeBio)||(!notAfterBio))
		return -2;
	if((!(notBeforeTime = X509_getm_notBefore(cert.get().get())))||
		(!(notAfterTime = X509_getm_notAfter(cert.get().get()))))
	{
		res = -2;
	}
	else
	{
		int cmp_res = X509_cmp_current_time(notBeforeTime); ///http://www.umich.edu/~x509/ssleay/x509_time.html
		if(0==cmp_res)
			res = -2;
		else if(cmp_res>0)
			res = -1;
		else
		{
			cmp_res = X509_cmp_current_time(notAfterTime);
			if(0==cmp_res)
				res = -2;
			else if(cmp_res<0)
				res = 1;
		}
	}
	return res;
}

bool VS_Certificate::GetCertPublicKey(VS_PKey *out) const
{
	BIO *bp(0);
	int len(0);
	BUF_MEM *pmem(0);
	char *buf(0);
	if(!out)
		return false;

	EVP_PKEY *pkey = X509_get_pubkey(cert.get().get());

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

void VS_Certificate::GetSubjectAltNames(SubjAltNameExtensionsSet &out)
{
	FetchExtensions(X509_get0_extensions(cert.get().get()), out);
}

VS_CertificateCheck::VS_CertificateCheck():cert(nullptr), store(X509_STORE_new())
{}
VS_CertificateCheck::~VS_CertificateCheck() = default;

bool VS_CertificateCheck::SetCert(const char *cert_source, const uint32_t sz, const VS_StorageType type)
{
	FILE	*fp(0);
	BIO		*bio;
	X509    *newCert;
	if(!cert_source || !sz || sz>0x100000)
		return false;
	switch(type)
	{
	case store_PEM_FILE:
		//прочитать из PEM файла
		if(!(fp = fopen(cert_source, "r"))||
			!(newCert = PEM_read_X509(fp, 0, 0, 0)))
		{
			if(fp)
				fclose(fp);
			return false;
		}
		fclose(fp);
		cert.get().reset(newCert);
		return true;
	case store_PEM_BUF:
		//прочитать из буфера
		bio = BIO_new(BIO_s_mem());
		if((0>=BIO_write(bio, cert_source, sz))||
		   !(newCert = PEM_read_bio_X509(bio, 0, 0, 0)))
		{
			BIO_free(bio);
			return false;
		}
		BIO_free(bio);
		cert.get().reset(newCert);
		return true;
	case store_DER_FILE:
	case store_DER_BUF:
	case store_PLAIN_BUF:
	case store_BER_FILE:
	case store_BER_BUF:	break;
	}
	return false;
}

bool VS_CertificateCheck::SetCertToChain(const char* cert_source, const uint32_t sz, const VS_StorageType type)
{
	FILE	*fp = 0;
	BIO		*bio;
	X509	*trusted_cert(0);

	if(!cert_source || !sz || sz>0x100000)
		return false;
	switch(type)
	{
	case store_PEM_FILE:
		if(!(fp = fopen(cert_source, "r")) ||
		   !(trusted_cert = PEM_read_X509(fp, 0, 0, 0))||
		   (X509_STORE_add_cert(store.get().get(), trusted_cert) != 1))
		{
			if(fp)
				fclose(fp);
			if(trusted_cert)
				X509_free(trusted_cert);
			return false;
		}
		X509_free(trusted_cert);
		return true;
	case store_PEM_BUF:
		bio = BIO_new(BIO_s_mem());
		if((0>=BIO_write(bio, cert_source, sz))||
		   !(trusted_cert = PEM_read_bio_X509(bio, 0, 0, 0))||
		   (X509_STORE_add_cert(store.get().get(), trusted_cert) != 1))
		{
			BIO_free(bio);
			if(trusted_cert)
				X509_free(trusted_cert);
			return false;
		}
		BIO_free(bio);
		X509_free(trusted_cert);
		return true;
	case store_DER_FILE:
	case store_DER_BUF:
	case store_PLAIN_BUF:
	case store_BER_FILE:
	case store_BER_BUF:	break;
	}
	return false;
}

bool VS_CertificateCheck::SetCRL(const char *crl_source, const VS_StorageType type)
{
	X509_CRL	*crl(0);
	BIO			*bio(0);
	FILE		*fp(0);
	switch(type)
	{
	case store_PEM_FILE:
		if(!(fp = fopen(crl_source, "r")) ||
		   !(crl = PEM_read_X509_CRL(fp, 0, 0, 0)) ||
		   (X509_STORE_add_crl(store.get().get(), crl) != 1))
		{
			if(fp)
				fclose(fp);
			if(crl)
				X509_CRL_free(crl);
			return false;
		}
		fclose(fp);
		X509_CRL_free(crl);
		return true;
	case store_PEM_BUF:
		if(!(bio = BIO_new(BIO_s_mem())) ||
			(0 >= BIO_write(bio, crl_source, static_cast<int>(strlen(crl_source) + 1))) ||
			!(crl = PEM_read_bio_X509_CRL(bio, 0, 0, 0)) ||
			(X509_STORE_add_crl(store.get().get(), crl) != 1))
		{
			if(bio)
				BIO_free(bio);
			if(crl)
				X509_CRL_free(crl);
			return false;
		}
		BIO_free(bio);
		X509_CRL_free(crl);
		return true;
	case store_DER_FILE:
	case store_DER_BUF:
	case store_PLAIN_BUF:
	case store_BER_FILE:
	case store_BER_BUF:	break;
	}
	return false;
}

bool VS_CertificateCheck::SetStoreDir(string_view cert_dir)
{
	return (1 == X509_STORE_load_locations(store.get().get(), 0, cert_dir.data()));
}

bool VS_CertificateCheck::VerifyCert(int *err, std::string* err_descr)
{
	X509_STORE_CTX* verify_ctx;
	if(!(verify_ctx = X509_STORE_CTX_new()))
		return false;
	VS_SCOPE_EXIT{X509_STORE_CTX_free(verify_ctx);};
	if(X509_STORE_CTX_init(verify_ctx, store.get().get(), cert.get().get(), 0) != 1)
		return false;
	bool bRes = (X509_verify_cert(verify_ctx) == 1);

	if (err || err_descr)
	{
		auto errorCode = X509_STORE_CTX_get_error(verify_ctx);
		if (err)
			*err = errorCode;
		if (err_descr)
			*err_descr = X509_verify_cert_error_string(errorCode);
	}

	return bRes;
}