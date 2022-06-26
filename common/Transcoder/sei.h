#pragma once

#include "std-generic/cpplib/hton.h"

#include <cassert>
#include <cstring>
#include <cstdint>

#pragma pack(push, 1)

namespace sei {
	const uint8_t uuid_magic1[16] = { 0x13, 0x9F, 0xB1, 0xA9, 0x44, 0x6A, 0x4D, 0xEC,
									  0x8C, 0xBF, 0x65, 0xB1, 0xE1, 0x2D, 0x2C, 0xFD };

	struct StreamLayoutMessage {
		uint8_t Type : 5;
		uint8_t NRI : 2;
		uint8_t F : 1;

		uint8_t PayloadType;
		uint8_t PayloadSize;

		uint8_t uuid[16];

		uint8_t LPB0_0 : 1;
		uint8_t LPB0_1 : 1;
		uint8_t LPB0_2 : 1;
		uint8_t LPB0_3 : 1;
		uint8_t LPB0_4 : 1;
		uint8_t LPB0_5 : 1;
		uint8_t LPB0_6 : 1;
		uint8_t LPB0_7 : 1;

		uint8_t LPB1_7[7];

		uint8_t P : 1;
		uint8_t R : 7;
		uint8_t LDSize;

		StreamLayoutMessage() {
			memset(this, 0, sizeof(StreamLayoutMessage));
			memcpy(uuid, uuid_magic1, sizeof(uuid_magic1));
		}
	};

	static_assert(sizeof(StreamLayoutMessage) == 29, "!");

	inline bool ParseStreamLayoutMessage(const unsigned char *data, size_t len, StreamLayoutMessage &msg) {
		if (sizeof(StreamLayoutMessage) > len) return false;

		StreamLayoutMessage *s = (StreamLayoutMessage *)data;
		if (s->Type != 6 || s->PayloadType != 5) return false;

		msg = *s;
		return memcmp(s->uuid, uuid_magic1, 16) == 0;
	}

	inline size_t WriteStreamLayoutMessage(const StreamLayoutMessage &msg, unsigned char *out_buf, size_t len) {
		if (len < sizeof(StreamLayoutMessage)) return 0;

		StreamLayoutMessage *s = (StreamLayoutMessage *)out_buf;
		*s = msg;

		return sizeof(StreamLayoutMessage);
	}

	enum {
		fps_7_5 = 0,
		fps_12_5 = 1,
		fps_15 = 2,
		fps_25 = 3,
		fps_30 = 4,
		fps_50 = 5,
		fps_60 = 6,
	};

	struct LayerDescription {
		uint16_t CodedWidth, CodedHeight;
		uint16_t DisplayWidth, DisplayHeight;
		uint32_t Bitrate;
		uint8_t LT : 3;
		uint8_t FPSIdx : 5;
		uint8_t R : 1;
		uint8_t CB : 1;
		uint8_t PRID : 6;
		uint16_t R2;
	};

	static_assert(sizeof(LayerDescription) == 16, "!");

	inline bool ParseLayerDescription(const unsigned char *data, size_t len, LayerDescription &l) {
		if (sizeof(LayerDescription) > len) return 0;

		l = *(const LayerDescription *)data;
		l.CodedWidth = vs_ntohs(l.CodedWidth);
		l.CodedHeight = vs_ntohs(l.CodedHeight);
		l.DisplayWidth = vs_ntohs(l.DisplayWidth);
		l.DisplayHeight = vs_ntohs(l.DisplayHeight);
		l.Bitrate = vs_ntohl(l.Bitrate);

		return true;
	}

	inline size_t WriteLayerDescription(const LayerDescription &l, unsigned char *out_buf, size_t len) {
		if (len < sizeof(LayerDescription)) return false;

		LayerDescription *_l = (LayerDescription *)out_buf;
		*_l = l;

		_l->CodedWidth = vs_htons(_l->CodedWidth);
		_l->CodedHeight = vs_htons(_l->CodedHeight);
		_l->DisplayWidth = vs_htons(_l->DisplayWidth);
		_l->DisplayHeight = vs_htons(_l->DisplayHeight);
		_l->Bitrate = vs_htonl(_l->Bitrate);

		return sizeof(LayerDescription);
	}

