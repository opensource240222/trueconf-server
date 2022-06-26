/**
 **************************************************************************
 * \file VSProxySettings.cpp
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief VSProxySettings Implementation. Only static members (one instanse)
 *
 * \b Project Client
 * \author SMirnovK
 * \date 13.08.2003
 *
 * $Revision: 9 $
 *
 * $History: VSProxySettings.cpp $
 *
 * *****************  Version 9  *****************
 * User: Smirnov      Date: 9.10.08    Time: 19:29
 * Updated in $/VSNA/VSClient
 * - some improvements
 *
 * *****************  Version 8  *****************
 * User: Smirnov      Date: 9.10.08    Time: 14:22
 * Updated in $/VSNA/VSClient
 * - bugfix with registry (old server deleting)
 *
 * *****************  Version 7  *****************
 * User: Smirnov      Date: 6.03.08    Time: 21:50
 * Updated in $/VSNA/VSClient
 * - bugfix with stack corrupt
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 26.02.08   Time: 16:54
 * Updated in $/VSNA/VSClient
 * - new servers coonect shceme
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 1.02.08    Time: 21:07
 * Updated in $/VSNA/VSClient
 * - set host-port
 *
 * *****************  Version 4  *****************
 * User: Dront78      Date: 18.01.08   Time: 17:02
 * Updated in $/VSNA/VSClient
 * - bigfixes from old architecture.
 * - ambiguous code marked as "old code"
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 16.01.08   Time: 15:15
 * Updated in $/VSNA/VSClient
 * - network settings repaired
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 22.11.07   Time: 16:41
 * Updated in $/VSNA/VSClient
 * - repare client
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/VSClient
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/VSClient
 *
 * *****************  Version 20  *****************
 * User: Smirnov      Date: 21.12.06   Time: 13:30
 * Updated in $/VS/VSClient
 * - client reconnected after proxy parametrs are changed
 *
 * *****************  Version 19  *****************
 * User: Smirnov      Date: 23.06.06   Time: 12:19
 * Updated in $/VS/VSClient
 * - do not reconnect transport if settings are not changed
 *
 * *****************  Version 18  *****************
 * User: Smirnov      Date: 22.06.06   Time: 19:54
 * Updated in $/VS/VSClient
 * - upnp OK!
 *
 * *****************  Version 17  *****************
 * User: Smirnov      Date: 21.06.06   Time: 16:17
 * Updated in $/VS/VSClient
 * - host and port behavior changed
 *
 * *****************  Version 16  *****************
 * User: Smirnov      Date: 22.05.06   Time: 12:07
 * Updated in $/VS/VSClient
 * - set/get host-port for current connection
 *
 * *****************  Version 15  *****************
 * User: Smirnov      Date: 17.04.06   Time: 14:18
 * Updated in $/VS/VSClient
 * - Registry keys and its values updated and documented
 *
 * *****************  Version 14  *****************
 * User: Smirnov      Date: 14.04.06   Time: 18:43
 * Updated in $/VS/VSClient
 * - on port update force reconnect to server throw home broker
 * - on network settings update force reconnect to server using current
 * broker
 *
 * *****************  Version 13  *****************
 * User: Smirnov      Date: 17.12.04   Time: 20:22
 * Updated in $/VS/VSClient
 * header to files
 *
 * *****************  Version 12  *****************
 * User: Smirnov      Date: 7.09.04    Time: 17:15
 * Updated in $/VS/VSClient
 * default setting - direct TCP
 *
 * *****************  Version 11  *****************
 * User: Smirnov      Date: 21.11.03   Time: 13:46
 * Updated in $/VS/VSClient
 * set Proxy password changed
 *
 * *****************  Version 10  *****************
 * User: Smirnov      Date: 28.10.03   Time: 14:44
 * Updated in $/VS/VSClient
 * set/get netType , sent to server
 *
 * *****************  Version 9  *****************
 * User: Smirnov      Date: 24.10.03   Time: 18:13
 * Updated in $/VS/VSClient
 * new mode httpTnlMsq
 *
 * *****************  Version 8  *****************
 * User: Smirnov      Date: 10.09.03   Time: 14:38
 * Updated in $/VS/VSClient
 * Update port for current server only
 *
 * *****************  Version 7  *****************
 * User: Smirnov      Date: 4.09.03    Time: 13:39
 * Updated in $/VS/VSClient
 * default Inet Opt
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 30.08.03   Time: 12:50
 * Updated in $/VS/VSClient
 * proxy port forse for all brokers
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 29.08.03   Time: 15:09
 * Updated in $/VS/VSClient
 * added Manual Port
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 26.08.03   Time: 12:07
 * Updated in $/VS/VSClient
 * do not write user and password
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 14.08.03   Time: 18:42
 * Updated in $/VS/vsclient
 * check new broker for valid configuration,
 * added new messages in bwt
 * proxy set rewrited
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 14.08.03   Time: 16:28
 * Updated in $/VS/vsclient
 * new methods of network mode
 *
 * *****************  Version 1  *****************
 * User: Smirnov      Date: 13.08.03   Time: 21:58
 * Created in $/VS/VSClient
 * Proxy Settings
 *
 ****************************************************************************/

