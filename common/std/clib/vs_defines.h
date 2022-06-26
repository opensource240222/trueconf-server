#pragma once

#if defined(__unix__)
#   include <linux/limits.h>
#   define MAX_PATH PATH_MAX
#endif