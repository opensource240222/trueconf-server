#ifdef _WIN32 // not ported
#include "VS_TranscodersDispatcher.h"
#include "VS_TranscoderControl.h"
#include "tools/Server/CommonTypes.h"
#include "std/cpplib/VS_Utils.h"
#include "std-generic/cpplib/VS_Container.h"
#include "std/cpplib/ThreadUtils.h"
#include "std/cpplib/VS_Protocol.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/debuglog/VS_Debug.h"

#include <string>
#include <vector>

#define DEBUG_CURRENT_MODULE VS_DM_MULTIGATEWAY

#define VS_DEFAULT_MAX_TRANSCODERS_ALLOWED	100
#define VS_DEFAULT_TRANSCODERS_POOL_SZ 5
#define VS_DEFAULT_TRANSCODERS_LOWER_LIMIT 2

#define VS_MAXIMUM_TRANSCODERS_POOL_SIZE 200
#define VS_MAXIMUM_MAX_TRANSCODERS_ALLOWED 300

VS_TranscodersDispatcher *VS_TranscodersDispatcher::m_instance = 0;
VS_Lock *VS_TranscodersDispatcher::m_lock_instance = new VS_Lock();

VS_TranscodersDispatcher * VS_TranscodersDispatcher::GetInstanceTranscodersDispatcher(const std::weak_ptr<ts::IPool> &pool, const std::weak_ptr<VS_TranscoderLogin>& transLogin)
{
	VS_AutoLock lock(m_lock_instance);
	if(!m_instance)
	{
		VS_RegistryKey rKey(false, CONFIGURATION_KEY);
		unsigned long max_allowed(0),pool_sz(0), lower_limit(0);
		if(0>=rKey.GetValue(&max_allowed,sizeof(max_allowed),VS_REG_INTEGER_VT,"trans_disp_max_allowed"))
			max_allowed = VS_DEFAULT_MAX_TRANSCODERS_ALLOWED;
		if(0>=rKey.GetValue(&pool_sz,sizeof(pool_sz),VS_REG_INTEGER_VT,"trans_disp_pool_size"))
			pool_sz = VS_DEFAULT_TRANSCODERS_POOL_SZ;
		if(0>=rKey.GetValue(&lower_limit,sizeof(lower_limit),VS_REG_INTEGER_VT,"trans_disp_lower_limit"))
			lower_limit = VS_DEFAULT_TRANSCODERS_LOWER_LIMIT;

		max_allowed = max_allowed<VS_MAXIMUM_MAX_TRANSCODERS_ALLOWED?max_allowed : VS_MAXIMUM_MAX_TRANSCODERS_ALLOWED;
		pool_sz = pool_sz < VS_MAXIMUM_TRANSCODERS_POOL_SIZE ? pool_sz : VS_MAXIMUM_TRANSCODERS_POOL_SIZE;

		if(pool_sz<lower_limit || lower_limit>=max_allowed || pool_sz>max_allowed)
		{
			max_allowed = VS_DEFAULT_MAX_TRANSCODERS_ALLOWED;
			pool_sz = VS_DEFAULT_TRANSCODERS_POOL_SZ;
			lower_limit = VS_DEFAULT_TRANSCODERS_LOWER_LIMIT;
		}
		m_instance = new VS_TranscodersDispatcher(pool_sz,0,lower_limit, pool, transLogin);
	}
	return m_instance;
}
VS_TranscodersDispatcher::VS_TranscodersDispatcher(const unsigned pool_sz, const unsigned long max_trans_allowed, const unsigned long lower_limit,
		const std::weak_ptr<ts::IPool> &pool, const std::weak_ptr<VS_TranscoderLogin>& transLogin)
	: m_shutdownEvent(true) // It is important that it is not auto-reset, we wait on it in 2 places.
	, m_max_value(max_trans_allowed)
	, m_lower_limit(lower_limit)
	, m_pool_sz(pool_sz), m_trans_in_use_count(0),m_zombieDetected(false),m_Stop(false)
	, m_transPool(pool), m_transLogin(transLogin)
{
	m_timeoutThread = std::thread([this]() {
		vs::SetThreadName("TranscoderDisp");
		while (true)
		{
			if (m_shutdownEvent.wait_for(std::chrono::milliseconds(500)))
				return;

			if (TryLock())
			{
				for (const auto& td : m_transcoders_pool)
					if (td.second->transcoder)
						td.second->transcoder->Timeout();
				FreeZombie();
				Start();
				UnLock();
			}
		}
	});
}

