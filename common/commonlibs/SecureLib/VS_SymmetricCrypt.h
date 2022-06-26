#pragma once
#include "SecureLib/OpenSSLTypesWrapDefs.h"
#include "SecureLib/SecureTypes.h"

/**
\file VS_SymmetricCrypt.h

\brief
	Файл содержит объявления классов, работающих с симметричной криптографией.
*/

/**
\brief
	Абстрагирует ключ в симметричной криптографии.

	Участник паттерна bridge (интерфейс абстракции). Определяет внешний интерфейс симметричного ключа.
	Реализатор млжет быть реализован на основе разных библиотек (на случай легкого перехода на
	другую реализацию), если от OpenSSL надо будет отказаться. Интерфейс реализатор определяет
	класс VS_SymmetricKey_Implementation.
*/
class VS_SymmetricKey
{
private:
	unsigned char		*m_key;		///< Сам ключ.
	uint32_t		m_keylen;	///< Длина ключа.
public:
	VS_SymmetricKey();
	~VS_SymmetricKey();

/**
\brief
	Инициализирует реализатор.

	Необходимо выполнить перед использованием.
	@param[in]	prov	Определяет поставщика (библиотеку), который
						будет использоваться для реализации крипто-процедур
						(в нашем случае только OpenSSL, а значение prov_OPENSSL).
	@return				Если инициализация прошла успешно, то вернет <b>true</b>,
						<b>false</b> в противном случае.
*/
	bool				Init();
/**
\brief
	Сгенерировать ключ определенной длины.

	@param[in]	length	Длина ключа в байтах.
	@param[out]	keybuf	Буфер, в который будет помещен сгенерированный ключ.
						Размер буфера должен быть не меньше, чем \e length.
	@return				В случае успешной выполнения операции вернет <b>true</b>,
						в противном случае <b>false</b>.

	@remark				Не задает ключ конкретному экземпляру. Для того, чтобы задать объекту ключ
						надо использовать VS_SymmetricKey::SetKey
*/
	bool				GenerateKey(const uint32_t length, unsigned char *keybuf) const;
/**
\brief
	Проверить ключ на стойкость.
	@return	Если ключ достаточной ндежности вернет <b>true</b>, иначе <b>false</b>.
*/
	bool				CheckKeyStrength() const;
/**
\brief
	Получить длину ключа.

*/
	uint32_t		GetKeyLength() const;
/**
\brief
	Задать ключ.

	@param[in]	length	Длина ключа.
	@param[in]	key		Сам ключ..
	@return				Если удалось задать ключ - <b>true</b>, <b>false</b> в противном случае.
*/
	bool				SetKey(const uint32_t lenght, const unsigned char *key);
/**
\brief
	Получить текущий ключ в буфер.

	@param[in]	buflen	Размер передаваемого буфера в байтах.
	@param[out]	keybuf	Сам буфер, куда будет помещен ключ. Размер буфера должен быть достаточного размера,
						чтобы поместился ключ. Для того, длину ключа вернет VS_SymmetricKey::GetKeyLength.
	@param[out]	keylen	Будет содержать длину ключа.

	@return	Если все в порядке, то <b>true</b>, <b>false</b> если размер буфера меньше, чем длина ключа.

*/
	bool				GetKey(const uint32_t buflen, unsigned char *keybuf, uint32_t *keylen) const;
/**
\brief
	Очистить ключ.

	Удаляет текущий ключ.
*/
	void				ClearKey();
};

