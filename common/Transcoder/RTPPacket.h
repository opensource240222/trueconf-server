/****************************************************************************
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 *
 * Project: TransCoder
 *
 * $History: RTPPacket.h $
 *
 * *****************  Version 2  *****************
 * User: Ktrushnikov  Date: 27.03.12   Time: 16:10
 * Updated in $/VSNA/Transcoder
 * Correct SeqNo:
 * - new queue for RTP UDP packets
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/Transcoder
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/Transcoder
 *
 * *****************  Version 10  *****************
 * User: Smirnov      Date: 14.12.05   Time: 17:42
 * Updated in $/VS/Transcoder
 * - added g722 audiocodec supporting
 *
 * *****************  Version 9  *****************
 * User: Smirnov      Date: 16.09.05   Time: 12:04
 * Updated in $/VS/Transcoder
 * - added g7221_24 support in gateway
 * - added h.263+ RTP support
 *
 * *****************  Version 8  *****************
 * User: Smirnov      Date: 18.08.05   Time: 20:34
 * Updated in $/VS/Transcoder
 * - added support for g729a
 *
 * *****************  Version 7  *****************
 * User: Smirnov      Date: 10.06.05   Time: 19:27
 * Updated in $/VS/Transcoder
 *  - new base rtp buffer
 *  - g728 codec are embeded
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 14.05.04   Time: 19:07
 * Updated in $/VS/Transcoder
 * added RTP H263 buffers
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 12.05.04   Time: 19:07
 * Updated in $/VS/Transcoder
 * added form of RTP header
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 7.05.04    Time: 18:51
 * Updated in $/VS/transcoder
 * debugged
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 20.04.04   Time: 13:30
 * Updated in $/VS/Transcoder
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 16.04.04   Time: 18:17
 * Updated in $/VS/Transcoder
 * Codec are wave datf Aligned
 *
 ****************************************************************************/

/****************************************************************************
 * \file RTPPacket.h
 * \brief RTP Paket definition
 ****************************************************************************/
#pragma once

/****************************************************************************
* Includes
****************************************************************************/
#include "std-generic/cpplib/hton.h"

#include "std-generic/compat/memory.h"
#include <cstdlib>
#include <cstring>

/**
**************************************************************************
Payload types enum.
See RFC 3551:

 PT   encoding    media type  clock rate   channels
      name                    (Hz)
 ___________________________________________________
 0    PCMU        A            8,000       1
 1    reserved    A
 2    reserved    A
 3    GSM         A            8,000       1
 4    G723        A            8,000       1
 5    DVI4        A            8,000       1
 6    DVI4        A           16,000       1
 7    LPC         A            8,000       1
 8    PCMA        A            8,000       1
 9    G722        A            8,000       1
 10   L16         A           44,100       2
 11   L16         A           44,100       1
 12   QCELP       A            8,000       1
 13   CN          A            8,000       1
 14   MPA         A           90,000       (see text)
 15   G728        A            8,000       1
 16   DVI4        A           11,025       1
 17   DVI4        A           22,050       1
 18   G729        A            8,000       1
 19   reserved    A
 20   unassigned  A
 21   unassigned  A
 22   unassigned  A
 23   unassigned  A
 dyn  G726-40     A            8,000       1
 dyn  G726-32     A            8,000       1
 dyn  G726-24     A            8,000       1
 dyn  G726-16     A            8,000       1
 dyn  G729D       A            8,000       1
 dyn  G729E       A            8,000       1
 dyn  GSM-EFR     A            8,000       1
 dyn  L8          A            var.        var.
 dyn  RED         A                        (see text)
 dyn  VDVI        A            var.        1

 Table 4: Payload types (PT) for audio encodings

 PT      encoding    media type  clock rate
         name                    (Hz)
 _____________________________________________
 24      unassigned  V
 25      CelB        V           90,000
 26      JPEG        V           90,000
 27      unassigned  V
 28      nv          V           90,000
 29      unassigned  V
 30      unassigned  V
 31      H261        V           90,000
 32      MPV         V           90,000
 33      MP2T        AV          90,000
 34      H263        V           90,000
 35-71   unassigned  ?
 72-76   reserved    N/A         N/A
 77-95   unassigned  ?
 96-127  dynamic     ?
 dyn     H263-1998   V           90,000

 Table 5: Payload types (PT) for video and combined
          encodings

****************************************************************************/
enum Rtp_PayloadType {
	RTP_PT_AUTO		= -1,
	RTP_PT_G711_ULAW= 0,
	RTP_PT_G711_ALAW= 8,
	RTP_PT_G723		= 4,
	RTP_PT_G722		= 9,
	RTP_PT_SID		= 13,
	RTP_PT_G728		= 15,
	RTP_PT_G729A	= 18,
	RTP_PT_H261		= 31,
	RTP_PT_H263		= 34,
	RTP_PT_DYN_AUDIO= 99,
	RTP_PT_DYN_VIDEO= 109
};

/****************************************************************************
 * RTPPacket class
 ****************************************************************************/
