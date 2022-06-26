#pragma once
/** \file VS_PublicKeyCrypt.h
* \brief Файл описывает интерфейсы и классы манипуляции Public и Private ключами
*/
#include "SecureLib/SecureTypes.h"
#include "SecureLib/OpenSSLTypesWrapDefs.h"

/**
\brief
Абстрагирует ключ в криптографии с открытым ключом (Public Key и Private Key)
*/
class VS_PKey
{
private:
	vs::EvpPKeyPtr m_pkey;
public:
	VS_PKey();
	~VS_PKey();
	VS_PKey(const VS_PKey&) = delete;
	VS_PKey(VS_PKey&&);

	VS_PKey& operator=(const VS_PKey&) = delete;
	VS_PKey& operator=(VS_PKey&&);
/**
	\brief Возвращает, какой алгоритм используется.
*/
	VS_PublicKeyAlg GetPKeyAlg() const;
/**
	\brief Генерит и задает ключ определенной длины.

	@param[in]	keylen	длина ключа.
	@param[in]	alg		алгоритм, для которого генерится ключ
	@return	Если все хорошо - <b>true</b>, <b>false</b> в противном случае.
*/
	bool			GenerateKeys(const uint32_t keylen, const VS_PublicKeyAlg alg);
/**
	\brief Проверяет ключ на стойкость.

	@return возвращает <b>true</b>, если ключ достаточно надежен, <b>false</b> в противном случае.
*/
	bool			CheckKeyStrength() const;
/**
	\brief Получить длину ключа.

	@return возврашает длину кдлюча или 0, если ключ не задан.
*/
	uint32_t	GetKeyLength() const;
/**
	\brief Задать открытый ключ.
	@param[in]	key_source	может быть именем файла, где сохранен ключ, либо буфером, который
							содержит этот коюч в определенном формате. Как интерпретировать этот аргумент зависит
							от значения \e type
	@param[in]	type		определяет, как интерпретировать \e key_source.
	@return	<b>true</b> если оператция выполнилась успешно, <b>false</b> в противном случае.
*/
	bool			SetPublicKey(const char *key_source, const VS_StorageType type);
/**
	\brief Задать закрытый ключ.

	@param[in]	key_source	может быть именем файла, где сохранен ключ, либо буфером, который
							содержит этот коюч в определенном формате. Как интерпретировать этот аргумент зависит
							от значения \e type
	@param[in]	type		определяет, как интерпретировать \e key_source.
	@return	<b>true</b> если оператция выполнилась успешно, <b>false</b> в противном случае.
*/
	bool			SetPrivateKey(const char *key_source, const VS_StorageType type, const char *pass = "");
/**
	\brief Записать открытый ключ в файл.

	@param[in]	file_name	имя файла, куда надо записать ключ.
	@param[in]	format		формат, в которм надо записать ключ (Сейчас поддерживается только формат PEM,
							то есть format может принимать значение только store_PEM_FILE)
	@return	Если все хорошо <b>true</b>, <b>false</b> в противном случае.
*/
	bool			WritePublicKey(const char *file_name, const VS_StorageType format);
/**
	\brief Получить открытый ключ в буфер.

	@param[in]	format		идентифицирует формат, в котором метод вернет открытый ключ.
	@param[out]	buf			указатель на буфер, куда метод запишет ключ.
	@param[in, out]	buflen	при входе содержит размер буфера, при выходе будет содержать
							количество записаных байт.
	@return					Возвращает <b>true</b>, если операция выполнена успешна, \e buflen
							будет содержать количество записанных байт. <b>false</b> при ошибке.
	@remark					Если \e buflen будет меньше
							необходимого, то метод вернет <b>false</b>, а \e buflen,
							при выходе из метода, будет содержать необоходимый размер буфера.
*/
	bool			GetPublicKey(const VS_StorageType format, char *buf, uint32_t *buflen);
/**
	\brief	Записать закрытый ключ в файл в заданном формате.

	@param[in]	fiename		имя файла, куда будет записан ключ.
	@param[in]	format		формат файла.
	@param[in]	enc			симметричный алгоритм шифрования, которым будет зашифрован закрытый ключ.
							Если значение равно alg_sym_NONE, то закрытый ключ шифроваться не будет.
	@param[in]	mode		режим симметричного шифрования (cbc, ecb и т.д.).
	@param[in]	pass		пароль, который будет использоваться для шифрования как ключ. Если параметр равен 0,
							то шифрации производиться не будет.
	@param[in]	pass_len	длина пароля.
	@return		При успешном выполнении операции вернет <b>true</b>, иначе <b>false</b>

	@remark		аргумент \e faormat сейчас может принимать толлько значение store_PEM_FILE.
				То есть поддерживается только формат PEM.
*/
	bool			WritePrivateKey(char *dist, uint32_t &sz, const VS_StorageType format,
						const VS_SymmetricAlg enc = alg_sym_NONE,
						const VS_SymmetricCipherMode mode = mode_NONE, const char *pass = "",
																		const int pass_len = 0);
/**
	\brief	Получить закрытый ключ в буфер.

	@param[in]		format	определяет формат, в котором ключ будет записан в буфер.
	@param[out]		buf		сюда будет записан ключ.
	@param[in,out]	buflen	задает длину буфера в байтах, после содержит
							количество записаных в буфер байт.
	@return			При успешном выполнении операции <b>true</b> иначе <b>false</b>
	@remark			Если размер буфера будет недостаточной длины, то метод вернет <b>false</b>,
					а \e buflen будет содержать необходимый размер буфера.
*/
	bool			GetPrivateKey(const VS_StorageType format, char *buf, uint32_t *buflen);
/**
	Получить указатель на контекст ключа.
	@return Указатель на контекст, 0, если ключ не задан.
	@remark	Контекст имеется в виду в рамках используемой библиотеки (провайдера),
	на основе которой выбрана реализация. Использовать можно с классами, основанными
	на той же библиотеке.
*/
	template<class ContextPtr /* = EVP_PKEY * */>
		ContextPtr GetKeyContext();
};

