#include "VS_UPnPInterface.h"
#include <comutil.h>
#include <process.h>

#include "../acs/Lib/VS_AcsLib.h"
#include "../acs/connection/VS_ConnectionTCP.h"
#include "std/cpplib/ThreadUtils.h"

// custom preprocessor message handling whit location by mose click
#define __STR2__(x) #x
#define __STR1__(x) __STR2__(x)
#define __LOC__ __FILE__ "("__STR1__(__LINE__)") : BOOKMARK : "
// usage: #pragma message (__LOC__"Hello world!")
// http://support.microsoft.com/kb/155196

const unsigned short DEFAULT_DIRECT_PORT = 5000;

const char * __upnp_id = "TrueConf";
const char * __upnp_old_id = "Visicron";
const char * __upnp_proto = "TCP";

const BSTR __upnp_tcp_proto = _com_util::ConvertStringToBSTR("TCP");
const BSTR __upnp_udp_proto = _com_util::ConvertStringToBSTR("UDP");
const BSTR __com_upnp_id = _com_util::ConvertStringToBSTR(__upnp_id);

static const string_view UPNP_EPNAME("UPnPHolder");

struct _short_port {
	unsigned short _1port;
	unsigned short _2port;
};

union _ip_addr {
	unsigned char _byte[4];
	unsigned long _addr;
};

enum _upnp_wm {_uIntIP, _uExtIP, _uIntExtIP};

const signed long _magic_chain = 0x17; // fixed port increment via bind select
const signed long _magic_load  = 0x0A; // max fixed port bind = 10
const signed long _magic_delta = _magic_load * _magic_chain;

VS_UPnPInterface *VS_UPnPInterface::iUPnP = 0; // singleton UPnP interface

VS_UPnPInterface* VS_UPnPInterface::Instance()
{
	if (iUPnP) return iUPnP;
	iUPnP = new VS_UPnPInterface();
	return iUPnP;
}

VS_UPnPInterface::VS_UPnPInterface(void)
: m_RouterInit(0), m_WorkInProgress(0), m_SYS_ERROR(-1),
m_EvInit(CreateEvent(0, true, false, 0)),
m_EvMapping(CreateEvent(0, true, false, 0)),
m_EvGarbage(CreateEvent(0, true, false, 0)),
m_ThreadID(0), m_Die(false), m_iNAT(0), m_iMapping(0), m_giMapping(0), m_iEnum(0), m_vMap(0),
m_ports(0), m_ports_count(0), m_source_ip(0), m_wm(_uIntIP), m_UPnP_count(0),
m_lastAcceptIP(0)
{
	if (!(VerifyWindowsVersion() && GetLocalIP()))
	{
		m_SYS_ERROR = -1;
		return;
	}
	HRESULT hr = S_OK; //CoInitializeEx(0, COINIT_MULTITHREADED);
	m_COM_SUCCEESS = (hr == S_OK || hr == S_FALSE);

	if (m_EvInit && m_EvMapping && m_EvGarbage) {
		_beginthread(&DoInitRouter, 0, this);
		_beginthread(&MappingThread, 0, this);
		_beginthread(&GarbageThread, 0, this);
	} else m_SYS_ERROR = -1;
}

VS_UPnPInterface::~VS_UPnPInterface(void)
{
	PrepareToDie();
//	CoUninitialize();
}

bool VS_UPnPInterface::GetLocalIP()
{
	if (!VS_AcsLibInitial()) return false;
	const unsigned long host_name_size = 512;
	char* host_name = new char[host_name_size];
	if (!host_name) return false;
	if (!VS_GetDefaultHostName(host_name, host_name_size)) {
		delete[] host_name; host_name = 0; return false;
	}

	unsigned long hosts_num = 5;
	unsigned int hosts_sz = 20;
	char** host = new char*[hosts_num];
	if (!host) { delete[] host_name; host_name = 0; return false; }

	unsigned int i = 0;
	while (i < hosts_num) {
		host[i] = new char[hosts_sz];
		if (host[i]) ++i; else break;
	}
	hosts_num = i; // hosts_num = number of allocated blocks
	unsigned int hosts = VS_GetHostsByName(host_name, host, hosts_num, hosts_sz);
	delete[] host_name; host_name = 0;

	if (!hosts) {
		for (i = 0; i < hosts_num; ++i) {
			delete[] host[i];
		}
		delete[] host; host = 0;
		return false;
	}

	for (i = 0; i < hosts; ++i) {
		if (VS_CheckIsInternetAddress(host[i])) {
			m_LocalIP.push_back(_strdup(host[i]));
		}
	}

	for (i = 0; i < hosts_num; ++i) {
		delete[] host[i];
	}
	delete[] host; host = 0;
	return true;
}

