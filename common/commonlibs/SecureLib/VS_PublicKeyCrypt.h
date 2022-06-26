#pragma once
/** \file VS_PublicKeyCrypt.h
* \brief ���� ��������� ���������� � ������ ����������� Public � Private �������
*/
#include "SecureLib/SecureTypes.h"
#include "SecureLib/OpenSSLTypesWrapDefs.h"

/**
\brief
������������ ���� � ������������ � �������� ������ (Public Key � Private Key)
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
	\brief ����������, ����� �������� ������������.
*/
	VS_PublicKeyAlg GetPKeyAlg() const;
/**
	\brief ������� � ������ ���� ������������ �����.

	@param[in]	keylen	����� �����.
	@param[in]	alg		��������, ��� �������� ��������� ����
	@return	���� ��� ������ - <b>true</b>, <b>false</b> � ��������� ������.
*/
	bool			GenerateKeys(const uint32_t keylen, const VS_PublicKeyAlg alg);
/**
	\brief ��������� ���� �� ���������.

	@return ���������� <b>true</b>, ���� ���� ���������� �������, <b>false</b> � ��������� ������.
*/
	bool			CheckKeyStrength() const;
/**
	\brief �������� ����� �����.

	@return ���������� ����� ������ ��� 0, ���� ���� �� �����.
*/
	uint32_t	GetKeyLength() const;
/**
	\brief ������ �������� ����.
	@param[in]	key_source	����� ���� ������ �����, ��� �������� ����, ���� �������, �������
							�������� ���� ���� � ������������ �������. ��� ���������������� ���� �������� �������
							�� �������� \e type
	@param[in]	type		����������, ��� ���������������� \e key_source.
	@return	<b>true</b> ���� ��������� ����������� �������, <b>false</b> � ��������� ������.
*/
	bool			SetPublicKey(const char *key_source, const VS_StorageType type);
/**
	\brief ������ �������� ����.

	@param[in]	key_source	����� ���� ������ �����, ��� �������� ����, ���� �������, �������
							�������� ���� ���� � ������������ �������. ��� ���������������� ���� �������� �������
							�� �������� \e type
	@param[in]	type		����������, ��� ���������������� \e key_source.
	@return	<b>true</b> ���� ��������� ����������� �������, <b>false</b> � ��������� ������.
*/
	bool			SetPrivateKey(const char *key_source, const VS_StorageType type, const char *pass = "");
/**
	\brief �������� �������� ���� � ����.

	@param[in]	file_name	��� �����, ���� ���� �������� ����.
	@param[in]	format		������, � ������ ���� �������� ���� (������ �������������� ������ ������ PEM,
							�� ���� format ����� ��������� �������� ������ store_PEM_FILE)
	@return	���� ��� ������ <b>true</b>, <b>false</b> � ��������� ������.
*/
	bool			WritePublicKey(const char *file_name, const VS_StorageType format);
/**
	\brief �������� �������� ���� � �����.

	@param[in]	format		�������������� ������, � ������� ����� ������ �������� ����.
	@param[out]	buf			��������� �� �����, ���� ����� ������� ����.
	@param[in, out]	buflen	��� ����� �������� ������ ������, ��� ������ ����� ���������
							���������� ��������� ����.
	@return					���������� <b>true</b>, ���� �������� ��������� �������, \e buflen
							����� ��������� ���������� ���������� ����. <b>false</b> ��� ������.
	@remark					���� \e buflen ����� ������
							������������, �� ����� ������ <b>false</b>, � \e buflen,
							��� ������ �� ������, ����� ��������� ������������ ������ ������.
*/
	bool			GetPublicKey(const VS_StorageType format, char *buf, uint32_t *buflen);
