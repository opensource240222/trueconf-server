#include "PcapLogger.h"
#include "std-generic/cpplib/hton.h"
#include "std/pcap.h"

namespace net {
	static bool FileExists(const std::string& file_name)
	{
		return std::ifstream{ file_name }.good();
	}
	static std::streamoff GetFileSize(const std::string& file_name)
	{
		return std::ifstream{ file_name, std::ifstream::ate | std::ifstream::binary }.tellg();
	}
	static void OpenFileForOutput(std::ofstream& fout, const std::string& file_name)
	{
		fout.open(file_name, std::fstream::binary | std::fstream::app);
	}
	PcapLogger::PcapLogger(uint64_t max_file_size, std::string initial_file_name, std::string old_file_name)
		:
		m_max_file_size(max_file_size),
		m_initial_file_name(std::move(initial_file_name)),
		m_old_file_name(std::move(old_file_name)),
		m_last_connection_id(0),
		m_last_id_before_maxsize_reached(0)
	{
		if (FileExists(m_initial_file_name))
		{
			m_bytes_written_to_initial = GetFileSize(m_initial_file_name);
			OpenFileForOutput(m_initial_file, m_initial_file_name);
		}
		else
		{
			OpenFileForOutput(m_initial_file, m_initial_file_name);
			WriteGlobalHeaderToInitialFile();
		}
	}

	void PcapLogger::Log(const void* data, size_t size, const net::address& src_addr, net::port src_port,
		const net::address& dst_addr, net::port dst_port, ConnectionInfo& conn_info, net::protocol proto, bool from_us)
	{
		bool write_to_initial_file = true;

		if (conn_info.m_id <= m_last_id_before_maxsize_reached.load(std::memory_order_relaxed))
		{
			if (conn_info.m_id != 0) // it is not a new connection, and its ID <= m_last_id_before_maxsize_reached, so we must write logs from it to gateway.old.pcap
				write_to_initial_file = false;
			else
				conn_info.m_id = m_last_connection_id.fetch_add(1, std::memory_order_relaxed) + 1; // it is a new connection, just give it ID
		}


		std::lock_guard<std::mutex> lock{ m_mtx };
		if (m_bytes_written_to_initial >= m_max_file_size)
		{
			if (!OnMaxFileSizeReached())
				return;
			m_last_id_before_maxsize_reached.store(m_last_connection_id, std::memory_order_relaxed);
			write_to_initial_file = false;
		}
		switch (proto)
		{
		case net::protocol::TCP:
			LogPacket<TCPHeader>(data, size, src_addr, src_port, dst_addr, dst_port, conn_info, src_addr.is_v4(), write_to_initial_file, from_us);
			break;
		case net::protocol::UDP:
			LogPacket<UDPHeader>(data, size, src_addr, src_port, dst_addr, dst_port, conn_info, src_addr.is_v4(), write_to_initial_file);
			break;

		case net::protocol::TLS:
			LogPacket< TCPHeader>(data, size, src_addr, src_port, dst_addr, dst_port, conn_info, src_addr.is_v4(), write_to_initial_file, from_us, true);
			break;
		default:
			assert(false);
		}
		WriteToFile(data, size, write_to_initial_file);
		if (write_to_initial_file)
			m_initial_file.flush();
		else
			m_old_file.flush();
	}

	void PcapLogger::WriteToFile(const void* data, size_t size, bool write_to_initial_file)
	{
		if (write_to_initial_file)
		{
			m_initial_file.write(static_cast<const char*>(data), size);
			m_bytes_written_to_initial += size;
		}
		else
			m_old_file.write(static_cast<const char*>(data), size);
	}

	void PcapLogger::WriteGlobalHeaderToInitialFile()
	{
		PcapGlobalHeader hdr;
		WriteToFile(&hdr, sizeof(hdr), true);
	}

	bool PcapLogger::OnMaxFileSizeReached()
	{
		m_initial_file.close();
		m_old_file.close();
		if (FileExists(m_old_file_name))
			if (remove(m_old_file_name.c_str()))
				return false;
		if (FileExists(m_initial_file_name))
			if (rename(m_initial_file_name.c_str(), m_old_file_name.c_str()))
				return false;
		OpenFileForOutput(m_initial_file, m_initial_file_name);
		OpenFileForOutput(m_old_file, m_old_file_name);
		WriteGlobalHeaderToInitialFile();
		m_bytes_written_to_initial = 0;
		return true;
	}

	template <>
	void PcapLogger::WriteIpHeader<IPv4Header>(const net::address& src_addr, const net::address& dst_addr, size_t payload_size, uint8_t tr_protocol_number, bool write_to_initial_file, bool secure)
	{
		IPv4Header ip_hdr;
		ip_hdr.total_length = vs_htons(static_cast<uint16_t>(sizeof(IPv4Header) + payload_size));
		ip_hdr.protocol = tr_protocol_number;
		ip_hdr.source_address = vs_htonl(src_addr.to_v4().to_ulong());
		ip_hdr.destination_address = vs_htonl(dst_addr.to_v4().to_ulong());
		if (secure)
			ip_hdr.ttl += 1; //All TLS packets have hop_limit = 43
		WriteToFile(&ip_hdr, sizeof(ip_hdr), write_to_initial_file);
	}

