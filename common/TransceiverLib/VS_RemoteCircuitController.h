#pragma once
#include "VS_RelayModulesMgr.h"
#include "streams/Relay/VS_ConfControlInterface.h"
#include "net/Port.h"
#include "std-generic/cpplib/atomic_shared_ptr.h"

#include <atomic>
#include <memory>

/**
	For control remote circuit process
*/
enum VS_Conference_Type : int;
enum VS_GroupConf_SubType : int;
class VS_MainRelayMessage;
namespace ts { class NetChannelInterface; }

class VS_RemoteCircuitController :	public VS_RelayModulesMgr,
									public VS_ConfControlInterface
{
private:
	const std::shared_ptr<ts::NetChannelInterface> m_transceiver_net_channel;
	std::atomic<net::port> m_live555_port;
	vs::atomic_shared_ptr<std::string> m_live555_secret;

public:
	VS_RemoteCircuitController(std::shared_ptr<ts::NetChannelInterface> ts_net_channel);
	virtual ~VS_RemoteCircuitController();

	void Stop();
	net::port GetLive555Port() const;
	std::string GetLive555Secret() const;

	// VS_ConfControlInterface
	void StartConference(const char* conf_name) override;
	void StartConference(const char* conf_name, const char* owner_name, VS_Conference_Type type, VS_GroupConf_SubType sub_type) override;
	void StopConference(const char* conf_name) override;
	void ParticipantConnect(const char* conf_name, const char* part_name) override;
	void ParticipantDisconnect(const char* conf_name,const char* part_name) override;
	void SetParticipantCaps(const char* conf_name, const char* part_name, const void* caps_buf, const unsigned long buf_sz) override;
	void RestrictBitrateSVC(const char* conferenceName, const char* participantName, long v_bitrate, long bitrate, long old_bitrate) override;
	void RequestKeyFrame(const char* conferenceName, const char* participantName) override;
};