VS_TranscodersDispatcher::~VS_TranscodersDispatcher()
{
	if(m_timeout_slot_conn.connected())
		m_timeout_slot_conn.disconnect();
	m_instance = 0;
}
bool VS_TranscodersDispatcher::Init(const char *our_endpoint, const char *our_service, const bool permittedAll)
{
	return true;
}

void VS_TranscodersDispatcher::CleanGarbage()
{
	VS_AutoLock lock(this);
	VS_RegistryKey rKey(false, TRANSCODERS_KEY, false);
	VS_RegistryKey trans_key;
	std::vector<std::string> to_delete;
	rKey.ResetKey();
	while(rKey.NextKey(trans_key))
	{
		if(m_transcoders_pool.find(trans_key.GetName()) == m_transcoders_pool.end())
			to_delete.emplace_back(trans_key.GetName());
	}
	for (const auto& x: to_delete)
		rKey.RemoveKey(x);
}


unsigned VS_TranscodersDispatcher::Start()
{
	VS_AutoLock	lock(this);
	if(m_Stop)
		return 0;
	if(m_transcoders_pool.size()>=m_max_value)
	{
		int stopped(0);
		for(auto iter = m_transcoders_pool.begin();iter!=m_transcoders_pool.end()&&m_transcoders_pool.size()-stopped>m_max_value;++iter)
		{
			if(e_started == iter->second->state)
			{
				stopped++;
				iter->second->transcoder->Stop();
			}
		}
		return 0;
	}
	if(m_transcoders_pool.size() - m_trans_in_use_count<0)
	{
		//impossible situation
		return 0;
	}
	if(!m_serverAddrs)
		return 0;
	unsigned free_count = m_transcoders_pool.size() - m_trans_in_use_count;

	if(free_count>=m_lower_limit)
		return 0;
	unsigned res_count(0);
	for(unsigned i =0;i<m_pool_sz-free_count&&m_transcoders_pool.size()<m_max_value;i++)
	{
		VS_SimpleStr name;
		GetNewName(name);

		boost::shared_ptr<VS_TranscoderControl> t = boost::signals2::deconstruct<VS_TranscoderControl>(name, this, m_transLogin);
		//TODO:NOT PORTED
		boost::signals2::connection sig_con = t->ConnectOnZombie([this](string_view dialogId) -> void
		{
			std::string tmp_dialog_id(dialogId);
			OnZombieTranscoder(tmp_dialog_id.c_str());
		});

		boost::shared_ptr<Transcoder_Descr> t_descr(new Transcoder_Descr);
		t_descr->state = e_none;
		t_descr->transcoder = t;
		t_descr->onZombieConn = sig_con;

		m_transcoders_pool[name] = t_descr;
		if(!t->Start(m_serverAddrs))
		{
			sig_con.disconnect();
			m_transcoders_pool.erase(name);
			continue;
		}
		res_count++;
	}
	return res_count;
}

boost::shared_ptr<VS_ClientControlInterface> VS_TranscodersDispatcher::GetTranscoder()
{
	VS_AutoLock lock(this);
	boost::shared_ptr<VS_ClientControlInterface> ret;
	if(m_Stop)
		return ret;

	auto &&iter = m_transcoders_pool.begin();
	while(iter!=m_transcoders_pool.end())
	{
		if(e_started == iter->second->state && iter->second->transcoder->IsReady())
		{
			ret = boost::static_pointer_cast<VS_ClientControlInterface>(iter->second->transcoder);
			iter->second->state = e_owned;
			dprint3("VS_TranscodersDispatcher::GetTranscoder Transcoder %s was owned. Current state = %d\n", iter->first.m_str, iter->second->state);
			m_trans_in_use_count++;
			break;
		}
		++iter;
	}
	return ret;
}

