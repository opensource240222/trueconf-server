#include "SystemChatLayerFixture.h"
#include "tests/UnitTestChat/TestHelpers.h"

#include "chatlib/helpers/AccountInfo.h"
#include "chatlib/factory_asio/make_layers_asio.h"
#include "chatlib/notify/ChatEventsFuncs.h"
#include "chatlib/storage/make_chat_storage.h"

#include "chatutils/ExternalComponentsStub.h"
#include "chatutils/GlobalConfigStub.h"

namespace chat_test
{
SystemChatLayerFixture::SystemChatLayerFixture()
	: m_atp(1)
	, m_notifier(std::make_shared<chat::notify::ChatEventsFuncs>())
	, m_resolver(std::make_shared<ResolverStub>())
{
	m_atp.Start();
}
SystemChatLayerFixture::~SystemChatLayerFixture()
{
	m_atp.Stop();
}
chat::ChatEventsNotifierPtr SystemChatLayerFixture::GetEventsNotifier() const
{
	return std::static_pointer_cast<chat::notify::ChatEventsNotifier>(m_notifier);
}
void SystemChatLayerFixture::AddResolveRecord(chat::CallIDRef callId, vs::CallIDType type,
	string_view bs, std::vector<chat::CallID>&& instances)
{
	m_resolver->Add(callId, type, bs, std::move(instances));
	m_resolver->AddAlias(chat::CallID(callId), chat::CallID(callId));
}
void SystemChatLayerFixture::AddCtxStamp(const chat::GlobalContext& stamp)
{
	for (const auto& cfg : m_cfgs)
		cfg.first->GetChatStorage()->SaveGlobalContext(stamp);
}
SystemChatLayerFixture::SysChatLayerWithMock
SystemChatLayerFixture::GetSysChatLayer(const chat::AccountInfoPtr& account)
{
	auto cfg = std::make_shared<GlobalConfigStub>(m_resolver, m_notifier);
	cfg->SetAccountInfo(account);
	auto sysChat = chat::asio_sync::MakeSystemChatLayer(cfg, m_atp.get_io_service());
	auto db_descr = chat_test::CreateSharedDBInMemory(account->GetCallID());
	auto bknd = chat::make_chat_storage(db_descr.dbConnParam.connectionString);
	auto storage = chat::asio_sync::MakeStorageLayer(bknd, m_atp.get_io_service());
	cfg->SetChatStorage(bknd);
	storage->Init(cfg);
	m_cfgs.emplace_back(cfg, std::move(db_descr.db));
	auto mock = std::make_shared<LayerInterface_Mock>();
	sysChat->SetNextLayer(mock);
	m_storages.emplace_back(storage);
	return { sysChat, mock };
}
}