/**
\brief
	Предоставляет внешний интерфейс к механизму шифрации данных
	симметричными алгоритмами.

	Участник паттерна bridge (интерфейс абстракции). Имплементатор
	может быть реализован на основе разных библиотек (на случай легкого
	перехода на другую реализацию), если от OpenSSL надо будет отказаться.
	Интерфейс имплементатора определяется классом VS_SymmetricCrypt_Implementation.
*/
class VS_SymmetricCrypt
{
	VS_SymmetricKey	m_key;	///< Ключ.
	vs::EvpCipherCtxRawPtr m_encrCTX;		///<Контекст для шифрации.
	vs::EvpCipherCtxRawPtr m_decrCTX;		///<Контекст для расшифровки
	VS_SymmetricAlg	m_Alg;			///<Идентификатор алгоритма
	VS_SymmetricCipherMode m_CipherMode;	///<Режим и вид шифра.
protected:
	template<typename CTX_PTR /*= EVP_CIPHER_CTX* */>
	CTX_PTR GetEncryptCtx();
	template<typename CTX_PTR /*= EVP_CIPHER_CTX* */>
	CTX_PTR GetDecryptCtx();
/**
\brief
	Определяет внутреннее состояние класса VS_SymmetricCryptOpenSSL_Impl
*/
	enum current_state
	{
		st_none		= 0x0,
		st_ctx_init	= 0x1

	} m_state;
public:
	VS_SymmetricCrypt();
	virtual ~VS_SymmetricCrypt();
/**
/brief
	Инициализация реализатора.

	@param[in]	prov			Определяет поставщика (библиотеку), который будет использоваться для реализации
								крипто-процедур (в нашем случае только OpenSSL, а значение prov_OPENSSL).
	@param[in]	alg				Идентификатор алгоритма, который будет использоваться. (См. VS_SymmetricAlg)
	@param[in]	mode			Режим и вид алгоритма (поточный, блочный, cbc, ebc и т. д.)
	@param[in]	key_len			Длина ключа

	@return						Если инициализация прошла успешно - <b>true</b>, <b>false</b> в противном случае.

*/
	bool					Init(const VS_SymmetricAlg alg, const VS_SymmetricCipherMode mode);
/**
\brief
	Сгенерировать ключ определенной длиный.

	@param[in]	len	Длина генерируемого ключа.
	@param[out]	buf	Буфер, который будет содержать ключ. Размер буфера должен быть не меньше, чем len

	@return		<b>true</b>, если все хорошо, <b>false</b> - в противном случае.
*/
	bool GenerateKey(const uint32_t len,unsigned char *buf) const;
/**
\brief
	Задать ключ для шифрации/расшифрации.

	@param[in]	len	Длтна ключа в байтах.
	@param[in]	buf	Буфер, содержащий ключ.

	@return			<b>true</b>, если операция прошла успешно, <b>false</b> - в противном случае.
*/
	bool					SetKey(const uint32_t len, const unsigned char *buf);
/**
\brief
	Получить ключ.

	@param[in]	buflen	Дина передаваемого буфера в байтах.
	@param[out]	keybuf	Буфер, который будет содержать ключ
	@param[out]	keylen	Здесь будет содержаться длина ключа.

	@return				<b>true</b>	- все OK, <b>false</b> - в противном случае.

*/
	bool					GetKey(const uint32_t buflen, unsigned char *keybuf, uint32_t *keylen) const;
/**
\brief
	Проверить	ключ на стойкость.

	@return	<b>true</b> ключ достаточной надежности и его можно использовать, <b>false</b> ключ ненадежен,
			лучше задать другой.
*/
	bool					ChekKeyStrength() const;
/**
\brief
	Получить размер блока.

	@return	Размер блока.
*/
	uint32_t			GetBlockSize() const;
	//uint32_t			SetBlockSize(const uint32_t size);
/**
\brief
	Получить длину ключа.
	@return	Длина ключа.
*/
	uint32_t			GetKeyLen() const;
/**
\brief
	Получить идентификатор используемого алгоритма.

	@return	Если не инициализирован, то возвращает alg_sym_NONE.
*/
	VS_SymmetricAlg			GetAlg() const;
/**
\brief
	Получить идентификатор типа шифра.

	@return	Если не инициализирован, то возвращает mode_NONE.
*/
	VS_SymmetricCipherMode	GetCipherMode() const;
/**
\brief
	Зашифровать даные.

	Производит шифрацию даных алгоритмом, указанным при инициализации и заданным ключом.
	@param[in]		data		Даные, которые надо зашифровать.
	@param[in]		data_len	Длина \e data в байтах
	@param[out]		encr_buf	Буфер, в который помещаются зашифрованные данные.
	@param[in,out]	buf_len		На входе размер \e encr_buf, на выходе размер шифротекста.

	@return			<b>true</b>, все OK, <b>false</b> - ошибка.
	@remark			Если размера \e encr_buf не достаточно, то метод вернет <b>false</b>, а \e buf_len
					будет содержать необходимый размер.
*/
	bool					Encrypt(const unsigned char* data, const uint32_t data_len,
										  unsigned char* encr_buf,	 uint32_t *buf_len /**[in,out]*/) const;
/**
\brief
	Расшифровать данные.

	Восстанавливает исходное сообщение по шифротексту.
	@param[in]		encr_buf	Буфер с шифротекстом.
	@param[in]		buf_len		Размер \e encr_buf
	@param[out]		decr_data	Буфер, в котором будет содержаться расшифрованное сообщение
	@param[in,out]	decr_len	На входе длина \e decr_data, на выходе длина расшифрованного сообщения.

	@return			<b>true</b>, если все OK, <b>false</b> - если произошла ошибка.
	@remark			Если размера \e decr_data не достаточно, то метод вернет <b>false<\b>, а \e decr_len
					будет содержать необходимый размер.
*/
	bool					Decrypt(const unsigned char* encr_buf, const uint32_t buf_len,
										  unsigned char* decr_data,		 uint32_t *decr_len/**[in,out]*/) const;
/**
\brief
	Всё ли сообщение было востановлено или переданные даные были неполные.

	Вспомогательный метод. Используется во внутренней реализации.
	Если было зашифровано и передано N байт, а получили и попытались расшифровать
	M<N байт, то, после расшифровки, метод вернет <b>false</b> до тех пор, пока не будут
	переданы на расшифровку оставшиеся N-M байт.
*/

	VS_SymmetricCrypt & operator =(const VS_SymmetricCrypt &src);
	VS_SymmetricCrypt(const VS_SymmetricCrypt&) = delete;
};