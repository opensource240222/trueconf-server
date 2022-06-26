#pragma once
#include "SecureLib/OpenSSLTypesWrapDefs.h"
#include "SecureLib/SecureTypes.h"

/**
\file VS_SymmetricCrypt.h

\brief
	���� �������� ���������� �������, ���������� � ������������ �������������.
*/

/**
\brief
	������������ ���� � ������������ ������������.

	�������� �������� bridge (��������� ����������). ���������� ������� ��������� ������������� �����.
	���������� ����� ���� ���������� �� ������ ������ ��������� (�� ������ ������� �������� ��
	������ ����������), ���� �� OpenSSL ���� ����� ����������. ��������� ���������� ����������
	����� VS_SymmetricKey_Implementation.
*/
class VS_SymmetricKey
{
private:
	unsigned char		*m_key;		///< ��� ����.
	uint32_t		m_keylen;	///< ����� �����.
public:
	VS_SymmetricKey();
	~VS_SymmetricKey();

/**
\brief
	�������������� ����������.

	���������� ��������� ����� ��������������.
	@param[in]	prov	���������� ���������� (����������), �������
						����� �������������� ��� ���������� ������-��������
						(� ����� ������ ������ OpenSSL, � �������� prov_OPENSSL).
	@return				���� ������������� ������ �������, �� ������ <b>true</b>,
						<b>false</b> � ��������� ������.
*/
	bool				Init();
/**
\brief
	������������� ���� ������������ �����.

	@param[in]	length	����� ����� � ������.
	@param[out]	keybuf	�����, � ������� ����� ������� ��������������� ����.
						������ ������ ������ ���� �� ������, ��� \e length.
	@return				� ������ �������� ���������� �������� ������ <b>true</b>,
						� ��������� ������ <b>false</b>.

	@remark				�� ������ ���� ����������� ����������. ��� ����, ����� ������ ������� ����
						���� ������������ VS_SymmetricKey::SetKey
*/
	bool				GenerateKey(const uint32_t length, unsigned char *keybuf) const;
/**
\brief
	��������� ���� �� ���������.
	@return	���� ���� ����������� ��������� ������ <b>true</b>, ����� <b>false</b>.
*/
	bool				CheckKeyStrength() const;
/**
\brief
	�������� ����� �����.

*/
	uint32_t		GetKeyLength() const;
/**
\brief
	������ ����.

	@param[in]	length	����� �����.
	@param[in]	key		��� ����..
	@return				���� ������� ������ ���� - <b>true</b>, <b>false</b> � ��������� ������.
*/
	bool				SetKey(const uint32_t lenght, const unsigned char *key);
/**
\brief
	�������� ������� ���� � �����.

	@param[in]	buflen	������ ������������� ������ � ������.
	@param[out]	keybuf	��� �����, ���� ����� ������� ����. ������ ������ ������ ���� ������������ �������,
						����� ���������� ����. ��� ����, ����� ����� ������ VS_SymmetricKey::GetKeyLength.
	@param[out]	keylen	����� ��������� ����� �����.

	@return	���� ��� � �������, �� <b>true</b>, <b>false</b> ���� ������ ������ ������, ��� ����� �����.

*/
	bool				GetKey(const uint32_t buflen, unsigned char *keybuf, uint32_t *keylen) const;
/**
\brief
	�������� ����.

	������� ������� ����.
*/
	void				ClearKey();
};

