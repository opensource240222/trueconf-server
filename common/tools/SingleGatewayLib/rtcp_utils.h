#pragma once

#include "std-generic/cpplib/hton.h"

#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdlib>

namespace rtcp_utils {
	enum eRTCPPayload : uint8_t {
		RTPFB = 205,
		PSFB = 206,
	};

	struct FeedbackMsgHeader {
		uint8_t FMT : 5;
		uint8_t P : 1;
		uint8_t v : 2;
		uint8_t PT;
		uint16_t length;
		uint32_t ssrc_packet_sender;
		uint32_t ssrc_media_source;
	};

	static_assert(sizeof(FeedbackMsgHeader) == 12, "!");

	inline bool ParseFeedbackMsgHeader(const unsigned char *buf, size_t len, FeedbackMsgHeader &fb_hdr) {
		if (len < sizeof(FeedbackMsgHeader)) return false;

		fb_hdr = *(FeedbackMsgHeader *)buf;
		if (fb_hdr.v != 2) return false;
		fb_hdr.length = vs_ntohs(fb_hdr.length);
		fb_hdr.ssrc_packet_sender = vs_ntohl(fb_hdr.ssrc_packet_sender);
		fb_hdr.ssrc_media_source = vs_ntohl(fb_hdr.ssrc_media_source);

		return true;
	}

	inline size_t WriteFeedbackMsgHeader(const FeedbackMsgHeader &fb_hdr, unsigned char *buf, size_t len) {
		if (len < sizeof(FeedbackMsgHeader)) return 0;

		FeedbackMsgHeader *_fb_hdr = (FeedbackMsgHeader *)buf;
		*_fb_hdr = fb_hdr;
		_fb_hdr->length = vs_htons(_fb_hdr->length);
		_fb_hdr->ssrc_packet_sender = vs_htonl(_fb_hdr->ssrc_packet_sender);
		_fb_hdr->ssrc_media_source = vs_htonl(_fb_hdr->ssrc_media_source);

		return sizeof(FeedbackMsgHeader);
	}

	enum eMSI : uint32_t {
		SourceNone = 0xFFFFFFFF,
		SourceAny = 0xFFFFFFFE,
	};

	struct VSRHeader {
		uint16_t afb_type;
		uint16_t length;
		uint32_t msi;
		uint16_t request_id;
		uint16_t _reserve1 = 0;
		uint8_t version;
		uint8_t A : 1;
		uint8_t _reserve2 : 7;
		uint8_t B, C;
		uint32_t _reserve3 = 0;
	};

	static_assert(sizeof(VSRHeader) == 20, "!");

	inline bool ParseVSRHeader(const unsigned char *buf, size_t len, VSRHeader &vsr_hdr) {
		if (len < sizeof(VSRHeader)) return false;

		vsr_hdr = *(VSRHeader *)buf;
		vsr_hdr.afb_type = vs_ntohs(vsr_hdr.afb_type);
		if (vsr_hdr.afb_type != 0x01) return false;
		vsr_hdr.length = vs_ntohs(vsr_hdr.length);
		vsr_hdr.msi = vs_ntohl(vsr_hdr.msi);
		vsr_hdr.request_id = vs_ntohs(vsr_hdr.request_id);

		return true;
	}

	inline size_t WriteVSRHeader(const VSRHeader &vsr_hdr, unsigned char *buf, size_t len) {
		if (len < sizeof(VSRHeader)) return false;

		VSRHeader *_vsr_hdr = (VSRHeader *)buf;
		*_vsr_hdr = vsr_hdr;
		_vsr_hdr->afb_type = vs_htons(_vsr_hdr->afb_type);
		_vsr_hdr->length = vs_htons(_vsr_hdr->length);
		_vsr_hdr->msi = vs_htonl(_vsr_hdr->msi);
		_vsr_hdr->request_id = vs_htons(_vsr_hdr->request_id);

		return sizeof(VSRHeader);
	}

	enum eUConfig : uint8_t {
		UCConfigMode1 = 1,
	};

	enum eAspect : uint8_t {
		_4x3 = 0x1,
		_16x9 = 0x2,
		_1x1 = 0x4,
		_3x4 = 0x8,
		_9x16 = 0x16,
		_20x3 = 0x32,
	};