void VS_TranscodersDispatcher::ReleaseTranscoder(boost::shared_ptr<VS_ClientControlInterface> t)
{
	boost::shared_ptr<Transcoder_Descr> t_descr;
	bool kill(false);
	Transcoder_State current_state(e_none);
	{
		VS_AutoLock lock(this);
		if(!t)
			return;
		auto &&iter = m_transcoders_pool.find(t->GetTranscoderID().c_str());
		if(iter==m_transcoders_pool.cend())
			return;
		t_descr = iter->second;
		t_descr->state = t_descr->state == e_owned? e_started : t_descr->state == e_wait_for_release? e_ready_for_free: t_descr->state;
		m_trans_in_use_count--;
		dprint3("VS_TranscodersDispatcher::ReleaseTranscoder Transcoder %s was released. Current state = %d\n", iter->first.m_str, iter->second->state);
		if((current_state = t_descr->state) == e_ready_for_free)
		{
			kill = true;
			m_transcoders_pool.erase(iter);
		}
	}
	if(kill)
		t_descr->onZombieConn.disconnect();
}

void VS_TranscodersDispatcher::FreeZombie()
{
	VS_AutoLock lock(this);
	if(m_zombieDetected)
	{
		auto &&i = m_transcoders_pool.begin();
		while(i!=m_transcoders_pool.end())
		{
			boost::shared_ptr<Transcoder_Descr> t_descr = i->second;
			if(t_descr->state==e_ready_for_free)
			{
				t_descr->onZombieConn.disconnect();
				i = m_transcoders_pool.erase(i);
			}
			else
				++i;
		}
		m_zombieDetected = false;
	}
	if(m_Stop && m_transcoders_pool.empty())
	{
		if(m_timeout_slot_conn.connected())
			m_timeout_slot_conn.disconnect();
		m_shutdownEvent.set();
	}
}

void VS_TranscodersDispatcher::OnZombieTranscoder(const char *name)
{
	VS_AutoLock lock(this);
	auto &&i = m_transcoders_pool.find(name);
	if(i!=m_transcoders_pool.cend())
	{
		i->second->state = i->second->state == e_owned || i->second->state == e_wait_for_release ? e_wait_for_release : e_ready_for_free;
		m_zombieDetected = true;
	}
}

void VS_TranscodersDispatcher::GetNewName(VS_SimpleStr &new_name)
{
	VS_AutoLock lock(this);
	while(true)
	{
		char tmp[0xff] = {0};
		VS_GenKeyByMD5(tmp);
		if(m_transcoders_pool.find(tmp) == m_transcoders_pool.end())
		{
			new_name = tmp;
			return;
		}
	}
}

void VS_TranscodersDispatcher::Shutdown()
{
	dprint3("VS_TranscodersDispatcher::Shutdown...\n");
	unsigned long tick = GetTickCount();
	{
		VS_AutoLock lock(this);
		m_Stop = true;
		for(auto i = m_transcoders_pool.begin();i!=m_transcoders_pool.end();++i)
			i->second->transcoder->Stop();

	}
	WaitForShutdown();
	dprint3("VS_TranscodersDispatcher was shtdowned. Waiting time = %ld msec\n", GetTickCount() - tick);
}
void VS_TranscodersDispatcher::WaitForShutdown()
{
	m_shutdownEvent.wait();
	VS_AutoLock lock(this); /// waiting for exit from timeout;
}

bool VS_TranscodersDispatcher::Processing(std::unique_ptr<VS_RouterMessage>&&recvMess)
{
	if(!recvMess)
		return true;
	VS_Container	cnt;
	m_recvMess = recvMess.get();

	switch(recvMess->Type())
	{
	case transport::MessageType::Invalid:
		break;
	case transport::MessageType::Request:
	case transport::MessageType::Reply:
		if (cnt.Deserialize(m_recvMess->Body(), m_recvMess->BodySize()))
		{
			const char *method = 0;
			if((method = cnt.GetStrValueRef(METHOD_PARAM)) != 0)
			{
				dprint3("TransDispSrv: method=%s\n", method);
				if(_stricmp(method,TRANSCODERINIT_METHOD) == 0)
					TranscoderInit_Method(cnt);
				else if(_stricmp(method,HANGUP_METHOD) == 0)
					Hangup_Method(cnt);
				else if(_stricmp(method,"SetAppData") == 0)
					SetAppData_Method(cnt);
				else if(_stricmp(method,INVITEREPLY_METHOD) == 0)
					InviteReply_Method(cnt);
				else if(_stricmp(method,INVITE_METHOD) == 0)
					Invite_Method(cnt);
				else if(_stricmp(method,SENDMESSAGE_METHOD) == 0)
					SendMessage_Method(cnt);
				else if(_stricmp(method,SENDCOMMAND_METHOD) == 0)
					SendCommand_Method(cnt);
				else if(_stricmp(method,PREPARETRANSCODERFORCALL_METHOD) == 0)
					PrepareTranscoderForCall_Method(cnt);
				else if(_stricmp(method,CLEARCALL_METHOD) == 0)
					ClearCall_Method(cnt);
				else if (_stricmp(method, SETMEDIACHANNELS_METHOD) == 0)
					SetMediaChannels_Method(cnt);
				else if(_stricmp(method, LOGINUSER_METHOD) == 0)
					LoginAsUser_Method(cnt);
				else if (_stricmp(method, FECC_METHOD) == 0)
					FECC_Method(cnt);
			}
		}
		break;

	};
	m_recvMess = nullptr;
	return true;
}

