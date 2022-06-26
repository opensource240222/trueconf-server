#include "VS_ConferenceDescription.h"
#include "../cpplib/VS_RegistryConst.h"
#include "../cpplib/VS_RegistryKey.h"

const char MMC_TYPE_TAG[] = "Type";
const char MMC_WEBPARAMS_KEY[] = "Web";
const char MMC_WEBPARAMS_PERMISSIONS[] = "Permissions";

// 0x00000200 - 1 spatial + 3 temporal layers
// 0x00010100 - 2 spatial + 2 temporal layers
// 0x00030100 - (3 spatial + 2 temporal layers) & (2 spatial + 2 temporal layers)
// 0x00070100 - (3 spatial + 2 temporal layers) & (2 spatial + 2 temporal layers) + decrease MB threshold spatial layers

#define SVC_MODE_VERSION 0x00070100

VS_ConferenceDescription::VS_ConferenceDescription(void) :
m_owner(0), m_state(CONFERENCE_CREATED), m_MaxParticipants(0),
m_type(-1),m_SubType(0), m_logCause(UNKNOWN), m_MaxCast(0), m_public(false), m_LstatusFlag(0), m_isBroadcastEnabled(false),
m_need_record(false), m_PlannedPartsOnly(false), m_svc_mode(SVC_MODE_VERSION), m_PrivacyType(e_PT_Private)
{
	m_CMR_guest = m_CMR_user = CMRFlags::ALL;
}

void VS_ConferenceDescription::SplitConfID(const vs_conf_id & conf_id, int & conference, std::string& OUT_broker_id)
{
	if (conf_id.Length()<15) {
		conference = 0;
		OUT_broker_id.clear();
	}
	else {
		if (sscanf(conf_id, "%08x", (unsigned int*)&conference) != 1) {
			conference = 0;
			OUT_broker_id.clear();
		}
		else
			OUT_broker_id = (const char*)conf_id + 9;
	}
}

bool VS_ConferenceDescription::GetMultyConfInfo(const VS_SimpleStr& call_id, bool& OUT_isPublic, int32_t& OUT_guest_flags)
{
	if (!call_id) return false;

	std::string keyName;
	keyName += MULTI_CONFERENCES_KEY;
	keyName += '\\';
	keyName += call_id;
	keyName += '\\';
	keyName += MMC_WEBPARAMS_KEY;

	const int size = 256;
	char buff[size] = { 0 };

	VS_RegistryKey confKey(false, keyName);
	if (confKey.GetValue(buff, sizeof(buff), VS_REG_STRING_VT, MMC_TYPE_TAG) <= 0) return false;
	OUT_isPublic = (strtol(buff, nullptr, 10) == 1L);


	memset(buff, 0, sizeof(buff));
	if (confKey.GetValue(buff, sizeof(buff), VS_REG_STRING_VT, MMC_WEBPARAMS_PERMISSIONS) <= 0) return false;
	OUT_guest_flags = strtol(buff, nullptr, 10);

	return true;
}

void VS_ConferenceDescription::SetTimeExp(long duration)
{
	m_timeExp = std::chrono::system_clock::now() + std::chrono::seconds(duration);
}
