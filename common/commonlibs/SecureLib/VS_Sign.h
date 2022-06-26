#pragma once
#include "SecureLib/OpenSSLTypesWrapDefs.h"
#include "SecureLib/SecureTypes.h"
#include "SecureLib/VS_PublicKeyCrypt.h"

/**
\file VS_Sign.h
\brief Файл содержит описание классов для работы с цифровой подписью данных (подписи RSA, DSA).
*/


const VS_SignArg zeroSignArg = {alg_pk_NONE,alg_hsh_NONE};

/**
\brief
	Предоставляет внешний интрфейс к механизму управления цифровой подписью.

	Участник паттерна bridge (интерфейс абстракции). Имплементатор может быть реализован на основе разных
	библиотек (на случай легкого перехода на другую реализацию), если от OpenSSL
	надо будет отказаться. Интерфейс имплиментатора определяется VS_Sign_Implementation.
*/
class VS_Sign
{
private:
/**
\brief
	Внутреннее состояние объекта.
*/
	enum current_state
	{
		st_none					= 0x0,
		st_init_arg				= 0x1,///<Объект инициализирован
		st_specify_public_key	= 0x2,///<Задан открытый ключ
		st_specify_private_key	= 0x4///<Задан закрытый ключ.
	};
	uint32_t m_flag;	///<Состояние объекта. Может быть логичечкой суммой значений из current_state
	vs::EvpPKeyPtr m_pkey;///<Собственно сам ключ.
	VS_SignArg m_sign_arg;	///<Параметры подписывания/верификации данных.

	bool IsInitSignArg() const;
	bool IsSpecifiedPrivateKey () const;
	bool IsSpecifiedPublicKey () const;
	bool VerifySignArg(const VS_SignArg sign_arg) const;
public:
	VS_Sign(void);
	~VS_Sign();
	VS_Sign(VS_Sign&&);
	VS_Sign(const VS_Sign&) = delete;
	VS_Sign& operator=(VS_Sign&&);
	VS_Sign& operator=(const VS_Sign&) = delete;
/**
\brief
	Задать открытый ключ.

	Задает объекту открытый ключ, для верфикации подписи.
	@param[in]	KeyStorage	Либо имя файла в котором хранится ключ, либо буфер содержащий
							ключ в определенном формате. Зависит от значения \e StorageType.
	@param[in]	size		Размет \e KeyStorage.
	@param[in]	StorageType	Тип VS_StorageType. Опрежеляет тип и формат хранилища ключа (файл, буфер, формат PEM, DER)

	@return	Если удалось задать новый ключ возвращает <b>true</b>, <b>false</b> в протинвном случае.
*/
	bool SetPublicKey(const char* KeyStorage, const uint32_t size, const int StorageType);
/**
\brief
	Задать закрытый ключ

	Задает объекту закрытый ключ. Закрытый ключ используется для подписи даных.
	@param[in]	KeyStorage	Либо имя файла в котором хранится ключ, либо буфер содержащий
							ключ в определенном формате. Зависит от значения \e StorageType.
	@param[in]	StorageType	Тип VS_StorageType. Опрежеляет тип и формат хранилища ключа (файл, буфер, формат PEM, DER)

	@return	Если удалось задать новый ключ возвращает <b>true</b>, <b>false</b> в протинвном случае.
*/
	bool SetPrivateKey(const char* KeyStorage, const int StorageType,const char *pass="");
/**
\brief
CallBack функция. Нужна для того, чтобы при чтении закрытого ключа задать пароль, которым он зашифрован.

Используется в VS_SignOpenSSL_Impl::SetPrivateKey.
@param[out]	buf		Буфер для пароля.
@param[in]	len		Длина буфера.
@param[in]	cb_arg	Указатель на VS_PrivateKeyStorage.

@return	Возвращает 1.
*/
int SetPassword(char *buf, int len, int rwflag, void * cb_arg);
/**
\brief
	Подписать даные.

	Перед тем как использовать метод, надо проинициализировать класс (вызвать метод VS_Sign::Init
	с подходящими параметрами)
	и задать закрытый ключ (VS_Sign::SetPrivateKey)

	@param[in]		data		Данные, которые надо подписать
	@param[in]		data_size	Размер \e data.
	@param[out]		sign		Цифровая подпись данных \e data
	@param[in,out]	sign_size	На входе размер буфера под подпись, при выходе содержит количество байт,
								записанных в \e sign.
	@return						Если подпись данных прошла успешно, то возвращает <b>true</b>, <b>false</b>
								в противном случае.

	@remark						Если размера буфера под подпись недостаточно, то метод возвращает <b>false</b>,
								а \e sign_size будет содержать необходимый размер.

*/
	bool SignData(const unsigned char *data, uint32_t data_size,unsigned char *sign, uint32_t *sign_size);
/**
\brief
	Верифицировать подпись.

	Перед теа, как импользовать метод, надо проинициализировать класс (вызвать метод VS_Sign::Init
	с подходящими параметрами) и задать открытый ключ (VS_Sign::SetPublicKey).

	@param[in]	data		Подписанные данные.
	@param[in]	data_size	Размер \e data.
	@param[in]	sign		Подпись, которую надо верифицировать.
	@param[in]	sign_size	Размер \e sign.

	@return					Если верификация прошла успешно (подпись подлиннвя), то возвращает <b>true</b>, <b>false</b> в противном случае.
*/
	int  VerifySign(const unsigned char *data, uint32_t data_size, const unsigned char *sign, const uint32_t sign_size);
/**
\brief
	Инициализация класса.

	Необходимо выполнить перед началом работы.
	@param[in]	prov		Определяет поставщика (библиотеку), который будет использоваться
							для реализации крипто-процедур (в нашем случае только OpenSSL, а значение prov_OPENSSL)
	@param[in]	sign_arg	Параметры для цифровой подписи (алгоритм для ключа, и хеш алгоритм).

	@return					При успешной инициализации возвращает <b>true</b>,
							<b>false</b> в противном случае.

*/
	bool Init(const VS_SignArg sign_arg);
/**
\brief
	Возвращает длину подписи.
*/
	uint32_t GetSignSize() const;
};
