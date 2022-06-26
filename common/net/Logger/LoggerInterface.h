#pragma once
#include <cstdint>
#include <boost/asio/ip/basic_endpoint.hpp>
#include "net/Address.h"
#include "net/Port.h"
#include "net/Protocol.h"

namespace net {
	class LoggerInterface
	{
	public:
		struct ConnectionInfo
		{
			uint64_t m_id = 0;
			uint32_t m_local_to_remote_seqNo = 0; // Only for TCP
			uint32_t m_remote_to_local_seqNo = 0; // Only for TCP
		};
		virtual ~LoggerInterface() = default;

		template <class Protocol>
		void Log(const void* data, size_t size, const boost::asio::ip::basic_endpoint<Protocol>& src, const boost::asio::ip::basic_endpoint<Protocol>& dst, ConnectionInfo& conn_info, net::protocol proto, bool from_us = false)
		{
			Log(data, size, src.address(), src.port(), dst.address(), dst.port(), conn_info, proto, from_us);
		}
		virtual void Log(const void* data, size_t size, const net::address& src_addr, net::port src_port, const net::address& dst_addr, net::port dst_port,
			ConnectionInfo& conn_info, net::protocol proto, bool from_us = false) = 0; // We need last param only for TCP for keeping right SeqNo

	};
} // namespace net
