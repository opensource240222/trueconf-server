#pragma once

#include <mutex>
#include <memory>
#include <vector>

#include <cstdint>

namespace net {

	enum class QoSTrafficType {
		Illegal = -1,

		BestEffort = 0,
		Background,
		ExcellentEffort,
		AudioVideo,
		Voice,
		Control
	};


	class QoSFlow;
	class QoSFlowImpl;

	typedef std::shared_ptr<QoSFlow> QoSFlowSharedPtr;
	typedef std::weak_ptr<QoSFlow> QoSFlowWeakPtr;

	class QoSFlow
	{
		public:
#ifdef _WIN32
			typedef uintptr_t qos_socket_t;
#else
			typedef int qos_socket_t;
#endif
		public:
			static QoSFlowSharedPtr Make();
			static QoSFlowSharedPtr Make(const uint8_t dscp_value);
			static QoSFlowSharedPtr Make(const QoSTrafficType traffic_type);

			bool IsInitialised(void);

			bool AddSocket(const qos_socket_t socket, const void *dst_sockaddr = nullptr); // Windows specific
			bool RemoveSocket(const qos_socket_t socket);

			bool SetDSCPValue(const uint8_t value);
			bool SetTrafficType(const QoSTrafficType traffic_type);

			uint8_t GetDSCPValue(void);
			QoSTrafficType GetTrafficType(void);
		private:
			QoSFlow();
			QoSFlow(const uint8_t dscp_value);
			QoSFlow(const QoSTrafficType traffic_type);

			QoSFlow(const QoSFlow &) = delete;
			QoSFlow &operator=(const QoSFlow &) = delete;
		private:
			std::unique_ptr<QoSFlowImpl> imp;
	};

	extern bool IsQoSAvailable(void);

}