	enum eFramerate : uint32_t {
		_7_5 = 0x1,
		_12_5 = 0x2,
		_15 = 0x4,
		_25 = 0x8,
		_30 = 0x10,
		_50 = 0x20,
		_60 = 0x40,
	};

	struct VSREntry {
		uint8_t pt;
		uint8_t uc_config_mode;
		uint8_t flags;
		uint8_t aspect;
		uint16_t max_w;
		uint16_t max_h;
		uint32_t min_bitrate;
		uint32_t reserve = 0;
		uint32_t bitrate_per_level;
		uint16_t bitrate_histogram[10] = {};
		uint32_t framerate_bitmask;
		uint16_t num_must_instances;
		uint16_t num_may_instances;
		uint16_t quality_report_histogram[8] = {};
		uint32_t max_num_pixels;
	};

	static_assert(sizeof(VSREntry) == 68, "!");

	inline bool ParseVSREntry(const unsigned char *buf, size_t len, VSREntry &vsr_ent) {
		if (len < sizeof(VSREntry)) return false;

		vsr_ent = *(VSREntry *)buf;
		vsr_ent.max_w = vs_ntohs(vsr_ent.max_w);
		vsr_ent.max_h = vs_ntohs(vsr_ent.max_h);
		vsr_ent.min_bitrate = vs_ntohl(vsr_ent.min_bitrate);
		vsr_ent.bitrate_per_level = vs_ntohl(vsr_ent.bitrate_per_level);
		for (int i = 0; i < 10; i++) {
			vsr_ent.bitrate_histogram[i] = vs_ntohs(vsr_ent.bitrate_histogram[i]);
		}
		vsr_ent.framerate_bitmask = vs_ntohl(vsr_ent.framerate_bitmask);
		vsr_ent.num_must_instances = vs_ntohs(vsr_ent.num_must_instances);
		vsr_ent.num_may_instances = vs_ntohs(vsr_ent.num_may_instances);
		for (int i = 0; i < 8; i++) {
			vsr_ent.quality_report_histogram[i] = vs_ntohs(vsr_ent.quality_report_histogram[i]);
		}
		vsr_ent.max_num_pixels = vs_ntohl(vsr_ent.max_num_pixels);

		return true;
	}

	inline size_t WriteVSREntry(const VSREntry &vsr_ent, unsigned char *buf, size_t len) {
		if (len < sizeof(VSREntry)) return 0;

		VSREntry *_vsr_ent = (VSREntry *)buf;
		*_vsr_ent = vsr_ent;
		_vsr_ent->max_w = vs_ntohs(_vsr_ent->max_w);
		_vsr_ent->max_h = vs_ntohs(_vsr_ent->max_h);
		_vsr_ent->min_bitrate = vs_ntohl(_vsr_ent->min_bitrate);
		_vsr_ent->bitrate_per_level = vs_ntohl(_vsr_ent->bitrate_per_level);
		for (int i = 0; i < 10; i++) {
			_vsr_ent->bitrate_histogram[i] = vs_ntohs(_vsr_ent->bitrate_histogram[i]);
		}
		_vsr_ent->framerate_bitmask = vs_ntohl(_vsr_ent->framerate_bitmask);
		_vsr_ent->num_must_instances = vs_ntohs(_vsr_ent->num_must_instances);
		_vsr_ent->num_may_instances = vs_ntohs(_vsr_ent->num_may_instances);
		for (int i = 0; i < 8; i++) {
			_vsr_ent->quality_report_histogram[i] = vs_ntohs(_vsr_ent->quality_report_histogram[i]);
		}
		_vsr_ent->max_num_pixels = vs_ntohl(_vsr_ent->max_num_pixels);

		return sizeof(VSREntry);
	}

