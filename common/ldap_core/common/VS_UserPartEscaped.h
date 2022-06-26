#pragma once
#include <string>

typedef std::string vs_userpart_escaped;		// no @server_name part, trust user_id is escaped (slash is encoded as %5c)
vs_userpart_escaped VS_GetUserPartEscaped(const char* call_id);
