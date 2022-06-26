#pragma once
/** \file VS_Certificate.h
* \brief ���� �������� �������� ������� ��� ������ � ������������� X509
*/
#include "SecureLib/OpenSSLTypesWrapDefs.h"
#include "SecureLib/SecureTypes.h"
#include "std-generic/cpplib/string_view.h"

#include <memory>
#include <stdint.h>

class VS_PKey;
/**
\brief
����� ����������� (������� ���������).

�������� �������� bridge. ���������� ������� ��������� ���������� ����������� ����� ������������.
������������� ����� ���� ���������� �� ������ ������ ��������� (�� ������ ������� ��������
�� ������ ����������), ���� �� OpenSSL ���� ����� ����������.
*/
class VS_Certificate
{
private:
	vs::X509Ptr cert; ///< ������ �� ��������� X509
public:
	VS_Certificate();
	VS_Certificate(const VS_Certificate& old);
	~VS_Certificate();
	VS_Certificate& operator=(const VS_Certificate& old);

	template<class X509Ptr/* =  X509* */>
	bool SetCert(X509Ptr newCert);
/**������ ����������.
	@param[in]	cert_source	����� ���� ���� ������ ����� �����������,
	���� �������, ������� �������� ���� ����������.
	@param[in]	type ��������� ��� �������� cert_source � � ����� �������
	@return		<b>true</b> � <b>false</b>
*/
	bool SetCert(const char *cert_source, const uint32_t sz, const VS_StorageType type);
    bool GetCertificateData(std::string &serverEP_u8, std::string &country_u8, std::string &organization_u8,
            std::string &contactPerson_u8, std::string &email_u8) const;
	bool GetSubjectEntry(const char *aName, std::string& entry) const;
	bool GetIssuerEntry(const char *aName, std::string& entry) const;
	bool GetExpirationTime(std::string& notBefore,
		std::string& notAfter, bool isFormating = true) const;
/**
	-2 - ���������� ������
	-1 - ���������� �� ����� �����������
	 0 - ���������� ��������� ������
	 1 - ���������� ���������
*/
	int	CheckExpirationTime() const;

	bool GetExtension(const char *aName, std::string& extStr) const;
/**�������� PublicKey
	@param[out]	out	������ VS_PKey, ������� ����� ��������� �������� ���� �����������
	@return	��� �������� ���������� <b>true</b>, � ��������� ������ <b>false</b>
*/
	bool GetCertPublicKey(VS_PKey *out) const;
	void GetSubjectAltNames(SubjAltNameExtensionsSet &out);
};

/**
\brief
������������� ������� ��������� � ��������� ����������� ������������ X509.

������������ ������� bridge. ���������� ��������� ���������� ����������� ������������.
������������� ����� ���� ���������� �� ������ ������ ��������� (�� ������ ������� ��������
�� ������ ����������), ���� �� OpenSSL ���� ����� ����������.
*/
class VS_CertificateCheck
{
private:
	vs::X509Ptr cert;	///<����������� ����������
	vs::X509StorePtr store;///< ��������� ������������, CRL, ���������� ������������. ��, ��� ����� ������������ ��� �����������.
public:
	VS_CertificateCheck();
	~VS_CertificateCheck();


/**
	������ ���������, ������� ���� ��������������.
	@param[in]	cert_source	����� ���� ���� ������ ����� �����������,
	���� �������, ������� �������� ���� ���������� (������� �� ������� ���������).
	@param[in]	type ��������� ��� �������� cert_source � � ����� �������.
	@return	��� ������� ���������� �������� ���������� <b>true</b>, ����� <b>false</b>
*/
	bool SetCert(const char *cert_source, const uint32_t sz, const VS_StorageType type);
/**
	�������� ���������� � ������� ������������ (��� ����������� �������� ������������).
	@param[in]	cert_source	����� ���� ���� ������ ����� �����������,
	���� �������, ������� �������� ���� ���������� (������� �� ������� ���������).
	@param[in]	type ��������� ��� �������� cert_source � � ����� �������.
	@return	��� ������� ���������� �������� ���������� <b>true</b>, ����� <b>false</b>
*/
	bool SetCertToChain(const char *cert_source, const uint32_t sz, const VS_StorageType type);
/**
	�������� CRL (Certificate Revocation List ������ ���������� ������������)
	@param[in]	crl_source	����� ���� ���� ������ ����� �����������,
	���� �������, ������� �������� ���� ���������� (������� �� ������� ���������).
	@param[in]	type ��������� ��� �������� cert_source � � ����� �������.
	@return	��� ������� ���������� �������� ���������� <b>true</b>, ����� <b>false</b>
*/
	bool SetCRL(const char *crl_source, const VS_StorageType type);

/**
	������ ����������, ��� ��������� ����������� CA � CRL.
	@param[in]	cert_dir	����������, ��� ��������� ����������� CA � CRL.
	@return	��� ������� ���������� �������� ���������� <b>true</b>, ����� <b>false</b>
*/
	bool SetStoreDir(string_view cert_dir);

/**
	�������������� ���������� (������� ������������)
	@return ���������� <b>true</b>, ���� ����������� ��������������, ���� ���, �� <b>false</b>
*/
	bool VerifyCert(int *err = nullptr, std::string* err_descr = nullptr);
};