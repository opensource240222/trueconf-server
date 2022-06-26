#pragma once
#include "SecureLib/OpenSSLTypesWrapDefs.h"
#include "SecureLib/SecureTypes.h"
#include "SecureLib/VS_PublicKeyCrypt.h"

/**
\file VS_Sign.h
\brief ���� �������� �������� ������� ��� ������ � �������� �������� ������ (������� RSA, DSA).
*/


const VS_SignArg zeroSignArg = {alg_pk_NONE,alg_hsh_NONE};

/**
\brief
	������������� ������� �������� � ��������� ���������� �������� ��������.

	�������� �������� bridge (��������� ����������). ������������� ����� ���� ���������� �� ������ ������
	��������� (�� ������ ������� �������� �� ������ ����������), ���� �� OpenSSL
	���� ����� ����������. ��������� �������������� ������������ VS_Sign_Implementation.
*/
class VS_Sign
{
private:
/**
\brief
	���������� ��������� �������.
*/
	enum current_state
	{
		st_none					= 0x0,
		st_init_arg				= 0x1,///<������ ���������������
		st_specify_public_key	= 0x2,///<����� �������� ����
		st_specify_private_key	= 0x4///<����� �������� ����.
	};
	uint32_t m_flag;	///<��������� �������. ����� ���� ���������� ������ �������� �� current_state
	vs::EvpPKeyPtr m_pkey;///<���������� ��� ����.
	VS_SignArg m_sign_arg;	///<��������� ������������/����������� ������.

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
	������ �������� ����.

	������ ������� �������� ����, ��� ���������� �������.
	@param[in]	KeyStorage	���� ��� ����� � ������� �������� ����, ���� ����� ����������
							���� � ������������ �������. ������� �� �������� \e StorageType.
	@param[in]	size		������ \e KeyStorage.
	@param[in]	StorageType	��� VS_StorageType. ���������� ��� � ������ ��������� ����� (����, �����, ������ PEM, DER)

	@return	���� ������� ������ ����� ���� ���������� <b>true</b>, <b>false</b> � ���������� ������.
*/
	bool SetPublicKey(const char* KeyStorage, const uint32_t size, const int StorageType);
/**
\brief
	������ �������� ����

	������ ������� �������� ����. �������� ���� ������������ ��� ������� �����.
	@param[in]	KeyStorage	���� ��� ����� � ������� �������� ����, ���� ����� ����������
							���� � ������������ �������. ������� �� �������� \e StorageType.
	@param[in]	StorageType	��� VS_StorageType. ���������� ��� � ������ ��������� ����� (����, �����, ������ PEM, DER)

	@return	���� ������� ������ ����� ���� ���������� <b>true</b>, <b>false</b> � ���������� ������.
*/
	bool SetPrivateKey(const char* KeyStorage, const int StorageType,const char *pass="");
/**
\brief
CallBack �������. ����� ��� ����, ����� ��� ������ ��������� ����� ������ ������, ������� �� ����������.

������������ � VS_SignOpenSSL_Impl::SetPrivateKey.
@param[out]	buf		����� ��� ������.
@param[in]	len		����� ������.
@param[in]	cb_arg	��������� �� VS_PrivateKeyStorage.

@return	���������� 1.
*/
int SetPassword(char *buf, int len, int rwflag, void * cb_arg);
/**
\brief
	��������� �����.

	����� ��� ��� ������������ �����, ���� ������������������� ����� (������� ����� VS_Sign::Init
	� ����������� �����������)
	� ������ �������� ���� (VS_Sign::SetPrivateKey)

	@param[in]		data		������, ������� ���� ���������
	@param[in]		data_size	������ \e data.
	@param[out]		sign		�������� ������� ������ \e data
	@param[in,out]	sign_size	�� ����� ������ ������ ��� �������, ��� ������ �������� ���������� ����,
								���������� � \e sign.
	@return						���� ������� ������ ������ �������, �� ���������� <b>true</b>, <b>false</b>
								� ��������� ������.

	@remark						���� ������� ������ ��� ������� ������������, �� ����� ���������� <b>false</b>,
								� \e sign_size ����� ��������� ����������� ������.

*/
	bool SignData(const unsigned char *data, uint32_t data_size,unsigned char *sign, uint32_t *sign_size);
/**
\brief
	�������������� �������.

	����� ���, ��� ������������ �����, ���� ������������������� ����� (������� ����� VS_Sign::Init
	� ����������� �����������) � ������ �������� ���� (VS_Sign::SetPublicKey).

	@param[in]	data		����������� ������.
	@param[in]	data_size	������ \e data.
	@param[in]	sign		�������, ������� ���� ��������������.
	@param[in]	sign_size	������ \e sign.

	@return					���� ����������� ������ ������� (������� ���������), �� ���������� <b>true</b>, <b>false</b> � ��������� ������.
*/
	int  VerifySign(const unsigned char *data, uint32_t data_size, const unsigned char *sign, const uint32_t sign_size);
/**
\brief
	������������� ������.

	���������� ��������� ����� ������� ������.
	@param[in]	prov		���������� ���������� (����������), ������� ����� ��������������
							��� ���������� ������-�������� (� ����� ������ ������ OpenSSL, � �������� prov_OPENSSL)
	@param[in]	sign_arg	��������� ��� �������� ������� (�������� ��� �����, � ��� ��������).

	@return					��� �������� ������������� ���������� <b>true</b>,
							<b>false</b> � ��������� ������.

*/
	bool Init(const VS_SignArg sign_arg);
/**
\brief
	���������� ����� �������.
*/
	uint32_t GetSignSize() const;
};
