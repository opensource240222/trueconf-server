#include "VS_ConnectionsCheck.h"
#include "../std/cpplib/VS_SimpleStr.h"
#include "std/cpplib/ThreadUtils.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std-generic/compat/condition_variable.h"
#include "../net/EndpointRegistry.h"
#include "../CheckSrv/CheckSrv/VS_ClientCheckSrv.h"
#include "../acs/VS_AcsDefinitions.h"

#include <boost/make_shared.hpp>

#include <chrono>
#include <thread>

namespace
{
class VS_ConnectionsCheckFastImpl
{
	VS_SimpleStr m_ep_name;
	boost::weak_ptr<VS_ConnectionsCheckFastImpl> m_this_ptr;
	vs::condition_variable m_condition;
	std::mutex m_mutex;
	bool m_result_fired;
	uint32_t m_response_mills;

	void Result(const unsigned id, const VS_CheckSrvResult ceck_srv_res, uint32_t server_response_mills)
	{
		VS_SCOPE_EXIT{ m_condition.notify_one(); };
		{
			if(e_chksrv_ok!=ceck_srv_res)
				return;
			std::unique_lock<std::mutex> lock(m_mutex);
			if(m_result_fired)
				return;
			m_result_fired = true;

			net::endpoint::MakeFirstConnectTCP(id + 1, m_ep_name.m_str);
			m_response_mills = server_response_mills;
		}
	}
	VS_ConnectionsCheckFastImpl():m_result_fired(false),m_response_mills(-1)
	{
	}

	friend class boost::signals2::deconstruct_access;
	template<typename T> friend
		void adl_postconstruct(const boost::shared_ptr<VS_ConnectionsCheckFastImpl> &p,T *inst)
	{
		p->m_this_ptr = p;
	}
public:
	~VS_ConnectionsCheckFastImpl()
	{
	}
	void ConnectionsCheck(const char *endpoint, const unsigned long mills, unsigned long *srv_response_time = 0)
	{
		if(strlen(endpoint)>VS_ACS_MAX_SIZE_ENDPOINT_NAME)
			return;
		m_ep_name = endpoint;
		unsigned n = net::endpoint::GetCountConnectTCP(endpoint);
		if(n>256)
			n = 256;
		{
			 std::unique_lock<std::mutex> lock(m_mutex);
			for(unsigned i=0;i<n;i++)
			{
				auto check_srv = boost::make_shared<VS_ClientCheckSrvFast>(net::endpoint::ReadConnectTCP(i + 1, endpoint), mills, 0);
				VS_ClientCheckSrvFast::ResultSignalType::slot_type slot(&VS_ConnectionsCheckFastImpl::Result,this,i,_1,_2);
				check_srv->ConnectToSignal(slot.track(m_this_ptr.lock()));
				std::thread([check_srv]() {
					vs::SetThreadName("AsyncCheckSrv");
					check_srv->Check();
				}).detach();
			}
		}
		if(n>0)
		{
			std::unique_lock<std::mutex> lock(m_mutex);
			if(std::cv_status::no_timeout == m_condition.wait_for(lock,std::chrono::milliseconds(mills)))
			{
				if(srv_response_time)
					*srv_response_time = m_response_mills;// + m_epCount/50;
			}
		}
	}
};
}

void VS_ConnectionsCheckFast(const char *endpoint, const unsigned long mills,  unsigned long *srv_response_time)
{
	boost::shared_ptr<VS_ConnectionsCheckFastImpl> check = boost::signals2::deconstruct<VS_ConnectionsCheckFastImpl>();
	check->ConnectionsCheck(endpoint, mills,srv_response_time);
}