void VS_TranscodersDispatcher::PrepareTranscoderForCall_Method(VS_Container &cnt)
{
	const char *name = cnt.GetStrValueRef(NAME_PARAM);
	if (!name || !*name)
		return;
	boost::shared_ptr<Transcoder_Descr> t_descr;
	{
		VS_AutoLock lock(this);
		auto &&i = m_transcoders_pool.find(name);
		if(i==m_transcoders_pool.cend()||e_ready_for_free == i->second->state || e_wait_for_release == i->second->state)
			return;
		t_descr = i->second;
	}
	t_descr->transcoder->PrepareForCallResponse(cnt);
}

void VS_TranscodersDispatcher::ClearCall_Method(VS_Container &cnt)
{
	const char *name = cnt.GetStrValueRef(NAME_PARAM);
	if (!name || !*name)
		return;
	boost::shared_ptr<Transcoder_Descr> t_descr;
	{
		VS_AutoLock lock(this);
		auto &&i = m_transcoders_pool.find(name);
		if(i==m_transcoders_pool.cend()||e_ready_for_free == i->second->state || e_wait_for_release == i->second->state)
			return;
		t_descr = i->second;
	}
	t_descr->transcoder->ClearCallResponse(cnt);
}

void VS_TranscodersDispatcher::SetMediaChannels_Method(VS_Container &cnt)
{
	const char *name = cnt.GetStrValueRef(NAME_PARAM);
	if (!name || !*name)
		return;
	boost::shared_ptr<Transcoder_Descr> t_descr;
	{
		VS_AutoLock lock(this);
		auto &&i = m_transcoders_pool.find(name);
		if (i==m_transcoders_pool.cend()||e_ready_for_free == i->second->state || e_wait_for_release == i->second->state)
			return;
		t_descr = i->second;
	}
	t_descr->transcoder->SetMediaChannelsResponse(cnt);
}

void VS_TranscodersDispatcher::TranscoderInit_Method(VS_Container &cnt)
{
	VS_Container rCnt;
	{
		VS_AutoLock lock(this);
		rCnt.AddValue(METHOD_PARAM, TRANSCODERINIT_METHOD);
		VS_SimpleStr cid = m_recvMess->SrcCID();

		const char *name = cnt.GetStrValueRef(NAME_PARAM);
		auto &&i = m_transcoders_pool.find(name);
		if(i==m_transcoders_pool.cend())
		{
			rCnt.AddValueI32(RESULT_PARAM, 1);
		}
		else if(e_none!=i->second->state)
		{
			rCnt.AddValueI32(RESULT_PARAM, 3);
			PostRequest(OurEndpoint(), i->first, rCnt, 0, TRANSCODERSDISPATCHER_SRV, default_timeout, TRANSCODERSDISPATCHER_SRV);
		}
		else if(i->second->transcoder->TranscoderInit(cid,cnt))
		{
			i->second->state = e_started;
			rCnt.AddValueI32(RESULT_PARAM, 0);
			rCnt.AddValue(NAME_PARAM,name);
		}
	}
	PostReply(rCnt);
}

void VS_TranscodersDispatcher::SendToTranscoder(VS_SimpleStr &name, VS_Container &cnt)
{
	if (!name)
		return;
	PostRequest(OurEndpoint(), name, cnt, 0, TRANSCODERSDISPATCHER_SRV, default_timeout, TRANSCODERSDISPATCHER_SRV);
}
void VS_TranscodersDispatcher::SendToUnAuthTranscoder(VS_SimpleStr &cid, VS_Container &cnt)
{
	if(!cid)
		return;
	PostUnauth(cid,cnt,0,TRANSCODERSDISPATCHER_SRV,default_timeout,TRANSCODERSDISPATCHER_SRV);
}