bool VS_UPnPInterface::FullAsyncPortMapping(unsigned long ThreadID, unsigned long *Ports, unsigned long Count, const char *SourceIp) {
	if ((m_Die) || (!ThreadID) || (!Ports) || (!Count) || (!SourceIp) || (!*SourceIp)) { PrepareToDie(); return false; }// threads already dead. nothing todo
	if (!m_COM_SUCCEESS) {
		// trying to repair COM after failure or some router bugs
		HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
		m_COM_SUCCEESS = (hr == S_OK || hr == S_FALSE);
		if (!m_COM_SUCCEESS) {
			return false;
		} else {
			ResetStates();
		}
	}
	if (InterlockedExchange(&m_WorkInProgress, 1)) return true; // lock thread

	m_accepts.clear();
	m_connects.clear();

	m_ThreadID = ThreadID;
	const unsigned n_atcp = net::endpoint::GetCountAcceptTCP(UPNP_EPNAME, true);
	if (n_atcp >= Count)
		m_ports_count = n_atcp;
	else m_ports_count = Count;
	delete[] m_ports; m_ports = 0;
	m_ports = new _short_port[m_ports_count];
	m_SYS_ERROR = 0;
	if (!m_ports) { InterlockedExchange(&m_WorkInProgress, 0); return false; }
	if (m_source_ip) { free(m_source_ip); m_source_ip = 0; }
	m_source_ip = _strdup(SourceIp);
	if (!m_source_ip) { delete[] m_ports; m_ports = 0; InterlockedExchange(&m_WorkInProgress, 0); return false; }
	if (m_lastAcceptIP) free(m_lastAcceptIP);
	m_lastAcceptIP = 0;
	memcpy((void *)m_ports, (void *)Ports, m_ports_count * sizeof(unsigned long));
	if (n_atcp)
	{
		for (unsigned i = 0; i < n_atcp; ++i)
		{
			auto rc = net::endpoint::ReadAcceptTCP(i + 1, UPNP_EPNAME, true);
			if (!rc || rc->host.empty() || rc->port == 0)
				continue;
			if (!m_lastAcceptIP && !VS_CheckIsInternetAddress(rc->host.c_str()))
				m_lastAcceptIP = _strdup(rc->host.c_str());
			m_ports[i]._1port = rc->port;
		}
		const unsigned n_ctcp = net::endpoint::GetCountConnectTCP(UPNP_EPNAME, true);
		for (unsigned i = 0; i < n_ctcp; ++i)
		{
			auto rc = net::endpoint::ReadConnectTCP(i + 1, UPNP_EPNAME, true);
			if (!rc) continue;
			signed long _mstate = 0;
			if (rc->port >= 80)
				++_mstate; // 1
			if (rc->port >= 443)
				++_mstate; // 2
			if (rc->port >  443 + _magic_delta)
				++_mstate; // 3
			switch (_mstate) {
				case 1 : { // 80
					for (unsigned i = 0; i < n_atcp; ++i) {
						if ((m_ports[i]._1port >= 80) && (m_ports[i]._1port < 443)) {
							m_ports[i]._2port = rc->port;
							break;
						}
					}
					break;
						 }
				case 2 : { // 443
					for (unsigned i = 0; i < n_atcp; ++i) {
						if ((m_ports[i]._1port >= 443) && (m_ports[i]._1port < DEFAULT_DIRECT_PORT)) {
							m_ports[i]._2port = rc->port;
							break;
						}
					}
					break;
						 }
				case 3 : { // 5050
					for (unsigned i = 0; i < n_atcp; ++i) {
						if (m_ports[i]._1port >= (443 + _magic_delta)) {
							m_ports[i]._2port = rc->port;
							break;
						}
					}
					break;
						 }
				default :{
					// nothing to do for now
						 }
			}
		}
	}

	net::endpoint::ClearAllConnectTCP(UPNP_EPNAME, true);
	net::endpoint::ClearAllAcceptTCP(UPNP_EPNAME, true);

	if (VS_CheckIsInternetAddress(m_source_ip)) { m_wm = _uExtIP; }

	if (InterlockedExchange(&m_RouterInit, 1)) {
		// router inited successfully, so wait for mapping thread finished
		SetEvent(m_EvMapping);
		return true;
	} else {
		// first init router thread, than mapping thread and after all - garbage
		SetEvent(m_EvInit);
		return true;
	}
	return true;
}

