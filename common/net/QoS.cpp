#include "QoS.h"

#ifdef _WIN32

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <qos2.h>

namespace net {

typedef uintptr_t QoSFlowID;

class QoSController {
public:
	static inline std::shared_ptr<QoSController> GetInstance();

	QoSController(void);
	~QoSController();

	inline bool IsInitialized(void);

	inline bool AddSocket(QoSFlowID &res_flow_id, const uintptr_t socket, const QoSTrafficType traffic_type, const void *dst_sockaddr = nullptr);
	inline bool RemoveSocket(const QoSFlowID &flow_id, const uintptr_t socket);

	inline bool SetDSCPValue(const QoSFlowID &flow_id, const uint8_t value);
	inline bool SetTrafficType(const QoSFlowID &flow_id, const QoSTrafficType traffic_type);

private:
	QoSController(const QoSController &) = delete;
	QoSController &operator=(const QoSController &) = delete;

private:
	static inline int GetWinQoSTrafficType(const QoSTrafficType traffic_type);
	void Init(void);
private:
	static std::shared_ptr<QoSController> g_qos_controller;
	static std::mutex g_qos_lock;
private:
	bool m_initialized;
	HANDLE m_qos_handle;

	HMODULE m_qwave_lib;
	BOOL (WINAPI *m_QOSCreateHandle)(PQOS_VERSION, PHANDLE);
	BOOL (WINAPI *m_QOSCloseHandle)(HANDLE);
	BOOL (WINAPI *m_QOSAddSocketToFlow)(HANDLE, SOCKET, PSOCKADDR, QOS_TRAFFIC_TYPE, DWORD, PQOS_FLOWID);
	BOOL (WINAPI *m_QOSRemoveSocketFromFlow)(HANDLE, SOCKET, QOS_FLOWID, DWORD);
	BOOL (WINAPI *m_QOSSetFlow)(HANDLE, QOS_FLOWID, QOS_SET_FLOW, ULONG, PVOID, DWORD, LPOVERLAPPED);
};

QoSController::QoSController() :
	m_initialized(false), m_qos_handle(nullptr), m_qwave_lib(nullptr),
	m_QOSCreateHandle(nullptr), m_QOSCloseHandle(nullptr), m_QOSAddSocketToFlow(nullptr),
	m_QOSRemoveSocketFromFlow(nullptr), m_QOSSetFlow(nullptr)
{
	Init();
}

QoSController::~QoSController()
{
	if (m_qos_handle != nullptr)
	{
		m_QOSCloseHandle(m_qos_handle);
		m_qos_handle = nullptr;
	}

	if (m_qwave_lib != nullptr)
	{
		FreeLibrary(m_qwave_lib);
		m_qwave_lib = nullptr;
	}

	m_initialized = false;
}

bool QoSController::IsInitialized(void)
{
	return m_initialized;
}

bool QoSController::AddSocket(QoSFlowID &res_flow_id, const uintptr_t socket, const QoSTrafficType traffic_type, const void *dst_sockaddr)
{
	int win_qos_traffic_type = GetWinQoSTrafficType(traffic_type);
	if (!m_initialized || win_qos_traffic_type == -1)
		return false;

	QOS_FLOWID flow_id = static_cast<QOS_FLOWID>(res_flow_id);
	if (m_QOSAddSocketToFlow(m_qos_handle, static_cast<SOCKET>(socket), const_cast<SOCKADDR *>(reinterpret_cast<const SOCKADDR *>(dst_sockaddr)), static_cast<QOS_TRAFFIC_TYPE>(win_qos_traffic_type), QOS_NON_ADAPTIVE_FLOW, &flow_id) != 0)
	{
		res_flow_id = flow_id;
		return true;
	}

	return false;
}

bool QoSController::RemoveSocket(const QoSFlowID &flow_id, const uintptr_t socket)
{
	if (!m_initialized)
		return false;
	return (m_QOSRemoveSocketFromFlow(m_qos_handle, static_cast<SOCKET>(socket), static_cast<QOS_FLOWID>(flow_id), 0) != FALSE);
}

bool QoSController::SetDSCPValue(const QoSFlowID &flow_id, uint8_t value)
{
	DWORD dscp_value = value;
	if (!m_initialized)
		return false;
	return (m_QOSSetFlow(m_qos_handle, static_cast<QOS_FLOWID>(flow_id), QOSSetOutgoingDSCPValue, sizeof(dscp_value), &dscp_value, 0, NULL) != FALSE);
}

bool QoSController::SetTrafficType(const QoSFlowID &flow_id, const QoSTrafficType traffic_type)
{
	int win_traffic_type = GetWinQoSTrafficType(traffic_type);
	QOS_TRAFFIC_TYPE qos_traffic_type;
	if (!m_initialized || win_traffic_type == -1)
		return false;
	qos_traffic_type = static_cast<QOS_TRAFFIC_TYPE>(win_traffic_type);
	return (m_QOSSetFlow(m_qos_handle, static_cast<QOS_FLOWID>(flow_id), QOSSetTrafficType, sizeof(qos_traffic_type), &qos_traffic_type, 0, NULL) != FALSE);
}

inline int QoSController::GetWinQoSTrafficType(const QoSTrafficType traffic_type)
{
	switch (traffic_type)
	{
		case QoSTrafficType::BestEffort:
			return QOSTrafficTypeBestEffort;
			break;
		case QoSTrafficType::Background:
			return QOSTrafficTypeBackground;
			break;
		case QoSTrafficType::ExcellentEffort:
			return QOSTrafficTypeExcellentEffort;
			break;
		case QoSTrafficType::AudioVideo:
			return QOSTrafficTypeAudioVideo;
			break;
		case QoSTrafficType::Voice:
			return QOSTrafficTypeVoice;
			break;
		case QoSTrafficType::Control:
			return QOSTrafficTypeControl;
			break;
		default:
			return -1;
			break;
	}
	return -1;
}

void QoSController::Init(void)
{
	m_qwave_lib = LoadLibraryA("qwave.dll");
	if (m_qwave_lib == nullptr)
	{
		return;
	}

	m_QOSCreateHandle = reinterpret_cast<BOOL(WINAPI *)(PQOS_VERSION, PHANDLE)>(GetProcAddress(m_qwave_lib, "QOSCreateHandle"));
	m_QOSCloseHandle = reinterpret_cast<BOOL(WINAPI *)(HANDLE)>(GetProcAddress(m_qwave_lib, "QOSCloseHandle"));
	m_QOSAddSocketToFlow = reinterpret_cast<BOOL(WINAPI *)(HANDLE, SOCKET, PSOCKADDR, QOS_TRAFFIC_TYPE, DWORD, PQOS_FLOWID)>(GetProcAddress(m_qwave_lib, "QOSAddSocketToFlow"));
	m_QOSRemoveSocketFromFlow = reinterpret_cast<BOOL(WINAPI *)(HANDLE, SOCKET, QOS_FLOWID, DWORD)>(GetProcAddress(m_qwave_lib, "QOSRemoveSocketFromFlow"));
	m_QOSSetFlow = reinterpret_cast<BOOL(WINAPI *)(HANDLE, QOS_FLOWID, QOS_SET_FLOW, ULONG, PVOID, DWORD, LPOVERLAPPED)>(GetProcAddress(m_qwave_lib, "QOSSetFlow"));

	if (m_QOSCreateHandle == nullptr ||
		m_QOSCloseHandle == nullptr ||
		m_QOSAddSocketToFlow == nullptr ||
		m_QOSRemoveSocketFromFlow == nullptr ||
		m_QOSSetFlow == nullptr)
	{
		return;
	}

	{
		QOS_VERSION ver = { 1, 0 };
		m_initialized = (m_QOSCreateHandle(&ver, &m_qos_handle) != FALSE);
		if (!m_initialized)
			m_qos_handle = NULL;
	}
}

std::shared_ptr<QoSController> QoSController::g_qos_controller;
std::mutex QoSController::g_qos_lock;

std::shared_ptr<QoSController> QoSController::GetInstance()
{
	std::lock_guard<std::mutex> lock(g_qos_lock);

	if (g_qos_controller == nullptr)
	{
		g_qos_controller = std::make_shared<QoSController>();
	}

	return g_qos_controller;
}


// !!! QoSFlowImpl for Win32 starts here !!!
class QoSFlowImpl
{
public:
	QoSFlowImpl();
	QoSFlowImpl(const uint8_t dscp_value);
	QoSFlowImpl(const QoSTrafficType traffic_type);

