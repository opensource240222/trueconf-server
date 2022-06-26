#include "VS_JsonClient.h"
#include "FakeClient/VS_ConferenceInfo.h"
#include "VS_JsonRequestHandler.h"
#include "std-generic/cpplib/utf8.h"
#include "std-generic/clib/strcasecmp.h"
#include "../std/cpplib/base64.h"
#include "../std/cpplib/VS_RcvFunc.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/MakeShared.h"
#include "std/debuglog/VS_Debug.h"

#include "../TransceiverLib/VS_ConfControlModule.h"
#include "../TransceiverLib/VS_TransceiverProxy.h"
#include "../tools/Server/VS_Server.h"
#include "../tools/Server/VS_ServerComponentsInterface.h"

#define DEBUG_CURRENT_MODULE VS_DM_FAKE_CLIENT

namespace {
#ifdef _SVKS_M_BUILD_
	const char* TrueConfID_PARAM = "svksId";
#else
	const char* TrueConfID_PARAM = "trueconfId";
#endif
}

VS_JsonClient::VS_JsonClient(std::unique_ptr<VS_FakeEndpoint> endpoint)
	: VS_FakeClient(std::move(endpoint))
	, m_webversion(0)
{
	EnshureClientAlive();
}

VS_JsonClient::~VS_JsonClient(void)
{
	m_endpoint->Stop(); // VS_FakeClient already does this, but we need to do it too because it must be done from the destructor of the most derived class.
}

bool VS_JsonClient::Request(json::Object &req, const std::string &ip)
{
	try
	{
		return ProcessRequest( req, ip );
	}
	catch (std::string)
	{
		return false;
	}
	catch(json::Exception)
	{
		return false;
	}
}

