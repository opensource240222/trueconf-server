#pragma once

#include "TransceiverLib/VS_RelayModule.h"
#include "streams/Relay/VS_ConfControlInterface.h"
#include "std-generic/cpplib/StrCompare.h"
#include "std-generic/asio_fwd.h"

#include "std-generic/compat/map.h"
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <vector>

class VS_RTPSessionInterface;
class VS_RTPModuleParameters;
class VS_FFLSourceCollection;
class VS_TransceiverPartsMgr;
class VS_MediaSourceCollection;
struct VS_MediaChannelInfo;
class VS_ClientCaps;
enum FakeVideo_Mode : int;
enum eSDP_ContentType : int;
enum VS_GroupConf_SubType : int;
struct SlideInfo;

class VS_RTPModuleReceiver : public VS_RelayModule, public VS_ConfControlInterface
{
public:
	VS_RTPModuleReceiver(boost::asio::io_service& ios, const boost::shared_ptr<VS_MediaSourceCollection>& collection, const std::shared_ptr<VS_TransceiverPartsMgr>& partsMgr);
	virtual ~VS_RTPModuleReceiver();

	virtual bool ProcessingMessage(const boost::shared_ptr<VS_NetworkRelayMessageBase>& msg);

private:
	void CreateSession(string_view id, string_view part_id, string_view sess_key);
	void DestroySession(string_view id);
	void SetConference(string_view id, string_view conf_name, string_view part_id, string_view owner, VS_GroupConf_SubType subtype, const VS_ClientCaps& conference_caps);
	void SetMediaChannels(string_view id, const std::vector<VS_MediaChannelInfo>& media_channels);
	void FullIntraframeRequest(string_view id, bool from_rtp);	// from_rtp == true => TC requests key frame from RTP (sip/h323), from_rtp == false => SIP/H323 requests key frame from TC;
	void SetFakeVideoMode(string_view id, FakeVideo_Mode mode);
	void ShowSlide(string_view id, const char* url);
	void SelectVideo(string_view id, eSDP_ContentType content);
	void PauseAudio(string_view id);
	void ResumeAudio(string_view id);
	void PauseVideo(string_view id);
	void ResumeVideo(string_view id);
	void ContentForward_Pull(string_view id);
	void ContentForward_Push(string_view id);
	void ContentForward_Stop(string_view id);
	void FarEndCameraControl(string_view id, eFeccRequestType type, int32_t extra_param);

	bool ClearOldSessions();

	std::shared_ptr<VS_RTPSessionInterface> GetSession(string_view id);
	std::shared_ptr<VS_RTPSessionInterface> GetSession(const char* conf_name, const char* part_id);

	void StartConference(const char* conf_name) override;
	void StartConference(const char* conf_name, const char* owner_name, VS_Conference_Type type, VS_GroupConf_SubType sub_type) override;
	void StopConference(const char* conf_name) override;
	void ParticipantConnect(const char* conf_name, const char* part_name) override;
	void ParticipantDisconnect(const char* conf_name, const char* part_name) override;
	void SetParticipantCaps(const char* conf_name, const char* part_name, const void *caps_buf, const unsigned long buf_sz) override;
	void RestrictBitrateSVC(const char* conferenceName, const char* participantName, long v_bitrate, long bitrate, long old_bitrate) override;
	void RequestKeyFrame(const char *conf_name, const char *part_name) override;

private:
	boost::asio::io_service& m_ios;
	std::set<std::shared_ptr<VS_RTPSessionInterface>> m_to_remove;
	vs::map<std::string, std::shared_ptr<VS_RTPSessionInterface>, vs::str_less> m_sessions;
	std::map<std::tuple<std::string/*conf_name*/, std::string/*part_id*/>, std::weak_ptr<VS_RTPSessionInterface>> m_sessions_by_part;

	std::shared_ptr<VS_RTPModuleParameters> m_parameters;
	std::shared_ptr<VS_TransceiverPartsMgr> m_partsMgr ;
	std::shared_ptr<VS_FFLSourceCollection> m_source_collection;

	std::mutex m_mutex;
	bool m_deleting;
};
