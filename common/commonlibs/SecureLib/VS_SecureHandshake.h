#pragma once
/**
\file	VS_SecureHandshake.h
\brief
	���� �������� �������� �������, ������������ SecureHandshake'��.
	(���������� ��������� ����������� ����������)
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
	������� ����������� ���������� ����������� ����������.

	��. ���������� � �������� VS_SecureHandshake.
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
	������ ��� ���������.

	�������� ����� ����������� �� ������� ������� � �� ������� �������.
*/
enum VS_SecureHandshakeType
{
	handshake_type_None		= 0,	///<�� ���������������.
	handshake_type_Client	= 1,	///<�������� ���������� �� ������� ������.
	handshake_type_Server	= 2		///<�������� ���������� �� ������� �������.
};

#pragma pack (1)

/**
\brief
	��������� �������� ������� ��������������������� ������� ������������ ������������.

	��������� ������������ ������ ������ VS_SecureHandshakeVer1_Impl. ��������� ����� ���������
	���������, ��������� ����� ��������� ������������� ������� �� ������-��������
	(������ VS_SecureHandshake::GetReadSymmetricCrypt ��� VS_SecureHandshake::GetWriteSymmetricCrypt).
*/
struct VS_ReadWriteCrypt
{
	VS_SymmetricCrypt	*pReadCrypt;
	VS_SymmetricCrypt	*pWriteCrypt;
};
#pragma pack ()

/**
\brief
	�������� ����������� ��������� ���������� Secure Handshake'��.

	���������� ��������� ��� ������� ���������� (�������� �������� bridge).
	��������� ������������ ��� ���������� ������������� �������. VS_SecureHandshake �������� ������ �� ���� ����� �,
	����� ������������� ���������� ���������� ����� ����������, ���������� ��� ������. ��������� �������� ����������.
	������������, ��� ������ � ������������� �������, ������ ������������ ����� VS_SecureHandshake.
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
	"������" ����������.

	����������-��������, ������ �� ������.
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
	���������� ���������� ��������� ���������� Secure Handshake'��.
	��������� ������ ��������� = 1.

	����� �� ������������ ��� �������� �������������, ������ ���� ������������ VS_SecureHandshake.

	@remark

	<b>�������� Secure Handshake'� ������ 1.</b>
	- ���� �������:
		-#	��������� ���������� X509, ����������� CA;
		-#	��������� ������ ����������, �������������� ��������;
		-#	�������� ����� � ������� � ��������������� ������������� ���������;
		-#	������������ ��������������� ������� VS_SymmetricCrypt.
	- ���� �������:
		-#	�������� � ��������� ����������. ���� ���������� �� �������� CA -
			��������� ����������;
		-#	�������� ������ �������������� ���������� � ������� ���������� (������������ AES);
		-#	������������� ����� ��� �����������/���������� ������, ����������� ��������������� �������
			VS_SymmetricCrypt;
		-#	����������� ����� � �������, ��������������� ���������, ����������� �����, ��������� �������� ����
			������� (������������ � �����������) � ��������� �������.
*/
class VS_SecureHandshakeVer1_Impl : public VS_SecureHandshake_Implementation
{
protected:
	virtual int GetVersion() const {return 1;}

/**
\brief
	���������� ��������� ������.

	��������������, � ����� ��������� ��������� SecureHandshake.
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
	VS_SecureHandshakeType	m_type;				///<��� ���������.
	bool					m_GostSupported;
	uint32_t			m_state;			///<���������� ��������� ���������.
	char					*m_somedata;		///<������� ������.
	uint32_t			m_size_somedata;	///<������� ������ \e m_somedata.
	VS_PKey					m_pub_key;				///<�������� �������� (�� ������� �������) ��� �������� (�� ������� �������) ����
	char			*m_private_key_buf; ///<��������� ���
	char			*m_cert_pem_buf;
	uint32_t	m_cert_buf_len;
#ifdef _DEBUG
	std::set<uint32_t> sizes;
#endif

	void PrepareClientErrorMessage(const VS_SecureHandshakeError err_code);
	bool ServerProcErrorMessage(const void *buf, const uint32_t sz);

/**
\brief
	���������� ������� ��� ���������.
	@return	<b>true</b>, ���� �������� �� ������� �������, <b>false</b>, ���� �� ������� �������.
*/
	bool	IsServerHandshake() const;
/**
\brief
	��������� ����������� ������ (������-��������).

	\e buf �������� ������ ������, ������� ���� �������� ��������� �����.
*/
	bool	ServerProcPacketSize(const void *buf,const uint32_t size);
/**
\brief
	��������� ����������� ������ (������-��������).

*/
	virtual bool	ServerProcPacket(const void *buf,const uint32_t size);
/**
\brief
	��������� ����������� ������ (������-��������).

	\e buf �������� ������ �����������, ������� ���� �������� ��������� �����.
*/
	bool	ClientProcCertSize(const void *buf,const uint32_t size);
/**
\brief
	���������� ���������� (������-��������).

	@param[in]	buf		����������
	@param[in]	size	���������� ����, ������������ � ������.
*/
	bool	ClientProcCert(const void *buf,const uint32_t size);
/**
\brief
	��������� ����������� ������ (������-��������).
*/
	bool	ClientProcPacketSize(const void *buf,const uint32_t size);
/**
\brief
	��������� ����������� ������ (������-��������).
*/
	virtual bool	ClientProcPacket(const void *buf,const uint32_t size);
public:
	VS_SecureHandshakeVer1_Impl();

