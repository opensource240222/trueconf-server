#pragma once
#include "VS_EnvelopeRelayMessage.h"

enum WebRtcMessageType : uint32_t
{
	e_ConnectPeerToConference,		// todo(kt): delete
	e_offer,
	e_answer,
	e_candidate,
	e_candidate_complete,
	e_bye,
	e_anydata // char* data in SDP param
};

class VS_WebrtcRelayMsg: public VS_EnvelopeRelayMessage
{
public:
	const static char module_name[];

	VS_WebrtcRelayMsg();
	virtual ~VS_WebrtcRelayMsg();

	uint32_t GetWebrtcMsgType() const;
	const char *GetPeerID() const;
	/**
		TODO: method from VS_MainRelayMessage.
	*/
	const char *GetConferenceName() const;
	const char *GetSDP() const;
	const char *GetTrueConfID() const;
	const char *GetLimitNPeers() const;

	const char *GetCandidate_Mid() const;
	uint32_t GetCandidate_MLineIndex() const;
	bool GetReceiveFromPeer() const;
	int GetMinPort() const;
	int GetMaxPort() const;

	bool MakeWebrtcMsg(WebRtcMessageType msg_type, const char* peer_id, const char* conf_id, const char *data=0, const char* trueconf_id=0, const char* limit_n_peers=0, bool receive_from_peer = false);
	bool MakeCandidate(const char* peer_id, const char* conf_id, const char *sdp, const char* Mid, unsigned long MLineIndex);
	bool MakeCandidateComplete(const char* peer_id, const char* conf_id);
	bool MakeAnyData(const char* peer_id, const char* conf_id, const char* data);

private:

	bool MakeWebrtcMsgImp(WebRtcMessageType msg_type, const char* peer_id, const char* conf_id, const char *data_ptr, unsigned long data_sz, const char* trueconf_id=0, const char* limit_n_peers=0, bool receive_from_peer = false);
};