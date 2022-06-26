#include "VS_WebrtcPeerConnectionService.h"
#include "TransceiverLib/VS_WebrtcRelayMsg.h"
#include "tools/Server/VS_Server.h"
#include "std/cpplib/json/writer.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/VS_Protocol.h"
#include "std/debuglog/VS_Debug.h"
#include "TransceiverLib/TransceiversPool.h"

#include <boost/make_shared.hpp>

#define DEBUG_CURRENT_MODULE VS_DM_WEBRTC_PEERCONNECTION

namespace
{
#ifdef _SVKS_M_BUILD_
	const char* TrueConfID_PARAM = "svks_id";
#else
	const char* TrueConfID_PARAM = "trueconf_id";
#endif
}

VS_WebrtcPeerConnectionService::VS_WebrtcPeerConnectionService(const std::weak_ptr<ts::IPool> &pool)
	: VS_RelayModule(VS_WebrtcRelayMsg::module_name)
	, m_transcPool(pool)
{
}

VS_WebrtcPeerConnectionService::~VS_WebrtcPeerConnectionService()
{
}

bool VS_WebrtcPeerConnectionService::Init()
{

	VS_JsonRequestHandler::SetInstance(std::static_pointer_cast<VS_JsonRequestHandler>(shared_from_this()));
	return true;
}

std::shared_ptr<VS_TransceiverProxy> GetProxy(const std::weak_ptr<ts::IPool> &trPool, const std::string& conf_id) {
	auto pool = trPool.lock();
	if (!pool) return nullptr;
	return pool->GetTransceiverProxy(conf_id);
}

void VS_WebrtcPeerConnectionService::ProcessJsonRequest(json::Object& rootObj)
{
	std::string my_peer_id;
	std::string conf_id;
	std::string sdp;
	std::string candidate;
	std::string type("");
	std::string sdpMid;
	unsigned long sdpMLineIndex(0);
	std::string trueconf_id;			// which participant from conf_id to get
	std::string ip;
	std::string browser = "no_user_agent_header";
	std::string method;
	auto msg = boost::make_shared<VS_WebrtcRelayMsg>();

	try
	{
		json::Object::const_iterator it;
		it = rootObj.Find("method");
		if (it != rootObj.End()) method = (const json::String) it->element;
		else throw false;

		it = rootObj.Find("conf_id");
		if (it != rootObj.End()) conf_id = (const json::String) it->element;
		else throw false;

		it = rootObj.Find("my_peer_id");
		if (it != rootObj.End()) my_peer_id = (const json::String) it->element;
		else throw false;


		if (method == "webrtc") {
			it = rootObj.Find("type");
			if (it != rootObj.End()) type = (const json::String) it->element;
			else throw false;

			it = rootObj.Find("sdp");
			if (it != rootObj.End()) sdp = (const json::String) it->element;

			it = rootObj.Find("candidate");
			if (it != rootObj.End()) candidate = (const json::String) it->element;

			it = rootObj.Find("sdpMid");
			if (it != rootObj.End()) sdpMid = (const json::String) it->element;

			it = rootObj.Find("sdpMLineIndex");
			if (it != rootObj.End()) sdpMLineIndex = (unsigned long)(const json::Number) it->element;

			it = rootObj.Find(TrueConfID_PARAM);
			if (it != rootObj.End()) trueconf_id = (const json::String) it->element;

			it = rootObj.Find("ip");
			if (it != rootObj.End()) ip = (const json::String) it->element;

			it = rootObj.Find("browser");
			if (it != rootObj.End()) browser = (const json::String) it->element;
		}
	}
	catch (json::Exception &)
	{
		dprint2("Bad json params");
		return;
	}
	catch (bool)
	{
		dprint3("Too few params in json query");
		return;
	}

	if (!VS_RelayModule::HasSender(conf_id)) {
		auto proxy = GetProxy(m_transcPool, conf_id);
		if (!proxy) return;

		proxy->SetRemoveFromSender([conf_id, w_self = this->weak_from_this()]() {
			if (auto self = w_self.lock())	self->RemoveMessageSender(conf_id);
		});
		VS_RelayModule::SetMessageSender(proxy, conf_id);
		proxy->RegisterModule(std::static_pointer_cast<VS_RelayModule>(shared_from_this()));


	}

	if (method == "webrtc") {
		dprint3("type %s from %s\n", type.c_str(), my_peer_id.c_str());

		if (type == "offer") {
			bool is_input_stream_present = my_peer_id.find('@') != std::string::npos;

			msg->MakeWebrtcMsg(e_offer, my_peer_id.c_str(), conf_id.c_str(), sdp.c_str(), trueconf_id.c_str(), "1000", is_input_stream_present);

			VS_RegistryKey key(false, CONFIGURATION_KEY);
			if (key.IsValid()) {
				int32_t min_port(DEFAULT_WEBRTC_MINPORT);
				int32_t max_port(DEFAULT_WEBRTC_MAXPORT);
				std::string external_ip;
				key.GetValue(&min_port, sizeof(min_port), VS_REG_INTEGER_VT, "WebRTC MinPort");
				key.GetValue(&max_port, sizeof(max_port), VS_REG_INTEGER_VT, "WebRTC MaxPort");
				key.GetString(external_ip, "WebRTC ExternalIP");
				bool changed(false);

				if (min_port && max_port && min_port <= max_port) {
					msg->SetParam("min_port", min_port);
					msg->SetParam("max_port", max_port);
					changed = true;
				}
				if (!external_ip.empty()) {
					msg->SetParam("external_ip", external_ip.c_str());
					changed = true;
				}
				if (changed)
					msg->Make();
			}

			dprint4("conf=%s peer=%s ip=%s browser=%s\n", conf_id.c_str(), my_peer_id.c_str(), ip.c_str(), browser.c_str());
		}
		else if (type == "answer") {
			msg->MakeWebrtcMsg(e_answer, my_peer_id.c_str(), conf_id.c_str(), sdp.c_str());
		}
		else if (type == "candidate") {
			dstream4 << "conf=" << conf_id << ", peer=" << my_peer_id << ", " << candidate << " (sdpMid=" << sdpMid << ", sdpMLineIndex=" << sdpMLineIndex << ")";
			msg->MakeCandidate(my_peer_id.c_str(), conf_id.c_str(), candidate.c_str(), sdpMid.c_str(), sdpMLineIndex);
		}
		else if (type == "bye") {
			msg->MakeWebrtcMsg(e_bye, my_peer_id.c_str(), conf_id.c_str());
			// bug#41351 disconnect peer channel at logout/error only
			//if (!m_handleThread->IsCurrent()) {
			//	boost::shared_ptr<VS_MessageData> mess(new MSG_PEER_DISCONNECTED(my_peer_id));
			//	m_handleThread->Post(m_this.lock(), mess);
			//}
		}
	}
	else if (method == MANAGELAYOUT_METHOD || method == SENDCOMMAND_METHOD){
		std::stringstream ss;
		json::Writer::Write(rootObj, ss);
		if (ss.str().length() > 0) {
			msg->MakeAnyData(my_peer_id.c_str(), conf_id.c_str(), ss.str().c_str());
		}
	}

	if (msg->IsValid())
		VS_RelayModule::SendMsg(msg,conf_id);
}

