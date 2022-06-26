#pragma once

#define INITGUID
#include "netfw.h"

#include <list>

enum VS_IP_PROTOCOL
{
	e_IP_PROTOCOL_TCP,
	e_IP_PROTOCOL_UDP
};
/**
\brief ��������� ����������� ���������.
*/
class VS_FirewallInterface
{
public:
	virtual ~VS_FirewallInterface(){}
	virtual bool	IsValid()		= 0;
	virtual int		IsFirewallOn()	= 0;
	virtual int		IsPortEnabled(const unsigned long port, const VS_IP_PROTOCOL protocol)	= 0;
	virtual int		EnableFirewall(const bool enable) = 0;
	virtual int		OpenPort(const unsigned long port, VS_IP_PROTOCOL protocol,
								const wchar_t *registerName) = 0;
	virtual int		ClosePort(const unsigned long port, VS_IP_PROTOCOL protocol) = 0;
	virtual int		AddApplication(const wchar_t *fileName,const wchar_t * registerName, const wchar_t *scope) = 0;
	virtual int		RemoveApplication(const wchar_t *fileName) = 0;
};

/**
\brief ��������� ��������� ����������� ���������� (����, ��������, � ������� ����� ��������� ���������)
*/
class VS_FirewallMgr
{
protected:
	VS_FirewallMgr();
	std::list<VS_FirewallInterface*> m_Firewalls;
private:
	static VS_FirewallMgr *instance;
public:
	virtual				~VS_FirewallMgr();
	static VS_FirewallMgr	*Instance();

	bool	AddFirewall(VS_FirewallInterface *);
	bool	IsValid();
	bool	OpenPort(const unsigned long port, VS_IP_PROTOCOL protocol, const wchar_t *registerName);
	bool	ClosePort(const unsigned long port, VS_IP_PROTOCOL protocol);
	bool	AddApplication(const wchar_t *fileName,const wchar_t * registerName, const wchar_t *scope);
	bool	RemoveApplication(const wchar_t *fileName);
};

/**
����� ���������� WinXPFirewall.
*/
class VS_WinXPFirewall : public VS_FirewallInterface
{
private:
	INetFwProfile* m_FireWallProfile;
	VS_WinXPFirewall();
	static VS_WinXPFirewall *instance;
public:
	virtual ~VS_WinXPFirewall();
/**
\brief �����, ����������� ��������� ���������� ������ (����� ���� ������ ����).
*/
	static VS_WinXPFirewall *Instance();
/**
\brief ���������,  ���������, ���������� �� fw
*/
	virtual bool	IsValid();
/**
\brief ������� �� firewall.

@return 0 - fw �� �������, 1 - fw �������, -1 - fw �� ����������, -2 - ������ COM
*/
	virtual int		IsFirewallOn();
/**
\brief ������ �� ������������ ����.

@return 0 - ���� ������������, 1 - ���� ������ ��� �������� ����������, -1 - fw �� ����������, -2 - ������ COM
*/
	virtual int		IsPortEnabled(const unsigned long port, const VS_IP_PROTOCOL protocol);
/**
\brief ��������/��������� �������.
*/
	virtual int		EnableFirewall(const bool enable);
/**
\brief ������� ����.

 @param[in]	port		- ����������� ����;
 @param[in] prot�ol		- ��������, ��� �������� ����������� ����. ����� ��������� �������� e_IP_PROTOCOL_TCP ��� e_IP_PROTOCOL_UDP
 @param[in]	registerName  - ��� �����.
*/
	virtual int		OpenPort(const unsigned long port, VS_IP_PROTOCOL protocol, const wchar_t *registerName);
/**
\brief ������� ����.

 @porom[in]	port		- ���������� ����
 @param[in]	protocol	- ��������, ��� �������� ����������� ����.
*/
	virtual int		ClosePort(const unsigned long port, VS_IP_PROTOCOL protocol);
/**
\brief ��������� ���������� ��������� ����������(���� ��������)

 @param[in]	fileName		- ���� � ����� ���������;
 @param[in]	registerName	- ��� ������;
 @param[in] scoe			- ���������� ��������� �������, � ������� ����� ��������� ������������.
							������:
							- 0 - �������� �� ���� �������;
							- L"LocalSubnet" -
							- ����� ������ network mask ��� ������� �������. ������� �������� ��������: "10.0.0.2/255.0.0.0", "10.0.0.2/8", "10.0.0.2";
*/
	virtual int		AddApplication(const wchar_t *fileName, const wchar_t *registerName, const wchar_t *scope);
/**
\brief ������� ��������� �� ������ �����������
*/
	virtual int		RemoveApplication(const wchar_t *fileName);
};