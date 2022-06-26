#include "VS_FirewallMgr.h"
#include <objbase.h>
#include <comutil.h>
VS_FirewallMgr* VS_FirewallMgr::instance = 0;

VS_FirewallMgr::VS_FirewallMgr()
{}

VS_FirewallMgr::~VS_FirewallMgr()
{
	for (auto fw : m_Firewalls)
		delete fw;
	m_Firewalls.clear();
	instance = 0;
}

VS_FirewallMgr *VS_FirewallMgr::Instance()
{
	if(instance)
		return instance;
	else
	{
		instance = new VS_FirewallMgr;
		if(instance)
			return instance;
		else
			return 0;
	}
}
bool VS_FirewallMgr::IsValid()
{
	if(m_Firewalls.empty())
		return false;
	else
		return true;
}
bool VS_FirewallMgr::AddFirewall(VS_FirewallInterface *fw)
{
	if((!fw)||(!fw->IsValid()))
		return false;
	if (std::find(m_Firewalls.begin(), m_Firewalls.end(), fw) != m_Firewalls.end())
		return true;
	m_Firewalls.push_back(fw);
	return true;
}
bool VS_FirewallMgr::OpenPort(const unsigned long port, VS_IP_PROTOCOL protocol, const wchar_t *registerName)
{
	if(!IsValid())
		return false;
	for (auto fw : m_Firewalls)
		fw->OpenPort(port, protocol, registerName);
	return true;
}
bool VS_FirewallMgr::ClosePort(const unsigned long port, VS_IP_PROTOCOL protocol)
{
	if(!IsValid())
		return false;
	for (auto fw : m_Firewalls)
		fw->ClosePort(port, protocol);
	return true;
}
bool VS_FirewallMgr::AddApplication(const wchar_t *fileName, const wchar_t *registerName, const wchar_t *scope)
{
	if(!IsValid())
		return false;
	for (auto fw : m_Firewalls)
		fw->AddApplication(fileName, registerName, scope);
	return true;
}

bool VS_FirewallMgr::RemoveApplication(const wchar_t *fileName)
{
	if(!IsValid())
		return false;
	for (auto fw : m_Firewalls)
		fw->RemoveApplication(fileName);
	return true;
}

VS_WinXPFirewall *VS_WinXPFirewall::instance = 0;

VS_WinXPFirewall * VS_WinXPFirewall::Instance()
{
	if(!instance)
	{
		instance = new VS_WinXPFirewall;
		if((!instance)||(!instance->IsValid()))
		{
			delete instance;
			return 0;
		}
	}
	return instance;
}
VS_WinXPFirewall::VS_WinXPFirewall():m_FireWallProfile(0)
{
	HRESULT	hr;
	INetFwMgr		*fwMgr = 0;
	INetFwPolicy	*fwPolicy = 0;

	// for server, the firewall manager only works without the next lines
	// if firewall managment in client doesn't work without these lines, then some differentiation for client code and server code must be done
	// otherwise the commented lines should be removed completely

	//hr = CoInitializeEx(NULL, COINIT_DISABLE_OLE1DDE | COINIT_APARTMENTTHREADED);
	//if(FAILED(hr))
	//	return;

	hr = CoCreateInstance(__uuidof(NetFwMgr),
						0,
						CLSCTX_INPROC_SERVER,
						__uuidof(INetFwMgr),
						(void**)&fwMgr);
	if(FAILED(hr))
		return;
	if((FAILED(fwMgr->get_LocalPolicy(&fwPolicy)))||(FAILED(fwPolicy->get_CurrentProfile(&m_FireWallProfile))))
	{
		fwMgr->Release();
		if(fwPolicy)
			fwPolicy->Release();
		if(m_FireWallProfile)
		{
			m_FireWallProfile->Release();
			m_FireWallProfile = 0;
		}
		return;
	}
}

