#include "VS_LicenseShareService.h"
#include "std/debuglog/VS_Debug.h"
#include "std/cpplib/VS_Protocol.h"
#include "std-generic/cpplib/VS_Container.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std-generic/cpplib/ignore.h"
#include "AppServer/Services/VS_Storage.h"
#include "VS_ConfMemStorage.h"
#include "std/cpplib/VS_CallIDUtils.h"

#define DEBUG_CURRENT_MODULE VS_DM_REGS

const char LOGIN_TYPE_TAG[] = "LoginType";
const char ADD_STRING_TAG[] = "AddString";
const char ARRIVED_INFO_TAG[] = "ArrivedInfo";
const char SRC_CID_TAG[] = "SrcCID";
const char SRC_USER_TAG[] = "SrcUser";
const char USER_DATA_TAG[] = "UserData";
static const char LIC_ID_TAG[] = "LicenseID";

bool lic::ShareService::Processing(std::unique_ptr<VS_RouterMessage>&& recvMess)
{
	switch (recvMess->Type())
	{
	case transport::MessageType::Invalid:
		break;
	case transport::MessageType::Request:
	case transport::MessageType::Reply:
	{
		VS_Container cnt;
		if (cnt.Deserialize(recvMess->Body(), recvMess->BodySize())) {
			const char* method = 0;
			if ((method = cnt.GetStrValueRef(METHOD_PARAM)) != 0) {
				dprint3("Processing %20s; cid:%s user:%s srv:%s \n", method, recvMess->SrcCID(), recvMess->SrcUser(), recvMess->SrcServer());

				if (strcasecmp(method, REQUEST_LICENSE_METHOD) == 0) {
					m_license_sharer.ProcessShareLicenseRequest(cnt, recvMess->SrcServer());
				}
				else if (strcasecmp(method, SHARE_LICENSE_METHOD) == 0) {
					ExpandLicenseOnLogin({}, &cnt, ExpandLicenseOnLoginStep::ReceiveLicense);
				}
				else if (strcasecmp(method, SHARED_LICENSE_CHECK_METHOD) == 0) {
					m_license_sharer.ReceiveSharedLicenseCheck(cnt, recvMess->SrcServer());
				}
				else if (strcasecmp(method, SHARED_LICENSE_CHECK_METHOD_RESP) == 0) {
					VS_License to_free;
					m_license_sharer.ReceiveSharedLicenseCheckResponse(cnt, to_free);
					if(to_free.HasSharedResources()) ClearOverhead(to_free);
				}
				else if (strcasecmp(method, RETURN_SHARED_LICENSE_METHOD) == 0) {
					m_license_sharer.ReceiveReturnedSharedResources(cnt, recvMess->SrcServer());
				}
				else if (strcasecmp(method, RETURN_SHARED_LICENSE_METHOD_RESP) == 0) {
					VS_License to_free;
					m_license_sharer.ReceiveReturnedSharedResourcesResp(cnt, to_free);
					if (to_free.HasSharedResources()) ClearOverhead(to_free);
				}
				else if (strcasecmp(method, RETURN_LICENSE_FORCE_TAG) == 0) {
					OnReturnLicenseByForce_Method(cnt, recvMess->SrcServer_sv());
				}
				else if (strcasecmp(method, MASTER_CHANGED_EVENT) == 0) {
					OnMasterChanged_Event();
				}


			}
		}
		break;
	}
	default:
		break;
	}

	return false;
}

void CheckMasterIsResolved() {
	assert(p_licWrap != nullptr);
	if (!p_licWrap->IsSlave()) return;

	static std::chrono::steady_clock::time_point last_check;

	auto now = std::chrono::steady_clock::now();
	if (now - last_check < std::chrono::minutes(1)) return;

	last_check = now;
	if(lic::Sharer::GetMasterServer().empty()) lic::Sharer::ResolveMasterServer();
}