	const uint8_t uuid_magic2[16] = { 0x05, 0xFB, 0xC6, 0xB9, 0x5A, 0x80, 0x40, 0xE5,
									  0xA2, 0x2A, 0xAB, 0x40, 0x20, 0x26, 0x7E, 0x26 };

	struct BitstreamInfoMessage {
		uint8_t Type : 5;
		uint8_t NRI	 : 2;
		uint8_t F    : 1;

		uint8_t PayloadType;
		uint8_t PayloadSize;

		uint8_t uuid[16];
		uint8_t ref_frm_cnt;
		uint8_t num_of_nal_unit;

		BitstreamInfoMessage() {
			memset(this, 0, sizeof(BitstreamInfoMessage));
			memcpy(uuid, uuid_magic2, sizeof(uuid_magic2));
		}
	};

	static_assert(sizeof(BitstreamInfoMessage) == 21, "!");

	inline bool ParseBitstreamInfoMessage(const unsigned char *data, size_t len, BitstreamInfoMessage &i) {
		if (sizeof(BitstreamInfoMessage) > len) return false;

		BitstreamInfoMessage *m = (BitstreamInfoMessage *)data;

		if (m->Type != 6 || m->PayloadType != 5) return false;

		i = *m;
		return memcmp(i.uuid, uuid_magic2, 16) == 0;
	}

	inline size_t WriteBitstreamInfoMessage(const BitstreamInfoMessage &i, unsigned char *out_buf, size_t len) {
		if (len < sizeof(BitstreamInfoMessage)) return 0;

		BitstreamInfoMessage *m = (BitstreamInfoMessage *)out_buf;
		*m = i;

		return sizeof(BitstreamInfoMessage);
	}

	struct PACSINalUnit {
		uint8_t Type : 5;
		uint8_t NRI : 2;
		uint8_t F : 1;
		//
		uint8_t PRID : 6;
		uint8_t I : 1;
		uint8_t R : 1;
		//
		uint8_t QID : 4;
		uint8_t DID : 3;
		uint8_t N : 1;
		//
		uint8_t RR : 2;
		uint8_t O : 1;
		uint8_t D : 1;
		uint8_t U : 1;
		uint8_t TID : 3;
		//
		uint8_t E : 1;
		uint8_t S : 1;
		uint8_t C : 1;
		uint8_t P : 1;
		uint8_t A : 1;
		uint8_t T : 1;
		uint8_t Y : 1;
		uint8_t X : 1;

		uint16_t DONC;
	};

	static_assert(sizeof(PACSINalUnit) == 7, "!");

	inline bool ParsePACSINalUnit(const unsigned char *data, size_t len, PACSINalUnit &p) {
		if (sizeof(PACSINalUnit) > len) return false;

		PACSINalUnit *m = (PACSINalUnit *)data;

		if (m->Type != 30) return false;

		p = *m;
		p.DONC = vs_ntohs(p.DONC);
		return true;
	}

	inline size_t WritePACSINalUnit(const PACSINalUnit &p, unsigned char *out_buf, size_t len) {
		if (len < sizeof(PACSINalUnit)) return 0;

		PACSINalUnit *m = (PACSINalUnit *)out_buf;
		*m = p;

		if (p.T) {
			m->DONC = vs_htons(m->DONC);
			return sizeof(PACSINalUnit);
		} else {
			return sizeof(PACSINalUnit) - 2;
		}
	}