/**
\brief
	Предоставляет внешний интерфейс к механизму шифрации данных используя
	криптографию открытого ключа (алгоритм RSA).

	Участник патерна bridge. Имплементатор может быть реализован на основе разных
	библиотек (на случай легкого перехода на другую реализацию), если от OpenSSL
	надо будет отказаться. Интерфейс имплиментатора определяется VS_PKeyCrypt_Implementation.
*/
class VS_PKeyCrypt
{
private:
	VS_PKey					key;
public:
/**
\brief
	Задать открытый ключ

	@param[in]	key_source	может быть либо именем файла ключа,
	либо буфером, который содержит сам ключ (зависит от второго параметра).
	@param[in]	type определет что содержит key_source и в каком формате.
	@return	<b>true</b>, если операция выполнилась успешно, <b>false</b> в противном случае.
*/
	bool			SetPublicKey(const char *key_source, const VS_StorageType type);
/**
\brief
	Задать открытый ключ, содержащийся в аргумента \e key.

	@param[in]	key	задаваемый ключ.
	@return		<b>true</b> если удалось задать ключ, в противном случае <b>false</b>
	(например, когда \e key не содержит открытого ключа).

*/
	bool			SetPublicKey(VS_PKey *key);
/**
\brief
	Задать закрытый ключ.

	@param[in]	key_source	может быть либо именем файла, где находится ключ,
	либо буфером, который содержит сам ключ (зависит от второго параметра).
	@param[in]	type определет что содержит key_source и в каком формате.
	@return	<b>true</b>, если операция выполнилась успешно, <b>false</b> в противном случае (например ключ не найден).
*/
	bool			SetPrivateKey(const char *key_source, const VS_StorageType type);
/**
\brief
	Задать закрытый ключ, содержащийся в аргумента \e key.

	@param[in]	key	задаваемый ключ.
	@return		<b>true</b> если удалось задать ключ, в противном случае <b>false</b>
	(например, когда \e key не содержит закрытого ключа).
*/
	bool			SetPrivateKey(VS_PKey *key);
/**
\brief
	Получить длину ключа.
*/
	uint32_t	GetKeyLength();
/**
\brief
	Зашифровать данные.

	Метод генерирует ключ для симметричного шифрования и iv (вектор инициализации), используя
	сгенерированные данные шифрует данные, которые были переданы на вход, сам ключ и iv шифруются
	открытым ключом RSA.

	@param[in]		data		Данные, которые надо зашифровать (они шифруются симметричным ключом).
	@param[in]		data_len	Размер \e data в байтах.
	@param[in]		alg			Идентификатор симметричного алгоритма, котоым будет
								произведена шифрация \e data.
	@param[out]		encr_buf	Буфер, который будет содержать зашифрованные данные из \e data.
	@param[in,out]	buf_len		На входе содержит размер \e encr_buf, на выходе количество байт записанных
								в \e encr_buf.
	@param[out]		iv			Будет содержать вектор инициализации, сгенерированный методом.
								Его размер должен быть 16 байт(необходим для шифрации данных).
	@param[out]		sym_key		Буфер, который будет содержать зашифрованный открытым ключом
								сессионный ключ, которым были зашифрован \e data.
	@param[in,out]	sym_key_len	При входе длина \e sym_key, при выходе содержит количество записанных
								байт.
	@return						При успешном выполнении операции возвращает <b>true</b>,
								<b>false</b> в противном случае.

	@remark						Если размер \e encr_buf будет недостаточным, метод вернет <b>false</b>,
								а \e buf_len будет содержать необходимый размер буфера. Если размер \e sym_key
								будет недостаточным, метод вернет <b>false</b>, а \e sym_key_len будет содержать
								необходимый размер буфера.

*/
	bool			Encrypt(const unsigned char *data, const uint32_t data_len,
							const VS_SymmetricAlg sym_alg,
							unsigned char *encr_buf, uint32_t *buf_len,
							unsigned char iv[16], unsigned char *sym_key,
							uint32_t *sym_key_len);
/**
\brief
	Расшифровать данные.

	Данные зашифрованы симметричным алгоритмом используя ключ, который, в свою очередь, зашифрован
	открытым ключом RSA. Таким образом, сначала расшифровывается симметричный ключ закрытым ключом RSA,
	а потом	сами данные.

	@param[in]		encr_buf	Буфер, содержащий зашифрованные данные.
	@param[in]		buf_len		Размер \e encr_buf.
	@param[in]		alg			Идентификатор алгоритма, которым были зашифрованы даные.
	@param[in]		iv			Вектор инициализации, который использовался для шифрации
								данных (генерируется в методе VS_PKey::Encrypt).
	@param[in]		encr_key	Зашифрованный открытым ключом сессионный ключ
								(для расшифровки данных).
	@param[in]		key_len		Размер \e encr_key в байтах.
	@param[out]		decr_data	Буфер, который будет содержать расшифрованные данные.
	@param[in,out]	decr_len	При входе размер \e decr_data, при выходе количество
								записанных байт.
	@return						Если удалось расшифровать данные <b>true<\b>, <b>false<\b>
								в противном случае.

	@remark						Если размера буфера \e decr_data будет недостаточно для записи расшифрованных данных,
								то метод вернет <b>false</b>, а @decr_len будет содержать необходимый размер дынных.

*/
	bool			Decrypt(const unsigned char *encr_buf, const uint32_t buf_len,
							const VS_SymmetricAlg sym_alg,const unsigned char iv[16],
							const unsigned char *encr_key, const uint32_t key_len,
							unsigned char *decr_data, uint32_t *decr_len);
};