bool lic::ShareService::Timer(unsigned long tickcount)
{
	CheckMasterIsResolved();

	if (!IsLicenseMasterActive()) ClearDelayedLogins();
	m_license_sharer.ObserveSharedResourses();
	m_license_sharer.ObserveLicenseOverhead();

	VS_License must_free;
	m_license_sharer.VerifyMasterStatus();
	if (!m_license_sharer.ObserveReceivedSharedResources(must_free) && must_free.HasSharedResources()) {
		ClearOverhead(must_free);
	}

	auto master_server = Sharer::GetMasterServer();
	if(!master_server.empty()){
		if ((std::chrono::steady_clock::now() - m_last_lic_check_query) > std::chrono::minutes(1)) {
			m_license_sharer.SendSharedLicenseCheck(master_server);
			m_last_lic_check_query = std::chrono::steady_clock::now();
		}
		m_license_sharer.ReturnLicenseOverhead(master_server);
	}
	return true;
}

bool lic::ShareService::Init()
{
	if (lic::Sharer::LAST_CHECK_TIMEOUT < m_TimeInterval) return false;	// we can't make check if interval of check less that interval of check call

	if (!m_license_sharer.Init()) return false;
	m_license_sharer.SetTransport(this);
	m_license_sharer.SetStartLicenseForSlave();
	m_license_sharer.RestoreSharedLicenses();	// in case server was fallen down
	return true;
}

int32_t CalculateRequest(int32_t /*what_i_have*/) {
	//int32_t what_i_need_more = what_i_have * 0.2;					// need additionally 20% from what have
	//what_i_need_more = std::max(what_i_need_more, 1);				// minimum to request is 1
	//what_i_need_more = std::min(what_i_need_more, lic::REQ_LIMIT);	// maximum to request is REQ_LIMIT

	return 1;	// for now request by one to avoid difficulties
}

bool CalculateLicenseRequest(const VS_ClientType client_type, const VS_License& my_resourses, VS_License& OUT_request) {
	switch (client_type)
	{
	case CT_SIMPLE_CLIENT:
	case CT_MOBILE:
		OUT_request.m_onlineusers = CalculateRequest(my_resourses.m_onlineusers);
		break;
	case CT_GATEWAY:
	case CT_TRANSCODER:
	case CT_TRANSCODER_CLIENT:
		OUT_request.m_gateways = CalculateRequest(my_resourses.m_gateways);
		break;
	case CT_TERMINAL:
		OUT_request.m_terminal_pro_users = CalculateRequest(my_resourses.m_terminal_pro_users);
		break;
	case CT_SDK:
	case CT_WEB_CLIENT:
	default:
		return false;
	}
	return true;
}

template<class T>
bool GetValue(const UserCtx &ctx, const std::string& key, T &out) {
	auto it = ctx.find(key);
	if (it == ctx.end())
		return false;

	auto p_value = boost::get<T>(&it->second); // slow, but faster than Serialization/Deserialization
	if (!p_value)
		return false;

	out = *p_value;
	return true;
}