	~QoSFlowImpl();

	inline bool IsInitialised(void);

	inline bool AddSocket(const uintptr_t socket, const void *dst_sockaddr = nullptr); // Windows specific
	inline bool RemoveSocket(const uintptr_t socket);

	inline bool SetDSCPValue(const uint8_t value);
	inline bool SetTrafficType(const QoSTrafficType traffic_type);

	inline uint8_t GetDSCPValue(void);
	inline QoSTrafficType GetTrafficType(void);
private:
	bool RemoveSocketInternal(const uintptr_t socket);
private:
	std::shared_ptr<QoSController> m_qos_controller;
	std::mutex m_lock;

	size_t m_count;
	QoSFlowID m_flow_id;
	QoSTrafficType m_traffic_type;
	uint8_t m_dscp_value;
};


QoSFlowImpl::QoSFlowImpl() :
	m_qos_controller(QoSController::GetInstance()), m_count(0), m_flow_id(0),
	m_traffic_type(QoSTrafficType::Illegal), m_dscp_value(0)
{
}

QoSFlowImpl::QoSFlowImpl(const uint8_t dscp_value)
	: QoSFlowImpl()
{
	m_dscp_value = dscp_value;
}

QoSFlowImpl::QoSFlowImpl(const QoSTrafficType traffic_type)
	: QoSFlowImpl()
{
	m_traffic_type = traffic_type;
}


QoSFlowImpl::~QoSFlowImpl()
{
	if (IsInitialised())
	{
		m_qos_controller->RemoveSocket(m_flow_id, static_cast<SOCKET>(NULL));
		m_count = 0;
		m_flow_id = 0;
		m_dscp_value = 0;
		m_traffic_type = QoSTrafficType::BestEffort;
	}
}

bool QoSFlowImpl::IsInitialised()
{
	std::lock_guard<std::mutex> lock(m_lock);
	if (m_flow_id != 0)
		return true;
	return false;
}

bool QoSFlowImpl::AddSocket(const uintptr_t socket, const void *dst_sockaddr)
{
	std::lock_guard<std::mutex> lock(m_lock);
	bool res;
	QoSTrafficType traffic_type = m_traffic_type;

	if (socket == 0)
	{
		return false;
	}

	if (m_dscp_value != 0)
	{
		traffic_type = QoSTrafficType::BestEffort;
	}

	res = m_qos_controller->AddSocket(m_flow_id, socket, traffic_type, const_cast<void *>(reinterpret_cast<const void*>(dst_sockaddr)));
	if (res == false || m_flow_id == 0)
		return false;

	m_count++;

	if (m_dscp_value != 0)
	{
		if (!m_qos_controller->SetDSCPValue(m_flow_id, m_dscp_value))
		{
			RemoveSocketInternal(socket);
			return false;
		}
	}

	return true;
}

bool QoSFlowImpl::RemoveSocket(const uintptr_t socket)
{
	std::lock_guard<std::mutex> lock(m_lock);
	return RemoveSocketInternal(socket);
}

bool QoSFlowImpl::RemoveSocketInternal(const uintptr_t socket)
{
	bool res;

	if (socket == 0)
	{
		return false;
	}

	res = m_qos_controller->RemoveSocket(m_flow_id, socket);
	if (res && socket == 0)
	{
		m_count = 0;
		m_flow_id = 0;
		return true;
	}

	if (res)
	{
		m_count--;
		if (m_count == 0)
		{
			//m_qos_controller->RemoveSocket(m_flow_id, 0);
			m_flow_id = 0;
		}
	}

	return res;
}

bool QoSFlowImpl::SetDSCPValue(const uint8_t value)
{
	std::lock_guard<std::mutex> lock(m_lock);
	if (m_count > 0)
	{
		return false;
	}

	bool res = true;
	if (m_flow_id != 0) // IsInitialised()
	{
		res = m_qos_controller->SetDSCPValue(m_flow_id, value);
	}

	if (res)
	{
		m_dscp_value = value;
		//m_traffic_type = QoSTrafficType::Illegal;
	}

	return res;
}

bool QoSFlowImpl::SetTrafficType(const QoSTrafficType traffic_type)
{
	std::lock_guard<std::mutex> lock(m_lock);
	if (m_count > 0)
	{
		return false;
	}

	bool res = true;
	if (traffic_type == QoSTrafficType::Illegal)
		return false;

	if (m_flow_id != 0) //IsInitialised()
	{
		res = m_qos_controller->SetTrafficType(m_flow_id, traffic_type);
	}

	if (res)
	{
		m_traffic_type = traffic_type;
		m_dscp_value = 0;
	}

	return res;
}

uint8_t QoSFlowImpl::GetDSCPValue(void)
{
	std::lock_guard<std::mutex> lock(m_lock);
	return m_dscp_value;
}

QoSTrafficType QoSFlowImpl::GetTrafficType(void)
{
	std::lock_guard<std::mutex> lock(m_lock);
	return m_traffic_type;
}

}

