#pragma once

#include "VS_ConferenceDescription.h"

#include <ostream>

template <class charT, class traits>
std::basic_ostream<charT, traits>& operator<<(std::basic_ostream<charT, traits>& s, const VS_ConferenceDescription& cd)
{
	s << "stream_id=";
	if (!cd.m_name.IsEmpty())
		s << cd.m_name.m_str;
	s << ", call_id=";
	if (!cd.m_call_id.IsEmpty())
		s << cd.m_call_id.m_str;
	s << ", type=" << cd.m_type << ", sub_type=" << cd.m_SubType;
	s << ", cast=" << cd.m_MaxCast << "x" << cd.m_MaxParticipants;
	s << ", owner=";
	if (!cd.m_owner.IsEmpty())
		s << cd.m_owner.m_str;
	s << ", topic=" << cd.m_topic;
	return s;
}