class RTPPacket
{
	uint8_t m_version;
	uint8_t m_contributor_count;
	bool m_marker;
	uint8_t m_payload_type;
	uint16_t m_seq_no;
	uint32_t m_timestamp;
	uint32_t m_ssrc;
	bool m_padding = false;

	size_t m_header_size; // This field is also used as packet validity indicator, structurally invalid packets will have it equal to 0.
	size_t m_data_size;
	const uint8_t* m_data_ptr;
	std::unique_ptr<uint8_t[]> m_data_copy;

	static const size_t c_fixed_header_size = 12;

public:
	RTPPacket()
		: m_version(2)
		, m_contributor_count(0)
		, m_marker(false)
		, m_payload_type(0)
		, m_seq_no(0)
		, m_timestamp(0)
		, m_ssrc(0)
		, m_header_size(c_fixed_header_size)
		, m_data_size(0)
		, m_data_ptr(nullptr)
	{
	}

	RTPPacket(const RTPPacket& x)
		: m_version(x.m_version)
		, m_contributor_count(x.m_contributor_count)
		, m_marker(x.m_marker)
		, m_payload_type(x.m_payload_type)
		, m_seq_no(x.m_seq_no)
		, m_timestamp(x.m_timestamp)
      , m_ssrc(x.m_ssrc)
		, m_padding(x.m_padding)
		, m_header_size(x.m_header_size)
		, m_data_size(x.m_data_size)
	{
		if (x.m_data_copy)
		{
			m_data_copy = vs::make_unique_default_init<uint8_t[]>(m_data_size);
			std::memcpy(m_data_copy.get(), x.m_data_copy.get(), m_data_size);
			m_data_ptr = m_data_copy.get();
		}
		else
			m_data_ptr = x.m_data_ptr;
	}

	RTPPacket(RTPPacket&& x) noexcept
		: m_version(x.m_version)
		, m_contributor_count(x.m_contributor_count)
		, m_marker(x.m_marker)
		, m_payload_type(x.m_payload_type)
		, m_seq_no(x.m_seq_no)
		, m_timestamp(x.m_timestamp)
      , m_ssrc(x.m_ssrc)
		, m_padding(x.m_padding)
		, m_header_size(x.m_header_size)
		, m_data_size(x.m_data_size)
		, m_data_ptr(x.m_data_ptr)
		, m_data_copy(std::move(x.m_data_copy))
	{
		x.m_data_size = 0;
		x.m_data_ptr = nullptr;
	}

	RTPPacket& operator=(const RTPPacket& x)
	{
		if (this == &x)
			return *this;

		m_version = x.m_version;
		m_contributor_count = x.m_contributor_count;
		m_marker = x.m_marker;
		m_payload_type = x.m_payload_type;
		m_seq_no = x.m_seq_no;
		m_timestamp = x.m_timestamp;
		m_padding = x.m_padding;
		m_ssrc = x.m_ssrc;
		m_header_size = x.m_header_size;
		m_data_size = x.m_data_size;

		if (x.m_data_copy)
		{
			auto tmp = vs::make_unique_default_init<uint8_t[]>(m_data_size);
			std::memcpy(tmp.get(), x.m_data_copy.get(), m_data_size);
			m_data_copy = std::move(tmp);
			m_data_ptr = m_data_copy.get();
		}
		else
		{
			m_data_copy.reset();
			m_data_ptr = x.m_data_ptr;
		}

		return *this;
	}

	RTPPacket& operator=(RTPPacket&& x) noexcept
	{
		if (this == &x)
			return *this;

		m_version = x.m_version;
		m_contributor_count = x.m_contributor_count;
		m_marker = x.m_marker;
		m_payload_type = x.m_payload_type;
		m_seq_no = x.m_seq_no;
		m_timestamp = x.m_timestamp;
		m_padding = x.m_padding;
		m_ssrc = x.m_ssrc;
		m_header_size = x.m_header_size;
		m_data_size = x.m_data_size;
		x.m_data_size = 0;
		m_data_ptr = x.m_data_ptr;
		x.m_data_ptr = nullptr;
		m_data_copy = std::move(x.m_data_copy);

		return *this;
	}