	inline void test_stream_layout_message() {
		const unsigned char in_buf[] = { 0x06, 0x05, 0x3a, 0x13, 0x9f, 0xb1, 0xa9, 0x44,
										 0x6a, 0x4d, 0xec, 0x8c, 0xbf, 0x65, 0xb1, 0xe1,
										 0x2d, 0x2c, 0xfd, 0x03, 0x00, 0x00, 0x00, 0x00,
										 0x00, 0x00, 0x00, 0x01, 0x10, 0x01, 0x40, 0x00,
										 0xb4, 0x01, 0x40, 0x00, 0xb4, 0x00, 0x00, 0xda,
										 0xc1, 0x00, 0x00, 0x00, 0x00 };

		StreamLayoutMessage msg;
		assert(ParseStreamLayoutMessage(in_buf, sizeof(in_buf), msg));

		assert(msg.Type == 6);
		assert(msg.PayloadType == 5);
		assert(msg.PayloadSize == in_buf[2]);
		assert(msg.LPB0_0 == 1);
		assert(msg.LPB0_1 == 1);
		assert(msg.LPB0_2 == 0);
		assert(msg.P == 1);
		assert(msg.LDSize == sizeof(LayerDescription));

		LayerDescription l;
		assert(ParseLayerDescription(in_buf + sizeof(StreamLayoutMessage), sizeof(in_buf) - sizeof(StreamLayoutMessage), l));

		assert(l.CodedWidth == 320);
		assert(l.CodedHeight == 180);
		assert(l.DisplayWidth == 320);
		assert(l.DisplayHeight == 180);
		assert(l.Bitrate == 56001);
		assert(l.LT == 0);
		assert(l.FPSIdx == 0);
		assert(l.R == 0);
		assert(l.CB == 0);
		assert(l.PRID == 0);
		assert(l.R2 == 0);

		unsigned char out_buf[128];
		size_t s = WriteStreamLayoutMessage(msg, out_buf, sizeof(out_buf));
		assert(s == sizeof(StreamLayoutMessage));
		assert(memcmp(out_buf, in_buf, s) == 0);

		s = WriteLayerDescription(l, out_buf, sizeof(out_buf));
		assert(s == sizeof(LayerDescription));
		assert(memcmp(out_buf, in_buf + sizeof(StreamLayoutMessage), s) == 0);
	}

	inline void test_bitstream_info_message() {
		const unsigned char in_buf[] = { 0x06, 0x05, 0x12, 0x05, 0xfb, 0xc6, 0xb9, 0x5a,
										 0x80, 0x40, 0xe5, 0xa2, 0x2a, 0xab, 0x40, 0x20,
										 0x26, 0x7e, 0x26, 0x01, 0x01 };

		BitstreamInfoMessage i;
		assert(ParseBitstreamInfoMessage(in_buf, sizeof(in_buf), i));

		assert(i.Type == 6);
		assert(i.PayloadType == 5);
		assert(i.PayloadSize == 18);
		assert(i.ref_frm_cnt == 1);
		assert(i.num_of_nal_unit == 1);

		unsigned char out_buf[128];
		size_t s = WriteBitstreamInfoMessage(i, out_buf, sizeof(out_buf));
		assert(s == sizeof(BitstreamInfoMessage));
		assert(memcmp(out_buf, in_buf, s) == 0);
	}

	inline void test_pacsi_nal_unit() {
		const unsigned char in_buf[] = { 0x7e, 0xc0, 0x80, 0x0f, 0x22, 0x00, 0x00 };

		PACSINalUnit p;
		assert(ParsePACSINalUnit(in_buf, sizeof(in_buf), p));

		assert(p.Type == 30);
		assert(p.NRI == 3);
		assert(p.I == 1);
		assert(p.R == 1);
		assert(p.N == 1);
		assert(p.RR == 3);
		assert(p.O == 1);
		assert(p.D == 1);
		assert(p.S == 1);
		assert(p.T == 1);

		unsigned char out_buf[128];
		size_t s = WritePACSINalUnit(p, out_buf, sizeof(out_buf));
		assert(s == sizeof(PACSINalUnit));
		assert(memcmp(out_buf, in_buf, s) == 0);
	}

	inline void test() {
		test_stream_layout_message();
		test_bitstream_info_message();
		test_pacsi_nal_unit();
	}
}

#pragma pack(pop)

