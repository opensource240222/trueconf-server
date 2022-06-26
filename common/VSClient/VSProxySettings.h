/**
 **************************************************************************
 * \file VSProxySettings.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief Proxy settings for Visicron transport level.
 * Set also port for server endpoints
 *
 * \b Project Client
 * \author SMirnovK
 * \date 13.08.2003
 *
 * $Revision: 2 $
 *
 * $History: VSProxySettings.h $
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 26.02.08   Time: 16:54
 * Updated in $/VSNA/VSClient
 * - new servers coonect shceme
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/VSClient
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/VSClient
 *
 * *****************  Version 11  *****************
 * User: Smirnov      Date: 22.06.06   Time: 19:54
 * Updated in $/VS/VSClient
 * - upnp OK!
 *
 * *****************  Version 10  *****************
 * User: Smirnov      Date: 22.05.06   Time: 12:07
 * Updated in $/VS/VSClient
 * - set/get host-port for current connection
 *
 * *****************  Version 9  *****************
 * User: Smirnov      Date: 17.04.06   Time: 14:18
 * Updated in $/VS/VSClient
 * - Registry keys and its values updated and documented
 *
 * *****************  Version 8  *****************
 * User: Smirnov      Date: 14.04.06   Time: 18:43
 * Updated in $/VS/VSClient
 * - on port update force reconnect to server throw home broker
 * - on network settings update force reconnect to server using current
 * broker
 *
 * *****************  Version 7  *****************
 * User: Smirnov      Date: 17.12.04   Time: 20:22
 * Updated in $/VS/VSClient
 * header to files
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 21.11.03   Time: 13:46
 * Updated in $/VS/VSClient
 * set Proxy password changed
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 28.10.03   Time: 14:44
 * Updated in $/VS/VSClient
 * set/get netType , sent to server
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 23.09.03   Time: 18:23
 * Updated in $/VS/VSClient
 * default port now 5050
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 10.09.03   Time: 14:38
 * Updated in $/VS/VSClient
 * Update port for current server only
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 29.08.03   Time: 15:09
 * Updated in $/VS/VSClient
 * added Manual Port
 *
 * *****************  Version 1  *****************
 * User: Smirnov      Date: 13.08.03   Time: 21:58
 * Created in $/VS/VSClient
 * Proxy Settings
 *
 ****************************************************************************/
#ifndef VS_PROXY_SETTINGS_H
#define VS_PROXY_SETTINGS_H

/****************************************************************************
 * Includes
 ****************************************************************************/
#include "../acs/ConnectionManager/VS_ConnectionManager.h"
#include "../net/EndpointRegistry_fwd.h"

#include "VS_ApplicationInfo.h"
#include <windows.h>
#include <vector>

/**
 **************************************************************************
 * \brief Type of proxy
 ****************************************************************************/
enum VSProxy_Types {
	VSPROXY_INVALID = -1,
	VSPROXY_HTTP,
	VSPROXY_SOCKS4,
	VSPROXY_SOCKS5
};

/**
 **************************************************************************
 * \brief Proxy settings for Visicron transport level. Save settings in registry
 ****************************************************************************/
class VS_SimpleStr;
class VSProxySettings : public VS_Authentication
{
public:
	VSProxySettings();
	~VSProxySettings();
	void SetTh(DWORD Th) {m_TrTh = Th;}
	unsigned Request(const char *protocol, const char *proxy, const unsigned short port,
					 char *user, const unsigned user_size,
					 char *passwd, const unsigned passwd_size );
	static void SetDialog(void * func);
	static void DialogEnd(int ret);
	static int  Set(int *type, char *proxy, unsigned short *port, char *user, char *passwd);
	static void Get(int *type, char *proxy, unsigned short *port, char *user, char *passwd);
	// cfg = -1 read current, 0 - set manual, 1 - set IntenetOpt, 2 - set proxy
	static void SetNetMode(int* cfg, bool update = false);
	// mode = 0 read current, 1 - read default, 2 - set current, 3 - set default
	static int  SetManualPort(unsigned short *port, char* host, int mode);
	// return type if mode<0, else set type = mode
	static int  SetNetType(int mode);
	// update endpoints protocol as set by SetNetType();
	static void UpdateEnpointsProtocol(const char *ep);
	// read parametrs in pointed object
	static bool ReadProxyTCP(net::endpoint::ConnectTCP& ProxyTCP);
private:
	static bool UpdateProxy(const char *ep, const net::endpoint::ConnectTCP& ProxyTCP);
	static void WriteCryptedAuth(const char *user, const char *passwd);
	static bool	ReadCryptedAuth(VS_SimpleStr &user, VS_SimpleStr &passwd);

	static unsigned long m_TrTh;
	static HWND			m_callback;
	static HANDLE		m_DialogClosed;
	static int			m_RetValue;

	static char			m_proxy_auth[];
	static char			m_proxy[];
	static char			m_host[];
	static char			m_port[];
	static char			m_user[];
	static char			m_pass[];
	static char			m_cfg[];
	static char			m_nettype[];
	static const unsigned short DEFAULT_PORT = 5050;
};

#endif
