#pragma once

#include <curl/curl.h>

struct CURL_deleter { void operator()(CURL* p) const { ::curl_easy_cleanup(p); } };
struct curl_free_deleter { void operator()(void* p) const { ::curl_free(p); } };
