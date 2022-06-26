#include "FakeClient/VS_FakeClientManager.h"
#include "FakeClient/VS_FakeClientInterface.h"
#include "FakeClient/VS_FakeEndpoint.h"
#include "std/cpplib/VS_Policy.h"
#if defined(_WIN32)
#include "std/cpplib/VS_WorkThreadIOCP.h"
#endif

#include <boost/asio/steady_timer.hpp>
#include <boost/make_shared.hpp>

#include "std-generic/compat/memory.h"
#include <cassert>

std::unique_ptr<VS_FakeClientManager> VS_FakeClientManager::s_instance;

struct VS_FakeClientManager::AsyncState
{
	explicit AsyncState(boost::asio::io_service& ios)
		: timer(ios)
		, login_policy("FakeClient")
	{
	}

	void ScheduleTimer(std::weak_ptr<AsyncState> self_weak);

	boost::asio::steady_timer timer;
	VS_Policy login_policy;
};

void VS_FakeClientManager::AsyncState::ScheduleTimer(std::weak_ptr<AsyncState> self_weak)
{
	timer.expires_from_now(std::chrono::seconds(1));
	timer.async_wait([self_weak = std::move(self_weak)](const boost::system::error_code& ec) {
		auto self = self_weak.lock();
		if (!self)
			return;
		if (ec == boost::asio::error::operation_aborted)
			return;

		self->login_policy.Timeout();
		self->ScheduleTimer(self_weak);
	});
}

void VS_FakeClientManager::Init(boost::asio::io_service& ios)
{
	if (s_instance)
		return;
	s_instance = vs::make_unique<VS_FakeClientManager>(ios);
}

void VS_FakeClientManager::DeInit()
{
	if (!s_instance)
		return;
	s_instance->Stop();
	s_instance = nullptr;
}

VS_FakeClientManager::VS_FakeClientManager(boost::asio::io_service& ios)
	: m_async_state(std::make_shared<AsyncState>(ios))
{
	m_async_state->ScheduleTimer(m_async_state);
#if defined(_WIN32)
	m_external_thread = boost::make_shared<VS_WorkThreadIOCP>();
	m_external_thread->Start("FakeEpMgr_Ext");
#endif
}

VS_FakeClientManager::~VS_FakeClientManager()
{
	Stop();
}

void VS_FakeClientManager::Stop()
{
	m_async_state->timer.cancel();

	decltype(m_clients) clients;
	{
		std::lock_guard<decltype(m_mutex)> lock(m_mutex);
		m_clients.swap(clients);
	}

	for (const auto& kv : clients)
	{
		auto client = kv.second.lock();
		if (!client)
			continue;
		client->GetEndpoint().Stop();
	}

#if defined(_WIN32)
	// TODO: stop correctly
	if (m_external_thread)
		m_external_thread->Stop();
	m_external_thread = nullptr;
#endif
}

void VS_FakeClientManager::RegisterClient(const std::shared_ptr<VS_FakeClientInterface>& client)
{
	std::lock_guard<decltype(m_mutex)> lock(m_mutex);
	m_clients[client->CID()] = client;
}

std::shared_ptr<VS_FakeClientInterface> VS_FakeClientManager::GetClient(string_view cid)
{
	std::shared_ptr<VS_FakeClientInterface> result;

	std::lock_guard<decltype(m_mutex)> lock(m_mutex);
	auto it = m_clients.find(cid);
	if (it == m_clients.end())
		return result;

	result = it->second.lock();
	if (!result)
		m_clients.erase(it);
	return result;
}

VS_Policy& VS_FakeClientManager::LoginPolicy()
{
	return m_async_state->login_policy;
}

#if defined(_WIN32)
const boost::shared_ptr<VS_WorkThread>& VS_FakeClientManager::GetExternalThread() const
{
	return m_external_thread;
}
#endif
