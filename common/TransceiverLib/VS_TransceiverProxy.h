#pragma once
#include <boost/weak_ptr.hpp>
#include <memory>
#include <boost/signals2/deconstruct.hpp>
#include <boost/signals2.hpp>
#include "streams/Relay/VS_CircuitStreamRelayInterface.h"
#include "streams/Relay/VS_TransmitFrameInterface.h"
#include "VS_NetworkRelayMessage.h"
#include "VS_RelayMessageSenderInterface.h"
#include "net/Port.h"
#include "std-generic/cpplib/VS_Container.h"
#include "std-generic/cpplib/VS_Container.h"
#include "std/cpplib/fast_mutex.h"

#include <unordered_map>

class VS_TransceiverCircuitHandler;
class VS_RelayModule;
class VS_RemoteCircuitController;
class VS_RTPModuleControl;
class VS_RTPModuleControlInterface;
class VS_ConfControlModule;
class VS_ConfRecorderModuleCtrl;
class VS_RTSPBroadcastModuleCtrl;
class VS_ConferenceDescription;

typedef std::tuple<std::string/*user_id*/, VS_BinBuff/*caps*/, int32_t/*fltr*/, std::string/*display_name*/, int32_t /*role*/> part_start_info;

class VS_TransceiverProxy :	public VS_CircuitStreamRelayInterface,
								public VS_RelayMessageSenderInterface
{

	bool	m_isStarted;

	vs::fast_mutex										m_confControlMtx;
	std::shared_ptr<VS_TransceiverCircuitHandler>		m_RelayHandler;
	std::weak_ptr<VS_RemoteCircuitController>		    m_circuitControl;
	std::weak_ptr<VS_RelayMessageSenderInterface>		m_msgSender;
	std::weak_ptr<VS_TransmitFrameInterface>			m_FrameTrans;
	std::weak_ptr<VS_ConfControlModule>					m_conf_control;
	std::weak_ptr<VS_RTPModuleControl>					m_rtp_control;
	std::shared_ptr<VS_ConfRecorderModuleCtrl>			m_conf_recorder_module;
	std::weak_ptr<VS_RTSPBroadcastModuleCtrl>			m_rtsp_broadcast_module;
	std::string											m_connectedTransceiverName;
	std::function<void(const std::string&)>				m_fireReturnToPool;
	boost::signals2::signal<void()>						m_fireRemoveMeFromSenders;

	///VS_CircuitStreamRelayInterface
	void TransmitFrame(const char *conf_name, const char *part, const stream::FrameHeader *frame_head, const void *frame_data)override;
	void StartConference(const char *conf_name) override;
	void StartConference(const char *conf_name, const char *owner_name, VS_Conference_Type type, VS_GroupConf_SubType sub_type) override;
	void StopConference(const char *conf_name) override;
	void ParticipantConnect(const char *conf_name, const char *part_name) override;
	void ParticipantDisconnect(const char *conf_name, const char *part_name) override;
	void SetParticipantCaps(const char *conf_name, const char *part_name, const void *caps_buf, const unsigned long buf_sz) override;
	void RestrictBitrateSVC(const char *conferenceName, const char *participantName, long v_bitrate, long bitrate, long old_bitrate) override;
	void RequestKeyFrame(const char *conferenceName, const char *participantName) override;

	void StartAndAnounceBroadcast(const std::shared_ptr<VS_RTSPBroadcastModuleCtrl> &broadcaster, const VS_ConferenceDescription& cd) const;
public:

	VS_TransceiverProxy(const std::shared_ptr<VS_RemoteCircuitController>& circuitControl,
		const std::shared_ptr<VS_RelayMessageSenderInterface>& msgSender,
		const std::shared_ptr<VS_TransmitFrameInterface>& frameTrans, const std::shared_ptr<VS_ConfControlModule>& conf_control,
		const std::shared_ptr<VS_RTPModuleControl>& rtp_control);
	virtual ~VS_TransceiverProxy();

	bool SendMsg(const boost::shared_ptr<VS_NetworkRelayMessageBase>& mess) override;
	net::address GetRemoteAddress() const override;

	bool RegisterModule(const std::shared_ptr<VS_RelayModule> &module, bool permanent = false);
	void UnregisterModule(const std::shared_ptr<VS_RelayModule> &module);

	std::shared_ptr<VS_ConfControlModule> ConfControl();
	std::shared_ptr<VS_RTPModuleControlInterface> GetRTPModule();
	void RestoreConferences(const std::vector<VS_ConferenceDescription> &to_restore, const std::unordered_map<std::string, std::vector<part_start_info>> &confs_users);
	void SetConfRecorderModule(const std::shared_ptr<VS_ConfRecorderModuleCtrl> &module);
	void SetRTSPBroadcastModule(const std::shared_ptr<VS_RTSPBroadcastModuleCtrl> &module);
	const std::string& GetTransceiverName() const;
	void SetTransceiverName(const std::string& name);
	net::port GetLive555Port() const;
	std::string GetLive555Secret() const;

	template<class Function>
	void SetReturnProxyToPool(Function && cb) {
		m_fireReturnToPool = std::forward<Function>(cb);
	}

	template<class Function>
	void SetRemoveFromSender(Function && cb) {
		m_fireRemoveMeFromSenders.connect(std::forward<Function>(cb));
	}
};