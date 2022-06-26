#pragma once

#include "VS_ClientControlAllocatorInterface.h"
#include "transport/Router/VS_TransportRouterServiceHelper.h"
#include "std/cpplib/event.h"
#include "std/cpplib/VS_Lock.h"
#include "std/cpplib/VS_SimpleStr.h"

#include <boost/signals2/connection.hpp>
#include <boost/shared_ptr.hpp>

#include <map>
#include <thread>

class VS_ClientControlInterface;
class VS_TranscoderControl;
namespace ts { struct IPool; }
class VS_TranscoderLogin;

/// transcoders manipulation class

/**
	одноврменно запускаются в режиме ожидание n траскодеров, если их становится меньше m, то запускается недостающее кол-во по таймауту.
	Общее кол-во транскодеров не может быть больше max_value;
*/
class VS_TranscodersDispatcher :public VS_ClientControlAllocatorInterface,
								public VS_TransportRouterServiceReplyHelper,
								public VS_Lock
{
	enum Transcoder_State
	{
		e_none,
		e_started,
		e_owned,
		e_wait_for_release,
		e_ready_for_free
	};

struct Transcoder_Descr
{
	Transcoder_Descr():state(e_none)
	{}
	Transcoder_State state;
	boost::shared_ptr<VS_TranscoderControl>	transcoder;
	boost::signals2::connection	onZombieConn;
};

	vs::event m_shutdownEvent;
	bool m_Stop;

	unsigned m_lower_limit;
	unsigned m_max_value; //maximum of transcoders
	unsigned m_pool_sz; //transoders pool size
	unsigned m_trans_in_use_count;

	bool	m_zombieDetected;
	std::map<VS_SimpleStr,boost::shared_ptr<Transcoder_Descr>>	m_transcoders_pool;
	VS_SimpleStr m_serverAddrs;

	void FreeZombie();
	void GetNewName(VS_SimpleStr &new_name);

	void WaitForShutdown();

	void TranscoderInit_Method(VS_Container &cnt);
	void Hangup_Method(VS_Container &cnt);
	void SetAppData_Method(VS_Container &cnt);
	void InviteReply_Method(VS_Container &cnt);
	void Invite_Method(VS_Container &cnt);
	void PrepareTranscoderForCall_Method(VS_Container &cnt);
	void ClearCall_Method(VS_Container &cnt);
	void SetMediaChannels_Method(VS_Container &cnt);
	void LoginAsUser_Method(VS_Container &cnt);
	void CleanGarbage();

	std::thread m_timeoutThread;
	boost::signals2::connection m_timeout_slot_conn;
	std::weak_ptr<ts::IPool> m_transPool;
	std::weak_ptr<VS_TranscoderLogin> m_transLogin;

protected:
	VS_TranscodersDispatcher(const unsigned pool_sz, const unsigned long max_trans_allowed, const unsigned long lower_limit, const std::weak_ptr<ts::IPool> &pool, const std::weak_ptr<VS_TranscoderLogin>& transLogin);
	static VS_TranscodersDispatcher *m_instance;
	static VS_Lock	*m_lock_instance;
public:
	static VS_TranscodersDispatcher *GetInstanceTranscodersDispatcher(const std::weak_ptr<ts::IPool> &pool, const std::weak_ptr<VS_TranscoderLogin>& transLogin);
	virtual ~VS_TranscodersDispatcher();
	unsigned Start();
	void SetMaxTranscoders(const int max_transcoders);
	void SetServerAddresses(const char *addrs);

	boost::shared_ptr<VS_ClientControlInterface> GetTranscoder() override;
	void ReleaseTranscoder(boost::shared_ptr<VS_ClientControlInterface> t);

	/////Service
	bool Init(const char* our_endpoint, const char* our_service, const bool permittedAll = false) override;
	bool Processing(std::unique_ptr<VS_RouterMessage>&& recvMess) override;

	void OnZombieTranscoder(const char*);
	void Shutdown();

	void SendToTranscoder(VS_SimpleStr &name, VS_Container &cnt);
	void SendToUnAuthTranscoder(VS_SimpleStr &cid, VS_Container &cnt);

	void SendMessage_Method(VS_Container &cnt);
	void SendCommand_Method(VS_Container &cnt);
	void FECC_Method(VS_Container &cnt);
};
