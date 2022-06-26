#pragma once
/**
\file	VS_SecureHandshake.h
\brief
	Файл содержит описание классов, управляющими SecureHandshake'ом.
	(Процедурой установки безопасного соединения)
*/
#include <cstring>
#include <memory>
#include "SecureTypes.h"
#include "VS_Certificate.h"
#include "VS_PublicKeyCrypt.h"
class VS_SymmetricCrypt;

#ifdef _DEBUG
#include <set>
#endif

/**
\brief
	Команды управляющие установкой безопасного соединения.

	См. Примечание в описании VS_SecureHandshake.
*/
enum VS_SecureHandshakeState
{
	secure_st_Error			= 0,
	secure_st_SendPacket	= 1,
	secure_st_SendCert		= 2,
	secure_st_GetPacket		= 3,
	secure_st_Finish		= 4
};
/**
\brief
	Задает тип хендшейка.

	Хендшейк может происходить со стороны сервера и со стороны клиента.
*/
enum VS_SecureHandshakeType
{
	handshake_type_None		= 0,	///<Не иництализирован.
	handshake_type_Client	= 1,	///<Хендшейк происходит со стороны клента.
	handshake_type_Server	= 2		///<Хендшейк происходит со стороны сервера.
};

#pragma pack (1)

/**
\brief
	Структура содержит готовые проинициализированные объекты симметричной криптографии.

	Структура используется внутри класса VS_SecureHandshakeVer1_Impl. Создается после успешного
	хендшейка, удаляется после получения пользователем каждого из крипто-объектов
	(вызовы VS_SecureHandshake::GetReadSymmetricCrypt или VS_SecureHandshake::GetWriteSymmetricCrypt).
*/
struct VS_ReadWriteCrypt
{
	VS_SymmetricCrypt	*pReadCrypt;
	VS_SymmetricCrypt	*pWriteCrypt;
};
#pragma pack ()

/**
\brief
	Итерфейс реализатора менеджера управления Secure Handshake'ом.

	Определяет интерфейс для классов реализации (участник паттерна bridge).
	Интерфейс предназначен для управления симметричными ключами. VS_SecureHandshake содержит ссылку на этот класс и,
	после инициализации конкретной реализации этого интерфейса, делегирует ему вызовы. Интерфейс является внутренним.
	Пользователь, для работы с симметричными ключами, должен использовать класс VS_SecureHandshake.
*/
class VS_SecureHandshake_Implementation
{
protected:
	static bool		m_ImprovedSecurity;

	virtual void	SetErrorCode(const VS_SecureHandshakeError err_code);
	virtual void	StopHandshakeAct()	= 0;
	virtual bool	InitAct(const VS_SecureHandshakeType type) = 0;

public:
	VS_SecureHandshakeError	m_ErrorCode;

	VS_SecureHandshake_Implementation();
	virtual ~VS_SecureHandshake_Implementation(){}

	bool							Init(const VS_SecureHandshakeType type);
	void							StopHandshake();
	static void						SetImprovedSecurityState(bool state);