/****************************************************************************
 * Includes
 ****************************************************************************/
#include "VSProxySettings.h"
#include "../net/EndpointRegistry.h"
#include "std-generic/cpplib/VS_Container.h"
#include "../std/cpplib/VS_RegistryKey.h"
#include "../std/cpplib/VS_Endpoint.h"
#include "../std/cpplib/VS_Utils.h"
#include "std/cpplib/utf8_old.h"
#include "SecureLib/VS_SymmetricCrypt.h"
#include <stdio.h>
#include "std-generic/cpplib/scope_exit.h"
///#include <algorithm>

/****************************************************************************
 * Classes
 ****************************************************************************/

unsigned long VSProxySettings::m_TrTh	= 0;
HWND VSProxySettings::m_callback		= 0;
HANDLE VSProxySettings::m_DialogClosed	= 0;
int	 VSProxySettings::m_RetValue		= 0;
char VSProxySettings::m_proxy[]			= "ProxyType";
char VSProxySettings::m_host[]			= "ProxyHost";
char VSProxySettings::m_port[]			= "ProxyPort";
char VSProxySettings::m_user[]			= "ProxyUserName";
char VSProxySettings::m_pass[]			= "ProxyPassword";
char VSProxySettings::m_cfg[]			= "NetworkMode";
char VSProxySettings::m_nettype[]		= "NetworkType";
char VSProxySettings::m_proxy_auth[]	= "ProxyInfo";

static const auto c_VSProxySettingsCryptKey = VS_GetPersistentKey();

VSProxySettings::VSProxySettings()
{
	m_DialogClosed = CreateEvent(NULL, FALSE, FALSE, NULL);
}

VSProxySettings::~VSProxySettings()
{
	CloseHandle(m_DialogClosed);
	m_DialogClosed = 0;
	m_callback = 0;
	m_TrTh = 0;
}

unsigned VSProxySettings::Request(	const char *protocol, const char *proxy, const unsigned short port,
									char *user, const unsigned user_size,
									char *passwd, const unsigned passwd_size)
{
	std::vector<wchar_t> w_user, w_pass;
	w_user.resize(user_size);
	w_pass.resize(passwd_size);

	VS_UTF8ToUCS(user, w_user.data(), w_user.size());
	VS_UTF8ToUCS(passwd, w_pass.data(), w_pass.size());

	m_RetValue = 0;
	if (m_callback) {
		PostMessage(m_callback, WM_USER + 33, (WPARAM)w_user.data(), (LPARAM)w_pass.data());
		if (WaitForSingleObject(m_DialogClosed, INFINITE)==WAIT_OBJECT_0) {
			VS_UCSToStr(w_user.data(), user, user_size);
			VS_UCSToStr(w_pass.data(), passwd, passwd_size);
			return m_RetValue;
		}
		else { // wrong params
			return 0;
		}
	}
	else {
		Sleep(500);
		return 2;
	}
}
void VSProxySettings::SetDialog(void * func)
{
	m_callback = (HWND)func;
}

void VSProxySettings::DialogEnd(int ret)
{
	m_RetValue = ret;
	SetEvent(m_DialogClosed);
}