	virtual ~VS_SecureHandshakeVer1_Impl();
/**
\brief
	������������� ���������� ����� ������.

	��. VS_SecureHandshake::Init
*/
	bool InitAct(const enum VS_SecureHandshakeType type) override; //���������� � ��������� ���������
/**
\brief
	���������� ��������.

	VS_SecureHandshake::StopHandshakeAct
*/
	void StopHandshakeAct() override;
/**
\brief
	���������� ������������� ��������, ������� ���� ��������� ��������� �����.

	��. VS_SecureHandshake::Next
*/
	VS_SecureHandshakeState Next() override;
/**
\brief
	����������� ����� ��� ��������/��������� �����.

	��. VS_SecureHandshake::PreparePacket
*/
	bool PreparePacket(void** buf, uint32_t* size) override;
/**
\brief
	���������� �����, �������������� PreparePacket.

	��. VS_SecureHandshake::FreePacketBuf
*/
	void FreePacketBuf(void** buf) const override;
/**
\brief
	���������� ���������� ����� �����.

	��. VS_SecureHandshake::ProcessPacket
*/
	bool ProcessPacket(const void* buf, const uint32_t size) override;
/**
\brief
	�������� ������ ��� ����������� ���������� ������ ��� ���������� ����������.

	��. VS_SecureHandshake::GetReadSymmetricCrypt.
*/
	VS_SymmetricCrypt* GetReadSymmetricCrypt() override;
/**
\brief
	�������� ������ ��� �������� ������������ ����� ��� ���������� ����������.

	��. VS_SecureHandshake::GetWriteSymmetricCrypt
*/
	VS_SymmetricCrypt* GetWriteSymmetricCrypt() override;
/**
\brief
	������ �������� ���� (����� ������������ ����� �������, ���������� ���������� �����).

	��. VS_SecureHandshake::SetPrivateKey
*/
	bool SetPrivateKey(VS_PKey* pkey) override;
/**
\brief
	���������� �������, ���������� � ������� ������� GetWriteSymmetricCrypt ��� GetReadSymmetricCrypt

	��. VS_SecureHandshake::ReleaseSymmetricCrypt
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
	������������� ������� ��������� � ��������� ������������ ����������� ���������� (Secure Handshake).

	�������� �������� bridge (��������� ����������). ������������� ��������� ������ ������ Handshake'�.
	��������� �������������� ������������ ������� VS_SecureHandshake_Implementation.

	@remark

	<b>������������ ��� ��������� ����������� ���������� ������ ������������ ������� ��������� �������:</b>
	- ������������������� ����� (VS_SecureHandshake::Init), ������ ������ ����� ������
	(������ �������������� ������ � 1) � ��� SecureHandshake'� (������ ��� ������);
	- ������� ����� VS_SecureHandshake::Next �, � ����������� �� ����������, ��������� ������������ ��������
	(��. �����);
	- �������� VS_SecureHandshake::Next � ��������� ��������� �� ��� ���, ���� ����� �� ������ <b>secure_st_Finish</b> ���
	<b>secure_st_Error</b>;
	- ���� VS_SecureHandshake::Next ������ <b>secure_st_Finish</b>, �� ���������� ���������� �����������.
	������ ������ VS_SecureHandshake::GetReadSymmetricCrypt � VS_SecureHandshake::GetWriteSymmetricCrypt,
	���� �������� �������������� �������, ������� ������ �������������� ��� �������� ������������ ������ � ���
	����������� ����������;
	- ���� VS_SecureHandshake::Next ������ <b>secure_st_Error</b>, �� ���������� �� �����������. ������� ���������� ��������,
	������ ����� VS_SecureHandshake::StopHandshake;
	- �������, ������� ���� �������� �������� VS_SecureHandshake::GetReadSymmetricCrypt �
	VS_SecureHandshake::GetWriteSymmetricCrypt, ��������� ������� ��������� VS_SecureHandshake::ReleaseSymmetricCrypt,
	����� ��� ����� �� �����;

	<b>��������� �������� ������������ ������� VS_SecureHandshake::Next</b>
	- <b>secure_st_SendCert</b>	- ����� ���� ������ �� ������� ������� (\e ������ - ���, ��� ������������� ����������).
	������� ��� ������� ���� ��������� peer'� ���������� X509, �������� CA;
	- <b>secure_st_SendPacket</b> - ����� ���� ��� �� ������� �������, ��� � ��������� �������. ������� ��� �������,
	���� ����������� ����� ��� ��������, ������ ����� VS_SecureHandshake::PreparePacket. ���� ����� ������
	<b>true</b>, �� ��������� �������������� ������, ����� �������� ���������� �����, ������ �����
	VS_SecureHandshake::FreePacket;
	- <b>secure_st_GetPacket</b> - ����� ���� ��� �� ������� �������, ��� � �� ������� �������. ������� ��� �������,
	���� ����������� ����� ��� ��������� ������, ������ ����� VS_SecureHandshake::PreparePacket, ���� ����� ������
	<b>true</b>, �� �������� ��������� ���������� ���� � ���� �����, ����� ��� ���������� �������
	VS_SecureHandshake::ProcessPacket (\e buf - ����� � ����������� ������, \e size - ���������� ������� ���������� ����),
	���������� �����, ������ ����� VS_SecureHandshake::FreePacket;
	- <b>secure_st_Error</b> - ����� ���� ��� �� ������� �������, ��� � �� ������� �������. ��� ��������� ����������
	��������� ������. ���������� �� �����������, ������� ��������� �������� ������� VS_SecureHandshake::StopHandshake.
	- <b>secure_st_Finish</b> - ����� ���� ��� �� ������� �������, ��� � �� ������� �������. ��������, ��� ����������
	���������� �����������. ������� ��� ��������/����������� ������ ������, �� ���� �������� ��� ������ �������
	VS_SecureHandshake::GetReadSymmetricCrypt (��� ����������� �������� ������)� VS_SecureHandshake::GetWriteSymmetricCrypt
	(��� ���������� ������������ ������)
*/
class VS_SecureHandshake
{
private:
	VS_SecureHandshake_Implementation*	imp;///< ��������� �� ���������� ��������� ��������� ����������� ���������� ������������ ������.
	uint32_t						m_Version;///< ����� ������. �� ������������� = 0xFFFFFFFF (-1)
	static	char						*m_RegistryRoot;
public:
	static void SetRegistryRoot(const char *root);
	static bool GetRegistryRoot(char *out, unsigned int &buflen);
	static void SetImprovedSecurityState(bool state);
/**
\brief
	����������� ��-���������.
*/
	VS_SecureHandshake(void);
/**
\brief
	����������� � �������������.

	�������� ����� VS_SecureHandshake::Init.
*/
	VS_SecureHandshake(const uint32_t Version, const VS_SecureHandshakeType type);
	~VS_SecureHandshake(void);
/**
\brief
	������������� ������.

	������ ���������. ���������� ������� ����� ������.
	@param[in]	Version	����� ������ Secure Handshake (�� ������� ������ ����������� 1-� ������).
	@param[in]	type	��� ���������. ���������� � ���� ������� ����� ����������� �������� (������, ������).
	@return		���� ������������� ������ ������� ������ <b>true</b>, <b>false</b> � ��������� ������ (�������� ����� ������ ������ �����������).
*/
	bool Init(const uint32_t Version, const VS_SecureHandshakeType type);
/**
\brief
	���������� ��������.

	����������� ��� �������, ���������� ��� ��������� � ������������� ��������� ������
	� �������������������� ��������� (��� ��������� ��������� ����� VS_SecureHandshake::Init ���� ������� �����).
*/
	void StopHandshake();
/**
\brief
	���������� ������������� ��������, ������� ���� ��������� ��������� �����.

	��. ����������.
*/
	VS_SecureHandshakeState Next();
/**
\brief
	����������� ����� � �������� ��� ��������� ������.

	����� �������� ������� ���������� ������� ��� �������� � ��������� ������. ����� ������ ������,
	������������ �������� ����� �������� ������� �, � ������ �������� ������, �������� ����������. ��� ���������
	� ���������� ������� ������� �� ���������� ����������� ������ VS_SecureHandshake::Next.

	@param[out]	buf		����� ��������� �� �������������� �����.
	@param[out]	size	����� ��������� ������ ����������� ������ � ������.
	@return				���� �� ���� ������ - <b>true</b>,<b>false</b> � ��������� ������.

	@remark	��� �������������� ������ ������ ���� ����������� ������� VS_SecureHandshake::FreePacket.
*/
	bool PreparePacket(void** buf, uint32_t* size);
/**
\brief
	����������� ������, �������������� ������� VS_SecureHandshake::PreparePacket.

	@param[in,out]	�����, �������������� VS_SecureHandshake::PreparePacket.
*/
	void FreePacket(void** buf);
/**
\brief
	��������� ����������� ������.

	���� ����� ���� ������� ����� ��������� ������ �� peer'�.
	@param[in]	buf		���������� ����� ������.
	@param[in]	size	������ \e buf.
	@return				���� ���������� ����� "����������" - ������ <b>true</b>, <b>false</b> � ��������� ������.
*/
	bool ProcessPacket(const void* buf, const uint32_t size);
/**
\brief
	�������� ������ ��� ����������� ���������� ������.

	���������� ����� ��������� ����������� ����������, ����� �������� ����������, ��� ������������������ ������
	��� ���������� ����������� ������.

	@remark	��� �������� �������, ����������� ����� ��������, ���� ��������������� ������� VS_SecureHandshake::ReleaseSymmetricCrypt.
*/
	VS_SymmetricCrypt* GetReadSymmetricCrypt();
/**
\brief
	�������� ������ ��� ���������� ������������ ������.

	���������� ����� ��������� ����������� ����������, ����� �������� ����������, ��� ������������������ ������
	��� ���������� ���������� ������.

	@remark	��� �������� �������, ����������� ����� ��������, ���� ��������������� ������� VS_SecureHandshake::ReleaseSymmetricCrypt.
*/
	VS_SymmetricCrypt* GetWriteSymmetricCrypt();
/**
\brief
	������ �������� ���� �����������.

	����� ������ ��� ��������� �� ������� �������, ����� ������������ �����, ��������� �� �������.
	@param[in]	pkey		�������� ����.

	@return <b>true</b> - ��� OK, <b>false</b> - ���� ������.
*/
	bool SetPrivateKey(VS_PKey* pkey);
/**
\brief
	����������� �������, ���������� �������� VS_SecureHandshake::GetReadSymmetricCrypt �
	VS_SecureHandshake::GetWriteSymmetricCrypt

	@param[in,out]	��������� �� ���������, ����������� �� ������ ���� VS_SymmetricCrypt, ��������� ����� ��
					������� VS_SecureHandshake::GetReadSymmetricCrypt ��� VS_SecureHandshake::GetWriteSymmetricCrypt.

*/
	static void				ReleaseSymmetricCrypt(VS_SymmetricCrypt **crypt);
    virtual std::shared_ptr<VS_Certificate> CloneCertificate() const;
	bool					GetCertificate(char *buf, uint32_t &sz);
/**
\brief
	�������� ����� ������ ������.
*/
	uint32_t GetVersion() const;
	VS_SecureHandshakeError	GetHandshakeErrorCode();
};