	virtual	VS_SecureHandshakeState	Next()														= 0;
	virtual	bool					PreparePacket(void **buf, uint32_t *size)				= 0;
	virtual	void					FreePacketBuf(void **buf) const 							= 0;
	virtual bool					ProcessPacket(const void *buf, const uint32_t size)	= 0;
	virtual VS_SymmetricCrypt*		GetReadSymmetricCrypt()										= 0;
	virtual VS_SymmetricCrypt*		GetWriteSymmetricCrypt()									= 0;
	virtual bool					SetPrivateKey(VS_PKey *pkey)								= 0;
	virtual void					ReleaseSymmetricCrypt(VS_SymmetricCrypt* crypt)const		= 0;
    virtual std::shared_ptr<VS_Certificate> CloneCertificate() const                            = 0;
	virtual bool					GetCertificate(char *buf, uint32_t &sz)				= 0;
};
/**
\brief
	"Пустая" реализация.

	Реализация-заглушка, ничего не делает.
*/
class VS_EmptySecureHandshake_Impl : public VS_SecureHandshake_Implementation
{
private:
public:
	VS_EmptySecureHandshake_Impl();
	virtual ~VS_EmptySecureHandshake_Impl();
	bool InitAct(const enum VS_SecureHandshakeType type) override;
	void StopHandshakeAct() override;
	VS_SecureHandshakeState Next() override;
	bool PreparePacket(void** buf, uint32_t* size) override;
	void FreePacketBuf(void** buf) const override;
	bool ProcessPacket(const void* buf,const uint32_t size) override;
	VS_SymmetricCrypt* GetReadSymmetricCrypt() override;
	VS_SymmetricCrypt* GetWriteSymmetricCrypt() override;
	bool SetPrivateKey(VS_PKey* pkey) override;
	void ReleaseSymmetricCrypt(VS_SymmetricCrypt*  crypt) const override;
    virtual std::shared_ptr<VS_Certificate> CloneCertificate() const override;
	bool GetCertificate(char* buf, uint32_t& sz) override;
};
/**
\brief
	Конкретная реализация менеджера управления Secure Handshake'ом.
	Реализует версию хендшейка = 1.

	Класс не предназначен для внешнего использования, вместо него использовать VS_SecureHandshake.

	@remark

	<b>Описание Secure Handshake'а версии 1.</b>
	- Шаги сервера:
		-#	отправить сертификат X509, подписанный CA;
		-#	отправить список алгоритмов, поддерживаемых сервером;
		-#	получить пакет с ключами и идентификатором симметричного алгоритма;
		-#	сформировать соответствующие объекты VS_SymmetricCrypt.
	- Шаги клиента:
		-#	получить и проверить сертификат. Если сертификат не подписан CA -
			разорвать соединение;
		-#	получить список поддерживаемых алгоритмов и выбрать подходящий (приоритетный AES);
		-#	сгенерировать ключи для расшифровки/шифрования данных, подготовить соответствующие объекты
			VS_SymmetricCrypt;
		-#	подготовить пакет с ключами, идентификатором алгоритма, зашифровать пакет, используя открытый ключ
			сервера (содержащийся в сертификате) и отправить серверу.
*/
class VS_SecureHandshakeVer1_Impl : public VS_SecureHandshake_Implementation
{
protected:
	virtual int GetVersion() const {return 1;}

/**
\brief
	Внутреннее состояние класса.

	Идентифицирует, в каком состоянии находится SecureHandshake.
*/
	enum current_state
	{
		st_none				= 0x0,
		st_init				= 0x1,
		st_start			= 0x2,
		st_send_cert		= 0x4,
		st_send_alg			= 0x6,
		st_get_packet_size	= 0x8,
		st_get_packet		= 0xA,
		st_get_cert_size	= 0xC,
		st_get_cert			= 0xE,
		st_send_result		= 0x10,
		st_finish			= 0x12,

		st_send_err			= 0x14,
		st_handshake_err	= 0x16
	};
	VS_SecureHandshakeType	m_type;				///<тип хендшейка.
	bool					m_GostSupported;
	uint32_t			m_state;			///<внутреннее состояние хендшейка.
	char					*m_somedata;		///<рабочие данные.
	uint32_t			m_size_somedata;	///<текущий размер \e m_somedata.
	VS_PKey					m_pub_key;				///<Содержит закрытый (со стороны сервера) или открытый (со стороны клиента) ключ
	char			*m_private_key_buf; ///<приватный клч
	char			*m_cert_pem_buf;
	uint32_t	m_cert_buf_len;
#ifdef _DEBUG
	std::set<uint32_t> sizes;
#endif

	void PrepareClientErrorMessage(const VS_SecureHandshakeError err_code);
	bool ServerProcErrorMessage(const void *buf, const uint32_t sz);

/**
\brief
	Определяет текущий тип хендшейка.
	@return	<b>true</b>, если хендшейк со стороны сервера, <b>false</b>, если со стороны клиента.
*/
	bool	IsServerHandshake() const;
/**
\brief
	Обработка полученного пакета (сервер-хендшейк).

	\e buf содержит размер пакета, который надо получить следующим шагом.
*/
	bool	ServerProcPacketSize(const void *buf,const uint32_t size);
/**
\brief
	Обработка полученного пакета (сервер-хендшейк).

*/
	virtual bool	ServerProcPacket(const void *buf,const uint32_t size);
/**
\brief
	Обработка полученного пакета (клиент-хендшейк).

	\e buf содержит размер сертификата, который надо получить следующим шагом.
*/
	bool	ClientProcCertSize(const void *buf,const uint32_t size);
/**
\brief
	Обработать сертификат (клиент-хендшейк).

	@param[in]	buf		Сертификат
	@param[in]	size	Количество байт, содержащихся в буфере.
*/
	bool	ClientProcCert(const void *buf,const uint32_t size);
/**
\brief
	Обработка полученного пакета (клиент-хендшейк).
*/
	bool	ClientProcPacketSize(const void *buf,const uint32_t size);
/**
\brief
	Обработка полученного пакета (клиент-хендшейк).
*/
	virtual bool	ClientProcPacket(const void *buf,const uint32_t size);
public:
	VS_SecureHandshakeVer1_Impl();

