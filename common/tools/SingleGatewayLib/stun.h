#pragma once

#include "std-generic/cpplib/hton.h"

#include <cstddef>
#include <memory>
#include <string>

#include <boost/crc.hpp>

#include <openssl/hmac.h>

namespace stun {
	enum eMsgType {
		BindingRequest				= 0x0001,
		SharedSecretRequest			= 0x0002,
		BindingResponse				= 0x0101,
		SharedSecretResponse		= 0x0102,
		BindingErrorResponse		= 0x0111,
		SharedSecretErrorResponse	= 0x0112,
	};

	enum eAttribType {
		MappedAddress		= 0x0001,
		UserName			= 0x0006,
		MessageIntegrity	= 0x0008,
		ErrorCode			= 0x0009,
		UnknownAttributes	= 0x000A,
		Realm				= 0x0014,
		Nonce				= 0x0015,
		XorMappedAddress	= 0x0020,

		MsImplementationVersion = 0x8070,
		Fingerprint			= 0x8028,
	};

	enum eAddrFamily {
		IPv4 = 0x01,
		IPv6 = 0x02,
	};

	const uint32_t magic = 0x2112A442;

	struct Header {
		uint16_t msg_type;
		uint16_t msg_len;
		uint32_t magic;
		uint32_t transaction_id[3];
	};

	static_assert(sizeof(Header) == 20, "!");

	inline bool ParseHeader(const unsigned char *data, unsigned len, Header &hdr) {
		if (len < sizeof(Header)) return false;
		hdr = *(const Header *)data;

		hdr.msg_type = vs_ntohs(hdr.msg_type);
		hdr.msg_len = vs_ntohs(hdr.msg_len);
		hdr.magic = vs_ntohl(hdr.magic);

		return hdr.magic == stun::magic;
	}

	inline unsigned WriteHeader(const Header *hdr, void *data, unsigned len) {
		Header *_hdr = (Header *)data;
		*_hdr = *hdr;

		_hdr->msg_type = vs_htons(_hdr->msg_type);
		_hdr->msg_len = vs_htons(_hdr->msg_len);
		_hdr->magic = vs_htonl(_hdr->magic);

		return sizeof(Header);
	}

	struct MappedAddressAttr {
		uint8_t family;
		uint16_t port;
		union {
			uint32_t addr_v4;
			uint32_t addr_v6[4];
		};
	};

	inline bool ParseMappedAddressAttr(const unsigned char *data, unsigned len, MappedAddressAttr &attr) {
		if (len < 8) return false;

		const unsigned char *p_data = data;
		p_data++; // skip

		MappedAddressAttr ret;
		ret.family = *p_data++;
		ret.port = vs_ntohs(*(uint16_t *)p_data);
		p_data += 2;
		if (ret.family == 0x01) {
			if (len != 8) return false;
			ret.addr_v4 = vs_ntohl(*(uint32_t *)p_data);
		} else if (ret.family == 0x02) {
			if (len != 20) return false;
			// todo ipv6
		} else {
			return false;
		}

		return true;
	}

	inline bool ParseUsernameAttr(const unsigned char *data, unsigned len, std::string &username) {
		if (!len) return false;

		const char *p_data = (const char *)data;
		username = std::string(p_data, len);

		return true;
	}

	inline unsigned WriteUsernameAttr(const std::string &user, unsigned char *data, int len) {
		int padding = (4 - (user.length() + 4) % 4) % 4;
		int len_w_padding = user.length() + padding;
		if (len < len_w_padding + 4) return 0;

		*((uint16_t *)&data[0]) = vs_htons(UserName);
		*((uint16_t *)&data[2]) = vs_htons(len_w_padding);

		memcpy(&data[4], user.data(), user.length());
		memset(&data[4 + user.length()], 0, padding);
		return len_w_padding + 4;
	}

	inline bool ParseMessageIntegrityAttr(const unsigned char *data, unsigned len, unsigned char hmac[20]) {
		if (len != 20) return false;
		memcpy(hmac, data, 20);
		return true;
	}

	inline bool ParseMessageFingerprintAttr(const unsigned char *data, unsigned len, void *crc) {
		if (len != 4) return false;
		memcpy(crc, data, 4);
		return true;
	}

	inline unsigned WriteXorMappedAddress(unsigned long ip, unsigned short port, uint32_t magic, unsigned char *data, int len) {
		if (len < 12) return 0;

		*((uint16_t *)&data[0]) = vs_htons(XorMappedAddress);
		*((uint16_t *)&data[2]) = vs_htons(8);

		data[4] = 0;
		data[5] = IPv4;

		uint16_t xport = port;
		xport ^= *((uint16_t *)&magic + 1);
		xport = vs_htons(xport);
		memcpy(&data[6], &xport, 2);

		uint32_t xip = ip;
		xip ^= magic;
		xip = vs_htonl(xip);
		memcpy(&data[8], &xip, 4);

		return 12;
	}

	inline unsigned WriteIntegrity(unsigned char hmac[20], unsigned char *data, int len) {
		if (len < 24) return 0;

		*((uint16_t *)&data[0]) = vs_htons(MessageIntegrity);
		*((uint16_t *)&data[2]) = vs_htons(20);

		memcpy(&data[4], hmac, 20);

		return 24;
	}

	inline unsigned WriteFingerprint(uint32_t f, unsigned char *data, int len) {
		if (len < 8) return 0;

		*((uint16_t *)&data[0]) = vs_htons(Fingerprint);
		*((uint16_t *)&data[2]) = vs_htons(4);

		f = vs_htonl(f);
		memcpy(&data[4], &f, 4);

		return 8;
	}

