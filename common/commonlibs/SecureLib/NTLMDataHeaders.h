#pragma once

#include <cstdint>

namespace ntlm{
#pragma pack(push, 1)
struct InfoFields {
	uint16_t responseLen;
	uint16_t responseMaxLen;
	uint32_t responseBufferOffset;
};

// defined here https://msdn.microsoft.com/en-us/library/cc236643.aspx
struct AUTHENTICATE_MESSAGE_HEADER {
	char signature[8];
	uint32_t messageType = 3;
	InfoFields lmChallengeResponse;
	InfoFields ntChallengeResponse;
	InfoFields domainName;
	InfoFields userName;
	InfoFields workstation;
	InfoFields encryptedRandomSessionKey;
	uint32_t negotiateFlags = 0;
	uint64_t version;
	uint8_t MIC[16];
};

// defined here https://msdn.microsoft.com/en-us/library/cc236646.aspx
struct AV_PAIR {
	uint16_t avId;
	uint16_t avLen;
};

// defined here https://msdn.microsoft.com/en-us/library/cc236642.aspx
struct CHALLENGE_MESSAGE_HEADER {
	char signature[8];
	uint32_t msgType;
	InfoFields tagetName;
	uint32_t negFlags;
	unsigned char serverChallenge[8];
	unsigned char reserved[8];	// must be zero and ignored
	InfoFields targetInfo;
	uint64_t version;
};

// defined here https://msdn.microsoft.com/en-us/library/cc422951.aspx
struct SSP_MESSAGE_SIGNATURE {
	uint32_t version;
	unsigned char randomPad[4];
	uint32_t checksum;
	uint32_t seqN;
};

// defined here https://msdn.microsoft.com/en-us/library/cc422952.aspx
struct SSP_MESSAGE_SIGNATURE_Ex {
	uint32_t version;
	unsigned char checksum[8];
	uint32_t seqN;
};

//  defined here https://msdn.microsoft.com/en-us/library/cc236652.aspx
struct CLIENT_CHALLENGE_HEADER{
	uint8_t respType;
	uint8_t hiRespType;
	unsigned char reserved[6];	// must be zeroed
	uint64_t timeStamp;
	unsigned char challengeFromClient[8];
	unsigned char reserved1[4]; // must be zeroed
};
#pragma pack(pop)
}