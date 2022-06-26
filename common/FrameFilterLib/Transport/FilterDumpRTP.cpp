#include "FrameFilterLib/Transport/FilterDumpRTP.h"

#if FFL_DUMP_RTP

#include "std-generic/cpplib/hton.h"

#include <chrono>
#include <sstream>

#include "std/pcap.h"
namespace ffl {

std::atomic<unsigned> FilterDumpRTP::s_cnt { 0 };

std::shared_ptr<AbstractSource> FilterDumpRTP::Create(const std::shared_ptr<AbstractSource>& src, string_view name)
{
	if (!src)
		return nullptr;
	return src->RegisterSinkOrGetCompatible(std::make_shared<FilterDumpRTP>(name));
}

FilterDumpRTP::FilterDumpRTP(string_view name)
	: m_name(name)
	, m_id(s_cnt.fetch_add(1, std::memory_order_relaxed))
{
	SetName("dump rtp");
}

auto FilterDumpRTP::ProcessFrame(const std::shared_ptr<AbstractSource>& /*src*/, vs::SharedBuffer& buffer, FrameMetadata& /*md*/) -> e_processingResult
{
	if (GetFormat().type == FilterFormat::e_rtp)
	{
		if (!m_file.is_open())
		{
			std::ostringstream fn;
			fn << FFL_DUMPDIR << "/chain" << ChainID() << '-' << m_name << '-' << m_id << ".pcap";
			m_file.open(fn.str().c_str(), std::ios::binary | std::ios::out);

			// Write pcap global header
			PcapGlobalHeader hdr;
			m_file.write(reinterpret_cast<char*>(&hdr), sizeof(hdr));
		}
		if (m_file)
		{
			const auto now = std::chrono::system_clock::now();

			// Write pcap packet header
			PcapPacketHeader pcap_hdr;
			pcap_hdr.ts_sec = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
			pcap_hdr.ts_usec = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch() - std::chrono::seconds(pcap_hdr.ts_sec)).count();
			pcap_hdr.orig_len = sizeof(IPv4Header) + sizeof(UDPHeader) + buffer.size();
			pcap_hdr.incl_len = std::min<uint32_t>(pcap_hdr.orig_len, 0xffff);
			m_file.write(reinterpret_cast<char*>(&pcap_hdr), sizeof(pcap_hdr));

			// Write fake IPv4 header
			IPv4Header ipv4_hdr;
			ipv4_hdr.total_length = vs_htons(sizeof(IPv4Header) + sizeof(UDPHeader) + buffer.size());
			m_file.write(reinterpret_cast<char*>(&ipv4_hdr), sizeof(ipv4_hdr));

			// Write fake UDP header
			UDPHeader udp_hdr;
			udp_hdr.length = vs_htons(sizeof(UDPHeader) + buffer.size());
			m_file.write(reinterpret_cast<char*>(&udp_hdr), sizeof(udp_hdr));

			// Write RTP data
			m_file.write(buffer.data<const char>(), buffer.size());
		}
	}

	return e_lastBuffer;
}

bool FilterDumpRTP::IsCompatibleWith(const AbstractSink* sink)
{
	auto p = dynamic_cast<const FilterDumpRTP*>(sink);
	if (!p)
		return false;
	return true;
}

}

#endif
