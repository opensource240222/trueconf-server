#ifndef _VS_UPNP_INTERFACE_
#define _VS_UPNP_INTERFACE_

// enable multithread COM apartment
#define _WIN32_DCOM

#pragma once

#include <windows.h>
#include "natupnp.h"
#include <vector>

#include "../net/EndpointRegistry.h"

struct _short_port; // one long in two short
enum _upnp_wm; // UPnP Interface working model. See wiki for details.

class VS_UPnPInterface
{
public:
	using accepts_t = std::vector<std::unique_ptr<net::endpoint::AcceptTCP>>;
	using connects_t = std::vector<std::unique_ptr<net::endpoint::ConnectTCP>>;
private:
	static VS_UPnPInterface* iUPnP;
	bool m_COM_SUCCEESS; // true = COM multithread apartment inited successfully
	volatile LONG  m_RouterInit; // >0 = router inited successfully
	volatile LONG m_WorkInProgress; // >0 = mapping thread in progress
	int m_SYS_ERROR; // system error indicator
	HANDLE m_EvInit;
	HANDLE m_EvMapping;
	HANDLE m_EvGarbage;
	unsigned long m_ThreadID;
	volatile bool m_Die;
// UPnP descriptions begin
	IUPnPNAT *m_iNAT;
	IStaticPortMappingCollection *m_iMapping;
// UPnP garbage sectrion
	IStaticPortMappingCollection *m_giMapping; // dup for garbage thread
	IUnknown *m_iEnum;
	IEnumVARIANT *m_vMap;
// UPnP descriptions end
	_short_port *m_ports;
	unsigned long m_ports_count;
	unsigned long m_UPnP_count;
	char *m_source_ip;
	_upnp_wm m_wm;
	accepts_t m_accepts;  // accept results storage
	connects_t m_connects; // connect results storage
	char *m_lastAcceptIP;
	std::vector<char *> m_LocalIP;
	VS_UPnPInterface(void);
	virtual ~VS_UPnPInterface(void);
	bool GetLocalIP(); // fill local ip addresses list
	bool ResetStates(); // reset UPnP router interface
	static void DoInitRouter(void *);
	static void MappingThread(void *);
	static void GarbageThread(void *);
	bool VerifyWindowsVersion();
	void Bind2IntIface();
	void Bind2ExtIface();
	void Bind2JointIface();
public:
	static VS_UPnPInterface* Instance();
	bool FullAsyncPortMapping(unsigned long ThreadID, unsigned long *Ports, unsigned long Count, const char *SourceIp);
	bool PrepareToDie();
	bool GetConnects(accepts_t& acceptTCP, connects_t& connectTCP);
};
#endif