#define GET_STRING_FOROM_JSON(name,str_name,container, def, usedef) std::string name = def;	\
	do { json::Object::const_iterator it = container.Find( #str_name );			\
		if (it != container.End() ) name = (const json::String) it->element;	\
		else if (!usedef) throw std::string( "element " #str_name " is missing in request"); \
	} while( 0 )

#define GET_STRING(name) GET_STRING_FOROM_JSON(name, name, req, "", false)
#define GET_STRING_OR_DEFAULT(name, def) GET_STRING_FOROM_JSON(name, name, req, def, true)

#define GET_BOOLEAN_FROM_JSON(name,str_name,container, def, usedef) bool name = def;	\
	do { json::Object::const_iterator it = container.Find( #str_name );			\
	if (it != container.End() ) name = (const json::Boolean) it->element;	\
		else if (!usedef) throw std::string( "element " #str_name " is missing in request"); \
	} while( 0 )

#define GET_BOOL(name) GET_BOOLEAN_FROM_JSON(name, name, req, false, false)
#define GET_BOOL_OR_DEFAULT(name, def) GET_BOOLEAN_FROM_JSON(name, name, req, def, true)

#define GET_INT_FROM_JSON(name,str_name,container, def, usedef) int name = def;	\
	do { json::Object::const_iterator it = container.Find( #str_name );			\
	if (it != container.End() ) name = (int)(0.5 + (const json::Number) it->element);	\
		else if (!usedef) throw std::string( "element " #str_name " is missing in request"); \
	} while( 0 )

#define GET_INT(name) GET_INT_FROM_JSON(name, name, req, 0, false)
#define GET_INT_OR_DEFAULT(name, def) GET_INT_FROM_JSON(name, name, req, def, true)


bool VS_JsonClient::ProcessRequest(json::Object &req, const std::string &ip)
{
	GET_STRING(CID);
	if (CID != this->CID())
		return false;

	GET_STRING(method);
	if (method == "getState")
	{
		SendCurrentState();
	}
	else if (method == "loginUser")
	{
		GET_STRING(login);
		GET_STRING_OR_DEFAULT(password, "");
		GET_STRING_OR_DEFAULT(md5Password, "");
		GET_STRING_OR_DEFAULT(AppID, "");
		GET_STRING_OR_DEFAULT(Key, "");
		GET_STRING_OR_DEFAULT(browser, "");
		GET_INT_OR_DEFAULT(version, 0);
		m_webversion = version;

		if (browser.length() != 0) {
			this->SetAppProperty("browser", browser);
		}
		this->SetAppId(AppID.c_str());
		{
			VS_AutoLock lock(this);
			m_state.autologin_key = Key;
		}
		this->LoginUserAsync(login, password, md5Password, CT_WEB_CLIENT, ip, std::vector<std::string>());
	}
	else if (method == "logoutUser")
	{
		this->Logout();
	}
	else if (method == "join")
	{
		GET_STRING(conferenceId);
		VS_ClientCaps caps = m_state.default_caps;

		bool res = this->JoinAsync(conferenceId, VS_ConferenceInfo(true, true), caps, "");
		if (!res)
		{
			auto conf_descriptor = vs::MakeShared<VS_ConferenceDescriptor>();
			onConferenceStateChange("error", *conf_descriptor);
		}
	}
	else if (method == "hangup")
	{
		GET_STRING(conferenceId);
		GET_BOOL_OR_DEFAULT(result, false);
		bool res = this->GetCurrentConference()->Hangup(result);
		if (!res)
		{
			auto conf_descriptor = vs::MakeShared<VS_ConferenceDescriptor>();
			onConferenceStateChange("error", *conf_descriptor);
		}
	}
	else if (method == "chat")
	{
		std::string current_to;
		std::string current_stream_conf_id;
		GET_STRING(message);
		try
		{
			GET_STRING(to);
			current_to = to;
		}
		catch (std::string)
		{
			GET_STRING(stream_conf_id);
			current_stream_conf_id = stream_conf_id;
		}

		this->SendChatMessage(current_stream_conf_id, current_to, message);
	}
	else if (method == "command")
	{
		GET_STRING(to);
		GET_STRING(command);
		const char* cmd = command.c_str();
		// recording
		bool allowedSend = strcasecmp(CCMD_RECORD_ACCEPT, cmd) == 0 || strcasecmp(CCMD_RECORD_REJECT, cmd) == 0;
		// slideshow
		if (!allowedSend)
			allowedSend = strncasecmp(SHOW_SLIDE_COMMAND, cmd, strlen(SHOW_SLIDE_COMMAND)) == 0 || strncasecmp(END_SLIDESHOW_COMMAND, cmd, strlen(END_SLIDESHOW_COMMAND)) == 0;
		// webrtc special commands
		if (!allowedSend)
			allowedSend = strncasecmp("webclient:", cmd, 10) == 0;
		// manage devices
		if (!allowedSend) {
			allowedSend = strncasecmp(CCMD_CAM_ON, cmd, 10) == 0 || strncasecmp(CCMD_MIC_ON, cmd, 10) == 0; //"remote:cam",	"remote:mic"
			bool IsLeader = GetCurrentConference()->m_role == PR_LEADER;
			allowedSend = allowedSend && IsLeader;
		}
		if (allowedSend)
			this->SendChatCommand(to, command);
	}
	else if (method == "AnswerReqInvite") {
		GET_STRING(UserName);
		GET_BOOL(accept);
		this->AnswerReqInvite(UserName, accept);
	}
	else if (method == "queryRole")
	{
		GET_STRING(UserName);
		GET_INT(role);
		this->QueryRole(UserName.c_str(), role);
	}
	else if (method == "answerRole")
	{
		GET_STRING(UserName);
		GET_INT(role);
		GET_INT(result);
		auto it = m_rolequery.find(UserName);
		if (it != m_rolequery.end() && it->second == role) {
			this->AnswerRole(UserName.c_str(), role, result);
			m_rolequery.erase(it);
		}
	}
	else if (method == "kick")
	{
		GET_STRING(UserName);
		this->KickFromConference(UserName);
	}
	else if (method == "takePodium")
	{
		GET_BOOL(podium);
		long fltr = podium ? VS_RcvFunc::FLTR_DEFAULT_MULTIS : 0;
		auto cd = GetCurrentConference();
		if (cd->m_role == PR_LEADER) {
			this->ConnectSender(fltr);
		}
	}
	else if (method == "webrtc")
	{
		req["ip"] = json::String(ip);
		auto p = VS_JsonRequestHandler::GetInstance();
		if (p) p->ProcessJsonRequest(req);
		{
			// example for layout message
			//GET_STRING(type);
			//GET_STRING(my_peer_id);
			//GET_STRING(conf_id);
			//if (type == "offer")
			//{
			//	json::Object l;
			//	l["method"] = json::String(MANAGELAYOUT_METHOD);
			//	l["func"] = json::String("SetP1");
			//	json::Array list;
			//	list.Insert(json::String(my_peer_id));
			//	l["UserName"] = list;
			//	l["conf_id"] = json::String(conf_id);
			//	l["my_peer_id"] = json::String(my_peer_id);
			//	if (p) p->ProcessJsonRequest(l);
			//}
		}
	}
	else if (method == DEVICESTATUS_METHOD)
	{
		GET_INT(value);
		this->SendDeviceStatus(value);
	}
	else if (method == DEVICESLIST_METHOD) {
		const json::String type(req[TYPE_PARAM]);

		std::vector < std::pair < std::string, std::string>> list;
		json::Object::const_iterator it = req.Find("list");
		if (it != req.End()) {
			json::Array l = it->element;
			for (int il = 0; il < l.Size(); il++) {
				json::Object &le = l[il];
				list.emplace_back(json::String(le[ID_PARAM]), json::String(le[NAME_PARAM]));
			}
		}

		this->DevicesList(type.Value(), list);
	}
	else if (method == DEVICECHANGED_METHOD) {
		const json::String type(req[TYPE_PARAM]);
		const json::String id(req[ID_PARAM]);
		const json::String name(req[NAME_PARAM]);

		this->DeviceChanged(type.Value(), id.Value(), name.Value());
	}
	else if (method == DEVICESTATE_METHOD) {
		const json::String type(req[TYPE_PARAM]);
		const json::String id(req[ID_PARAM]);
		json::Object::const_iterator it = req.Find(MUTE_PARAM);

		if (it != req.End())
			this->DeviceMute(type.Value(), id.Value(), json::Boolean(it->element));
	}
	else if (method == MANAGELAYOUT_METHOD)
	{
		if (1) {
			auto cd = this->GetCurrentConference();

			GET_STRING(func);
			if (func != "Get") {
				if (cd->m_role != PR_LEADER)
					return true;

			}
			req["my_peer_id"] = (json::String)m_state.trueconfId;
			req["conf_id"] = (json::String)cd->stream_conf_id;
			auto p = VS_JsonRequestHandler::GetInstance();
			if (p) p->ProcessJsonRequest(req);
		}
		else {
			// not imlemented yet in server
			GET_STRING(func);
			if (func == "PriorityType") {
				GET_INT(type);
				this->ManageLayoutPT(type);
			}
			else {
				std::string id1, id2;
				json::Object::const_iterator it = req.Find("UserName");
				if (it != req.End()) {
					json::Array list = it->element;
					if (list.Size() > 0)
						id1 = (const json::String) list[0];
					if (list.Size() > 1)
						id2 = (const json::String) list[1];
				}
				this->ManageLayoutFunc(func.c_str(), id1.c_str(), id2.c_str());
			}
		}
	}
	else return false;

	return true;
}

void VS_JsonClient::onLoginResponse(VS_UserLoggedin_Result res)
{
	json::Object ans;
	ans["method"] = json::String("loginResponse");

	if (res == USER_LOGGEDIN_OK)
	{
		ans["CID"] = json::String(CID());

		VS_AutoLock lock(this);
		ans["displayName"] = json::String( this->m_state.displayName );
		ans[TrueConfID_PARAM] = json::String( this->m_state.trueconfId );
		ans["Key"] = json::String(this->m_state.autologin_key);
		ans[RIGHTS_PARAM] = json::Number(this->m_state.rigths);

		auto p = VS_JsonRequestHandler::GetInstance();
		if (p) p->RegisterWebPeerCallback(this->m_state.trueconfId, std::bind(&VS_JsonClient::onWebPeerMessage,
				std::static_pointer_cast<VS_JsonClient>(shared_from_this()), std::placeholders::_1));
		m_waitLogoff.reset();
	}
	else
		m_waitLogoff.set();

	ans["result"] = json::Number( (long)res );

	DispatchResponse( ans );

	if (res == USER_LOGGEDIN_OK)
		SendAppProps();
}

void VS_JsonClient::EnshureClientAlive()
{
	StartWaiting("keepAlive", std::chrono::seconds(8), [this]() { EnshureClientAlive(); });
	json::Object ans;
	DispatchResponse(ans);
}

void VS_JsonClient::onLogoutResponse(VS_UserLoggedout_Cause cause)
{
	json::Object ans;
	ans["method"] = json::String("logoutResponse");
	ans["result"] = json::String( "ok" );
	ans["cause"] = json::Number((long)cause);
	DispatchResponse(ans);
	m_waitLogoff.set();

	auto p = VS_JsonRequestHandler::GetInstance();
	if (p) p->DeleteWebPeerCallback(this->m_state.trueconfId);
}

void VS_JsonClient::onConferenceStateChange(const char *method, const VS_ConferenceDescriptor &conf)
{
	json::Object conference;
	FillJsonByConference(conference, conf);
	if (conf.conf_type == CT_MULTISTREAM && conf.conf_subtype == GCST_ALL_TO_OWNER && conf.my_id != conf.owner) {
		conference["connectPartId"] = json::String(conf.owner);
	} else {
		conference["connectPartId"] = json::String("");
	}
	conference["message"] = json::String(method);

	json::Object ans;
	ans["method"] = json::String("conferenceStateChange");
	ans["conference"] = conference;
	DispatchResponse( ans );
}

void VS_JsonClient::onCommand(const char *from, const char *command)
{
	// #29791: temporal hotfix to auto-accept recording requests at webrtc (tcs437)
	//if (_stricmp(command, CCMD_RECORD_QUERY) == 0)
	//{
	//	SendChatCommand(from, CCMD_RECORD_ACCEPT);
	//	return;
	//}
	// hotfix removed at tcs440

	json::Object ans;
	ans["method"] = json::String("command");
	ans["command"] = json::String(command);
	ans["from"] = json::String(from);
	DispatchResponse( ans );
}

void VS_JsonClient::onChat(const char *to, const char *conf, const char *from, const char *dname, const char *text)
{
	json::Object ans;
	ans["method"] = json::String("chat");
	ans["message"] = json::String(text);
	ans["from"] = json::String(from);
	ans["displayname"] = json::String(dname);
	if (to) ans["to"] = json::String(to);
	if (conf) ans[CONFERENCE_PARAM] = json::String(conf);
	DispatchResponse( ans );
}

void VS_JsonClient::onFile(const char *to, const char *conf, const char *from, const char *dname, const char *text, const char* fname, const char* link, const char* url, const char* about)
{
	json::Object ans;
	ans["method"] = json::String("file");
	ans["message"] = json::String(text);
	ans["from"] = json::String(from);
	ans["displayname"] = json::String(dname);
	if (to) ans["to"] = json::String(to);
	if (conf) ans[CONFERENCE_PARAM] = json::String(conf);
	ans["filename"] = json::String(fname);
	ans["url"] = json::String(url);
	DispatchResponse(ans);
}


void VS_JsonClient::onPartList(const VS_ConferenceDescriptor &conf, const char* user, int32_t type)
{
	json::Object ans;
	json::Array part_list;
	if (m_webversion < 1)
		type = PLT_OLD;

	ans["method"] = json::String(SENDPARTSLIST_METHOD);
	ans["type"] = json::Number(type);

	auto fillP = [&](const PartInfo &info) {
		json::Object part;
		part[TrueConfID_PARAM] = json::String(info.user_id);
		part["displayname"] = json::String(info.dn);
		part["role"] = json::Number(info.role);
		part["podium"] = json::Boolean(info.onpodium);
		part[DEVICESTATUS_PARAM] = json::Number(info.device_status);
		part[IS_OPERATOR_PARAM] = json::Boolean(info.is_operator);
		part_list.Insert(part);
	};

	if (type == PLT_OLD || type == PLT_ALL) {
		for (auto &it : conf.partList) {
			fillP(it.second);
		}
	}
	else if (type == PLT_ADD || type == PLT_UPD) {
		if (user && *user) {
			auto it = conf.partList.find(string_view(user));
			if (it != conf.partList.end())
				fillP(it->second);
		}
	}
	else if (type == PLT_DEL) {
		if (user && *user) {
			PartInfo p;
			p.user_id = user;
			fillP(p);
		}
	}

	ans["list"] = part_list;
	DispatchResponse(ans);
}

void VS_JsonClient::onRoleEvent(const VS_ConferenceDescriptor &conf, VS_RoleEvent_Type type, VS_Participant_Role role, VS_Broadcast_Status bs, VS_RoleInqury_Answer result, const char* user)
{
	json::Object ans;
	ans["method"] = json::String(ROLEEVENT_METHOD);
	ans["type"] = json::Number((long) type );
	ans["role"] = json::Number((long) role );
	if (type == RET_NOTIFY)
		ans["podium"] = json::Boolean((bs&BS_SND_PAUSED) == 0);
	ans["result"] = json::Number((long) result);
	ans[USERNAME_PARAM] = json::String(user);
	ans[CONFERENCE_PARAM] = json::String(conf.conf_id);

	if (type == RET_INQUIRY)
		m_rolequery.emplace(user, role);

	DispatchResponse( ans );
}

void VS_JsonClient::onDeviceStatus(const char * name, long device_status)
{
	json::Object ans;
	ans["method"] = json::String(DEVICESTATUS_METHOD);
	ans[USERNAME_PARAM] = json::String(name);
	ans[DEVICESTATUS_PARAM] = json::Number(device_status);

	DispatchResponse(ans);
}

void VS_JsonClient::onListenersFltr(int32_t fltr) {
	json::Object ans;
	ans["method"] = json::String(LISTENERSFLTR_METHOD);
	ans[MEDIAFILTR_PARAM] = json::Number(fltr);

	DispatchResponse(ans);
}

void VS_JsonClient::onChangeDevice(string_view type, string_view id, string_view name)
{
	json::Object ans;
	ans["method"] = json::String(CHANGEDEVICE_METHOD);
	ans[TYPE_PARAM] = json::String(std::string(type));
	ans[ID_PARAM] = json::String(std::string(id));
	ans[NAME_PARAM] = json::String(std::string(name));

	DispatchResponse(ans);
}

void VS_JsonClient::onSetDeviceMute(string_view type, string_view id, bool mute)
{
	json::Object ans;
	ans["method"] = json::String(SETDEVICESTATE_METHOD);
	ans[TYPE_PARAM] = json::String(std::string(type));
	ans[ID_PARAM] = json::String(std::string(id));
	ans[MUTE_PARAM] = json::Boolean(mute);

	DispatchResponse(ans);
}

void VS_JsonClient::onRequestKeyFrame()
{
	Lock();
	auto cd = this->GetCurrentConference();

	json::Object ans;
	ans["method"] = json::String(SENDCOMMAND_METHOD);
	ans[REQESTKEYFRAME_PARAM] = json::Boolean(true);
	ans["my_peer_id"] = (json::String)m_state.trueconfId;
	ans["conf_id"] = (json::String)cd->stream_conf_id;
	UnLock();

	// to peer
	auto p = VS_JsonRequestHandler::GetInstance();
	if (p) p->ProcessJsonRequest(ans);
}

void VS_JsonClient::onReqInvite(const char * from, const char* dn)
{
	json::Object ans;
	ans["method"] = json::String(REQINVITE_METHOD);
	ans[USERNAME_PARAM] = json::String(from);
	ans["displayname"] = json::String(dn);

	DispatchResponse(ans);
}

void VS_JsonClient::OnError(unsigned /*error*/)
{
	Lock();
	auto cd = GetCurrentConference();
	if (!cd->stream_conf_id.empty())
		LeaveStreamConference(*cd);
	UnLock();

	// disconnect callback at WebrtcPeerConnectionSRV
	auto p = VS_JsonRequestHandler::GetInstance();
	if (p) p->DeleteWebPeerCallback(this->m_state.trueconfId);

	// channel to server is broken
	//VS_FakeEndpointManager::Instance().DeleteEndpoint(CID());
	m_endpoint->Stop();
}

void VS_JsonClient::onWSError(unsigned err)
{
	auto tcid = GetTrueconfID();
	{
		auto ds = dstream4;
		ds << "onWSError = " << err << " for " << tcid << "(" << CID() << ")";
		if (!tcid.empty()) {
			this->Logout();
			auto now1 = std::chrono::system_clock::now();
			bool res = m_waitLogoff.wait_for(std::chrono::milliseconds(20)); // need some unknown time to Log Out being sent
			auto now2 = std::chrono::system_clock::now();
			ds << ", wait res " << res << ", time = " << std::chrono::duration_cast<std::chrono::milliseconds>(now2 - now1).count() << " ms";
			// disconnect callback at WebrtcPeerConnectionSRV
			auto p = VS_JsonRequestHandler::GetInstance();
			if (p) p->DeleteWebPeerCallback(tcid);
		}
	}
}

void VS_JsonClient::SendCurrentState()
{
	VS_AutoLock lock(this);

	json::Object ans;
	ans["method"] = json::String("getStateResponse");
	ans["CID"] = json::String(CID());
	ans["displayName"] = json::String( this->m_state.displayName );
	ans[TrueConfID_PARAM] = json::String( this->m_state.trueconfId );

	json::Array conf_list;
	json::Object res;
	FillJsonByConference(res, *this->GetCurrentConference());
	conf_list.Insert( res );

	ans["conferences"] = conf_list;
	DispatchResponse( ans );
}

void VS_JsonClient::SendAppProps()
{
	auto c = GetPropsFromServer();
	if (!c.IsValid() || c.IsEmpty())
		return;


	json::Object ans;

	c.Reset();
	while (c.Next())
		ans[c.GetName()] = json::String(c.GetStrValueRef());

	// bug#56066: add TURN params as props
	VS_RegistryKey k(false, CONFIGURATION_KEY);
	for (auto name: {"TURN Host", "TURN Username", "TURN Password"})
	{
		std::string value;
		if (k.GetString(value, name))
			ans[name] = json::String(value);
	}

	DispatchResponse( ans );
}

void VS_JsonClient::FillJsonByConference( json::Object &conference, const VS_ConferenceDescriptor &conf ) const
{
	conference["conferenceId"] = json::String(conf.conf_id);
	conference["streamConferenceId"] = json::String(conf.stream_conf_id);
	conference["topic"] = json::String(conf.topic);
	conference["owner"] = json::String(conf.owner);

	conference["state"] = json::Number( (long)conf.m_state );

	conference["lastRejectReason"] = json::Number( (long)conf.m_last_reject_reason );
	conference["type"] = json::Number( (long)conf.conf_type );
	conference["subtype"] = json::Number((long)conf.conf_subtype);
	conference["cmr_flags"] = json::Number(conf.m_cmr_flags);
}

void VS_JsonClient::JoinStreamConference(VS_ConferenceDescriptor &d)
{
}

void VS_JsonClient::LeaveStreamConference(VS_ConferenceDescriptor &d)
{
	// force disconnecting
	this->SendUsageStat();

	json::Object req;
	req["method"] = json::String("webrtc");
	req["my_peer_id"] = (json::String)m_state.trueconfId;
	req["conf_id"] = (json::String)d.stream_conf_id;
	req["type"] = (json::String)"bye";

	m_rolequery.clear();

	auto p = VS_JsonRequestHandler::GetInstance();
	if (p) p->ProcessJsonRequest( req );
}

void VS_JsonClient::onWebPeerMessage(json::Object&& obj)
{
	json::Object &req = obj;
	GET_STRING_OR_DEFAULT(method, "");

	if (method == LOGPARTSTAT_METHOD) {
		GET_STRING_OR_DEFAULT(data, "");
		size_t output_length = sizeof(TConferenceStatistics);
		base64_decode(data.c_str(), data.length(), &this->m_conf_stat, output_length);
	}
	else if (method == "") {
		obj["method"] = json::String("webrtc");
		obj["CID"] = json::String(CID());
		DispatchResponse(obj);
	}
	else {
		// unknown method from peer
	}
}

void VS_JsonClient::SetResponceCallBack(std::shared_ptr<void> &&holder, boost::function< bool(const json::Object&) > &&handler)
{
	VS_AutoLock lock(this);
	m_responce_instance = std::move(holder);
	m_responce_handler = std::move(handler);
}

void VS_JsonClient::DispatchResponse(const json::Object&obj)
{
	std::shared_ptr<void> ptr;
	boost::function<bool(const json::Object&)> f;
	{
		VS_AutoLock lock(this);
		ptr = m_responce_instance.lock();
		f = m_responce_handler;
	}
	if (!!ptr)
		f(obj);
}

void VS_JsonClient::RegisterAtWebrtcSRV()
{
	auto p = VS_JsonRequestHandler::GetInstance();
	if (p) p->RegisterWebPeerCallback(this->m_state.trueconfId, std::bind(&VS_JsonClient::onWebPeerMessage,
		std::static_pointer_cast<VS_JsonClient>(shared_from_this()), std::placeholders::_1));
}