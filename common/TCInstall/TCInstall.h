#pragma once

#include <stdbool.h>

#if defined(_MSC_VER) && defined TCINSTALL_EXPORTS
#	define TCINSTALL_API __declspec(dllexport)
#elif defined(_MSC_VER) && defined TCINSTALL_DLL
#	define TCINSTALL_API __declspec(dllimport)
#else
#	define TCINSTALL_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Checks if library 'name' can be loaded.
bool TCINSTALL_API VSTryLoadLibrary(const char*);

#ifdef __cplusplus
}
#endif