bool VS_WebrtcPeerConnectionService::ProcessingMessage(const boost::shared_ptr<VS_NetworkRelayMessageBase> &mess)
{
	boost::shared_ptr<VS_WebrtcRelayMsg> wrtc_mess(new VS_WebrtcRelayMsg);
	if (!wrtc_mess->SetMessage(mess->GetMess()))
		return false;
	auto conf_name = wrtc_mess->GetConferenceName();
	assert(conf_name != nullptr);
	dprint3("conf2web: conf=%s, peer_id=%s, type=%u\n", conf_name, wrtc_mess->GetPeerID(), wrtc_mess->GetWebrtcMsgType());

	std::string peer_id = wrtc_mess->GetPeerID();
	json::Object obj;
	auto msg_type = wrtc_mess->GetWebrtcMsgType();
	if (msg_type == e_answer) {
		obj["type"] = json::String("answer");
		obj["sdp"] = json::String(wrtc_mess->GetSDP());
	} else if (msg_type == e_candidate) {
		obj["type"] = json::String("candidate");
		obj["candidate"] = json::String(wrtc_mess->GetSDP());
		obj["sdpMid"] = json::String(wrtc_mess->GetCandidate_Mid());
		obj["sdpMLineIndex"] = json::Number(wrtc_mess->GetCandidate_MLineIndex());
	} else if (msg_type == e_candidate_complete) {
		obj["type"] = json::String("candidate_complete");
	} else if (msg_type == e_bye) {
		obj["type"] = json::String("bye");
		const char* reason = wrtc_mess->GetSDP();
		if (reason&&*reason)
			obj["reason"] = json::String(reason);
	}
	else if (msg_type == e_anydata) {
		const char* data = wrtc_mess->GetSDP(); // json in string
		if (!data)
			return false;

		std::string	sd(data);
		std::stringstream ss(sd);
		json::Reader::Read(obj, ss);
	}
	else {
		dprint0("invalid webrtc msg type\n");
		return false;
	}

	JsonCallback cb;
	{
		auto lock = m_peers.lock();
		auto it = lock->find(peer_id);
		if (it != lock->end()) {
			cb = it->second;
		}
	}
	if (cb)
		cb(std::move(obj));
	return true;
}

void VS_WebrtcPeerConnectionService::Shutdown()
{
	VS_JsonRequestHandler::SetInstance(nullptr);
}

void VS_WebrtcPeerConnectionService::DeleteWebPeerCallback(string_view peerId)
{
	m_peers->erase(std::string(peerId));
}

void VS_WebrtcPeerConnectionService::RegisterWebPeerCallback(string_view peerId, const JsonCallback& cb)
{
	m_peers->emplace(peerId, cb);
}