	RTPPacket(const void* buffer, size_t size, bool deep_copy = false, bool count_padding = true)
		: m_header_size(0)
		, m_data_size(0)
		, m_data_ptr(nullptr)
	{
		if (size < c_fixed_header_size)
			return;

		auto p = static_cast<const uint8_t*>(buffer);

		m_version           = (p[0] & 0xc0) >> 6;
		uint8_t padding     = (p[0] & 0x20) ? p[size - 1] : 0;
		bool extension      = (p[0] & 0x10) != 0;
		m_contributor_count =  p[0] & 0x0f;
		m_marker            = (p[1] & 0x80) != 0;
		m_payload_type      =  p[1] & 0x7f;
		m_seq_no            = vs_ntohs(*(reinterpret_cast<const uint16_t*>(p + 2)));
		m_timestamp         = vs_ntohl(*(reinterpret_cast<const uint32_t*>(p + 4)));
		m_ssrc              = vs_ntohl(*(reinterpret_cast<const uint32_t*>(p + 8)));
		m_padding			= (p[0] & 0x20)!=0;
		if (!count_padding) padding = 0;

		m_header_size = c_fixed_header_size + 4 * m_contributor_count;
		if (extension)
		{
			if (m_header_size + 4 /*RTP header extension fixed part*/ > size)
			{
				// incorrect rtp packet
				m_header_size = 0;
				return;
			}
			m_header_size += 4 + 4 * vs_ntohs(*(reinterpret_cast<const uint16_t*>(p + m_header_size + 2)));
		}
		if (m_header_size + padding > size)
		{
			// incorrect rtp packet
			m_header_size = 0;
			return;
		}
		m_data_size = size - m_header_size - padding;

		if (deep_copy)
		{
			m_data_copy = vs::make_unique_default_init<uint8_t[]>(m_data_size);
			std::memcpy(m_data_copy.get(), p + m_header_size, m_data_size);
			m_data_ptr = m_data_copy.get();
		}
		else
			m_data_ptr = p + m_header_size;
	}

	void Clean() {
		m_header_size = c_fixed_header_size;
		m_data_size = 0;
		m_data_ptr = nullptr;
		m_data_copy.reset();
	}

	bool IsValid() const {
		return (m_header_size >= c_fixed_header_size);
	}

	uint8_t Version() const { return m_version; }
	uint8_t ContributorCount() const { return m_contributor_count; }
	bool Marker() const { return m_marker; }
	void Marker(bool val) { m_marker = val; }
	uint8_t PayloadType() const { return m_payload_type; }
	void PayloadType(uint8_t val) { m_payload_type = val; }
	uint16_t SeqNo() const { return m_seq_no; }
	void SeqNo(uint16_t val) { m_seq_no = val; }
	uint32_t Timestamp() const { return m_timestamp; }
	void Timestamp(uint32_t val) { m_timestamp = val; }
	bool Padding() const { return m_padding; }
	void Padding(bool val) { m_padding = val; }
	uint32_t SSRC() const { return m_ssrc; }
	void SSRC(uint32_t val) { m_ssrc = val; }

	static uint16_t SeqNo(const void* packet, size_t size)
	{
		if (size < 12)
			return 0;
		return vs_htons(*reinterpret_cast<const uint16_t*>(static_cast<const uint8_t*>(packet) + 2));
	}
	static uint32_t Timestamp(const void* packet, size_t size)
	{
		if (size < 12)
			return 0;
		return vs_htonl(*reinterpret_cast<const uint32_t*>(static_cast<const uint8_t*>(packet) + 4));
	}
	static uint32_t SSRC(const void* packet, size_t size)
	{
		if (size < 12)
			return 0;
		return vs_htonl(*reinterpret_cast<const uint32_t*>(static_cast<const uint8_t*>(packet) + 8));
	}

	size_t HeaderSize() const { return m_header_size; }
	size_t DataSize() const { return m_data_size; }
	const uint8_t* Data() const { return m_data_ptr; }

	void SetData(const void* buffer, size_t size, bool deep_copy = false)
	{
		m_data_size = size;
		if (deep_copy)
		{
			auto tmp = vs::make_unique_default_init<uint8_t[]>(m_data_size);
			std::memcpy(tmp.get(), buffer, m_data_size);
			m_data_copy = std::move(tmp);
			m_data_ptr = m_data_copy.get();
		}
		else
		{
			m_data_copy.reset();
			m_data_ptr = static_cast<const uint8_t*>(buffer);
		}
	}

	// buffer is required to be at least 12 bytes long
	void GetHeader(void* buffer) const
	{
		// reset header
		memset(buffer, 0, c_fixed_header_size);

		auto p = static_cast<uint8_t*>(buffer);
		p[0] |= 0x80; // V = 2, P = 0, X = 0, CC = 0;
		m_padding ? p[0] |= 0x20 : p[0] &= 0xdf;
		p[1] |= m_marker ? 0x80: 0x00;
		p[1] |= m_payload_type & 0x7f;
		*reinterpret_cast<uint16_t*>(p + 2) = vs_htons(m_seq_no);
		*reinterpret_cast<uint32_t*>(p + 4) = vs_htonl(m_timestamp);
		*reinterpret_cast<uint32_t*>(p + 8) = vs_htonl(m_ssrc);
	}

	size_t GetPacket(void* buffer) const
	{
		if (buffer)
		{
			GetHeader(buffer);
			if (m_data_ptr)
				std::memcpy(static_cast<uint8_t*>(buffer) + c_fixed_header_size, m_data_ptr, m_data_size);
		}
		return c_fixed_header_size + m_data_size;
	}

	bool operator<(const RTPPacket& r) const
	{
		return m_seq_no < r.m_seq_no;
	}

	bool operator>(const RTPPacket& r) const
	{
		return m_seq_no > r.m_seq_no;
	}
};
