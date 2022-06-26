#include "VS_UUID.h"

#include "std-generic/clib/sha1.h"

#include <cstdio>
#include <cstring>

namespace VS_UUID_Constants {
	// from MS-SIPRE http://interoperability.blob.core.windows.net/files/MS-SIPRE/[MS-SIPRE].pdf
	// http://pidgin-sipe.sourcearchive.com/documentation/1.7.1/uuid_8c-source.html
	const char epid_ns_uuid[] = "fcacfb03-8a73-46ef-91b1-e5ebeeaba4fe";
	const int UUID_OFFSET_TO_LAST_SEGMENT = 24;
}

VS_UUID::VS_UUID(const char *str) {
	using namespace VS_UUID_Constants;

	uint16_t v1, v2;
	sscanf(str, "%08x-%04hx-%04hx-%02hx%02hx-", &time_low, &time_mid, &time_hi_and_version, &v1, &v2);

	clock_seq_hi_and_reserved = static_cast<uint8_t>(v1);
	clock_seq_low = static_cast<uint8_t>(v2);

	for (int i = 0; i < 6; i++) {
		uint16_t v;
		sscanf(&str[UUID_OFFSET_TO_LAST_SEGMENT + i * 2], "%02hx", &v);
		node[i] = static_cast<uint8_t>(v);
	}
}

VS_UUID::VS_UUID(const unsigned char hash[20]) {
	*this = *reinterpret_cast<const VS_UUID *>(hash);
	time_hi_and_version &= 0x0FFF;
	time_hi_and_version |= 0x5000;
	clock_seq_hi_and_reserved &= 0x3F;
	clock_seq_hi_and_reserved |= 0x80;
}

VS_UUID::operator std::string()
{
	using namespace VS_UUID_Constants;

	char str[UUID_OFFSET_TO_LAST_SEGMENT + 12 + 1] = { 0 };

	sprintf(str, "%08x-%04x-%04x-%02x%02x-", time_low, time_mid, time_hi_and_version,
		clock_seq_hi_and_reserved, clock_seq_low);
	size_t pos = UUID_OFFSET_TO_LAST_SEGMENT;
	for (int i = 0; i < 6; i++)
	{
		pos += sprintf(str + pos, "%02x", node[i]);
	}
	return str;
}

std::string VS_UUID::GenEpid(string_view self_sip_uri, string_view hostname, string_view ip_address) {
	SHA1 sha1;
	sha1.Update(self_sip_uri);
	sha1.Update(":");
	sha1.Update(hostname);
	sha1.Update(":");
	sha1.Update(ip_address);
	sha1.Final();

	char hash_str[41];
	sha1.GetString(hash_str);
	return std::string(hash_str + 30, 10);
}

VS_UUID VS_UUID::GenUUID(string_view epid) {
	using namespace VS_UUID_Constants;

	VS_UUID res(epid_ns_uuid);

	SHA1 sha1;
	sha1.Update(&res, sizeof(res));
	sha1.Update(epid.data());
	sha1.Final();

	unsigned char hash[20];
	sha1.GetBytes(hash);
	res = VS_UUID(hash);
	return res;
}


VS_UUID& VS_UUID::operator=(const VS_UUID& src)
{
	if (this == &src)
	{
		return *this;
	}
	if(*this != src)
	{
		this->time_low                  = src.time_low;
		this->time_mid                  = src.time_mid;
		this->time_hi_and_version       = src.time_hi_and_version;
		this->clock_seq_hi_and_reserved = src.clock_seq_hi_and_reserved;
		this->clock_seq_low             = src.clock_seq_low;
		for (int i = 0; i < 6; ++i)
		{
			this->node[i] = src.node[i];
		}
	}

	return *this;
}

VS_UUID::VS_UUID(const VS_UUID& src)
	:time_low(src.time_low),
	 time_mid(src.time_mid),
	 time_hi_and_version(src.time_hi_and_version),
	 clock_seq_hi_and_reserved(src.clock_seq_hi_and_reserved),
	 clock_seq_low(src.clock_seq_low)
{
	for (int i = 0; i < 6; ++i)
		{
			this->node[i] = src.node[i];
		}
}

bool VS_UUID::operator!=(const VS_UUID& src)const
{

	if (this->time_low != src.time_low ||
		this->time_mid != src.time_mid ||
		this->time_hi_and_version != src.time_hi_and_version ||
		this->clock_seq_hi_and_reserved != src.clock_seq_hi_and_reserved ||
		this->clock_seq_low != src.clock_seq_low)
	{
		return true;
	}

	for (int i = 0; i < 6; ++i)
	{
		if (this->node[i] != src.node[i])
		{
			return true;
		}
	}
	return false;
}