	inline unsigned WriteMsImplementationVersion(unsigned v, unsigned char *data, int len) {
		if (len < 8) return 0;

		*((uint16_t *)&data[0]) = vs_htons(MsImplementationVersion);
		*((uint16_t *)&data[2]) = vs_htons(4);

		*((uint32_t *)&data[4]) = vs_htonl(v);

		return 8;
	}

	inline void CalculateIntegrity(const unsigned char *data, unsigned len, const std::string &pass, unsigned char hmac[20]) {
		// different from rfc, that's how lync do it (see test)
		unsigned padding = (64 - len % 64) % 64;
		unsigned msg_size_padded = len + padding;
		std::unique_ptr<unsigned char[]> temp_buf(new unsigned char[msg_size_padded]);
		memcpy(&temp_buf[0], &data[0], len);
		memset(&temp_buf[len], 0, msg_size_padded - len);

		HMAC(EVP_sha1(), pass.data(), pass.length(), &temp_buf[0], msg_size_padded, hmac, NULL);
	}

	inline void CalculateFingerprint(const unsigned char *data, unsigned len, uint32_t *res) {
		boost::crc_32_type result;
		result.process_bytes(data, len);
		*res = result.checksum();
		*res ^= 0x5354554e;
	}

	inline void test_integrity() {
		const unsigned char stun_req[] = { 0x00, 0x01, 0x00, 0x54, 0x21, 0x12, 0xa4, 0x42, 0x3a, 0x8f, 0x72, 0x12, 0x8d, 0x7a, 0x46, 0x1d, 0x17, 0x1a, 0xec, 0xd2, 0x00, 0x06, 0x00, 0x0c, 0x36, 0x52, 0x31, 0x54, 0x3a, 0x63, 0x5a, 0x67, 0x56, 0x00, 0x00, 0x00, 0x00, 0x24, 0x00, 0x04, 0x6e, 0xff, 0xfc, 0xff, 0x80, 0x29, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x6a, 0x86, 0x62, 0x62, 0x80, 0x54, 0x00, 0x04, 0x32, 0x00, 0x00, 0x00, 0x80, 0x70, 0x00, 0x04, 0x00, 0x00, 0x00, 0x02, 0x00, 0x08, 0x00, 0x14, 0x69, 0xa1, 0xb8, 0xd7, 0xff, 0xa6, 0xc1, 0x9f, 0x93, 0xdb, 0x42, 0x42, 0x17, 0x88, 0xb3, 0x1a, 0x92, 0xc8, 0x68, 0x69, 0x80, 0x28, 0x00, 0x04, 0x61, 0x42, 0x3e, 0xda };
		const char pass[] = "niMwkhZcv9/Ggtcv1HL0q2aB";

		const unsigned char *p_data = stun_req + 20;
		unsigned off = 0, data_len = stun_req[3];
		while (off < data_len) {
			uint16_t attrib_type = vs_ntohs(*(uint16_t *)&p_data[off]);
			uint16_t attrib_len = vs_ntohs(*(uint16_t *)&p_data[off + 2]);
			off += 4;

			if (attrib_type == MessageIntegrity) {
				unsigned char hmac_sha1[20];
				if (ParseMessageIntegrityAttr(&p_data[off], attrib_len, hmac_sha1)) {
					unsigned char hmac[20] = { 0 };
					CalculateIntegrity(stun_req, off - 4 + 20, pass, hmac);

					assert(memcmp(hmac_sha1, hmac, 20) == 0);
					return;
				}
			}

			off += attrib_len;

			// skip padding
			while ((off & 0x03) != 0 && off < data_len) {
				off++;
			}
		}

		assert(false);
	}

	inline void test_fingerprint() {
		const unsigned char stun_resp[] = { 0x01, 0x01, 0x00, 0x44, 0x21, 0x12, 0xa4, 0x42, 0x42, 0xf2, 0x27, 0x68, 0xf7, 0x81, 0xb1, 0x6d, 0x3e, 0xe7, 0x0a, 0xf9, 0x00, 0x20, 0x00, 0x08, 0x00, 0x01, 0x4a, 0x22, 0xe1, 0xba, 0xe6, 0x7b, 0x00, 0x06, 0x00, 0x0c, 0x62, 0x48, 0x74, 0x4a, 0x3a, 0x6e, 0x34, 0x45, 0x43, 0x00, 0x00, 0x00, 0x80, 0x70, 0x00, 0x04, 0x00, 0x00, 0x00, 0x02, 0x00, 0x08, 0x00, 0x14, 0xfc, 0x48, 0x63, 0x2e, 0x6e, 0x8f, 0xfb, 0x0d, 0xf5, 0x48, 0xa1, 0xa3, 0xdd, 0x16, 0x76, 0xbf, 0x30, 0xcf, 0x0a, 0xce, 0x80, 0x28, 0x00, 0x04, 0xb6, 0x10, 0x8b, 0xce };

		const unsigned char *p_data = stun_resp + 20;
		unsigned off = 0, data_len = stun_resp[3];
		while (off < data_len) {
			uint16_t attrib_type = vs_ntohs(*(uint16_t *)&p_data[off]);
			uint16_t attrib_len = vs_ntohs(*(uint16_t *)&p_data[off + 2]);
			off += 4;

			if (attrib_type == Fingerprint) {
				uint32_t crc;
				if (ParseMessageFingerprintAttr(&p_data[off], attrib_len, &crc)) {
					crc = vs_ntohl(crc);

					uint32_t v;
					CalculateFingerprint(stun_resp, off - 4 + 20, &v);

					assert(crc == v);
					return;
				}
			}

			off += attrib_len;

			// skip padding
			while ((off & 0x03) != 0 && off < data_len) {
				off++;
			}
		}

		assert(false);
	}

	inline void test() {
		test_integrity();
		test_fingerprint();
	}
}