void lic::ShareService::ExpandLicenseOnLogin(UserCtx && userCtx, const VS_Container *license_response, const ExpandLicenseOnLoginStep step) {
	if (!p_licWrap->IsSlave()) return;

	switch (step)
	{
	case ExpandLicenseOnLoginStep::RequestLicense:	// request more licenses from master
	{
		if (userCtx.empty())
			break;
		auto master_server = Sharer::GetMasterServer(true);
		if (master_server.empty()) {
			dstream3 << "Error\t" << LICENSE_MASTER_TAG << " not specified!\n";
			ClearDelayedLogin(std::move(userCtx));
			break;
		}
		m_delayed_logins.push_back(userCtx);

		LoginType login_type = LoginType::UNKNOWN;
		GetValue(userCtx, LOGIN_TYPE_TAG, login_type);

		bool is_guest(false);
		VS_ClientType client_type(CT_SIMPLE_CLIENT);

		std::string login;
		if (login_type == LoginType::SIMPLE_LOGIN) {
			VS_Container arrived_cnt;
			GetValue(userCtx, ARRIVED_INFO_TAG, arrived_cnt);

			login = arrived_cnt.GetStrValueRef(LOGIN_PARAM);

			VS_SimpleStr real_login, display_name;
			is_guest = VS_SimpleStorage::GetGuestParams(login.c_str(), real_login, display_name);
			arrived_cnt.GetValueI32(CLIENTTYPE_PARAM, client_type);
		}
		else if (login_type == LoginType::AUTHORIZED_LOGIN) {
			VS_UserData ud;
			if (GetValue(userCtx, USER_DATA_TAG, ud)) {
				login = std::string(SimpleStrToStringView(ud.m_name));
				is_guest = VS_SimpleStorage::GetGuestParams(ud.m_name.m_str, vs::ignore<VS_SimpleStr>(), vs::ignore<VS_SimpleStr>());
			}
			GetValue(userCtx, CLIENTTYPE_PARAM, client_type);
		}
		else {
			ClearDelayedLogin(std::move(m_delayed_logins.back()));
			m_delayed_logins.pop_back();
			return;

		}

		VS_License to_request;
		to_request.m_error = 0;
		to_request.m_id = ++m_licReqId == 0 ? ++m_licReqId : m_licReqId;	// avoid zero id
		m_delayed_logins.back().emplace(LIC_ID_TAG, to_request.m_id);

		auto my_resourses = p_licWrap->GetLicSum();
		if (is_guest) {
			to_request.m_max_guests = CalculateRequest(my_resourses.m_max_guests);
		}
		else {
			if (!CalculateLicenseRequest(client_type, my_resourses, to_request)) {
				ClearDelayedLogin(std::move(m_delayed_logins.back()));
				m_delayed_logins.pop_back();
				return;
			}
			if ((client_type == CT_TRANSCODER_CLIENT && !VS_IsRTPCallID(login)) || // when we call SIP/h323 by ip CT == CT_TRANSCODER_CLIENT, second statement must fail
				client_type == CT_TERMINAL)										 // TerminalPro eats OnlineUsers also
			{
				CalculateLicenseRequest(CT_SIMPLE_CLIENT, my_resourses, to_request);
			}
		}

		m_license_sharer.RequestLicense(to_request, master_server);
		m_lic_share_req_answered = false;
		m_last_lic_share_req_time = std::chrono::steady_clock::now();
	}
	break;
	case ExpandLicenseOnLoginStep::ReceiveLicense:	// receive license share, try to login users in m_delayed_logins queue
	{
		if (!license_response) break;

		m_lic_share_req_answered = true;
		uint64_t recvLicId(0);
		if (!m_license_sharer.ReceiveLicenseShare(*license_response, recvLicId)) {	// master has no resourses
			if (recvLicId != 0) {
				ClearDelayedLogin(recvLicId);
			}
			else {
				dstream3 << "lic::Share Received license with worng ID. Clear all delayed logins!\n";
				ClearDelayedLogins();
			}
		}
		else {
			while (!m_delayed_logins.empty()) {
				auto&& userCtx = m_delayed_logins.front();
				LoginType login_type = LoginType::UNKNOWN;
				GetValue(userCtx, LOGIN_TYPE_TAG, login_type);

				VS_SCOPE_EXIT{ if (!m_delayed_logins.empty())  m_delayed_logins.pop_front(); };


				if (login_type == LoginType::SIMPLE_LOGIN) {
					size_t arrived_cnt_size = 0;
					VS_Container arrived_cnt;
					GetValue(userCtx, ARRIVED_INFO_TAG, arrived_cnt);
					std::string src_cid;
					GetValue(userCtx, SRC_CID_TAG, src_cid);
					std::string srcUser;
					GetValue(userCtx, SRC_USER_TAG, srcUser);
					std::string additionalStr;
					GetValue(userCtx, ADD_STRING_TAG, additionalStr);
					VS_SimpleStr src_cid_tmp(src_cid.c_str());
					if (LoginUser(true, arrived_cnt, src_cid_tmp, srcUser.c_str(), additionalStr.c_str()) == LICENSE_USER_LIMIT) {
						break;	// we have reached our limit again, wait until next license share will come
					}
				}
				else if (login_type == LoginType::AUTHORIZED_LOGIN) {
					VS_UserData ud;
					if (!GetValue(userCtx, USER_DATA_TAG, ud)) {
						ClearDelayedLogin(std::move(userCtx));
						continue;
					}

					VS_Container props;
					GetValue(userCtx, PROPERTY_PARAM, props);
					VS_ClientType t;
					GetValue(userCtx, CLIENTTYPE_PARAM, t);
					std::string src_cid;
					GetValue(userCtx, SRC_CID_TAG, src_cid);
					std::string key;
					GetValue(userCtx, KEY_PARAM, key);
					std::string addStr;
					GetValue(userCtx, ADD_STRING_TAG, addStr);
					if (AuthorizedLogin(src_cid.c_str(), ud, key.c_str(), props, t, addStr) == LICENSE_USER_LIMIT) {
						break;	// we have reached our limit again, wait until next license share will come
					}
				}

			}	// end of loop
		}
	}
	break;
	default:
		break;
	}
}

