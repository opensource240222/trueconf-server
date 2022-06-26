#pragma once

#include "ConferencesConditions.h"

#include <mutex>
#include <vector>

namespace stream {

class ConferencesConditionsSplitter : public ConferencesConditions
{
public:
	// ConferencesConditions
	void CreateConference(const char* conf_name) override;
	void RemoveConference(const char* conf_name, const ConferenceStatistics& cs) override;
	void AddParticipant(const char* conf_name, const char* part_name) override;
	void RemoveParticipant(const char* conf_name, const char* part_name, const ParticipantStatistics& ps) override;
	int RestrictBitrate(const char* conf_name, const char* part_name, int bitrate) override;
	int RestrictBitrateSVC(const char* conf_name, const char* part_name, int v_bitrate, int bitrate, int old_bitrate) override;
	void RequestKeyFrame(const char* conf_name, const char* part_name) override;
	void SetParticipantCaps(const char* conf_name, const char* part_name, const void* caps_buf, size_t buf_sz) override;

	bool ConnectToSplitter(ConferencesConditions* conf_cond);
	void DisconnectFromSplitter(ConferencesConditions* conf_cond);

private:
	std::vector<ConferencesConditions*> GetConditions() const;

private:
	mutable std::mutex m_mutex;
	std::vector<ConferencesConditions*> m_conditions;
};

}
