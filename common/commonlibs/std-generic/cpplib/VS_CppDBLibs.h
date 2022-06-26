#pragma once

// Those are dependencies of cppdb and libpq
#if defined(_WIN32)
#	pragma comment(lib, "Secur32.lib")
#	pragma comment(lib, "Shell32.lib")
#	pragma comment(lib, "Ws2_32.lib")
#endif