/**
	\brief	�������� �������� ���� � ���� � �������� �������.

	@param[in]	fiename		��� �����, ���� ����� ������� ����.
	@param[in]	format		������ �����.
	@param[in]	enc			������������ �������� ����������, ������� ����� ���������� �������� ����.
							���� �������� ����� alg_sym_NONE, �� �������� ���� ����������� �� �����.
	@param[in]	mode		����� ������������� ���������� (cbc, ecb � �.�.).
	@param[in]	pass		������, ������� ����� �������������� ��� ���������� ��� ����. ���� �������� ����� 0,
							�� �������� ������������� �� �����.
	@param[in]	pass_len	����� ������.
	@return		��� �������� ���������� �������� ������ <b>true</b>, ����� <b>false</b>

	@remark		�������� \e faormat ������ ����� ��������� ������� �������� store_PEM_FILE.
				�� ���� �������������� ������ ������ PEM.
*/
	bool			WritePrivateKey(char *dist, uint32_t &sz, const VS_StorageType format,
						const VS_SymmetricAlg enc = alg_sym_NONE,
						const VS_SymmetricCipherMode mode = mode_NONE, const char *pass = "",
																		const int pass_len = 0);
/**
	\brief	�������� �������� ���� � �����.

	@param[in]		format	���������� ������, � ������� ���� ����� ������� � �����.
	@param[out]		buf		���� ����� ������� ����.
	@param[in,out]	buflen	������ ����� ������ � ������, ����� ��������
							���������� ��������� � ����� ����.
	@return			��� �������� ���������� �������� <b>true</b> ����� <b>false</b>
	@remark			���� ������ ������ ����� ������������� �����, �� ����� ������ <b>false</b>,
					� \e buflen ����� ��������� ����������� ������ ������.
*/
	bool			GetPrivateKey(const VS_StorageType format, char *buf, uint32_t *buflen);
/**
	�������� ��������� �� �������� �����.
	@return ��������� �� ��������, 0, ���� ���� �� �����.
	@remark	�������� ������� � ���� � ������ ������������ ���������� (����������),
	�� ������ ������� ������� ����������. ������������ ����� � ��������, �����������
	�� ��� �� ����������.
*/
	template<class ContextPtr /* = EVP_PKEY * */>
		ContextPtr GetKeyContext();
};

