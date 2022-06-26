#include "ConferencesConditionsSplitter.h"

#include <algorithm>
#include <cassert>

namespace stream {

std::vector<ConferencesConditions*> ConferencesConditionsSplitter::GetConditions() const
{
	std::lock_guard<decltype(m_mutex)> lock(m_mutex);
	return m_conditions;
}

void ConferencesConditionsSplitter::CreateConference(const char* conf_name)
{
	for (const auto& c : GetConditions())
		c->CreateConference(conf_name);
}

void ConferencesConditionsSplitter::RemoveConference(const char* conf_name, const stream::ConferenceStatistics& cs)
{
	for (const auto& c : GetConditions())
		c->RemoveConference(conf_name, cs);
}

void ConferencesConditionsSplitter::AddParticipant(const char* conf_name, const char* part_name)
{
	for (const auto& c : GetConditions())
		c->AddParticipant(conf_name, part_name);
}

void ConferencesConditionsSplitter::RemoveParticipant(const char* conf_name, const char* part_name, const stream::ParticipantStatistics& ps)
{
	for (const auto& c : GetConditions())
		c->RemoveParticipant(conf_name, part_name, ps);
}

int ConferencesConditionsSplitter::RestrictBitrate(const char* conf_name, const char* part_name, int bitrate)
{
	int result = 0;
	auto conditions = GetConditions();
	for (auto it = conditions.rbegin(), it_end = conditions.rend(); it != it_end; ++it)
		result = (*it)->RestrictBitrate(conf_name, part_name, bitrate);
	return result;
}

int ConferencesConditionsSplitter::RestrictBitrateSVC(const char* conf_name, const char* part_name, int v_bitrate, int bitrate, int old_bitrate)
{
	int result = 0;
	auto conditions = GetConditions();
	for (auto it = conditions.rbegin(), it_end = conditions.rend(); it != it_end; ++it)
		result = (*it)->RestrictBitrateSVC(conf_name, part_name, v_bitrate, bitrate, old_bitrate);
	return result;
}

void ConferencesConditionsSplitter::RequestKeyFrame(const char* conf_name, const char* part_name)
{
	for (const auto& c : GetConditions())
		c->RequestKeyFrame(conf_name, part_name);
}

void ConferencesConditionsSplitter::SetParticipantCaps(const char* conf_name, const char* part_name, const void* caps_buf, size_t buf_sz)
{
	for (const auto& c : GetConditions())
		c->SetParticipantCaps(conf_name, part_name, caps_buf, buf_sz);
}

bool ConferencesConditionsSplitter::ConnectToSplitter(stream::ConferencesConditions* conf_cond)
{
	std::lock_guard<decltype(m_mutex)> lock(m_mutex);
	if (std::find(m_conditions.begin(), m_conditions.end(), conf_cond) != m_conditions.end())
		return false;
	m_conditions.push_back(conf_cond);
	return true;
}

void ConferencesConditionsSplitter::DisconnectFromSplitter(stream::ConferencesConditions* conf_cond)
{
	std::lock_guard<decltype(m_mutex)> lock(m_mutex);
	auto it = std::find(m_conditions.begin(), m_conditions.end(), conf_cond);
	if (it == m_conditions.end())
		return;
	m_conditions.erase(it);
	assert(std::find(m_conditions.begin(), m_conditions.end(), conf_cond) == m_conditions.end());
}

}