int VSProxySettings::Set(int *type,char* proxy,unsigned short *port,char* user,char* passwd)
{
	long Port = *port;
	VS_RegistryKey key(true, REG_CurrentConfiguratuon, false, true);

	if (*type == VSPROXY_HTTP || *type == VSPROXY_SOCKS4 || *type == VSPROXY_SOCKS5)
		key.SetValue(type, 4, VS_REG_INTEGER_VT, m_proxy);
	else
		return 1;
	key.SetString(proxy, m_host);
	key.SetValue(&Port, 4, VS_REG_INTEGER_VT, m_port);
	if (!c_VSProxySettingsCryptKey.empty())
		WriteCryptedAuth(user,passwd);
	else
	{
		key.SetString(user, m_user);
		key.SetString(passwd, m_pass);
	}
	return 0;
}
void VSProxySettings::WriteCryptedAuth(const char *user, const char *passwd)
{
	VS_SymmetricCrypt crypt;
	if (!crypt.Init(alg_sym_AES256, mode_CBC)
	 || !crypt.SetKey(c_VSProxySettingsCryptKey.size(), c_VSProxySettingsCryptKey.data()))
		return;
	VS_Container	cnt;
	cnt.AddValue("u",user);
	cnt.AddValue("p",passwd);
	void *s_buf(0);
	size_t s_buf_sz(0);
	cnt.SerializeAlloc(s_buf,s_buf_sz);
	if(!s_buf_sz)
		return;
	VS_SCOPE_EXIT{ if (s_buf) ::free(s_buf); };
	///unsigned char *encr_buf(0);
	std::vector<unsigned char>	encr_buf;
	uint32_t encr_buf_sz(0);
	crypt.Encrypt((unsigned char *)s_buf,s_buf_sz,0,&encr_buf_sz);
	if(!encr_buf_sz)
		return;
	encr_buf.resize(encr_buf_sz);
	if(!crypt.Encrypt((unsigned char *)s_buf,s_buf_sz,&encr_buf[0],&encr_buf_sz))
		return;
	VS_RegistryKey key(true, REG_CurrentConfiguratuon, false, true);
	key.SetValue(&encr_buf[0],encr_buf_sz,VS_REG_BINARY_VT,m_proxy_auth);
}
bool VSProxySettings::ReadCryptedAuth(VS_SimpleStr &user, VS_SimpleStr &passwd)
{
	VS_SymmetricCrypt crypt;
	if (!crypt.Init(alg_sym_AES256, mode_CBC)
	 || !crypt.SetKey(c_VSProxySettingsCryptKey.size(), c_VSProxySettingsCryptKey.data()))
		return false;
	VS_RegistryKey key(true, REG_CurrentConfiguratuon);
	std::unique_ptr<unsigned char, free_deleter> buf;
	uint32_t sz(0);
	if((sz=key.GetValue(buf,VS_REG_BINARY_VT,m_proxy_auth))<=0)
		return false;
	std::vector<unsigned char> decr_data;
	uint32_t decr_sz(0);
	crypt.Decrypt(buf.get(), sz, 0, &decr_sz);
	decr_data.resize(decr_sz);
	if (!decr_sz || !crypt.Decrypt(buf.get(), sz, &decr_data[0], &decr_sz))
		return false;
	VS_Container cnt;
	if(!cnt.Deserialize(&decr_data[0],decr_sz))
		return false;
	user = cnt.GetStrValueRef("u");
	passwd = cnt.GetStrValueRef("p");
	return true;
}

int VSProxySettings::SetNetType(int mode)
{
	VS_RegistryKey key(true, REG_CurrentConfiguratuon, false, true);
	long param = 0;
	if (mode<0) {
		if (key.GetValue(&param, 4, VS_REG_INTEGER_VT, m_nettype)<=0)
			key.SetValue(&param, 4, VS_REG_INTEGER_VT, m_nettype);
		return param;
	}
	else {
		param = mode;
		key.SetValue(&param, 4, VS_REG_INTEGER_VT, m_nettype);
		return -1;
	}
}


