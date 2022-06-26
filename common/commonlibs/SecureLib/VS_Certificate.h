#pragma once
/** \file VS_Certificate.h
* \brief Файл содержит описание классов для работы с сертификатами X509
*/
#include "SecureLib/OpenSSLTypesWrapDefs.h"
#include "SecureLib/SecureTypes.h"
#include "std-generic/cpplib/string_view.h"

#include <memory>
#include <stdint.h>

class VS_PKey;
/**
\brief
Класс сертификата (внешний интерфейс).

Участник паттерна bridge. Определяет внешний интерфейс абстракции манипуляции одним сертификатом.
Имплементатор может быть реализован на основе разных библиотек (на случай легкого перехода
на другую реализацию), если от OpenSSL надо будет отказаться.
*/
class VS_Certificate
{
private:
	vs::X509Ptr cert; ///< ссылка на сертифкат X509
public:
	VS_Certificate();
	VS_Certificate(const VS_Certificate& old);
	~VS_Certificate();
	VS_Certificate& operator=(const VS_Certificate& old);

	template<class X509Ptr/* =  X509* */>
	bool SetCert(X509Ptr newCert);
/**Задать сертификат.
	@param[in]	cert_source	может быть либо именем файла сертификата,
	либо буфером, который содержит тело сертификат.
	@param[in]	type определет что содержит cert_source и в каком формате
	@return		<b>true</b> и <b>false</b>
*/
	bool SetCert(const char *cert_source, const uint32_t sz, const VS_StorageType type);
    bool GetCertificateData(std::string &serverEP_u8, std::string &country_u8, std::string &organization_u8,
            std::string &contactPerson_u8, std::string &email_u8) const;
	bool GetSubjectEntry(const char *aName, std::string& entry) const;
	bool GetIssuerEntry(const char *aName, std::string& entry) const;
	bool GetExpirationTime(std::string& notBefore,
		std::string& notAfter, bool isFormating = true) const;
/**
	-2 - невалидные данные
	-1 - сертификат не начал действовать
	 0 - сертификат действует сейчас
	 1 - сертификат просрочен
*/
	int	CheckExpirationTime() const;

	bool GetExtension(const char *aName, std::string& extStr) const;
/**Получить PublicKey
	@param[out]	out	объект VS_PKey, который будет содержать открытый ключ сертификата
	@return	При успешном выполнении <b>true</b>, в противном случае <b>false</b>
*/
	bool GetCertPublicKey(VS_PKey *out) const;
	void GetSubjectAltNames(SubjAltNameExtensionsSet &out);
};

/**
\brief
Предоставляет внешний интерфейс к механизму верификации сертификатов X509.

Используется паттерн bridge. Определяет интерфейс абстракции верификации сертификатов.
Имплиментатор может быть реализован на основе разных библиотек (на случай легкого перехода
на другую реализацию), если от OpenSSL надо будет отказаться.
*/
class VS_CertificateCheck
{
private:
	vs::X509Ptr cert;	///<Проверяемый сертификат
	vs::X509StorePtr store;///< хранилище сертификатов, CRL, доверенных сертификатов. То, что будет использовано для верификации.
public:
	VS_CertificateCheck();
	~VS_CertificateCheck();


/**
	Задать серификат, который надо верифицировать.
	@param[in]	cert_source	может быть либо именем файла сертификата,
	либо буфером, который содержит тело сертификат (зависит от второго параметра).
	@param[in]	type определет что содержит cert_source и в каком формате.
	@return	При удачном выполнении операции возвращает <b>true</b>, иначе <b>false</b>
*/
	bool SetCert(const char *cert_source, const uint32_t sz, const VS_StorageType type);
/**
	Добавить сертификат в цепочку сертификатов (для веоификации иерархии сертификатов).
	@param[in]	cert_source	может быть либо именем файла сертификата,
	либо буфером, который содержит тело сертификат (зависит от второго параметра).
	@param[in]	type определет что содержит cert_source и в каком формате.
	@return	При удачном выполнении операции возвращает <b>true</b>, иначе <b>false</b>
*/
	bool SetCertToChain(const char *cert_source, const uint32_t sz, const VS_StorageType type);
/**
	Добавить CRL (Certificate Revocation List список отозванных сертификатов)
	@param[in]	crl_source	может быть либо именем файла сертификата,
	либо буфером, который содержит тело сертификат (зависит от второго параметра).
	@param[in]	type определет что содержит cert_source и в каком формате.
	@return	При удачном выполнении операции возвращает <b>true</b>, иначе <b>false</b>
*/
	bool SetCRL(const char *crl_source, const VS_StorageType type);

/**
	Задать директорию, где находятся сертификаты CA и CRL.
	@param[in]	cert_dir	директория, где находятся сертификаты CA и CRL.
	@return	При удачном выполнении операции возвращает <b>true</b>, иначе <b>false</b>
*/
	bool SetStoreDir(string_view cert_dir);

/**
	Верифицировать сертификат (цепочку сертификатов)
	@return Возвращает <b>true</b>, если сертификаты верифицированы, если нет, то <b>false</b>
*/
	bool VerifyCert(int *err = nullptr, std::string* err_descr = nullptr);
};