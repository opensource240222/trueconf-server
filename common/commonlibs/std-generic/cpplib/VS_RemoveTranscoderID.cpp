#include "VS_RemoveTranscoderID.h"

void VS_RemoveTranscoderID(std::string& call_id)
{
	auto sep_pos = call_id.find(TRANSCODER_ID_SEPARATOR);
	if (sep_pos != call_id.npos)
		call_id.erase(sep_pos);
}