void lic::ShareService::OnPointConnected_Method(const VS_Container & cnt)
{
	auto pSlaveName = cnt.GetStrValueRef(USERNAME_PARAM);	// for server username is server id here
	if (!pSlaveName) return;
	m_license_sharer.SlaveConnectedEvent(pSlaveName);
}
void lic::ShareService::OnPointDisconnected_Method(const VS_Container & cnt)
{
	auto pEndpointName = cnt.GetStrValueRef(USERNAME_PARAM);	// for server username is server id here
	if (!pEndpointName) return;
	m_license_sharer.SlaveDisconnectedEvent(pEndpointName);
	m_license_sharer.MasterDisconnectedEvent(pEndpointName);
}

void lic::ShareService::OnReturnLicenseByForce_Method(const VS_Container & cnt, const string_view from)
{
	VS_License arrivedLic;
	if (lic::Sharer::GetLicense(cnt, __FUNCTION__, arrivedLic)) {
		dstream2 << "Return shared resources to master" << from << " by force!\n";
		m_license_sharer.ReturnSharedResoursesToMaster(arrivedLic, from);
	}
}

void lic::ShareService::OnMasterChanged_Event()
{
	if (!p_licWrap->IsSlave()) return;
	// resolve new master
	lic::Sharer::ResolveMasterServer();
}

void lic::ShareService::ClearDelayedLogins()
{
	while (!m_delayed_logins.empty()) {
		auto&& login_info = m_delayed_logins.front();
		ClearDelayedLogin(std::move(login_info));
		m_delayed_logins.pop_front();
	}
}
void lic::ShareService::ClearDelayedLogin(UserCtx && userCtx)
{
	VS_Container	cnt;
	cnt.AddValue(METHOD_PARAM, USERLOGGEDIN_METHOD);
	cnt.AddValueI32(RESULT_PARAM, LICENSE_USER_LIMIT);
	std::string src_cid;
	if(GetValue(userCtx, SRC_CID_TAG, src_cid))
		PostUnauth(src_cid.c_str(), cnt);
}
void lic::ShareService::ClearDelayedLogin(uint64_t licenseID)
{
	auto it = std::find_if(m_delayed_logins.begin(), m_delayed_logins.end(), [&](const UserCtx & ctx) {
		uint64_t id(0);
		GetValue(ctx, LIC_ID_TAG, id);
		return id == licenseID;
	});
	if (it == m_delayed_logins.end()) {
		dstream4 << "lic::Share\t Can't find delayed login with licenseID=" << licenseID;
		return;
	}
	ClearDelayedLogin(std::move(*it));
	m_delayed_logins.erase(it);
}
bool lic::ShareService::IsLicenseMasterActive()
{
	if (m_lic_share_req_answered) return true;	// master did answer on our request - all ok
	if ((std::chrono::steady_clock::now() - m_last_lic_share_req_time) > std::chrono::seconds(5)) return false;
	return true;
}

void lic::ShareService::ClearOverhead(VS_License & to_clear)
{
	std::vector<vs_user_id> to_logout;
	g_storage->GetUsersFiltered(to_clear.m_onlineusers, CT_SIMPLE_CLIENT, to_logout);
	g_storage->GetUsersFiltered(to_clear.m_terminal_pro_users, CT_TERMINAL, to_logout);

	// for gateways we have three types
	auto found = g_storage->GetUsersFiltered(to_clear.m_gateways, CT_GATEWAY, to_logout); to_clear.m_gateways -= found;
	found = g_storage->GetUsersFiltered(to_clear.m_gateways, CT_TRANSCODER, to_logout); to_clear.m_gateways -= found;
	found = g_storage->GetUsersFiltered(to_clear.m_gateways, CT_TRANSCODER_CLIENT, to_logout); to_clear.m_gateways -= found;

	g_storage->GetUsersFiltered(to_clear.m_max_guests, CT_SIMPLE_CLIENT /*param ignored*/, to_logout, true, &to_logout);

	for (const auto& user : to_logout) {
		LogoutUser_Method(user, e_logout_user);
	}
}
