#include "SIPParserTestBase.h"

void strreplace(std::string &str, string_view _old, string_view _new)
{
	const size_t pos = str.find(std::string(_old));
	if (pos == std::string::npos) return;
	str.replace(pos, _old.length(), _new.data(), _new.length());
}

std::string CombineInviteAndSDP(string_view inv, string_view sdp)
{
	std::string res(inv);
	res += "\r\n";
	res += sdp;
	char buff[10] = { 0 };
	snprintf(buff, 10, "%u", (uint32_t)sdp.length());
	strreplace(res, "__contentlength__", buff);
	return res;
}

void ConstructResponse(std::string &response, string_view orig, string_view callId, string_view branch)
{
	response = std::string(orig);
	strreplace(response, "__callid__", callId);
	strreplace(response, "__branch__", branch);
}