	inline size_t make_video_source_request(uint32_t ssrc_packet_sender, uint32_t ssrc_media_source, uint8_t pt, unsigned char *data, size_t len) {
		if (len < sizeof(FeedbackMsgHeader) + sizeof(VSRHeader) + sizeof(VSREntry)) return 0;

		static int counter = 0;
		counter++;

		FeedbackMsgHeader fm_hdr = {};
		fm_hdr.FMT = 15;
		fm_hdr.P = 0;
		fm_hdr.v = 2;
		fm_hdr.PT = PSFB;
		fm_hdr.ssrc_packet_sender = ssrc_packet_sender;
		fm_hdr.ssrc_media_source = SourceAny;
		fm_hdr.length = (sizeof(FeedbackMsgHeader) + sizeof(VSRHeader) + sizeof(VSREntry)) / 4 - 1;

		VSRHeader vsr_hdr = {};
		vsr_hdr.afb_type = 0x01;
		vsr_hdr.length = sizeof(VSRHeader) + sizeof(VSREntry);
		vsr_hdr.msi = SourceAny;
		vsr_hdr.request_id = counter;
		vsr_hdr.version = 0;
		vsr_hdr.A = 1;
		vsr_hdr.B = 1;
		vsr_hdr.C = 0x44;

		VSREntry vsr_ent = {};
		vsr_ent.pt = pt;
		vsr_ent.uc_config_mode = UCConfigMode1;
		vsr_ent.flags = 0;// 0x2 | 0x8;
		vsr_ent.aspect = _16x9;// _4x3 | _16x9 | _1x1 | _9x16 | _20x3;
		vsr_ent.max_w = 1280;
		vsr_ent.max_h = 1088;
		vsr_ent.min_bitrate = 2500000;
		vsr_ent.bitrate_per_level = 10000;
		for (int i = 0; i < 1; i++) {
			vsr_ent.bitrate_histogram[i] = 1;
		}
		vsr_ent.framerate_bitmask = _25 /*| _25 | _15 | _12_5 | _7_5*/;
		vsr_ent.num_must_instances = 1;
		vsr_ent.num_may_instances = 0;
		for (int i = 0; i < 1; i++) {
			vsr_ent.quality_report_histogram[i] = 1;
		}
		vsr_ent.max_num_pixels = 921600 * 4/*103680*16*4*10*/;

		size_t off = 0;
		off += WriteFeedbackMsgHeader(fm_hdr, &data[off], len - off);
		off += WriteVSRHeader(vsr_hdr, &data[off], len - off);
		off += WriteVSREntry(vsr_ent, &data[off], len - off);

		return off;
	}

	struct SenderReportHeader {
		uint8_t RC : 5;
		uint8_t P : 1;
		uint8_t v : 2;
		uint8_t PT = 200;
		uint16_t length;
		uint32_t ssrc;
		uint32_t timestamp_msw;
		uint32_t timestamp_lsw;
		uint32_t rtp_timestamp;
		uint32_t packet_count;
		uint32_t octet_count;
	};

	static_assert(sizeof(SenderReportHeader) == 28, "!");

	inline bool ParseSenderReportHeader(const unsigned char *buf, size_t len, SenderReportHeader &sr_hdr) {
		if (len < sizeof(SenderReportHeader)) return false;

		sr_hdr = *(SenderReportHeader *)buf;
		sr_hdr.length = vs_ntohs(sr_hdr.length);
		sr_hdr.ssrc = vs_ntohl(sr_hdr.ssrc);
		sr_hdr.timestamp_msw = vs_ntohl(sr_hdr.timestamp_msw);
		sr_hdr.timestamp_lsw = vs_ntohl(sr_hdr.timestamp_lsw);
		sr_hdr.rtp_timestamp = vs_ntohl(sr_hdr.rtp_timestamp);
		sr_hdr.packet_count = vs_ntohl(sr_hdr.packet_count);
		sr_hdr.octet_count = vs_ntohl(sr_hdr.octet_count);

		return true;
	}

	inline size_t WriteSenderReportHeader(const SenderReportHeader &sr_hdr, unsigned char *buf, size_t len) {
		if (len < sizeof(SenderReportHeader)) return 0;

		SenderReportHeader *_sr_hdr = (SenderReportHeader *)buf;
		*_sr_hdr = sr_hdr;
		_sr_hdr->length = vs_htons(_sr_hdr->length);
		_sr_hdr->ssrc = vs_htonl(_sr_hdr->ssrc);
		_sr_hdr->timestamp_msw = vs_htonl(_sr_hdr->timestamp_msw);
		_sr_hdr->timestamp_lsw = vs_htonl(_sr_hdr->timestamp_lsw);
		_sr_hdr->rtp_timestamp = vs_htonl(_sr_hdr->rtp_timestamp);
		_sr_hdr->packet_count = vs_htonl(_sr_hdr->packet_count);
		_sr_hdr->octet_count = vs_htonl(_sr_hdr->octet_count);

		return sizeof(SenderReportHeader);
	}