	virtual ~VS_SecureHandshakeVer1_Impl();
/**
\brief
	Инициализация внутренних полей класса.

	См. VS_SecureHandshake::Init
*/
	bool InitAct(const enum VS_SecureHandshakeType type) override; //установить в начальное состояние
/**
\brief
	Остановить хендшейк.

	VS_SecureHandshake::StopHandshakeAct
*/
	void StopHandshakeAct() override;
/**
\brief
	Возвращает идентификатор действия, которое надо выполнить следующим шагом.

	См. VS_SecureHandshake::Next
*/
	VS_SecureHandshakeState Next() override;
/**
\brief
	Подготовить буфер для отправки/получения даных.

	См. VS_SecureHandshake::PreparePacket
*/
	bool PreparePacket(void** buf, uint32_t* size) override;
/**
\brief
	Освободить буфер, подготовленный PreparePacket.

	См. VS_SecureHandshake::FreePacketBuf
*/
	void FreePacketBuf(void** buf) const override;
/**
\brief
	Обработать полученный пакет даных.

	См. VS_SecureHandshake::ProcessPacket
*/
	bool ProcessPacket(const void* buf, const uint32_t size) override;
/**
\brief
	Получить объект для расшифровки полученных данных при безопасном соединении.

	См. VS_SecureHandshake::GetReadSymmetricCrypt.
*/
	VS_SymmetricCrypt* GetReadSymmetricCrypt() override;
/**
\brief
	Получить объект для шифрации отправляемых даных при безопасном соединении.

	См. VS_SecureHandshake::GetWriteSymmetricCrypt
*/
	VS_SymmetricCrypt* GetWriteSymmetricCrypt() override;
/**
\brief
	Задать закрытый ключ (чтобы расшифровать пакет клиента, содержащий сессионные ключи).

	См. VS_SecureHandshake::SetPrivateKey
*/
	bool SetPrivateKey(VS_PKey* pkey) override;
/**
\brief
	Освободить объекты, полученные с помощью методов GetWriteSymmetricCrypt или GetReadSymmetricCrypt

	См. VS_SecureHandshake::ReleaseSymmetricCrypt
*/
	void ReleaseSymmetricCrypt(VS_SymmetricCrypt* crypt) const override;
    virtual std::shared_ptr<VS_Certificate> CloneCertificate() const override;
	bool GetCertificate(char* buf, uint32_t& sz) override;
};

class VS_SecureHandshakeVer2_Impl: public VS_SecureHandshakeVer1_Impl
{
protected:
	bool ServerProcPacket(const void* buf,const uint32_t size) override;
	bool ClientProcPacket(const void* buf,const uint32_t size) override;
	int GetVersion() const override { return 2; }

	char	m_rnd[256];
public:
	VS_SecureHandshakeVer2_Impl()
	{
		memset(m_rnd,0,256);
	}
	virtual ~VS_SecureHandshakeVer2_Impl(){}
	VS_SecureHandshakeState Next() override;
	bool PreparePacket(void** buf, uint32_t* size) override;
};