	void PcapLogger::WritePacketHeader(size_t payload_size, bool write_to_initial_file)
	{
		const auto now = std::chrono::system_clock::now();
		// Write pcap packet header
		PcapPacketHeader pcap_hdr;
		pcap_hdr.ts_sec = static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count());
		pcap_hdr.ts_usec = static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch() - std::chrono::seconds(pcap_hdr.ts_sec)).count());
		pcap_hdr.orig_len = payload_size;
		pcap_hdr.incl_len = std::min<uint32_t>(pcap_hdr.orig_len, 0xffff);
		WriteToFile(&pcap_hdr, sizeof(pcap_hdr), write_to_initial_file);
	}

	template <>
	void PcapLogger::WriteIpHeader<IPv6Header>(const net::address& src_addr, const net::address& dst_addr, size_t payload_size, uint8_t tr_protocol_number, bool write_to_initial_file, bool secure)
	{
		IPv6Header ip_hdr;
		ip_hdr.total_length = vs_htons(static_cast<uint16_t>(sizeof(IPv6Header) + payload_size));
		ip_hdr.protocol = tr_protocol_number;
		auto addr_src = src_addr.to_v6().to_bytes();
		auto addr_dst = dst_addr.to_v6().to_bytes();
		std::copy(addr_src.data(), addr_src.data() + addr_src.size(), reinterpret_cast<unsigned char*>(&ip_hdr.source_address));
		std::copy(addr_dst.data(), addr_dst.data() + addr_dst.size(), reinterpret_cast<unsigned char*>(&ip_hdr.destination_address));
		if (secure)
			ip_hdr.hop_limit += 1; //All TLS packets have hop_limit = 43
		WriteToFile(&ip_hdr, sizeof(ip_hdr), write_to_initial_file);
	}

	template <>
	void PcapLogger::WriteTransportHeader<TCPHeader>(net::port src_port, net::port dst_port, size_t payload_size, ConnectionInfo& conn_info, bool write_to_initial_file, bool from_us)
	{
		TCPHeader transport_hdr;
		transport_hdr.source_port = vs_htons(src_port);
		transport_hdr.destination_port = vs_htons(dst_port);
		if (from_us)
		{
			transport_hdr.sequence_number = vs_htonl(conn_info.m_local_to_remote_seqNo); // To force Wireshark to not treat the packet as TCP retransmission
			conn_info.m_local_to_remote_seqNo += payload_size;
		}
		else
		{
			transport_hdr.sequence_number = vs_htonl(conn_info.m_remote_to_local_seqNo); // To force Wireshark to not treat the packet as TCP retransmission
			conn_info.m_remote_to_local_seqNo += payload_size;
		}
		WriteToFile(&transport_hdr, sizeof(transport_hdr), write_to_initial_file);
	}

	template <>
	void PcapLogger::WriteTransportHeader<UDPHeader>(net::port src_port, net::port dst_port, size_t payload_size, ConnectionInfo& conn_info, bool write_to_initial_file, bool from_us)
	{
		UDPHeader transport_hdr;
		transport_hdr.source_port = vs_htons(src_port);
		transport_hdr.destination_port = vs_htons(dst_port);
		transport_hdr.length = vs_htons(static_cast<uint16_t>(sizeof(UDPHeader) + payload_size));
		WriteToFile(&transport_hdr, sizeof(transport_hdr), write_to_initial_file);
	}

	template <class TransportHeader>
	void PcapLogger::LogPacket(const void* data, size_t size, const net::address& src_addr, net::port src_port,
		const net::address& dst_addr, net::port dst_port, ConnectionInfo& conn_info, bool isIPv4, bool write_to_initial_file, bool from_us, bool secure)
	{
		if (isIPv4)
		{
			WritePacketHeader(sizeof(IPv4Header) + sizeof(TransportHeader) + size, write_to_initial_file);
			WriteIpHeader<IPv4Header>(src_addr, dst_addr, sizeof(TransportHeader) + size, TransportHeader::protocol_number, write_to_initial_file, secure);
		}
		else
		{
			WritePacketHeader(sizeof(IPv6Header) + sizeof(TransportHeader) + size, write_to_initial_file);
			WriteIpHeader<IPv6Header>(src_addr, dst_addr, sizeof(TransportHeader) + size, TransportHeader::protocol_number, write_to_initial_file, secure);
		}
		WriteTransportHeader<TransportHeader>(src_port, dst_port, size, conn_info, write_to_initial_file, from_us);
	}
} //namespace net
