#pragma once

#if defined(_WIN32)
#    include <string.h>
#    define wcscasecmp  _wcsicmp
#    define wcsncasecmp  _wcsnicmp
#elif defined(__unix__)
#    include <wchar.h>
#endif