/**
\brief
	Предоставляет внешний интерфейс к менеджеру установления безопасного соединения (Secure Handshake).

	Участник паттерна bridge (интерфейс абстракции). Имплементатор реализует разные версии Handshake'а.
	Интерфейс имплементатора определяется классом VS_SecureHandshake_Implementation.

	@remark

	<b>Пользователь для установки безопасного соединения должен пользоваться классом следующим образом:</b>
	- Проинициализировать класс (VS_SecureHandshake::Init), указав нужный номер версии
	(сейчас поддерживается только № 1) и тип SecureHandshake'а (сервер или клиент);
	- Вызвать метод VS_SecureHandshake::Next и, в зависимости от результата, выполнить определенные действия
	(см. нижке);
	- Вызывать VS_SecureHandshake::Next и следовать указаниям до тех пор, пока метод не вернет <b>secure_st_Finish</b> или
	<b>secure_st_Error</b>;
	- Если VS_SecureHandshake::Next вернул <b>secure_st_Finish</b>, то безопасное соединение установлено.
	Вызвав методы VS_SecureHandshake::GetReadSymmetricCrypt и VS_SecureHandshake::GetWriteSymmetricCrypt,
	надо получить подготовленные объекты, которые должны использоваться для шифрации отправляемых данных и для
	расшифровки получаемых;
	- Если VS_SecureHandshake::Next вернул <b>secure_st_Error</b>, то соединение не установлено. Следует остановить хендшейк,
	вызвав метод VS_SecureHandshake::StopHandshake;
	- Объекты, которые были получены методами VS_SecureHandshake::GetReadSymmetricCrypt и
	VS_SecureHandshake::GetWriteSymmetricCrypt, нобходимо удалить используя VS_SecureHandshake::ReleaseSymmetricCrypt,
	когда они будут не нужны;

	<b>Пояснение значений возвращаемых методом VS_SecureHandshake::Next</b>
	- <b>secure_st_SendCert</b>	- может быть только со стороны сервера (\e сервер - тот, кто предоставляет сертификат).
	Получив эту команду надо отправить peer'у сертификат X509, выданный CA;
	- <b>secure_st_SendPacket</b> - может быть как со стороны сервера, так и состороны клиента. Получив эту команду,
	надо подготовить буфер для отправки, вызвав метод VS_SecureHandshake::PreparePacket. Если метод вернул
	<b>true</b>, то отправить подготовленные данные, после отправки освободить буфер, вызвав метод
	VS_SecureHandshake::FreePacket;
	- <b>secure_st_GetPacket</b> - может быть как со стороны сервера, так и со стороны клиента. Получив эту команду,
	надо подготовить буфер для получения данных, вызвав метод VS_SecureHandshake::PreparePacket, если метод вернул
	<b>true</b>, то получить указанное количество байт в этот буфер, затем его обработать методом
	VS_SecureHandshake::ProcessPacket (\e buf - буфер с полученными даными, \e size - количество реально полученных байт),
	освободить буфер, вызвав метод VS_SecureHandshake::FreePacket;
	- <b>secure_st_Error</b> - может быть как со стороны сервера, так и со стороны клиента. При установке соединения
	произошла ошибка. Соединение не установлено, следует завершить хендшейк вызовом VS_SecureHandshake::StopHandshake.
	- <b>secure_st_Finish</b> - может быть как со стороны сервера, так и со стороны клиента. Означает, что безопасное
	соединение установлено. Объекты для шифрации/расшифровки данных готовы, их надо получить при помощи методов
	VS_SecureHandshake::GetReadSymmetricCrypt (для расшифровки принятых данных)и VS_SecureHandshake::GetWriteSymmetricCrypt
	(для шифрования отправляемых данных)
*/
class VS_SecureHandshake
{
private:
	VS_SecureHandshake_Implementation*	imp;///< Указатель на реализатор менеджера установки безопасного соединения определенной версии.
	uint32_t						m_Version;///< Номер версии. До инициализации = 0xFFFFFFFF (-1)
	static	char						*m_RegistryRoot;
public:
	static void SetRegistryRoot(const char *root);
	static bool GetRegistryRoot(char *out, unsigned int &buflen);
	static void SetImprovedSecurityState(bool state);
/**
\brief
	Конструктор по-умолчанию.
*/
	VS_SecureHandshake(void);
/**
\brief
	Конструктор и инициализатор.

	Вызывает метод VS_SecureHandshake::Init.
*/
	VS_SecureHandshake(const uint32_t Version, const VS_SecureHandshakeType type);
	~VS_SecureHandshake(void);
/**
\brief
	Инициализация класса.

	Начало хендшейка. Необходимо вызвать перед начлом.
	@param[in]	Version	Номер версии Secure Handshake (на текущий момент реализована 1-я версия).
	@param[in]	type	Тип хендшейка. Определяет с чьей стороны будет происходить хендшейк (сервер, клиент).
	@return		Если инициализация прошла успешно вернет <b>true</b>, <b>false</b> в противном случае (например номер версии указан некорректно).
*/
	bool Init(const uint32_t Version, const VS_SecureHandshakeType type);
/**
\brief
	Остановить хендшейк.

	Освобождает все ресурсы, выделенные для хендшейка и устанавливает состояние класса
	в неинициализированное состояние (при повторном хендщейке метод VS_SecureHandshake::Init надо вызвать снова).
*/
	void StopHandshake();
/**
\brief
	Возвращает идентификатор действия, которое надо выполнить следующим шагом.

	См. Примечание.
*/
	VS_SecureHandshakeState Next();
/**
\brief
	Подготовить пакет к отправке или получению данных.

	Класс скрывает процесс подготовки буферов для отправки и получения данных. После вызова метода,
	пользователь получает буфер готового размера и, в случае отправки пакета, готового содержания. Как поступать
	с полученным буфером зависит от результато предыдущего вызова VS_SecureHandshake::Next.

	@param[out]	buf		Будет указывать на подготовленный буфер.
	@param[out]	size	Будет содержать размер полученного буфера в байтах.
	@return				Если не было ошибок - <b>true</b>,<b>false</b> в противном случае.

	@remark	Все аодготовленные пакеты должны быть освобождены методом VS_SecureHandshake::FreePacket.
*/
	bool PreparePacket(void** buf, uint32_t* size);
/**
\brief
	Освобождает буферы, подготовленные методом VS_SecureHandshake::PreparePacket.

	@param[in,out]	Буфер, подготовленный VS_SecureHandshake::PreparePacket.
*/
	void FreePacket(void** buf);
/**
\brief
	Обработка полученного пакета.

	Этот метод надо вызвать после получения пакета от peer'а.
	@param[in]	buf		Полученный пакет данных.
	@param[in]	size	размер \e buf.
	@return				Если полученный пакет "правильный" - вернет <b>true</b>, <b>false</b> в противном случае.
*/
	bool ProcessPacket(const void* buf, const uint32_t size);
/**
\brief
	Получить объект для расшифровки полученных данных.

	Вызывается после установки безопасного соединения, чтобы получить конкретный, уже инициализированный объект
	для выполнения расшифровки данных.

	@remark	Для удаления объекта, полученного таким способом, надо воспользоваться методом VS_SecureHandshake::ReleaseSymmetricCrypt.
*/
	VS_SymmetricCrypt* GetReadSymmetricCrypt();
/**
\brief
	Получить объект для шифрования отправляемых данных.

	Вызывается после установки безопасного соединения, чтобы получить конкретный, уже инициализированный объект
	для выполнения шифрования данных.

	@remark	Для удаления объекта, полученного таким способом, надо воспользоваться методом VS_SecureHandshake::ReleaseSymmetricCrypt.
*/
	VS_SymmetricCrypt* GetWriteSymmetricCrypt();
/**
\brief
	Задать закрытый ключ сертификата.

	Нужен только при хендшейке со стороны сервера, чтобы расшифровать ключи, пришедшие от клиента.
	@param[in]	pkey		Закрытый ключ.

	@return <b>true</b> - все OK, <b>false</b> - была ошибка.
*/
	bool SetPrivateKey(VS_PKey* pkey);
/**
\brief
	Освобождает объекты, полученные методами VS_SecureHandshake::GetReadSymmetricCrypt и
	VS_SecureHandshake::GetWriteSymmetricCrypt

	@param[in,out]	Указатель на указаьель, указывающий на объект типа VS_SymmetricCrypt, полченный одним из
					методов VS_SecureHandshake::GetReadSymmetricCrypt или VS_SecureHandshake::GetWriteSymmetricCrypt.

*/
	static void				ReleaseSymmetricCrypt(VS_SymmetricCrypt **crypt);
    virtual std::shared_ptr<VS_Certificate> CloneCertificate() const;
	bool					GetCertificate(char *buf, uint32_t &sz);
/**
\brief
	Получить номер текуще версии.
*/
	uint32_t GetVersion() const;
	VS_SecureHandshakeError	GetHandshakeErrorCode();
};