/**
\brief
	������������� ������� ��������� � ��������� �������� ������
	������������� �����������.

	�������� �������� bridge (��������� ����������). �������������
	����� ���� ���������� �� ������ ������ ��������� (�� ������ �������
	�������� �� ������ ����������), ���� �� OpenSSL ���� ����� ����������.
	��������� �������������� ������������ ������� VS_SymmetricCrypt_Implementation.
*/
class VS_SymmetricCrypt
{
	VS_SymmetricKey	m_key;	///< ����.
	vs::EvpCipherCtxRawPtr m_encrCTX;		///<�������� ��� ��������.
	vs::EvpCipherCtxRawPtr m_decrCTX;		///<�������� ��� �����������
	VS_SymmetricAlg	m_Alg;			///<������������� ���������
	VS_SymmetricCipherMode m_CipherMode;	///<����� � ��� �����.
protected:
	template<typename CTX_PTR /*= EVP_CIPHER_CTX* */>
	CTX_PTR GetEncryptCtx();
	template<typename CTX_PTR /*= EVP_CIPHER_CTX* */>
	CTX_PTR GetDecryptCtx();
/**
\brief
	���������� ���������� ��������� ������ VS_SymmetricCryptOpenSSL_Impl
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
	������������� �����������.

	@param[in]	prov			���������� ���������� (����������), ������� ����� �������������� ��� ����������
								������-�������� (� ����� ������ ������ OpenSSL, � �������� prov_OPENSSL).
	@param[in]	alg				������������� ���������, ������� ����� ��������������. (��. VS_SymmetricAlg)
	@param[in]	mode			����� � ��� ��������� (��������, �������, cbc, ebc � �. �.)
	@param[in]	key_len			����� �����

	@return						���� ������������� ������ ������� - <b>true</b>, <b>false</b> � ��������� ������.

*/
	bool					Init(const VS_SymmetricAlg alg, const VS_SymmetricCipherMode mode);
/**
\brief
	������������� ���� ������������ ������.

	@param[in]	len	����� ������������� �����.
	@param[out]	buf	�����, ������� ����� ��������� ����. ������ ������ ������ ���� �� ������, ��� len

	@return		<b>true</b>, ���� ��� ������, <b>false</b> - � ��������� ������.
*/
	bool GenerateKey(const uint32_t len,unsigned char *buf) const;
/**
\brief
	������ ���� ��� ��������/�����������.

	@param[in]	len	����� ����� � ������.
	@param[in]	buf	�����, ���������� ����.

	@return			<b>true</b>, ���� �������� ������ �������, <b>false</b> - � ��������� ������.
*/
	bool					SetKey(const uint32_t len, const unsigned char *buf);
/**
\brief
	�������� ����.

	@param[in]	buflen	���� ������������� ������ � ������.
	@param[out]	keybuf	�����, ������� ����� ��������� ����
	@param[out]	keylen	����� ����� ����������� ����� �����.

	@return				<b>true</b>	- ��� OK, <b>false</b> - � ��������� ������.

*/
	bool					GetKey(const uint32_t buflen, unsigned char *keybuf, uint32_t *keylen) const;
/**
\brief
	���������	���� �� ���������.

	@return	<b>true</b> ���� ����������� ���������� � ��� ����� ������������, <b>false</b> ���� ���������,
			����� ������ ������.
*/
	bool					ChekKeyStrength() const;
/**
\brief
	�������� ������ �����.

	@return	������ �����.
*/
	uint32_t			GetBlockSize() const;
	//uint32_t			SetBlockSize(const uint32_t size);
/**
\brief
	�������� ����� �����.
	@return	����� �����.
*/
	uint32_t			GetKeyLen() const;
/**
\brief
	�������� ������������� ������������� ���������.

	@return	���� �� ���������������, �� ���������� alg_sym_NONE.
*/
	VS_SymmetricAlg			GetAlg() const;
/**
\brief
	�������� ������������� ���� �����.

	@return	���� �� ���������������, �� ���������� mode_NONE.
*/
	VS_SymmetricCipherMode	GetCipherMode() const;
/**
\brief
	����������� �����.

	���������� �������� ����� ����������, ��������� ��� ������������� � �������� ������.
	@param[in]		data		�����, ������� ���� �����������.
	@param[in]		data_len	����� \e data � ������
	@param[out]		encr_buf	�����, � ������� ���������� ������������� ������.
	@param[in,out]	buf_len		�� ����� ������ \e encr_buf, �� ������ ������ �����������.

	@return			<b>true</b>, ��� OK, <b>false</b> - ������.
	@remark			���� ������� \e encr_buf �� ����������, �� ����� ������ <b>false</b>, � \e buf_len
					����� ��������� ����������� ������.
*/
	bool					Encrypt(const unsigned char* data, const uint32_t data_len,
										  unsigned char* encr_buf,	 uint32_t *buf_len /**[in,out]*/) const;
/**
\brief
	������������ ������.

	��������������� �������� ��������� �� �����������.
	@param[in]		encr_buf	����� � ������������.
	@param[in]		buf_len		������ \e encr_buf
	@param[out]		decr_data	�����, � ������� ����� ����������� �������������� ���������
	@param[in,out]	decr_len	�� ����� ����� \e decr_data, �� ������ ����� ��������������� ���������.

	@return			<b>true</b>, ���� ��� OK, <b>false</b> - ���� ��������� ������.
	@remark			���� ������� \e decr_data �� ����������, �� ����� ������ <b>false<\b>, � \e decr_len
					����� ��������� ����������� ������.
*/
	bool					Decrypt(const unsigned char* encr_buf, const uint32_t buf_len,
										  unsigned char* decr_data,		 uint32_t *decr_len/**[in,out]*/) const;
/**
\brief
	�� �� ��������� ���� ������������ ��� ���������� ����� ���� ��������.

	��������������� �����. ������������ �� ���������� ����������.
	���� ���� ����������� � �������� N ����, � �������� � ���������� ������������
	M<N ����, ��, ����� �����������, ����� ������ <b>false</b> �� ��� ���, ���� �� �����
	�������� �� ����������� ���������� N-M ����.
*/

	VS_SymmetricCrypt & operator =(const VS_SymmetricCrypt &src);
	VS_SymmetricCrypt(const VS_SymmetricCrypt&) = delete;
};