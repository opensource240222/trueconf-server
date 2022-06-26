#pragma once

#include "std-generic/cpplib/StrCompare.h"
#include "std-generic/cpplib/string_view.h"
#include "std-generic/asio_fwd.h"

#include <boost/asio/steady_timer.hpp>
#include <boost/shared_ptr.hpp>

#include "std-generic/compat/functional.h"
#include "std-generic/compat/map.h"
#include <memory>
#include <mutex>

class VS_FakeClientInterface;
class VS_Policy;
class VS_WorkThread;

class VS_FakeClientManager
{
public:
	static void Init(boost::asio::io_service& ios);
	static void DeInit();
	static VS_FakeClientManager& Instance()
	{
		return *s_instance;
	}

public:
	explicit VS_FakeClientManager(boost::asio::io_service& ios);
	~VS_FakeClientManager();

	void Stop();
	void RegisterClient(const std::shared_ptr<VS_FakeClientInterface>& client);
	std::shared_ptr<VS_FakeClientInterface> GetClient(string_view cid);
	VS_Policy& LoginPolicy();

#if defined(_WIN32)
	const boost::shared_ptr<VS_WorkThread>& GetExternalThread() const;
#endif

private:
	std::mutex m_mutex;
	vs::map<std::string, std::weak_ptr<VS_FakeClientInterface>, vs::str_less> m_clients;

	struct AsyncState;
	std::shared_ptr<AsyncState> m_async_state;

#if defined(_WIN32)
	boost::shared_ptr<VS_WorkThread> m_external_thread;
#endif

private:
	static std::unique_ptr<VS_FakeClientManager> s_instance;
};