#else // implementation for UNIX-like systems.

#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include <assert.h>

namespace net {

class QoSFlowImpl
{
	friend QoSFlow;
public:
	QoSFlowImpl();
	QoSFlowImpl(const uint8_t dscp_value);
	QoSFlowImpl(const QoSTrafficType traffic_type);

	~QoSFlowImpl();

	inline bool IsInitialised(void);

	inline bool AddSocket(const int socket, const void *dst_sockaddr = nullptr); // Windows specific
	inline bool RemoveSocket(const int socket);

	inline bool SetDSCPValue(const uint8_t value);
	inline bool SetTrafficType(const QoSTrafficType traffic_type);

	inline uint8_t GetDSCPValue(void);
	inline QoSTrafficType GetTrafficType(void);
private:
	static inline int TrafficTypeToDSCP(const QoSTrafficType traffic_type);

	inline bool SetTOS(const int socket, const uint8_t value);
private:
	std::mutex m_lock;

	QoSTrafficType m_traffic_type;
	uint8_t m_dscp_value;
	size_t m_count;
	bool m_is_ipv6;
};


QoSFlowImpl::QoSFlowImpl() :
	m_traffic_type(QoSTrafficType::Illegal), m_dscp_value(0), m_count(0), m_is_ipv6(false)
{
}

QoSFlowImpl::QoSFlowImpl(const uint8_t dscp_value)
	: QoSFlowImpl()
{
	m_dscp_value = dscp_value;
}

QoSFlowImpl::QoSFlowImpl(const QoSTrafficType traffic_type)
	: QoSFlowImpl()
{
	m_traffic_type = traffic_type;
}


QoSFlowImpl::~QoSFlowImpl()
{
	assert(m_count == 0);
}

bool QoSFlowImpl::IsInitialised()
{
	return true;
}

bool QoSFlowImpl::AddSocket(const int socket, const void *dst_sockaddr)
{
	std::lock_guard<std::mutex> lock(m_lock);
	sockaddr addr;
	if (socket == 0)
	{
		return false;
	}

	socklen_t len = static_cast<socklen_t>(sizeof(addr));
	if (getsockname(socket, &addr, &len) != 0)
	{
		return false;
	}

	switch (addr.sa_family)
	{
		case AF_INET:
		{
			const sockaddr_in *in = reinterpret_cast<const sockaddr_in *>(dst_sockaddr);
			if (m_count == 0)
			{
				m_is_ipv6 = false;
			}
			else if (m_is_ipv6)
			{
				return false;
			}

			if (in)
			{
				if (in->sin_family != addr.sa_family)
					return false;
			}
		} break;
		case AF_INET6:
		{
			const sockaddr_in6 *in = reinterpret_cast<const sockaddr_in6 *>(dst_sockaddr);
			if (m_count == 0)
			{
				m_is_ipv6 = true;
			}
			else if (!m_is_ipv6)
			{
				return false;
			}

			if (in)
			{
				if (in->sin6_family != addr.sa_family)
					return false;
			}
		} break;
		default:
			return false;
			break;
	}

	uint8_t dscp = m_dscp_value;
	if (dscp == 0)
	{
		int traffic_dscp_val = TrafficTypeToDSCP(m_traffic_type);
		if (traffic_dscp_val < 0)
			return false;

		dscp = static_cast<uint8_t>(traffic_dscp_val);
	}

	auto res = SetTOS(socket, dscp);
	if (res)
	{
		m_count++;
	}

	return res;
}

bool QoSFlowImpl::RemoveSocket(const int socket)
{
	std::lock_guard<std::mutex> lock(m_lock);
	sockaddr addr;
	if (socket == 0 || m_count == 0)
	{
		return false;
	}

	socklen_t len = static_cast<socklen_t>(sizeof(addr));
	if (getsockname(socket, &addr, &len) != 0)
	{
		return false;
	}

	switch (addr.sa_family)
	{
		case AF_INET:
			if (m_is_ipv6)
			{
				return false;
			}
			break;
		case AF_INET6:
			if (!m_is_ipv6)
			{
				return false;
			}
			break;
		default:
			return false;
			break;
	}

	bool res = SetTOS(socket, 0);
	if (res)
	{
		m_count--;
	}

	return res;
}

bool QoSFlowImpl::SetDSCPValue(const uint8_t value)
{
	std::lock_guard<std::mutex> lock(m_lock);
	if (m_count > 0)
	{
		return false;
	}
	m_dscp_value = value;
	return true;
}

bool QoSFlowImpl::SetTrafficType(const QoSTrafficType traffic_type)
{
	std::lock_guard<std::mutex> lock(m_lock);
	if (m_count > 0)
	{
		return false;
	}
	m_traffic_type = traffic_type;
	return true;
}

uint8_t QoSFlowImpl::GetDSCPValue(void)
{
	std::lock_guard<std::mutex> lock(m_lock);
	return m_dscp_value;
}

QoSTrafficType QoSFlowImpl::GetTrafficType(void)
{
	std::lock_guard<std::mutex> lock(m_lock);
	return m_traffic_type;
}

bool QoSFlowImpl::SetTOS(const int socket, const uint8_t dscp_value)
{
	int level;
	if (m_is_ipv6)
	{
		level = IPPROTO_IPV6;
	}
	else
	{
		level = IPPROTO_IP;
	}

	unsigned int tos = dscp_value;
	return setsockopt(socket, level, IP_TOS, &tos, sizeof(tos)) == 0;
}


int QoSFlowImpl::TrafficTypeToDSCP(const QoSTrafficType traffic_type)
{

	switch (traffic_type)
	{
		case QoSTrafficType::Background:
			return 0x08;
			break;
		case QoSTrafficType::ExcellentEffort:
			return 0x28;
			break;
		case QoSTrafficType::AudioVideo:
			return 0x28;
			break;
		case QoSTrafficType::Voice:
			return 0x38;
			break;
		case QoSTrafficType::Control:
			return 0x38;
			break;
		case QoSTrafficType::BestEffort:
			return 0;
			break;
		default:
			return -1;
			break;
	}
	return -1;
}

}

