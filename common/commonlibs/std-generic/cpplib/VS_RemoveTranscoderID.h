#pragma once

#include "std-generic/cpplib/string_view.h"

const char TRANSCODER_ID_SEPARATOR = '/';

void VS_RemoveTranscoderID(std::string& call_id);	// modify inplace

inline string_view VS_RemoveTranscoderID_sv(string_view call_id)	// makes a copy of string_view
{
	auto pos = call_id.find_last_of(TRANSCODER_ID_SEPARATOR);
	if (pos != call_id.npos)
		call_id.remove_suffix(call_id.length() - pos);
	return call_id;
}
