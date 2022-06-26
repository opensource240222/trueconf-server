#pragma once

namespace transport_test {

static const unsigned char serialized_handshake_rnd_data[] = {
	0x1d, 0xf5, 0xb7, 0xd0, 0xe2, 0xb1, 0x76, 0x8b,
	0xf8, 0x00, 0x31, 0x42, 0xbe, 0xc9, 0x47, 0xd4,
	0xda, 0x3f, 0x62, 0xdc,
};

static const char serialized_handshake_private_key[] = // openssl genrsa 512
	"-----BEGIN RSA PRIVATE KEY-----\n"
	"MIIBPAIBAAJBAMp9uubT4PfkEuFUmLV1jdU4uEq1YN69TshvTtlLvM75PuO5v+LB\n"
	"aIAjClk3T2iwbXKTm0IW+0N9ZGmhsVFdwDsCAwEAAQJBAMiTVFCTbhN+mukS2BVe\n"
	"tTiQ75QpKCRalrBZ+Vuq9wJ4A6F15+Ty7zZSE3/c8iDsE8raJMnu8T74oMIoFwNW\n"
	"5JECIQDz3p966UeWrKyJcbMsx/5IuQgXbi9W79JjyUWR44n0KQIhANSQNBMD0gDk\n"
	"AjjrhV+hVHiDNjPnk3mihs5gEdKaoz3DAiEA8eD7OUOrt6yorOJTHrV2ZtbvSZ4F\n"
	"4iqX+FUnZZ3TtJECIQCRBBxyyI1yDBw0BwJD/sWpPiMxgBDN0ALWMxaVrugljwIg\n"
	"ZCvLpjaetHJG4hZRGee/Dp58FKESLh2iSbatNu2T8Yg=\n"
	"-----END RSA PRIVATE KEY-----\n"
	;

static const unsigned char serialized_handshake[] = {
	'_', 'V', 'S', '_', 'T', 'R', 'A', 'N', 'S', 'P', 'O', 'R', 'T', '_', 0, 0,
	0xd9, // version=9|0x10, head_cksum
	0xf9, // head_cksum, body_cksum,
	0x78, // body_cksum, body_length
	0x03, // body_length
	0x09, 'C', 'l', 'i', 'e', 'n', 't', ' ', 'I', 'D', '\0',
	0x09, 'S', 'e', 'r', 'v', 'e', 'r', ' ', 'I', 'D', '\0',
	0x0c,
	0x14, 0x00,
	0x1d, 0xf5, 0xb7, 0xd0, 0xe2, 0xb1, 0x76, 0x8b,
	0xf8, 0x00, 0x31, 0x42, 0xbe, 0xc9, 0x47, 0xd4,
	0xda, 0x3f, 0x62, 0xdc,
	0x40, 0x00,
	0xc6, 0x4e, 0x23, 0xf8, 0xee, 0xd6, 0x58, 0x1f,
	0xdd, 0xb8, 0x5f, 0x72, 0x8b, 0x79, 0xcc, 0xb4,
	0x90, 0xf5, 0x75, 0x45, 0x8d, 0x94, 0xe0, 0x5d,
	0x35, 0xfd, 0x4b, 0x84, 0x81, 0x3b, 0xa6, 0xee,
	0xfb, 0x12, 0xc8, 0x10, 0xcd, 0x58, 0xa6, 0x44,
	0xd4, 0x57, 0x7e, 0x06, 0x6b, 0x1e, 0x4b, 0x78,
	0x10, 0x0a, 0x3c, 0x43, 0x61, 0x10, 0x89, 0x93,
	0x8a, 0x0f, 0xc4, 0xba, 0xc1, 0xdb, 0x78, 0x12,
	0x01,
};

static const unsigned char serialized_handshake_nosign[] = {
	'_', 'V', 'S', '_', 'T', 'R', 'A', 'N', 'S', 'P', 'O', 'R', 'T', '_', 0, 0,
	0x19, // version=9|0x10, head_cksum
	0x33, // head_cksum, body_cksum,
	0xbd, // body_cksum, body_length
	0x00, // body_length
	0x09, 'C', 'l', 'i', 'e', 'n', 't', ' ', 'I', 'D', '\0',
	0x09, 'S', 'e', 'r', 'v', 'e', 'r', ' ', 'I', 'D', '\0',
	0x00,
	0x01,
};

static const unsigned char serialized_handshake_reply[] = {
	'_', 'V', 'S', '_', 'T', 'R', 'A', 'N', 'S', 'P', 'O', 'R', 'T', '_', 0, 0,
	0x39, // version=9|0x10, head_cksum
	0xd1, // head_cksum, body_cksum,
	0xda, // body_cksum, body_length
	0x00, // body_length
	0x00,
	0x80, 0x0d,
	0x4e,
	0x0c,
	0x09, 'S', 'e', 'r', 'v', 'e', 'r', ' ', 'I', 'D', '\0',
	0x09, 'C', 'l', 'i', 'e', 'n', 't', ' ', 'I', 'D', '\0',
	0x01,
};

static const unsigned char serialized_handshake_oldarch[] = {
	'_', 'V', 'S', '_', 'T', 'R', 'A', 'N', 'S', 'P', 'O', 'R', 'T', '_', 0, 0,
	0xb9, // version=9|0x10, head_cksum
	0xb2, // head_cksum, body_cksum,
	0x46, // body_cksum, body_length
	0x00, // body_length
	0x00,
	0x02,
	0x00, 0x00,
	0x00,
	0x01, '0', '\0',
	0x00,
};

// To test body_cksum field body has to be larger (by at least 16 bytes) than
// the header (see transport::GetMessageBodyChecksum for details).
static const char serialized_message_body[] =
	"This has to be a really long text to test body_cksum field\n"
	"0123456789\n"
	"tHIS HAS TO BE A REALLY LONG TEXT TO TEST BODY_CKSUM FIELD\n"
	;

static const unsigned char serialized_message[] = {
	0xc1, // version==1, request==1, mark1==1
	0xc5, // head_cksum
	0x6b, 0x00, // head_length
	0x48, // body_cksum
	0x81, 0x00, 0x00, // body_length
	0x7b, 0x00, 0x00, 0x00, // seq_number
	0x15, 0x03, 0x00, 0x00, // ms_life_count
	0x06, 'S', 'r', 'c', 'C', 'I', 'D', '\0',
	0x0a, 'S', 'r', 'c', 'S', 'e', 'r', 'v', 'i', 'c', 'e', '\0',
	0x09, 'A', 'd', 'd', 'S', 't', 'r', 'i', 'n', 'g', '\0',
	0x06, 'D', 's', 't', 'C', 'I', 'D', '\0',
	0x0a, 'D', 's', 't', 'S', 'e', 'r', 'v', 'i', 'c', 'e', '\0',
	0x07, 'S', 'r', 'c', 'U', 's', 'e', 'r', '\0',
	0x07, 'D', 's', 't', 'U', 's', 'e', 'r', '\0',
	0x09, 'S', 'r', 'c', 'S', 'e', 'r', 'v', 'e', 'r', '\0',
	0x09, 'D', 's', 't', 'S', 'e', 'r', 'v', 'e', 'r', '\0',
	'T', 'h', 'i', 's', ' ', 'h', 'a', 's', ' ', 't', 'o', ' ', 'b', 'e', ' ', 'a', ' ', 'r', 'e', 'a', 'l', 'l', 'y', ' ', 'l', 'o', 'n', 'g', ' ', 't', 'e', 'x', 't', ' ', 't', 'o', ' ', 't', 'e', 's', 't', ' ', 'b', 'o', 'd', 'y', '_', 'c', 'k', 's', 'u', 'm', ' ', 'f', 'i', 'e', 'l', 'd', '\n',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '\n',
	't', 'H', 'I', 'S', ' ', 'H', 'A', 'S', ' ', 'T', 'O', ' ', 'B', 'E', ' ', 'A', ' ', 'R', 'E', 'A', 'L', 'L', 'Y', ' ', 'L', 'O', 'N', 'G', ' ', 'T', 'E', 'X', 'T', ' ', 'T', 'O', ' ', 'T', 'E', 'S', 'T', ' ', 'B', 'O', 'D', 'Y', '_', 'C', 'K', 'S', 'U', 'M', ' ', 'F', 'I', 'E', 'L', 'D', '\n',
	'\0',
};
static_assert(sizeof(serialized_message_body) >= sizeof(serialized_message) - sizeof(serialized_message_body) + 16, "Message body is too small to test body_cksum field.");

}