#endif

// !!! QoSFlow starts here !!!
namespace net {
QoSFlow::QoSFlow() :
	imp(std::make_unique<QoSFlowImpl>())
{
}

QoSFlow::QoSFlow(const uint8_t dscp_value) :
	imp(std::make_unique<QoSFlowImpl>(dscp_value))
{
}

QoSFlow::QoSFlow(const QoSTrafficType traffic_type) :
	imp(std::make_unique<QoSFlowImpl>(traffic_type))
{
}

QoSFlowSharedPtr QoSFlow::Make()
{
	std::shared_ptr<QoSFlow> new_ptr(new QoSFlow());
	return new_ptr;
}

QoSFlowSharedPtr QoSFlow::Make(const uint8_t dscp_value)
{
	std::shared_ptr<QoSFlow> new_ptr(new QoSFlow(dscp_value));
	return new_ptr;
}

QoSFlowSharedPtr QoSFlow::Make(const QoSTrafficType traffic_type)
{
	std::shared_ptr<QoSFlow> new_ptr(new QoSFlow(traffic_type));
	return new_ptr;
}

bool QoSFlow::IsInitialised()
{
	return imp->IsInitialised();
}

bool QoSFlow::AddSocket(const qos_socket_t socket, const void *dst_sockaddr)
{
	return imp->AddSocket(socket, dst_sockaddr);
}

bool QoSFlow::RemoveSocket(const qos_socket_t socket)
{
	return imp->RemoveSocket(socket);
}

bool QoSFlow::SetDSCPValue(const uint8_t value)
{
	return imp->SetDSCPValue(value);
}

bool QoSFlow::SetTrafficType(const QoSTrafficType traffic_type)
{
	return imp->SetTrafficType(traffic_type);
}

uint8_t QoSFlow::GetDSCPValue(void)
{
	return imp->GetDSCPValue();
}

QoSTrafficType QoSFlow::GetTrafficType(void)
{
	return imp->GetTrafficType();
}

// !!! Check if QoS is available

bool IsQoSAvailable(void)
{
#ifdef _WIN32
	// Try to check if QoSController is initialized (and qwave.dll is loaded on Windows).
	return QoSController::GetInstance()->IsInitialized();
#elif defined (__linux__)
	// Setting IP_TOS socket option works for everybody on Linux.
	return true;
#else
	return true; // Let's assume than on most of the systems we do not need any special priveleges for QoS.
#endif
}

}