void VS_TranscodersDispatcher::Hangup_Method(VS_Container &cnt)
{
	const char *name = cnt.GetStrValueRef(NAME_PARAM);
	boost::shared_ptr<Transcoder_Descr> t_descr;
	if (!name || !*name)
		return;
	{
		VS_AutoLock lock(this);
		auto &&i = m_transcoders_pool.find(name);
		if(i==m_transcoders_pool.end()||e_ready_for_free == i->second->state || e_wait_for_release == i->second->state)
			return;
		t_descr = i->second;
	}
	t_descr->transcoder->HangupFromVisi();
}

void VS_TranscodersDispatcher::SetAppData_Method(VS_Container &cnt)
{
	const char *name = cnt.GetStrValueRef(NAME_PARAM);
	if (!name || !*name)
		return;
	VS_Container cnt2;
	if (!cnt.GetValue(DATA_PARAM, cnt2))
		return ;
	int32_t type(0);
	if (!cnt2.GetValue(H323_APP_DATA_TYPE, type))
		return ;
	if (type!=e_fastUpdatePicture)
		return ;
	boost::shared_ptr<Transcoder_Descr> t_descr;
	{
		VS_AutoLock lock(this);
		auto &&i = m_transcoders_pool.find(name);
		if(i==m_transcoders_pool.cend()||e_ready_for_free == i->second->state || e_wait_for_release == i->second->state)
			return;
		t_descr = i->second;
	}
	t_descr->transcoder->FastUpdatePictureFromVisi();
}

void VS_TranscodersDispatcher::InviteReply_Method(VS_Container &cnt)
{
	VS_CallConfirmCode confirm_code = e_call_none;
	int32_t x = e_call_none;
	if (!cnt.GetValue(RESULT_PARAM, x))
		return ;
	confirm_code = ( VS_CallConfirmCode )x;

	const char *name = cnt.GetStrValueRef(NAME_PARAM);
	if (!name || !*name)
		return;
	bool isGroupConf = false;
	cnt.GetValue(TYPE_PARAM, isGroupConf);

	boost::shared_ptr<Transcoder_Descr> t_descr;
	{
		VS_AutoLock lock(this);
		auto &&i = m_transcoders_pool.find(name);
		if(i==m_transcoders_pool.cend()||e_ready_for_free == i->second->state || e_wait_for_release == i->second->state)
			return;
		t_descr = i->second;
	}
	t_descr->transcoder->InviteReplyFromVisi(confirm_code, isGroupConf);
}

void VS_TranscodersDispatcher::Invite_Method(VS_Container &cnt)
{
	VS_SimpleStr from = cnt.GetStrValueRef(FROM_PARAM);
	VS_SimpleStr to = cnt.GetStrValueRef(TO_PARAM);
	const char *name = cnt.GetStrValueRef(NAME_PARAM);
	VS_SimpleStr dn_from_utf8 = cnt.GetStrValueRef(DISPLAYNAME_PARAM);
	bool isGroupConf = false;
	cnt.GetValue(TYPE_PARAM, isGroupConf);
	if (!name || !*name)
		return;
	boost::shared_ptr<Transcoder_Descr> t_descr;
	{
		VS_AutoLock lock(this);
		auto &&i = m_transcoders_pool.find(name);
		if(i==m_transcoders_pool.cend()||e_ready_for_free == i->second->state || e_wait_for_release == i->second->state)
			return;
		t_descr = i->second;
	}
	t_descr->transcoder->InviteFromVisi(from, to, isGroupConf, dn_from_utf8);
}