void VS_UPnPInterface::DoInitRouter(void *p) {
	vs::SetThreadName("UPnP_Init");
	VS_UPnPInterface *__iUPnP = (VS_UPnPInterface *)p;
	HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
	__iUPnP->m_COM_SUCCEESS &= (hr == S_OK || hr == S_FALSE);
	if (!__iUPnP->m_COM_SUCCEESS) { __iUPnP->ResetStates(); }
	while (true) {
		WaitForSingleObject(__iUPnP->m_EvInit, INFINITE);
		ResetEvent(__iUPnP->m_EvInit);
		if (__iUPnP->m_Die) break;

		if (!(!__iUPnP->m_SYS_ERROR && (S_OK ==  CoCreateInstance(__uuidof(UPnPNAT), 0, CLSCTX_ALL, __uuidof(IUPnPNAT), (void **) &__iUPnP->m_iNAT))
			&& __iUPnP->m_iNAT)) {
				__iUPnP->ResetStates();
		}

		if (!(!__iUPnP->m_SYS_ERROR && (S_OK == __iUPnP->m_iNAT->get_StaticPortMappingCollection(&__iUPnP->m_iMapping))
			&& __iUPnP->m_iMapping)) {
				__iUPnP->ResetStates();
		}
		if (!(!__iUPnP->m_SYS_ERROR && (S_OK == __iUPnP->m_iNAT->get_StaticPortMappingCollection(&__iUPnP->m_giMapping)) // garbage dup
			&& __iUPnP->m_giMapping)) {
				__iUPnP->ResetStates();
		}
		SetEvent(__iUPnP->m_EvMapping); // now start mapping thread
	}
	CloseHandle(__iUPnP->m_EvInit);
	CoUninitialize();
	return;
}

void VS_UPnPInterface::MappingThread(void *p) {
	vs::SetThreadName("UPnP_Mapping");
	VS_UPnPInterface *__iUPnP = (VS_UPnPInterface *)p;
	HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
	__iUPnP->m_COM_SUCCEESS &= (hr == S_OK || hr == S_FALSE);
	if (!__iUPnP->m_COM_SUCCEESS) { __iUPnP->ResetStates(); }
	while (true) {
		WaitForSingleObject(__iUPnP->m_EvMapping, INFINITE);
		ResetEvent(__iUPnP->m_EvMapping);
		if (__iUPnP->m_Die) break;
		__iUPnP->m_UPnP_count = 0;
		if (__iUPnP->m_wm == _uIntIP) {
			std::vector<char *>::iterator _vi = __iUPnP->m_LocalIP.begin();
			while (_vi != __iUPnP->m_LocalIP.end()) {
				if (strcmp((*_vi), __iUPnP->m_source_ip)) {
					++_vi;
				} else {
					delete[] (*_vi);
					_vi = __iUPnP->m_LocalIP.erase(_vi);
				}
			}
			if (__iUPnP->m_LocalIP.size()) __iUPnP->m_wm = _uIntExtIP;
		}
		switch (__iUPnP->m_wm) {
			case _uIntIP : {
				__iUPnP->Bind2IntIface();
				break;
						   }
			case _uExtIP : {
				__iUPnP->Bind2ExtIface();
				break;
						   }
			case _uIntExtIP : {
				__iUPnP->Bind2JointIface();
				break;
							  }
			default: {
				// nothing to do :)
					 }
		};

		PostThreadMessage(__iUPnP->m_ThreadID, WM_APP+99, 0, 0);
		if (__iUPnP->m_EvGarbage) {
			SetEvent(__iUPnP->m_EvGarbage);
		} else {
			if (!__iUPnP->m_UPnP_count) __iUPnP->ResetStates();
			InterlockedExchange(&__iUPnP->m_WorkInProgress, 0);
		}

	}
	CloseHandle(__iUPnP->m_EvMapping);
	CoUninitialize();
	return;
}

