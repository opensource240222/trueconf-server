#include "VS_WebrtcRelayMsg.h"

const char VS_WebrtcRelayMsg::module_name[] = "WebrtcPeerConnectionRelayModule";

namespace {
#ifdef _SVKS_M_BUILD_
	const char* TrueConfID_PARAM = "SvksID";
#else
	const char* TrueConfID_PARAM = "TrueConfID";
#endif
}

VS_WebrtcRelayMsg::VS_WebrtcRelayMsg() : VS_EnvelopeRelayMessage(module_name)
{

}

VS_WebrtcRelayMsg::~VS_WebrtcRelayMsg()
{

}


bool VS_WebrtcRelayMsg::MakeWebrtcMsgImp(WebRtcMessageType msg_type, const char* peer_id, const char* conf_id, const char *data_ptr, unsigned long data_sz, const char* trueconf_id, const char* limit_n_peers, bool receive_from_peer)
{
	ClearContainer();
	if(	!SetParam("Type",(int32_t)msg_type) ||
		!SetParam("PeerID",peer_id) ||
		!SetParam("ConfID",conf_id) ||
		!SetParam("ReceiveFromPeer", receive_from_peer)) return false;

	if(data_sz && data_ptr && strlen(data_ptr)<10000){
		if(!SetParam("SDP",data_ptr)) return false;
		if(msg_type==e_candidate){
			const char* buf = data_ptr+strlen(data_ptr)+1;
			if(!SetParam("Mid",buf)) return false;
			if(!SetParam("MLineIndex",*((int32_t*)(buf+strlen(buf)+1)))) return false;
		}
	}
	if(!!trueconf_id)
		if(!SetParam(TrueConfID_PARAM,trueconf_id)) return false;
	if(!!limit_n_peers)
		if(!SetParam("Limit",limit_n_peers)) return false;
	return Make();
}

bool VS_WebrtcRelayMsg::MakeWebrtcMsg(WebRtcMessageType msg_type, const char* peer_id, const char* conf_id, const char *data, const char* trueconf_id, const char* limit_n_peers, bool receive_from_peer)
{
	return MakeWebrtcMsgImp(msg_type,peer_id,conf_id,data, (data)? strlen(data)+1: 0, trueconf_id, limit_n_peers, receive_from_peer);
}

bool VS_WebrtcRelayMsg::MakeCandidate(const char* peer_id, const char* conf_id, const char *sdp, const char* Mid, unsigned long MLineIndex)
{
	char buff[8192] = {0};
	char* ptr = &buff[0];
	int len = sizeof(buff);

	// sdp
	int l = strlen(sdp);
	if (l>len)
		return false;
	memcpy(&buff[0], sdp, l);
	len -= l;
	ptr += l+1;

	// Mid
	l = strlen(Mid);
	if (l>len)
		return false;
	memcpy(ptr, Mid, l);
	len -= l;
	ptr += l+1;

	// MLineIndex
	l = sizeof(uint32_t);
	if (l>len)
		return false;

	memcpy(ptr, &MLineIndex, sizeof(uint32_t));
	len -= l;
	ptr += l+1;

	MakeWebrtcMsgImp(e_candidate, peer_id, conf_id, buff, ptr-&buff[0]);
	return true;
}


bool VS_WebrtcRelayMsg::MakeCandidateComplete(const char* peer_id, const char* conf_id)
{
	MakeWebrtcMsgImp(e_candidate_complete, peer_id, conf_id, 0, 0);
	return true;
}

bool VS_WebrtcRelayMsg::MakeAnyData(const char* peer_id, const char* conf_id, const char* data)
{
	if (!peer_id || !conf_id || !data)
		return false;
	MakeWebrtcMsgImp(e_anydata, peer_id, conf_id, data, strlen(data) + 1);
	return true;
}



uint32_t VS_WebrtcRelayMsg::GetWebrtcMsgType() const
{
	int32_t type;
	if(GetParam("Type",type))
		return (uint32_t)type;
	return 0;
}

const char * VS_WebrtcRelayMsg::GetPeerID() const
{
	return GetStrValRef("PeerID");
}

const char * VS_WebrtcRelayMsg::GetConferenceName() const
{
	return GetStrValRef("ConfID");
}

const char * VS_WebrtcRelayMsg::GetSDP() const
{
	return GetStrValRef("SDP");
}

const char * VS_WebrtcRelayMsg::GetCandidate_Mid() const
{
	return GetStrValRef("Mid");
}

uint32_t VS_WebrtcRelayMsg::GetCandidate_MLineIndex() const
{
	int32_t MLineIndex;
	if (!GetParam("MLineIndex", MLineIndex))
		return 0;
	return MLineIndex;
}

const char * VS_WebrtcRelayMsg::GetTrueConfID() const
{
	return GetStrValRef(TrueConfID_PARAM);
}

const char * VS_WebrtcRelayMsg::GetLimitNPeers() const
{
	return GetStrValRef("Limit");
}

bool VS_WebrtcRelayMsg::GetReceiveFromPeer() const
{
	bool res = false;
	GetParam("ReceiveFromPeer", res);
	return res;
}

int VS_WebrtcRelayMsg::GetMinPort() const
{
	int32_t val;
	if (!GetParam("min_port", val))
		return 0;
	return val;
}

int VS_WebrtcRelayMsg::GetMaxPort() const
{
	int32_t val;
	if (!GetParam("max_port", val))
		return 0;
	return val;
}