/// \return = 0 - OK, 1 - bad param, 2 - bad registry
int VSProxySettings::SetManualPort(unsigned short *port, char* host, int mode)
{
	if (!port || !host)
		return 1;

	if (mode==0) { // read current
		*port = 0;
		*host = 0;
		char broker[256];
		if (VS_ReadAS(broker)) {
			auto tcp = net::endpoint::ReadConnectTCP(1, broker);
			if (tcp) {
				*port = tcp->port;
				strncpy(host, tcp->host.c_str(), 250);
				return 0;
			}
		}
		return 2;
	}
	else {
		unsigned short Oldport = 0;
		char Oldhost[256] = {0};
		// return if host and port the same
		if (SetManualPort(&Oldport, Oldhost, 0)==0 && strncmp(Oldhost, host, 255)==0 && Oldport==*port)
			return 0;
		net::endpoint::ClearAllConnectTCP(REG_TempServerName);
		net::endpoint::AddFirstConnectTCP({ host, *port, net::endpoint::protocol_tcp }, REG_TempServerName);
		UpdateEnpointsProtocol(REG_TempServerName);
		if (m_TrTh) {
			PostThreadMessage(m_TrTh, WM_APP+100, 0, 1);
		}
		return 0;
	}
}


void VSProxySettings::Get(int *type, char* proxy,unsigned short *port,char* user,char* passwd)
{
	if (!type || !proxy || !port || !user || !passwd)
		return;

	char buff[256];
	long Port = 0;
	VS_SimpleStr user_str,pass_str;
	VS_RegistryKey key(true, REG_CurrentConfiguratuon);
	if (key.GetValue(type, 4, VS_REG_INTEGER_VT, m_proxy)<=0) {
		*type = -1;
		return;
	}

	if (key.GetValue(buff, sizeof(buff), VS_REG_STRING_VT, m_host))
		strncpy(proxy, buff, 255);
	if (key.GetValue(&Port, sizeof(long), VS_REG_INTEGER_VT, m_port))
		*port = (unsigned short)Port;

	if(ReadCryptedAuth(user_str,pass_str))
	{
		strncpy(user,user_str.m_str,255);
		strncpy(passwd,pass_str.m_str,255);
	}
	else
	{
		if (key.GetValue(buff, sizeof(buff), VS_REG_STRING_VT, m_user))
			strncpy(user, buff, 255);
		if (key.GetValue(buff, sizeof(buff), VS_REG_STRING_VT, m_pass))
			strncpy(passwd, buff, 255);
	}
}

bool VSProxySettings::UpdateProxy(const char *ep, const net::endpoint::ConnectTCP& ProxyTCP)
{
	if (!ep || !*ep) {
		VS_RegistryKey key(true, "Endpoints", false);
		key.ResetKey();
		VS_RegistryKey keyIn;
		while(key.NextKey(keyIn))
			if (VS_IsBrokerFormat(keyIn.GetName()))
				UpdateProxy(keyIn.GetName(), ProxyTCP);
		return true;
	}
	else {
		const unsigned tcpNum = net::endpoint::GetCountConnectTCP(ep, true);
		for (int i = 1; i<=tcpNum; i++) {
			auto pctcp = net::endpoint::ReadConnectTCP(i, ep, true);
			if (!pctcp) continue;
			pctcp->protocol_name = ProxyTCP.protocol_name;
			pctcp->socks_host = ProxyTCP.socks_host;
			pctcp->socks_port = ProxyTCP.socks_port;
			pctcp->socks_user = ProxyTCP.socks_user;
			pctcp->socks_password = ProxyTCP.socks_password;
			pctcp->socks_version = ProxyTCP.socks_version;
			pctcp->http_host = ProxyTCP.http_host;
			pctcp->http_port = ProxyTCP.http_port;
			pctcp->http_user = ProxyTCP.http_user;
			pctcp->http_password = ProxyTCP.http_password;
			net::endpoint::UpdateConnectTCP(i, *pctcp, ep, true);
		}
		return false;
	}
}