VS_WinXPFirewall::~VS_WinXPFirewall()
{
	if(m_FireWallProfile)
	{
		m_FireWallProfile->Release();
		m_FireWallProfile = 0;
	}
	instance = 0;
}
bool VS_WinXPFirewall::IsValid()
{
	bool res(false);
	if(m_FireWallProfile)
		res = true;
	else
		res = false;
	return res;
}
int VS_WinXPFirewall::IsFirewallOn()
{
	int res(0);
	VARIANT_BOOL	isEnabled(VARIANT_FALSE);
	if(!IsValid())
		res = -1;
	else if(FAILED(m_FireWallProfile->get_FirewallEnabled(&isEnabled)))
		res = -2;
	else
		res = isEnabled == VARIANT_TRUE? 1 : 0;
	return res;
}
int VS_WinXPFirewall::EnableFirewall(const bool enable)
{
	int res(0);
	if(!IsValid())
		res = -1;
	else if(FAILED(m_FireWallProfile->put_FirewallEnabled(enable ? VARIANT_TRUE : VARIANT_FALSE)))
		res = -2;
	else
		res = 1;
	return res;
}

int VS_WinXPFirewall::IsPortEnabled(const unsigned long port, const VS_IP_PROTOCOL protocol)
{
	int res(0);
	INetFwOpenPorts	*openPorts(0);
	INetFwOpenPort	*openPort(0);
	VARIANT_BOOL	portEnabled(VARIANT_FALSE);
	if(!IsValid())
		res = -1;
	else if(!IsFirewallOn())
		res = 1;
	else {
		if(SUCCEEDED(m_FireWallProfile->get_GloballyOpenPorts(&openPorts)))
		{
			if(SUCCEEDED(openPorts->Item(port,
										protocol == e_IP_PROTOCOL_TCP ? NET_FW_IP_PROTOCOL_TCP : NET_FW_IP_PROTOCOL_UDP,
										&openPort)))
			{
				if(FAILED(openPort->get_Enabled(&portEnabled)))
					res = -2;
				else if(VARIANT_TRUE == portEnabled)
					res = 1;
				else
					res = 0;
			}
		}
		if(openPort)
			openPort->Release();
		if(openPorts)
			openPorts->Release();
	}
	return res;
}
int VS_WinXPFirewall::OpenPort(const unsigned long port, VS_IP_PROTOCOL protocol, const wchar_t *registerName)
{
	INetFwOpenPorts	*openPorts(0);
	INetFwOpenPort	*openPort(0);
	BSTR bstrRegisterName(0);
	int	res(0);
	if(!IsValid())
		res = -1;
	else if(!IsFirewallOn())
		res = 1;
	else if(1 == IsPortEnabled(port,protocol))
		res = 1;
	else
	{
		if((FAILED(m_FireWallProfile->get_GloballyOpenPorts(&openPorts)))||
			(FAILED(CoCreateInstance(__uuidof(NetFwOpenPort),0,CLSCTX_INPROC_SERVER,
									__uuidof(INetFwOpenPort),(void**)&openPort))))
		{
			if(openPorts)
				openPorts->Release();
			res = -2;
		}
		else
		{
			bstrRegisterName = SysAllocString(registerName);
			if(0 == SysStringLen(bstrRegisterName))
				res = -2;
			else if((FAILED(openPort->put_Name(bstrRegisterName)))||
					(FAILED(openPort->put_Port(port)))||(FAILED(openPort->put_Protocol(protocol == e_IP_PROTOCOL_TCP? NET_FW_IP_PROTOCOL_TCP : NET_FW_IP_PROTOCOL_UDP)))||
					(FAILED(openPort->put_Enabled(VARIANT_TRUE)))||
					(FAILED(openPorts->Add(openPort))))
				res = -2;
			else
				res = 1;
		}
		SysFreeString(bstrRegisterName);
		openPort->Release();
		openPorts->Release();
	}
	return res;
}
int VS_WinXPFirewall::ClosePort(const unsigned long port, VS_IP_PROTOCOL protocol)
{
	int	res(0);
	INetFwOpenPorts	*openPorts = 0;
	if(!IsValid())
		res = -1;
	else if(!IsFirewallOn())
		return 0;
	else if(!IsPortEnabled(port,protocol))
		res = 1;
	else if((FAILED(m_FireWallProfile->get_GloballyOpenPorts(&openPorts)))||
		(FAILED(openPorts->Remove(port,e_IP_PROTOCOL_TCP == protocol ? NET_FW_IP_PROTOCOL_TCP : NET_FW_IP_PROTOCOL_UDP))))
		res = -2;
	else
		res = 1;
	if(openPorts)
		openPorts->Release();
	return res;
}
int VS_WinXPFirewall::AddApplication(const wchar_t *fileName, const wchar_t *registerName, const wchar_t *scope)
{
	static wchar_t defRemoteAddress[] = L"*";

	int res(0);
	HRESULT	hr;
	INetFwAuthorizedApplications	*authApps(0);
	INetFwAuthorizedApplication		*authApp(0);

	BSTR							bstrFileName(0),
									bstrRegisterName(0),
									bstrRemoteAddress(0);
	if(!IsValid())
		res = -1;
	else if((FAILED(m_FireWallProfile->get_AuthorizedApplications(&authApps)))||
		(0 == SysStringLen((bstrFileName = SysAllocString(fileName))))||
		(0 == SysStringLen(bstrRegisterName = SysAllocString(registerName))))
		res = -2;
	else if((FAILED(hr = authApps->Item(bstrFileName,&authApp)))&&
			(HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr))
	{
		if((FAILED(CoCreateInstance(__uuidof(NetFwAuthorizedApplication),
									0,
									CLSCTX_INPROC_SERVER,
									__uuidof(INetFwAuthorizedApplication),
									(void**)&authApp)))||
		   (FAILED(authApp->put_Enabled(VARIANT_TRUE)))||
		   (FAILED(authApp->put_Name(bstrRegisterName)))||
		   (FAILED(authApp->put_ProcessImageFileName(bstrFileName)))||
		   ((scope)&&(0 == SysStringLen(bstrRemoteAddress = SysAllocString(scope))))||
		   (FAILED(authApp->put_RemoteAddresses( !scope ? defRemoteAddress : bstrRemoteAddress )))||
		   (FAILED(authApps->Add(authApp))))
		   res = -2;
		else
			res = 1;

	}
	else if(SUCCEEDED(hr))
	{
		if((FAILED(authApp->put_Name(bstrRegisterName)))||
			(FAILED(authApp->put_Enabled(VARIANT_TRUE))))
			res = -2;
		else if(((scope)&&(0 == SysStringLen(bstrRemoteAddress = SysAllocString(scope))))||
				(FAILED(authApp->put_RemoteAddresses( !scope ? defRemoteAddress : bstrRemoteAddress ))))
				res = -2;
		else
			res = 1;
	}
	else
		res = -2;
	if(bstrFileName)
		SysFreeString(bstrFileName);
	if(bstrRegisterName)
		SysFreeString(bstrRegisterName);
	if(bstrRemoteAddress)
		SysFreeString(bstrRemoteAddress);
	if(authApp)
		authApp->Release();
	if(authApps)
		authApps->Release();
	return res;
}
int VS_WinXPFirewall::RemoveApplication(const wchar_t *fileName)
{
	BSTR	bstrFileName(0);
	INetFwAuthorizedApplications	*authApps(0);
	int res(0);
	if(!IsValid())
		res = -1;
	else if(!fileName)
		res = 0;
	else if((FAILED(m_FireWallProfile->get_AuthorizedApplications(&authApps)))||
		(0 == SysStringLen(bstrFileName = SysAllocString(fileName)))||
		(FAILED(authApps->Remove(bstrFileName))))
		res = -2;
	else
		res = 1;
	if(bstrFileName)
		SysFreeString(bstrFileName);
	if(authApps)
		authApps->Release();
	return res;
}