void VS_TranscodersDispatcher::SetMaxTranscoders(const int max_transcoders)
{
	VS_AutoLock lock(this);
	if(max_transcoders==-2)
		m_max_value = VS_DEFAULT_MAX_TRANSCODERS_ALLOWED;
	else if(max_transcoders>0)
		m_max_value = max_transcoders;
	else
		m_max_value = 0;
	dprint3("VS_TranscodersDispatcher::SetMaxTranscoders new max value = %d\n",m_max_value);
}
void VS_TranscodersDispatcher::SetServerAddresses(const char *addrs)
{
	VS_AutoLock lock(this);
	if(!addrs || !*addrs)
		return;
	m_serverAddrs = addrs;
	dprint4("VS_TranscodersDispatcher::SetServerAddresses serverAddr = %s\n", m_serverAddrs.m_str);
}
void VS_TranscodersDispatcher::SendMessage_Method(VS_Container &cnt)
{
	const char *name = cnt.GetStrValueRef(NAME_PARAM);
	if (!name || !*name)
		return;
	boost::shared_ptr<Transcoder_Descr> t_descr;
	{
		VS_AutoLock lock(this);
		std::map<VS_SimpleStr,boost::shared_ptr<Transcoder_Descr>>::iterator i = m_transcoders_pool.find(name);
		if(i==m_transcoders_pool.end()||e_ready_for_free == i->second->state || e_wait_for_release == i->second->state)
			return;
		t_descr = i->second;
	}

	const char* from = cnt.GetStrValueRef(FROM_PARAM);
	const char* to = cnt.GetStrValueRef(TO_PARAM);
	const char* dn = cnt.GetStrValueRef(FROM_PARAM);
	const char* mess = cnt.GetStrValueRef(MESSAGE_PARAM);

	t_descr->transcoder->ChatFromVisi(from, to, dn, mess);
}

void VS_TranscodersDispatcher::SendCommand_Method(VS_Container &cnt)
{
	const char *name = cnt.GetStrValueRef(NAME_PARAM);
	if (!name || !*name)
		return;
	boost::shared_ptr<Transcoder_Descr> t_descr;
	{
		VS_AutoLock lock(this);
		std::map<VS_SimpleStr,boost::shared_ptr<Transcoder_Descr>>::iterator i = m_transcoders_pool.find(name);
		if(i==m_transcoders_pool.end()||e_ready_for_free == i->second->state || e_wait_for_release == i->second->state)
			return;
		t_descr = i->second;
	}

	const char* from = cnt.GetStrValueRef(FROM_PARAM);
	const char* comm = cnt.GetStrValueRef(MESSAGE_PARAM);

	t_descr->transcoder->CommandFromVisi(from, comm);
}

void VS_TranscodersDispatcher::FECC_Method(VS_Container &cnt)
{
	const char *name = cnt.GetStrValueRef(NAME_PARAM);
	if (!name || !*name)
		return;

	boost::shared_ptr<Transcoder_Descr> t_descr;
	{
		VS_AutoLock lock(this);
		auto &&i = m_transcoders_pool.find(name);
		if (i == m_transcoders_pool.end() || e_ready_for_free == i->second->state || e_wait_for_release == i->second->state)
			return;
		t_descr = i->second;
	}

	const char* from = cnt.GetStrValueRef(FROM_PARAM);
	const char* to = cnt.GetStrValueRef(TO_PARAM);
	eFeccRequestType type;
	int32_t type_value;
	if (cnt.GetValue(TYPE_PARAM, type_value))
		type = decltype(type)(type_value);
	int32_t extra_param;
	if (type == eFeccRequestType::SAVE_PRESET ||
		type == eFeccRequestType::USE_PRESET)
		cnt.GetValue(PRESET_NUM_PARAM, extra_param);
	else if (type == eFeccRequestType::SET_STATE ||
		type == eFeccRequestType::MY_STATE)
		cnt.GetValue(FECC_STATE_PARAM, extra_param);

	t_descr->transcoder->FECCFromVisi(from, to, type, extra_param);
}

void VS_TranscodersDispatcher::LoginAsUser_Method(VS_Container &cnt)
{
	const char *name = cnt.GetStrValueRef(NAME_PARAM);
	if (!name || !*name)
		return;

	boost::shared_ptr<Transcoder_Descr> t_descr;
	{
		VS_AutoLock lock(this);
		std::map<VS_SimpleStr,boost::shared_ptr<Transcoder_Descr>>::iterator i = m_transcoders_pool.find(name);
		if(i==m_transcoders_pool.end()||e_ready_for_free == i->second->state || e_wait_for_release == i->second->state)
			return;
		t_descr = i->second;
	}
	t_descr->transcoder->LoginAsUserResponse( cnt );
}
#endif