void VS_UPnPInterface::GarbageThread(void *p) {
	vs::SetThreadName("UPnP_Garbage");
	VS_UPnPInterface *__iUPnP = (VS_UPnPInterface *)p;
	HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
	__iUPnP->m_COM_SUCCEESS &= (hr == S_OK || hr == S_FALSE);
	if (!__iUPnP->m_COM_SUCCEESS) { __iUPnP->ResetStates(); }
	while (true) {
		WaitForSingleObject(__iUPnP->m_EvGarbage, INFINITE);
		ResetEvent(__iUPnP->m_EvGarbage);
		if (__iUPnP->m_Die) break;
		if (!(!__iUPnP->m_SYS_ERROR && (S_OK == __iUPnP->m_giMapping->get__NewEnum(&__iUPnP->m_iEnum)) &&
			__iUPnP->m_iEnum)) {
				__iUPnP->ResetStates();
				break;
		}
		if (!(!__iUPnP->m_SYS_ERROR && (S_OK == __iUPnP->m_iEnum->QueryInterface(IID_IEnumVARIANT, (void **)&__iUPnP->m_vMap)) &&
			__iUPnP->m_vMap)) {
				__iUPnP->ResetStates();
				break;
		}
		__iUPnP->m_vMap->Reset();
		long __map_count = 0;
		if (!(!__iUPnP->m_SYS_ERROR && (S_OK == __iUPnP->m_giMapping->get_Count(&__map_count)))
			&& (__map_count >= 0)) {
				__iUPnP->ResetStates();
				break;
		}
		while (__map_count) {
			VARIANT *vr = new VARIANT[__map_count];
			if (!vr) { break; }
			for (long i = 0; i < __map_count; ++i) {
				VariantInit(&vr[i]);
			}
			ULONG maxvr = 0;
			HRESULT hr = __iUPnP->m_vMap->Next(__map_count, vr, &maxvr);
			if (!(!__iUPnP->m_SYS_ERROR && ((S_OK == hr) || (S_FALSE == hr)) && maxvr)) {
				delete[] vr;
				break;
			}
			__map_count -= maxvr;
			if (__map_count < 0) __map_count = 0;
			for (unsigned long i = 0; i < maxvr; ++i) {
				if (vr[i].vt == VT_EMPTY) { continue; }
				IStaticPortMapping *__pMapId = 0;
				IDispatch *__pDispMap = V_DISPATCH(&vr[i]);
				if (!(!__iUPnP->m_SYS_ERROR && __pDispMap
					&& (S_OK == __pDispMap->QueryInterface(IID_IStaticPortMapping, (void **)&__pMapId))
					&& __pMapId)) {
						break;
				}
				BSTR __com_string = 0;
				if (!(!__iUPnP->m_SYS_ERROR && (S_OK == __pMapId->get_Description(&__com_string)))) {
					if (__pMapId) __pMapId->Release();
					break;
				}
				char *cStr = _com_util::ConvertBSTRToString(__com_string);
				if (cStr && !strcmp(cStr, __upnp_old_id)) {
					long _ext_port = 0;
					if (!(!__iUPnP->m_SYS_ERROR && (S_OK == __pMapId->get_ExternalPort(&_ext_port)) &&
						_ext_port)) {
							if (__pMapId) __pMapId->Release();
							break;
					}
					if (!(!__iUPnP->m_SYS_ERROR && (S_OK == __iUPnP->m_giMapping->Remove(_ext_port, __upnp_tcp_proto)))) {
							if (__pMapId) __pMapId->Release();
							break;
					}
				} delete[] cStr;
			} delete[] vr;
		} break;
	}
	if (__iUPnP->m_vMap) __iUPnP->m_vMap->Release(); __iUPnP->m_vMap = 0;
	if (__iUPnP->m_iEnum) __iUPnP->m_iEnum->Release(); __iUPnP->m_iEnum = 0;
	if (__iUPnP->m_giMapping) __iUPnP->m_giMapping->Release(); __iUPnP->m_giMapping = 0;
	CloseHandle(__iUPnP->m_EvGarbage);
	__iUPnP->m_EvGarbage = 0;
	CoUninitialize();
	if (!__iUPnP->m_UPnP_count) __iUPnP->ResetStates();
	InterlockedExchange(&__iUPnP->m_WorkInProgress, 0);
	return;
}

