#pragma once
#include <fstream>
#include <mutex>
#include <atomic>
#include <string>
#include "LoggerInterface.h"

namespace net {
	class PcapLogger : public LoggerInterface
	{
		uint64_t      m_max_file_size;
		std::string   m_initial_file_name;
		std::string   m_old_file_name;
		std::ofstream m_initial_file;
		std::ofstream m_old_file;
		std::mutex    m_mtx;

		std::atomic<uint64_t> m_last_connection_id;
		uint64_t m_bytes_written_to_initial = 0;

		std::atomic<uint64_t> m_last_id_before_maxsize_reached;
	public:
		explicit PcapLogger(uint64_t max_file_size, std::string initial_file_name , std::string old_file_name);
		void Log(const void* data, size_t size, const net::address& src_addr, net::port src_port, const net::address& dst_addr, net::port dst_port,
			ConnectionInfo& conn_info, net::protocol proto, bool from_us = false) override;
	private:
		void WriteToFile(const void* data, size_t size, bool write_to_initial_file);
		void WriteGlobalHeaderToInitialFile();
		bool OnMaxFileSizeReached();

		void WritePacketHeader(size_t payload_size, bool write_to_initial_file);

		template <class IPHeader>
		void WriteIpHeader(const net::address& src_addr, const net::address& dst_addr, size_t payload_size, uint8_t tr_protocol_number, bool write_to_initial_file, bool secure = false);

		template <class TransportHeader>
		void WriteTransportHeader(net::port src_port, net::port dst_port, size_t payload_size, ConnectionInfo& conn_info, bool write_to_initial_file, bool from_us = false);

		template <class TransportHeader>
		void LogPacket(const void* data, size_t size, const net::address& src_addr, net::port src_port,
			const net::address& dst_addr, net::port dst_port, ConnectionInfo& conn_info, bool write_to_initial_file, bool isIPv4, bool from_us = false, bool secure = false);
	};
} //namespace net
