#ifdef _WIN32
#pragma once

#include <functional>
#include <string>

namespace vs {
	extern bool RunNTServiceDispatcher(int &service_exit_status, const std::function<int(void)> &service_body, const std::function<void(void)> &on_stop);
}

#endif // _WIN32
