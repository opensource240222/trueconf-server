#include "VS_UserPartEscaped.h"
#include "std/cpplib/VS_UserData.h"

vs_userpart_escaped VS_GetUserPartEscaped(const char* call_id)
{
	VS_RealUserLogin r(nullptr == call_id ? "" : call_id);
	vs_userpart_escaped userpart_escaped = r.GetUser();

	// replace all
	std::string from = "\\";
	std::string to = "%5c";
	if (from.empty())
		return userpart_escaped;
	size_t start_pos = 0;
	while ((start_pos = userpart_escaped.find(from, start_pos)) != std::string::npos) {
		userpart_escaped.replace(start_pos, from.length(), to);
		start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
	}

	//std::unique_ptr<CURL, CURL_deleter> curl(::curl_easy_init());
	//if (curl) {
	//	std::unique_ptr<char, curl_free_deleter> escaped(::curl_easy_escape(curl.get(), r.GetUser().c_str(), r.GetUser().length()));
	//	if (escaped)
	//		userpart_escaped = escaped.get();
	//}
	if (userpart_escaped.empty())
		userpart_escaped = r.GetUser();
	return userpart_escaped;
}
