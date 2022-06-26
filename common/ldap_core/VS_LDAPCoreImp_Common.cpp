#include "VS_LDAPCoreImp_Common.h"

namespace tc {

	// Java example from https://ldapwiki.com/wiki/ObjectSID
	std::string ConvectSIDtoString(string_view sid)
	{
		if (sid.length() < 8)
			return {};
		std::string s = "S-";

		// get byte(0) - revision level
		s += std::to_string(sid[0]);

		//next byte byte(1) - count of sub-authorities
		uint8_t countSubAuths = sid[1] & 0xFF;

		//byte(2-7) - 48 bit authority ([Big-Endian])
		uint64_t authority = 0;
		for (int i = 2; i <= 7; i++) {
			authority |= static_cast<uint64_t>(sid[i]) << (8 * (5 - (i - 2)));
		}
		s += "-";
		s += std::to_string(authority);	// strSid.append(Long.toHexString(authority));

		//iterate all the sub-auths and then countSubAuths x 32 bit sub authorities ([Little-Endian])
		int offset = 8;
		int size = 4; //4 bytes for each sub auth
		for (int j = 0; j < countSubAuths; j++) {
			uint32_t subAuthority = 0;
			for (int k = 0; k < size; k++) {
				auto index = offset + k;
				if (index >= sid.length())
					return {};
				subAuthority |= (uint32_t)(sid[index] & 0xFF) << (8 * k);
			}
			// format it
			s += "-";
			s += std::to_string(subAuthority);
			offset += size;
		}
		return s;
	}

} // namespace tc