	struct PeerInfoExchange {
		uint16_t type;
		uint16_t length;
		uint32_t ssrc;
		uint32_t inbound_link_bandwidth;
		uint32_t outbound_link_bandwidth;
		uint8_t NC : 1;
		uint8_t _reserve1 : 7;
		uint8_t _reserve2 = 0;
		uint16_t _reserve3 = 0;
	};

	static_assert(sizeof(PeerInfoExchange) == 20, "!");

	inline bool ParsePeerInfoExchange(const unsigned char *buf, size_t len, PeerInfoExchange &info) {
		if (len < sizeof(PeerInfoExchange)) return false;

		info = *(PeerInfoExchange *)buf;
		info.type = vs_ntohs(info.type);
		if (info.type != 0x000C) return false;
		info.length = vs_ntohs(info.length);
		info.ssrc = vs_ntohl(info.ssrc);
		info.inbound_link_bandwidth = vs_ntohl(info.inbound_link_bandwidth);
		info.outbound_link_bandwidth = vs_ntohl(info.outbound_link_bandwidth);

		return true;
	}

	inline size_t WritePeerInfoExchange(const PeerInfoExchange &info, unsigned char *buf, size_t len) {
		if (len && len < sizeof(PeerInfoExchange)) return 0;

		PeerInfoExchange *_info = (PeerInfoExchange *)buf;
		*_info = info;
		_info->type = vs_htons(info.type);
		_info->length = vs_htons(info.length);
		_info->ssrc = vs_htonl(info.ssrc);
		_info->inbound_link_bandwidth = vs_htonl(info.inbound_link_bandwidth);
		_info->outbound_link_bandwidth = vs_htonl(info.outbound_link_bandwidth);

		return sizeof(PeerInfoExchange);
	}

	using ntp_fractions = std::chrono::duration<uint32_t, std::ratio<1, 0x100000000>>;
	inline void ntp_time(uint32_t& seconds, uint32_t& fractions)
	{
		const auto time_since_ntp_epoch = std::chrono::system_clock::now().time_since_epoch() + std::chrono::seconds(2208988800);
		auto sec = std::chrono::duration_cast<std::chrono::seconds>(time_since_ntp_epoch);
		seconds = sec.count();
		fractions = std::chrono::duration_cast<ntp_fractions>(time_since_ntp_epoch - sec).count();
	}
	inline uint64_t ntp_time()
	{
		uint32_t seconds;
		uint32_t fractions;
		ntp_time(seconds, fractions);
		return (static_cast<uint64_t>(seconds) << 32) | fractions;
	}

	inline size_t make_sender_report(uint32_t ssrc_packet_sender, uint32_t ssrc_media_source, unsigned char *data, size_t len) {
		if (len < sizeof(SenderReportHeader) + sizeof(PeerInfoExchange)) return 0;

		SenderReportHeader sr_hdr = {};
		sr_hdr.P = 0;
		sr_hdr.RC = 0;
		sr_hdr.v = 2;
		sr_hdr.length = (sizeof(SenderReportHeader) + sizeof(PeerInfoExchange)) / 4 - 1;
		sr_hdr.ssrc = ssrc_packet_sender;

		uint32_t seconds, seconds_frac;
		ntp_time(seconds, seconds_frac);
		sr_hdr.timestamp_msw = seconds;
		sr_hdr.timestamp_lsw = seconds_frac;
		sr_hdr.rtp_timestamp = 0;
		sr_hdr.packet_count = 220;
		sr_hdr.octet_count = 105370;

		PeerInfoExchange info = {};
		info.type = 0x000C;
		info.length = sizeof(PeerInfoExchange);
		info.ssrc = ssrc_packet_sender;
		info.inbound_link_bandwidth = 2147483647;
		info.outbound_link_bandwidth = 2147483647;
		info.NC = 0;

		size_t off = 0;
		off += WriteSenderReportHeader(sr_hdr, &data[off], len - off);
		off += WritePeerInfoExchange(info, &data[off], len - off);

		return off;
	}
}