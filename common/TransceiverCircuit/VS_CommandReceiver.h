#pragma once
#include "std-generic/compat/memory.h"
#include "TransceiverLib/VS_RelayMessageSenderInterface.h"
#include "TransceiverLib/VS_RelayModulesMgr.h"
#include "TransceiverLib_v2/NetChannelImp.h"
#include "VS_ConfConditionConnector.h"
#include "std-generic/asio_fwd.h"

#include <boost/signals2.hpp>
#include <string>
#include <set>
#include <mutex>

/**
	Class receives data from command transmitter and transmit to up level;
**/

enum VS_Conference_Type : int;
enum VS_GroupConf_SubType : int;
class VS_NetworkRelayMessageBase;
class VS_MainRelayMessage;
class VS_ConfControlInterface;
class VS_MediaFormat;

class VS_CommandReceiver : public vs::enable_shared_from_this<VS_CommandReceiver>,
	  public VS_ConfConditionConnector,
	  public VS_RelayModulesMgr,
	  public VS_RelayMessageSenderInterface
{
public:
	typedef boost::signals2::signal<void (bool /**result*/)>	ServerConnectSignal;
	typedef ServerConnectSignal::slot_type	ServerConnectSlot;

	virtual ~VS_CommandReceiver();
	bool ConnectToServer(std::function<void()>&& onConnDie = nullptr);
	bool ConnectToServerAsync(const ServerConnectSlot& connectSlot);

	void ConnectToAllConfControl(const std::shared_ptr<VS_ConfControlInterface> &conf_ctrl_cb);
	void DisconnectFromAllConfControl(const std::shared_ptr<VS_ConfControlInterface>	&conf_ctrl_cb);

	void ConnectToConfControl(const char *conf_name, const std::shared_ptr<VS_ConfControlInterface> &conf_ctrl_cb) override;
	void DisconnectFromConfControl(const char *conf_name, const std::shared_ptr<VS_ConfControlInterface>	&conf_ctrl_cb) override;

////VS_RelayModulesMgr
	virtual bool RegisterModule(const std::shared_ptr<VS_RelayModule> &module);
	virtual void UnregisterModule(const std::shared_ptr<VS_RelayModule> &module);

protected:
	VS_CommandReceiver(const std::string &addrs, std::string circuit_name, const unsigned char *secretData, const unsigned long sz, boost::asio::io_service& ios);
	static void PostConstruct(std::shared_ptr<VS_CommandReceiver>& p)
	{
		p->m_sender_interface = p;
	}

public:
	std::shared_ptr<ts::NetChannelInterface>& GetNetChannel();

	bool SendMsg(const boost::shared_ptr<VS_NetworkRelayMessageBase>& mess) override;
	net::address GetRemoteAddress() const override;
private:
	std::weak_ptr<VS_RelayMessageSenderInterface>		m_sender_interface;
	std::shared_ptr<ts::NetChannelInterface>			m_net_channel;

	typedef boost::signals2::signal<void (const char *, const char *, VS_Conference_Type, VS_GroupConf_SubType)> StartConferenceSignal;
	typedef boost::signals2::signal<void (const char *)> StopConferenceSignal;
	typedef boost::signals2::signal<void (const char *, const char *)> ParticipantConnectSignal;
	typedef boost::signals2::signal<void (const char *, const char *)> ParticipantDisconnectSignal;
	typedef boost::signals2::signal<void (const char *, const char *, const void *, const unsigned long)> SetParticipantCapsSignal;
	typedef boost::signals2::signal<void (const char *, const char *, const long)> UpdateFiltrSignal;
	typedef boost::signals2::signal<void (const char *, const char *, long, long, long)> RestrictBitrateSVCSignal;
	typedef boost::signals2::signal<void(const char *, const char *)> RequestKeyFrameSignal;



	StartConferenceSignal				m_fireConferenceStart;
	StopConferenceSignal				m_fireConferenceStop;
	ParticipantConnectSignal			m_fireParticipantConnect;
	ParticipantDisconnectSignal			m_fireParticipantDisconnect;
	SetParticipantCapsSignal			m_fireSetPartCaps;
	RestrictBitrateSVCSignal			m_fireRestrictBitrateSVC;
	RequestKeyFrameSignal				m_fireRequestKeyFrame;


	struct ConfConditionsSignals
	{
		StartConferenceSignal				fireStartConf;
		StopConferenceSignal				fireStopConf;
		ParticipantConnectSignal			firePartConnect;
		ParticipantDisconnectSignal			firePartDisconnect;
		SetParticipantCapsSignal			fireSetPartCaps;
		RestrictBitrateSVCSignal			fireRestrictBitrateSVC;
		RequestKeyFrameSignal				fireRequestKeyFrame;
	};
	struct ConfCtrlConnections
	{
		void DisconnectAll()
		{
			startConfConn.disconnect();
			stoptConfConn.disconnect();
			partConnectConn.disconnect();
			partDisconnectConn.disconnect();
			setPartCapsConn.disconnect();
			restrictBitrateSVCConn.disconnect();
			requestKeyFrameConn.disconnect();
		}
		boost::signals2::connection	startConfConn;
		boost::signals2::connection	stoptConfConn;
		boost::signals2::connection	partConnectConn;
		boost::signals2::connection	partDisconnectConn;
		boost::signals2::connection setPartCapsConn;
		boost::signals2::connection restrictBitrateSVCConn;
		boost::signals2::connection requestKeyFrameConn;
	};

	std::map<std::string,boost::shared_ptr<ConfConditionsSignals>>	m_confConditionsSignals;
	std::map<std::tuple<std::string, VS_ConfControlInterface*>, ConfCtrlConnections> m_connsCollection;

	boost::shared_ptr<ConfConditionsSignals>	GetConfConditionsSignals(const char *conf_name);

	template <class Protocol>
	bool Connect(std::function<void()>&& onConnDie);

	virtual void ProcessingRcvMessage(boost::shared_ptr<VS_MainRelayMessage>& recvMess);

	void StartConference(const char *conf_name, const char *owner_name, VS_Conference_Type type, VS_GroupConf_SubType sub_type);
	void StopConference(const char *conf_name);
	void ParticipantConnect(const char *conf_name, const char *part_name);
	void ParticipantDisconnect(const char *conf_name, const char *part_name);
	void SetPartCaps(const char *conf_name, const char *part_name,const void *caps_buf,const unsigned long caps_len);
	void RestrictBitrateSVC(const char *conferenceName, const char *participantName, long v_bitrate, long bitrate, long old_bitrate);
	void RequestKeyFrame(const char *conferenceName, const char *participantName);

	std::string m_serverAddrList;
	const std::string					m_circuit_name;
	std::vector<unsigned char>			m_secretData;
	bool								m_isConnectInProgress;
	std::mutex							m_lock;
	boost::asio::io_service&			m_ios;
};
