#pragma once
/** \file SecureTypes.h
 * \brief Файл содержит описание общеиспользуемых типов.
*/
#include <string>
#include <vector>

#define VS_PASS_SIZE		256
#define VS_MAX_KEY_LENGTH	256	///< Максимальная длина ключа
#define VS_DEBUG_SSL_TAG	"Debug SSL"
//#define SERVER_CERT_PRIVATE_KEY_PASS "cKe4-vbmre 0re=48uhgnpPiGf#$%^kltRR;lmkdpo"

//extern const char *VS_PEM_CACERT;
#pragma pack (1)
/**
\brief Список поддерживаемых криптоалгоритмов с открытыми ключами
*/
enum VS_PublicKeyAlg
{
	alg_pk_NONE	= 0,	///< алгоритм не определен
	alg_pk_RSA	= 1,	///< RSA
};
/**
\brief Список поддерживаемых хеш алгоритмов
*/
enum VS_HashAlg
{
	alg_hsh_NONE	= 0,	///< алгоритм не определен
	alg_hsh_SHA1	= 4,	///< SHA1
	alg_hsh_SHA256 = 8 ///< SHA256
};
#define VS_COUNT_SYMMETRIC_ALG	10	///< Количество поддерживаемых симметричных алгоритмов
/**
\brief
Список поддерживаемых симметричных криптоалгоритмов
*/
enum VS_SymmetricAlg
{
	alg_sym_NONE		= 0,	///<алгоритм не определен
	alg_sym_AES128		= 1,	///<AES, 128-bit key
	alg_sym_AES256		= 11,	///<AES, 256-bit key
	alg_sym_GOST		= 12	///<GOST
};

/**
\brief
Определяет тип симметричного шифра и режим шифрования
*/
enum VS_SymmetricCipherMode
{
	mode_NONE	= 0,	///<режим не определен
	mode_STREAM	= 1,	///<поточный шифр
	mode_ECB	= 2,	///<блочный шифр, режим ECB
	mode_CBC	= 3,	///<блочный шифр, режим CBC
	mode_CFB	= 4,	///<блочный шифр, режим CFB
	mode_OFB	= 5		///<блочный шифр, режим OFB
};

/**
\brief
* Формат хранилища данных
*
* Обычно используется в связке с неким буфером, чтобы определить какого
* рода хранилище используется для передачи данных. В зависимости от значения, буфер может
* интерпретироваться как имя файла или как буфер, содержащий даные
*/
enum VS_StorageType
{
	store_PEM_FILE	= 1,	///<даные в файле в формате PEM
	store_PEM_BUF	= 2,	///<даные в буфере в формате PEM
	store_DER_FILE	= 3,	///<даные в файле в формате DER
	store_DER_BUF	= 4,	///<даные в буфере в формате DER
	store_PLAIN_BUF	= 5,	///<---
	store_BER_FILE	= 6,	///<даные в файле в формате BER
	store_BER_BUF	= 7		///<даные в буфере в формате BER
};
/**
\brief
Параметры для подписывания данных.
*/
struct VS_SignArg
{
	VS_PublicKeyAlg	PKtype;		///<Идентификатор алгоритма подписи (тип ключа)
	VS_HashAlg		HASHtype;	///<Идентификатор хеш-агоритма, который будет использоваться для одписи данных.
};

/**
\brief
	Содердит информацию о том, где хранится закрытый ключ и каким паролем он зашифрован.
*/
struct VS_PrivateKeyStorage
{
	unsigned char	pass[VS_PASS_SIZE];
	char			*storage;
	uint32_t	len; //len of storage
};
enum VS_SecureHandshakeError
{
	e_err_NoError,
	e_err_NotInit,
	e_err_SymmetricCryptAlgIsNotSupport,
	e_err_BufSizeExpectedValue,
	e_err_DataAreNotCertificate,
	e_err_VerificationFailed,
	e_err_ArgNotValid,
	e_err_DecryptKeys,
	e_err_UnknownError,

    e_err_Cert_expired,     //
    e_err_Cert_not_yet_valid,   //
    e_err_Cert_is_invalid,
    e_err_Srv_name_is_invalid,
    e_err_Version_do_not_match,
};

enum class encryption_method : uint8_t {
	padding,
	cts,			// ciphertext stealing
	unknown
};

struct encryption_meta {
	bool succsess;            // encryption/decryption succsess
	encryption_method encr_m; // used enctyption method
	encryption_meta() :succsess(false), encr_m(encryption_method::unknown) {}
	encryption_meta(bool _succsess, encryption_method _encr_m) :succsess(_succsess), encr_m(_encr_m) {}
};
#pragma pack ()

typedef enum {
	SUBJ_ALT_NAME_DNS,
	SUBJ_ALT_NAME_IP,
	SUBJ_ALT_NAME_EMAIL,

	SUBJ_ALT_NAME_MAX
} SubjectAltNameType;

typedef std::pair<SubjectAltNameType, std::string> SubjAltNamePair;
typedef std::vector<SubjAltNamePair> SubjAltNameExtensionsSet;