bool VS_UPnPInterface::PrepareToDie() {
	m_Die = true;
	SetEvent(m_EvInit);
	SetEvent(m_EvMapping);
	SetEvent(m_EvGarbage);
	return m_Die;
}

bool VS_UPnPInterface::ResetStates() {
	m_SYS_ERROR = -1;
	ResetEvent(m_EvInit);
	ResetEvent(m_EvMapping);
	InterlockedExchange(&m_RouterInit, 0);
	if (m_iMapping) m_iMapping->Release(); m_iMapping = 0;
	if (m_iNAT) m_iNAT->Release(); m_iNAT = 0;
	return true;
}

bool VS_UPnPInterface::VerifyWindowsVersion()
{
	OSVERSIONINFOEX osvi = {};
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	if (!GetVersionEx ((OSVERSIONINFO *)&osvi)) {
		osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
		if (!GetVersionEx ((OSVERSIONINFO *)&osvi)) return false;
	}
	if ((osvi.dwPlatformId == VER_PLATFORM_WIN32_NT)
		&& ((osvi.dwMajorVersion >= 6) // Windows Vista support enable
		 || ((osvi.dwMajorVersion == 5) && (osvi.dwMinorVersion))))
		 return true;
	return false;
}

void VS_UPnPInterface::Bind2IntIface() {
	VS_ConnectionTCP *__TCP = new VS_ConnectionTCP();
	unsigned short _port  = m_ports[0]._1port;
	unsigned short _eport = m_ports[0]._2port;
	for (int i = 0; i < _magic_load; ++i) {
		if (__TCP->Bind(m_source_ip, _port,false)) {
			__TCP->Close(); // must do this because of rebind requerements
			if (m_accepts.size() < 1)
				m_accepts.resize(1);
			m_accepts[0].reset(new net::endpoint::AcceptTCP{ m_source_ip, _port, __upnp_proto });
			net::endpoint::AddAcceptTCP(*m_accepts[0], UPNP_EPNAME, true);
			if (!m_SYS_ERROR) { // UPnP is ok?
				if (_eport) { // already got mapping
					IStaticPortMapping *_iPortMap = 0;
					if (S_OK == m_iMapping->get_Item(_eport, __upnp_tcp_proto, &_iPortMap) && _iPortMap) {
						BSTR _icli = 0; BSTR _ecli = 0; char *_iip = 0; char *_eip = 0;
						if ((S_OK == _iPortMap->get_InternalClient(&_icli)) && (S_OK == _iPortMap->get_ExternalIPAddress(&_ecli))) {
							_iip = _com_util::ConvertBSTRToString(_icli);
							_eip = _com_util::ConvertBSTRToString(_ecli);
						}
						if (_iip && _eip) {
							if (!strcmp(_iip, m_source_ip) && (S_OK == _iPortMap->EditInternalPort(_port))) {
								++ m_UPnP_count;
								m_connects.emplace_back(new net::endpoint::ConnectTCP{ _eip, _eport, __upnp_proto });
								net::endpoint::AddConnectTCP(*m_connects.back(), UPNP_EPNAME, true);
							} else if (m_lastAcceptIP && !strcmp(_iip, m_lastAcceptIP)) {
								BSTR __icli = _com_util::ConvertStringToBSTR(m_lastAcceptIP);
								if (S_OK == _iPortMap->EditInternalClient(__icli) && (S_OK == _iPortMap->EditInternalPort(_port))) {
									++ m_UPnP_count;
									m_connects.emplace_back(new net::endpoint::ConnectTCP{ _eip, _eport, __upnp_proto });
									net::endpoint::AddConnectTCP(*m_connects.back(), UPNP_EPNAME, true);
								}
								SysFreeString(__icli);
							}
						}
						delete[] _iip; delete[] _eip; SysFreeString(_icli); SysFreeString(_ecli);
					} else {
						BSTR _cl = _com_util::ConvertStringToBSTR(m_source_ip), _ecli = 0;
						char *_eip = 0;
						if (S_OK == m_iMapping->Add(_eport, __upnp_tcp_proto, _port, _cl, true, __com_upnp_id, &_iPortMap) && _iPortMap && (S_OK == _iPortMap->get_ExternalIPAddress(&_ecli))) {
							_eip = _com_util::ConvertBSTRToString(_ecli);
							++ m_UPnP_count;
							m_connects.emplace_back(new net::endpoint::ConnectTCP{ _eip, _eport, __upnp_proto });
							net::endpoint::AddConnectTCP(*m_connects.back(), UPNP_EPNAME, true);
						}
						SysFreeString(_cl); SysFreeString(_ecli); delete[] _eip;
					}
				} else {
					_eport = _port; IStaticPortMapping *_iPortMap = 0;
					BSTR _icli = _com_util::ConvertStringToBSTR(m_source_ip);
					BSTR _ecli = 0; char *_eip = 0;
					for (int j = 0; j < _magic_load; ++j) {
						if (S_OK == m_iMapping->Add(_eport, __upnp_tcp_proto, _port, _icli, true, __com_upnp_id, &_iPortMap) && _iPortMap && (S_OK == _iPortMap->get_ExternalIPAddress(&_ecli))) {
							_eip = _com_util::ConvertBSTRToString(_ecli);
							if (_eip) {
								++ m_UPnP_count;
								m_connects.emplace_back(new net::endpoint::ConnectTCP{ _eip, _eport, __upnp_proto });
								net::endpoint::AddConnectTCP(*m_connects.back(), UPNP_EPNAME, true);
							} break;
						} else {
							_eport += _magic_chain;
						}
					} delete[] _eip; SysFreeString(_icli); SysFreeString(_ecli);
				}
			} break;
		} _port += _magic_chain;
	}

	if (!m_SYS_ERROR) { // now we'll try UPnP to bind fixed ports
		for (unsigned long i = 1; i < m_ports_count; ++i) {
			unsigned short _lport = m_ports[i]._1port;
			unsigned short _leport = _lport;
			for (int j = 0; j < _magic_load; ++j) {
				bool _success = false;
				if (__TCP->Bind(m_source_ip, _lport,false)) {
					__TCP->Close(); // must do this because of rebind requerements
					BSTR _ic = _com_util::ConvertStringToBSTR(m_source_ip);
					IStaticPortMapping *_iPortMap = 0;
					if (S_OK == m_iMapping->get_Item(_leport, __upnp_tcp_proto, &_iPortMap) && _iPortMap) {
						BSTR _icli = 0, _ecli = 0;
						if (S_OK == _iPortMap->get_ExternalIPAddress(&_ecli)) {
							if (S_OK == _iPortMap->get_InternalClient(&_icli)) {
								char *_ciip = _com_util::ConvertBSTRToString(_icli);
								char *_ceip = _com_util::ConvertBSTRToString(_ecli);
								if (!strcmp(m_source_ip, _ciip) && (S_OK == _iPortMap->EditInternalPort(_lport))) { // ok! it's our ip!
									++ m_UPnP_count;
									m_connects.emplace_back(new net::endpoint::ConnectTCP{ _ceip, _leport, __upnp_proto });
									net::endpoint::AddConnectTCP(*m_connects.back(), UPNP_EPNAME, true);
									m_accepts.emplace_back(new net::endpoint::AcceptTCP{ m_source_ip, _lport, __upnp_proto });
									net::endpoint::AddAcceptTCP(*m_accepts.back(), UPNP_EPNAME, true);
								} else if (m_lastAcceptIP && !strcmp(m_lastAcceptIP, _ciip)){ // ok! our previous ip!
									BSTR _descr = 0;
									char *_cdescr = 0;
									if (S_OK == _iPortMap->get_Description(&_descr)) {
										_cdescr = _com_util::ConvertBSTRToString(_descr);
									}
									if (_cdescr && !strcmp(__upnp_id, _cdescr) && (S_OK == _iPortMap->EditInternalClient(_ic) && (S_OK == _iPortMap->EditInternalPort(_lport)))) { // ok!. we've got it!
										++ m_UPnP_count;
										m_connects.emplace_back(new net::endpoint::ConnectTCP{ _ceip, _leport, __upnp_proto });
										net::endpoint::AddConnectTCP(*m_connects.back(), UPNP_EPNAME, true);
										m_accepts.emplace_back(new net::endpoint::AcceptTCP{ m_source_ip, _lport, __upnp_proto });
										net::endpoint::AddAcceptTCP(*m_accepts.back(), UPNP_EPNAME, true);
									}
									SysFreeString(_descr);	delete[] _cdescr;
								} delete[] _ceip; delete[] _ciip;
							}
						}
						SysFreeString(_icli); SysFreeString(_ecli);
					} else {
						_iPortMap = 0;
					}
					if (!_iPortMap && S_OK == m_iMapping->Add(_leport, __upnp_tcp_proto, _lport, _ic, true, __com_upnp_id, &_iPortMap) && _iPortMap) {
						BSTR _eip = 0;
						if (S_OK == _iPortMap->get_ExternalIPAddress(&_eip)) {
							char *_ceip = _com_util::ConvertBSTRToString(_eip);
							++ m_UPnP_count;
							m_connects.emplace_back(new net::endpoint::ConnectTCP{ _ceip, _leport, __upnp_proto });
							net::endpoint::AddConnectTCP(*m_connects.back(), UPNP_EPNAME, true);
							delete[] _ceip; SysFreeString(_eip);
							m_accepts.emplace_back(new net::endpoint::AcceptTCP{ m_source_ip, _lport, __upnp_proto });
							net::endpoint::AddAcceptTCP(*m_accepts.back(), UPNP_EPNAME, true);
						}
					}
					SysFreeString(_ic);
					break;
				} else {
					_lport += _magic_chain;
				}
			}
		}
	}
	__TCP->Close();
	delete __TCP;
	return;
}

void VS_UPnPInterface::Bind2ExtIface() {
	Bind2IntIface();
	VS_ConnectionTCP *__TCP = new VS_ConnectionTCP();
	for (unsigned long i = 1; i < m_ports_count; ++i) {
		if (__TCP->Bind(m_source_ip, m_ports[i]._1port,false)) {
			__TCP->Close(); // must do this because of rebind requerements
			m_accepts.emplace_back(new net::endpoint::AcceptTCP{ m_source_ip, m_ports[i]._1port, __upnp_proto });
		}
	}
	delete __TCP;
	return;
}

void VS_UPnPInterface::Bind2JointIface() {
	Bind2IntIface();
	if (m_source_ip) free(m_source_ip);
	m_source_ip = 0;
	if (m_LocalIP.size()) {
		m_source_ip = _strdup(m_LocalIP.at(0));
	}
	if (m_source_ip) {
		Bind2ExtIface();
	}
	return;
}

bool VS_UPnPInterface::GetConnects(accepts_t& acceptTCP, connects_t& connectTCP)
{
	acceptTCP = std::move(m_accepts);
	connectTCP = std::move(m_connects);
	return acceptTCP.size() > 0;
}