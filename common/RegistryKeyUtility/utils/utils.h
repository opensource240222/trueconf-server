#pragma once
#include "std-generic/cpplib/string_view.h"

bool is_valid_key_name(const string_view str);
string_view cut_front_end_delimeter(string_view str);

bool is_int_numbers(const char *in);
bool is_base64(const char *in);