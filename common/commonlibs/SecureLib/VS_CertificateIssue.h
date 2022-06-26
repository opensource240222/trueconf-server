#pragma once
#include "SecureLib/OpenSSLTypesWrapDefs.h"
#include "SecureLib/SecureTypes.h"
#include "SecureLib/VS_Certificate.h"
#include "SecureLib/VS_PublicKeyCrypt.h"

#include <chrono>

/**\brief Внешний интерфейс
*/
class VS_CertificateRequest
{
private:
	VS_PKey	m_PublicKey;
	VS_PKey	m_PrivateKey;

	vs::X509ReqPtr m_req;
	vs::X509NamePtr m_subj;
	vs::StackOfX509ExtPtr m_extlist;
	bool m_IsSigned;
public:
	VS_CertificateRequest();
	~VS_CertificateRequest();
	bool	SetPKeys(VS_PKey *aPublicKey, VS_PKey *aPrivateKey/*,const char *aPass = 0, const uint32_t aSz = 0*/);
	bool	SetEntry(const char *aName, string_view aValue);
	bool	GetEntry(const char *aName, char *aValue, uint32_t &aSz);
	bool	SetExtension(const char *aName, const char *aValue);
	bool	SignRequest();
	bool	SaveTo(char *aDest, uint32_t &sz, const VS_StorageType aType);
	bool    SetSubjectAltName(SubjectAltNameType type, string_view value);
	void    GetSubjectAltNames(SubjAltNameExtensionsSet &out);
};

class VS_Certificate;
/**
\brief CA (выпускает сертификаты)
*/
class VS_CertAuthority
{
private:
	VS_PKey m_CAKey;
	vs::X509Ptr m_CACert;
	vs::X509Ptr m_cert;
	vs::X509ReqPtr m_req;
	vs::X509v3CtxPtr m_ctx;
	vs::X509NamePtr m_subj;

	bool						m_IsVerifed;
	bool						m_IsReqData;
	bool						m_IsCAData;
	bool						m_IsSigned;

	bool						SetReqData();
	bool						SetCAData();
	bool						SignCert(const VS_HashAlg& digest);
public:
	VS_CertAuthority();
	~VS_CertAuthority();

	bool	SetCertRequest(const char *aReqSource, const uint32_t aSz, const VS_StorageType	aType);
	bool	SetCertRequest(VS_CertificateRequest *aReq);
	bool	SetCACertificate(const char *aSource,const uint32_t aSz, const VS_StorageType aType);
	bool	SetCAPrivateKey(const char *aSource, const VS_StorageType aType, const char *aPass = 0);
	bool	SetCAPrivateKey(VS_PKey	*aPrivateKey);
	bool	VerifyRequestSign();
	bool	GetRequestEntry(const char *aName, char *aValue, uint32_t &aValSz);
	bool	GetRequestPublicKey(VS_PKey *out);
	bool	SetExtension(const char *aName, const char *aValue);

	bool	SetEntry(const char *aName, string_view aValue);

	bool	SetSerialNumber(const int32_t	aSerial);
	bool	SetSerialNumber(const char *aSerial, const uint32_t aSz);
	bool	SetExpirationTime(const uint32_t aSec);
	bool	SetExpirationTime(const std::chrono::system_clock::time_point notBefore, const std::chrono::system_clock::time_point notAfter);
	bool	IssueCertificate(char *aDest, uint32_t &aSz, const VS_StorageType aType, const VS_HashAlg &digest_alg = VS_HashAlg::alg_hsh_SHA256);
	bool    SetSubjectAltName(SubjectAltNameType type, string_view value);
};