// No include guard or #pragma once, it's safe to include this file multiple times.

// This file removes defines for WinAPI functions that have both ANSI and
// UNICODE version which conflict with identifiers used in our code.
// Add new undefs as required, but please, keep the list sorted.

// Maintenance note:
// List of all problematic defines can be generated with:
//     grep -ohr '#define *\([A-Z][A-Za-z0-9]*[a-z][A-Za-z0-9]*\) *\1A' /c/Program\ Files\ \(x86\)/Windows\ Kits/10/Include/10.0.14393.0/ | sed -e 's:^#define *\([^ ]*\).*$:\1:' | sort -u > winapi-defines.txt
// After that we can find all places in our code where we use these names:
//     grep --color=auto -rwI --exclude '*.map' -f winapi-defines.txt tc3/

#if defined(_WIN32) // This is Windows-only problem

#ifdef GetMessage
#undef GetMessage
#endif

#ifdef GetMonitorInfo
#undef GetMonitorInfo
#endif

#ifdef PostMessage
#undef PostMessage
#endif

#ifdef SendMessage
#undef SendMessage
#endif

#ifdef SetPort
#undef SetPort
#endif

#endif