/**
\brief
	������������� ������� ��������� � ��������� �������� ������ ���������
	������������ ��������� ����� (�������� RSA).

	�������� ������� bridge. ������������� ����� ���� ���������� �� ������ ������
	��������� (�� ������ ������� �������� �� ������ ����������), ���� �� OpenSSL
	���� ����� ����������. ��������� �������������� ������������ VS_PKeyCrypt_Implementation.
*/
class VS_PKeyCrypt
{
private:
	VS_PKey					key;
public:
/**
\brief
	������ �������� ����

	@param[in]	key_source	����� ���� ���� ������ ����� �����,
	���� �������, ������� �������� ��� ���� (������� �� ������� ���������).
	@param[in]	type ��������� ��� �������� key_source � � ����� �������.
	@return	<b>true</b>, ���� �������� ����������� �������, <b>false</b> � ��������� ������.
*/
	bool			SetPublicKey(const char *key_source, const VS_StorageType type);
/**
\brief
	������ �������� ����, ������������ � ��������� \e key.

	@param[in]	key	���������� ����.
	@return		<b>true</b> ���� ������� ������ ����, � ��������� ������ <b>false</b>
	(��������, ����� \e key �� �������� ��������� �����).

*/
	bool			SetPublicKey(VS_PKey *key);
/**
\brief
	������ �������� ����.

	@param[in]	key_source	����� ���� ���� ������ �����, ��� ��������� ����,
	���� �������, ������� �������� ��� ���� (������� �� ������� ���������).
	@param[in]	type ��������� ��� �������� key_source � � ����� �������.
	@return	<b>true</b>, ���� �������� ����������� �������, <b>false</b> � ��������� ������ (�������� ���� �� ������).
*/
	bool			SetPrivateKey(const char *key_source, const VS_StorageType type);
/**
\brief
	������ �������� ����, ������������ � ��������� \e key.

	@param[in]	key	���������� ����.
	@return		<b>true</b> ���� ������� ������ ����, � ��������� ������ <b>false</b>
	(��������, ����� \e key �� �������� ��������� �����).
*/
	bool			SetPrivateKey(VS_PKey *key);
/**
\brief
	�������� ����� �����.
*/
	uint32_t	GetKeyLength();
/**
\brief
	����������� ������.

	����� ���������� ���� ��� ������������� ���������� � iv (������ �������������), ���������
	��������������� ������ ������� ������, ������� ���� �������� �� ����, ��� ���� � iv ���������
	�������� ������ RSA.

	@param[in]		data		������, ������� ���� ����������� (��� ��������� ������������ ������).
	@param[in]		data_len	������ \e data � ������.
	@param[in]		alg			������������� ������������� ���������, ������ �����
								����������� �������� \e data.
	@param[out]		encr_buf	�����, ������� ����� ��������� ������������� ������ �� \e data.
	@param[in,out]	buf_len		�� ����� �������� ������ \e encr_buf, �� ������ ���������� ���� ����������
								� \e encr_buf.
	@param[out]		iv			����� ��������� ������ �������������, ��������������� �������.
								��� ������ ������ ���� 16 ����(��������� ��� �������� ������).
	@param[out]		sym_key		�����, ������� ����� ��������� ������������� �������� ������
								���������� ����, ������� ���� ���������� \e data.
	@param[in,out]	sym_key_len	��� ����� ����� \e sym_key, ��� ������ �������� ���������� ����������
								����.
	@return						��� �������� ���������� �������� ���������� <b>true</b>,
								<b>false</b> � ��������� ������.

	@remark						���� ������ \e encr_buf ����� �������������, ����� ������ <b>false</b>,
								� \e buf_len ����� ��������� ����������� ������ ������. ���� ������ \e sym_key
								����� �������������, ����� ������ <b>false</b>, � \e sym_key_len ����� ���������
								����������� ������ ������.

*/
	bool			Encrypt(const unsigned char *data, const uint32_t data_len,
							const VS_SymmetricAlg sym_alg,
							unsigned char *encr_buf, uint32_t *buf_len,
							unsigned char iv[16], unsigned char *sym_key,
							uint32_t *sym_key_len);
/**
\brief
	������������ ������.

	������ ����������� ������������ ���������� ��������� ����, �������, � ���� �������, ����������
	�������� ������ RSA. ����� �������, ������� ���������������� ������������ ���� �������� ������ RSA,
	� �����	���� ������.

	@param[in]		encr_buf	�����, ���������� ������������� ������.
	@param[in]		buf_len		������ \e encr_buf.
	@param[in]		alg			������������� ���������, ������� ���� ����������� �����.
	@param[in]		iv			������ �������������, ������� ������������� ��� ��������
								������ (������������ � ������ VS_PKey::Encrypt).
	@param[in]		encr_key	������������� �������� ������ ���������� ����
								(��� ����������� ������).
	@param[in]		key_len		������ \e encr_key � ������.
	@param[out]		decr_data	�����, ������� ����� ��������� �������������� ������.
	@param[in,out]	decr_len	��� ����� ������ \e decr_data, ��� ������ ����������
								���������� ����.
	@return						���� ������� ������������ ������ <b>true<\b>, <b>false<\b>
								� ��������� ������.

	@remark						���� ������� ������ \e decr_data ����� ������������ ��� ������ �������������� ������,
								�� ����� ������ <b>false</b>, � @decr_len ����� ��������� ����������� ������ ������.

*/
	bool			Decrypt(const unsigned char *encr_buf, const uint32_t buf_len,
							const VS_SymmetricAlg sym_alg,const unsigned char iv[16],
							const unsigned char *encr_key, const uint32_t key_len,
							unsigned char *decr_data, uint32_t *decr_len);
};