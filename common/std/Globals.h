#pragma once

#include <string>

// In most these variables should be treated as read-only. Obvious exception is initialization logic.

extern std::string g_tr_endpoint_name;

namespace vs {

// Returns path to the directory in which all log files should be created.
// Returns "./" unless the path was explicitly set via SetLogDirectory.
// Path will either be absolute or relative to the "Working Directory", in any
// case it can be directly used as a prefix when creating path for log files.
// Returned path will always have a directory separator ('/' or '\') on the end.
const std::string& GetLogDirectory();

// Sets path returned by GetLogDirectory(), appends a directory separator if necessary.
void SetLogDirectory(std::string value);

}