void VSProxySettings::SetNetMode(int *cfg, bool update)
{
	if (!cfg) return;
	VS_RegistryKey key(true, REG_CurrentConfiguratuon, false, true);
	int	val = -1;
	key.GetValue(&val, 4, VS_REG_INTEGER_VT, m_cfg);
	if (*cfg==-1)
		if (val==-1) {
			*cfg=0;
		}
		else {
			*cfg=val;
			return;
		}
	else if (*cfg==1 && *cfg==val) {
		char broker[256];
		if (VS_ReadAS(broker)) {
			auto tcp = net::endpoint::ReadConnectTCP(1, broker);
			if (tcp) {
				net::endpoint::ConnectTCP ptcp;
				ReadProxyTCP(ptcp);
				bool isc =
					   ptcp.protocol_name == tcp->protocol_name
					&& ptcp.socks_host == tcp->socks_host
					&& ptcp.socks_port==tcp->socks_port
					&& ptcp.socks_password == tcp->socks_password
					&& ptcp.socks_user == tcp->socks_user
					&& ptcp.socks_version == tcp->socks_version
					&& ptcp.http_host == tcp->http_host
					&& ptcp.http_port == tcp->http_port
					&& ptcp.http_password == tcp->http_password
					&& ptcp.http_user == tcp->http_user;
				if (isc)
					return;
			}
		}
	}
	else if (*cfg==val)
		return;
	key.SetValue(cfg, 4, VS_REG_INTEGER_VT, m_cfg);
	UpdateEnpointsProtocol(0);
	if (update && m_TrTh)
		PostThreadMessage(m_TrTh, WM_APP+100, 0, 2);
}

bool VSProxySettings::ReadProxyTCP(net::endpoint::ConnectTCP& ProxyTCP)
{
	long cfg = 0;

	VS_RegistryKey key(true, REG_CurrentConfiguratuon);
	key.GetValue(&cfg, 4, VS_REG_INTEGER_VT, m_cfg);
	if		(cfg==1) {
		char host[256] = {0};
		long Port = 0, type = -1;
		key.GetValue(&type, 4, VS_REG_INTEGER_VT, m_proxy);

		key.GetValue(host, 255, VS_REG_STRING_VT, m_host);
		key.GetValue(&Port, sizeof(long), VS_REG_INTEGER_VT, m_port);
		unsigned short port = (unsigned short)Port;
		VS_SimpleStr user,pass;
		if(!ReadCryptedAuth(user,pass))
		{
			char buf[256] = {0};
			key.GetValue(buf, 255, VS_REG_STRING_VT, m_user);
			user = buf;
			memset(buf,0,256);
			key.GetValue(buf, 255, VS_REG_STRING_VT, m_pass);
			pass = buf;
		}

		const char* user_s = user ? user.m_str : "";
		const char* pass_s = pass ? pass.m_str : "";
		if		(type==VSPROXY_HTTP)
			ProxyTCP = net::endpoint::ConnectTCP{ {}, 0, net::endpoint::protocol_http_tnl, {}, 0, {}, {}, 0, host, port, user_s, pass_s };
		else if (type==VSPROXY_SOCKS4)
			ProxyTCP = net::endpoint::ConnectTCP{ {}, 0, net::endpoint::protocol_socks, host, port, user_s, pass_s, 4, {}, 0, {}, {} };
		else if (type==VSPROXY_SOCKS5)
			ProxyTCP = net::endpoint::ConnectTCP{ {}, 0, net::endpoint::protocol_socks, host, port, user_s, pass_s, 5, {}, 0, {}, {} };
		else
			ProxyTCP = net::endpoint::ConnectTCP{ {}, 0, net::endpoint::protocol_tcp };
	}
	else if (cfg==2)
		ProxyTCP = net::endpoint::ConnectTCP{ {}, 0, net::endpoint::protocol_internet_options };
	else if (cfg==3)
		ProxyTCP = net::endpoint::ConnectTCP{ {}, 0, net::endpoint::protocol_http_tnl_msq };
	else
		ProxyTCP = net::endpoint::ConnectTCP{ {}, 0, net::endpoint::protocol_tcp };

	return true;
}

void VSProxySettings::UpdateEnpointsProtocol(const char *ep)
{
	net::endpoint::ConnectTCP ProxyTCP;
	ReadProxyTCP(ProxyTCP);
	UpdateProxy(ep, ProxyTCP);
}
