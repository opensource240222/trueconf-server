#include "VS_FakeClientControlAllocator.h"
#include "FakeClient/VS_FakeClient.h"
#include "FakeClient/VS_FakeClientManager.h"
#include "FakeClient/VS_FakeEndpoint.h"
#include "../../std/cpplib/VS_RegistryKey.h"
#include "../../std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/MakeShared.h"
#include "../../tools/Server/VS_Server.h"
#include "../../TransceiverLib/VS_RTPModuleControl.h"
#include "../../TransceiverLib/VS_TransceiverProxy.h"
#ifdef _WIN32
#include "VS_TranscodersDispatcher.h"
#endif
#include "VS_FakeClientControl.h"
#include "TransceiverLib/TransceiversPool.h"

#include <boost/make_shared.hpp>
#include <boost/algorithm/string/predicate.hpp>

std::unique_ptr<VS_ClientControlAllocatorInterface> VS_ClientControlAllocatorInterface::s_instance;

void VS_ClientControlAllocatorInterface::Init(const std::weak_ptr<ts::IPool>& pool, const std::weak_ptr<VS_TranscoderLogin>& transLogin)
{
	if (s_instance)
		return;

	std::string type;
	VS_RegistryKey(false, CONFIGURATION_KEY, false, true).GetString(type, "RTP Module");
#ifdef _WIN32
	if      (boost::iequals(type, "Transcoder"))
		s_instance.reset(VS_TranscodersDispatcher::GetInstanceTranscodersDispatcher(pool, transLogin));
	else
#endif
	/*if (boost::iequals(type, "FakeClient") || true)*/
		s_instance = std::make_unique<VS_FakeClientControlAllocator>(pool, transLogin);
}

VS_FakeClientControlAllocator::VS_FakeClientControlAllocator(const std::weak_ptr<ts::IPool> &pool, const std::weak_ptr<VS_TranscoderLogin>& transLogin)
	: m_maxClients(0)
	, m_TransPool(pool)
	, m_transLogin(transLogin)
{
}

VS_FakeClientControlAllocator::~VS_FakeClientControlAllocator(void)
{
}

boost::shared_ptr<VS_ClientControlInterface> VS_FakeClientControlAllocator::GetTranscoder()
{

	boost::shared_ptr<VS_FakeClientControl> client = boost::make_shared<VS_FakeClientControl>(m_TransPool, m_transLogin);

	auto ep = VS_FakeEndpointFactory::Instance().Create();
	if (!ep)
		return nullptr;
	auto fc = vs::MakeShared<VS_FakeClient>(std::move(ep));
	client->SetFakeClientInterface(fc);

	std::lock_guard<std::mutex> lock(m_data_lock);

	if (m_maxClients <= m_clients.size()) return boost::shared_ptr<VS_ClientControlInterface>();
	m_clients.insert(client);
	VS_FakeClientManager::Instance().RegisterClient(fc);
	return client;
}

void VS_FakeClientControlAllocator::ReleaseTranscoder(boost::shared_ptr<VS_ClientControlInterface> t)
{
	std::lock_guard<std::mutex> lock(m_data_lock);
	m_clients.erase(t);
}

void VS_FakeClientControlAllocator::SetMaxTranscoders(const int max_transcoders)
{
	std::lock_guard<std::mutex> lock(m_data_lock);
	m_maxClients = max_transcoders;
}