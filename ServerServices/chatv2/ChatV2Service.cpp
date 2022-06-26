#include "ChatV2Service.h"

#include "chatlib/helpers/AccountInfo.h"
#include "chatlib/helpers/ExternalComponentsInterface.h"
#include "chatlib/helpers/ResolverInterface.h"
#include "chatlib/interface/GlobalConfigInterface.h"
#include "chatlib/interface/TransportChannel.h"
#include "chatlib/msg/attr.h"
#include "chatlib/msg/ChatMessage.h"
#include "chatlib/notify/ChatEventsFuncs.h"
#include "chatlib/storage/make_chat_storage.h"
#include "chatlib/utils/chat_utils.h"

#include "std-generic/cpplib/VS_Container_io.h"
#include "std-generic/compat/memory.h"
#include "std-generic/cpplib/synchronized.h"
#include "std/cpplib/VS_CallIDUtils.h"
#include "std/cpplib/VS_Protocol.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/VS_Utils.h"
#include "std/cpplib/MakeShared.h"
#include "std/debuglog/VS_Debug.h"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

#define DEBUG_CURRENT_MODULE VS_DM_CHATS
namespace detail
{
inline std::shared_ptr<chat::ChatStorage>
GetChatStorageBackEnd(string_view serverName)
{
	// FIXME: Use common connection string setting for DB when chat DB will be integrated in TCS;
	static const char CHAT_DB_CONNECTION_STRING_TAG[] = "Chat DB Connection String";
	auto cfg = VS_RegistryKey(false, CONFIGURATION_KEY);
	std::unique_ptr<char, free_deleter> val;
	std::string connString;
	cfg.GetString(connString, CHAT_DB_CONNECTION_STRING_TAG);
	if (!connString.empty())
	{
		if(boost::starts_with(connString, "postgresql:"))
			return chat::make_chat_storage(connString);
	}
	std::string db_name("sqlite3:db=");
	db_name += serverName;
	std::replace(db_name.begin(), db_name.end(), '/', '_');
	db_name += ".sqlite";
	return chat::make_chat_storage(db_name);
}
template<class SendFunc>
class Channel
	: public chat::TransportChannel
{
	SendFunc m_sendFunc;
	nod::signal<void()> m_fireOnDie;
public:
	Channel(
		SendFunc&& f)
		: m_sendFunc(std::move(f))
	{}
	~Channel()
	{
		m_fireOnDie();
	}
	nod::connection ConnectToOnDie(const nod::signal<void()>::slot_type &slot)
	{
		return m_fireOnDie.connect(slot);
	}
	void Send(chat::msg::ChatMessagePtr &&m, std::vector<chat::CallID> &&endpoints)
	{
		m_sendFunc(std::move(m), std::move(endpoints));
	}
	void OnMsgArrived(chat::msg::ChatMessagePtr&&m)
	{
		ForwardAboveMessage(std::move(m), {});
	}
};
class ChatEnv
	: public chat::GlobalConfigInterface
	, public vs::ExternalComponentsInterface
	, public vs::enable_shared_from_this<ChatEnv>
{
	static const uint32_t s_maxChainLen = 0x1000;
	static const uint32_t s_bucketCapacity = 0x20;
public:
	chat::asio_sync::AppLayerPtr MakeLayout(
		const chat::AccountInfoPtr account,
		const vs::ResolverPtr& resolver,
		std::unique_ptr<chat::TransportChannel>&&ch)
	{
		m_account = account;
		m_resolver = resolver;
		m_externalComp = shared_from_this();
		auto chatStorage = GetChatStorageBackEnd(m_account->GetCallID());
		m_chatStorage = chatStorage;
		auto storageLayer = chat::asio_sync::MakeStorageLayer(chatStorage,
			m_ios);
		storageLayer->Init(shared_from_this());
		m_appLayer = chat::asio_sync::MakeAppLayer(shared_from_this(), storageLayer, m_ios);
		auto integrity = chat::asio_sync::MakeIntegrityLayer(shared_from_this(), m_ios);
		m_syncChat = integrity;
		auto delivery = chat::asio_sync::MakeDeliveryLayer(shared_from_this(), m_ios);
		auto transport = chat::asio_sync::MakeTransportLayer(
			shared_from_this(),
			std::move(ch),
			m_ios);
		auto system_chat = chat::asio_sync::MakeSystemChatLayer(shared_from_this(), m_ios);
		m_appLayer->SetNextLayer(system_chat);
		system_chat->SetNextLayer(storageLayer);
		storageLayer->SetNextLayer(integrity);
		integrity->SetNextLayer(delivery);
		delivery->SetNextLayer(transport);
		return m_appLayer;
	}
protected:
	ChatEnv(boost::asio::io_service &ios)
		:m_ios(ios)
	{}
private:
	chat::ChatStoragePtr GetChatStorage() const override
	{
		return m_chatStorage.lock();
	}
	chat::ChatEventsNotifierPtr GetEventsNotifier() const override
	{
		return m_chatEvents;
	}
	chat::ChatEventsSubscriptionPtr GetEventsSubscription() const override
	{
		return m_chatEvents;
	}
	vs::ExternalComponentsPtr GetExternalComponents() const
	{
		return m_externalComp;
	}
	chat::AccountInfoPtr GetCurrentAccount() const override
	{
		return m_account;
	}
	chat::ClockWrapperPtr GetClockWrapper() const
	{
		return m_clockWrapper;
	}
	uint32_t GetMaxChainLen() const override
	{
		return s_maxChainLen;
	}
	uint32_t GetDefaultChainLen() const
	{
		return s_maxChainLen;
	}
	uint32_t GetBucketCapacity() const override
	{
		return s_bucketCapacity;
	}
	virtual uint32_t GetTailLength() const override
	{
		return s_maxChainLen;
	}
	std::shared_ptr<chat::SyncChatInterface>
		GetSyncChat() const override
	{
		return m_syncChat;
	}
	vs::ResolverPtr GetResolver() const override
	{
		return m_resolver;
	}

	chat::AccountInfoPtr m_account;
	std::weak_ptr<chat::ChatStorage> m_chatStorage;
	chat::asio_sync::AppLayerPtr m_appLayer;
	std::shared_ptr<chat::SyncChatInterface> m_syncChat;
	std::shared_ptr<chat::notify::ChatEventsFuncs>	m_chatEvents
		= std::make_shared<chat::notify::ChatEventsFuncs>();
	chat::ClockWrapperPtr m_clockWrapper
		= std::make_shared<steady_clock_wrapper>();
	vs::ResolverPtr m_resolver;
	vs::ExternalComponentsPtr m_externalComp;
	boost::asio::io_service &m_ios;
};
inline string_view to_vs(const char *str)
{
	if (str)
		return str;
	else
		return {};
}
}
bool ChatV2Service::IsChatV2Enabled()
{
	static const char CHAT_V2_SERVICE_ENABLED_TAG[] = "ChatV2 enabled";
	uint32_t val(0);
	auto cfg = VS_RegistryKey(false, CONFIGURATION_KEY);
	cfg.GetValue(&val, sizeof(val), RegistryVT::VS_REG_INTEGER_VT, CHAT_V2_SERVICE_ENABLED_TAG);
	return val > 0;
}
bool ChatV2Service::Processing(std::unique_ptr<VS_RouterMessage>&& recvMess)
{
	if (!recvMess)
		return true;
	static int s_count = 0;
	++s_count;
	dstream4 << "s_count = " << s_count;
	VS_Container cnt;
	if (!cnt.Deserialize(recvMess->Body(), recvMess->BodySize()))
		return true;
	auto method = detail::to_vs(cnt.GetStrValueRef(METHOD_PARAM));
	dstream4 << "ChatV2Service: method = " << method;
	if (method == SENDMESSAGE_METHOD)
	{
		VS_Container msg_cnt;
		std::vector<chat::CallID> dst;
		cnt.Reset();
		bool msg_for_me(false);
		while (cnt.Next())
		{
			auto name = detail::to_vs(cnt.GetName());
			if (name == CALLID_PARAM)
			{
				auto call_id = detail::to_vs(cnt.GetStrValueRef());
				if (call_id != OurEndpoint())
					dst.emplace_back(call_id);
				else
					msg_for_me = true;
			}
		}
		if ((dst.empty() && !msg_for_me) || !cnt.GetValue(MESSAGE_PARAM, msg_cnt))
		{
			dstream4 << "ChatV2Service: " << (
				dst.empty()
				? "CALLID_PARAM"
				: "MESSAGE_PARAM" ) << " is empty";
			return true;
		}
		dstream4 << "In:\n" << msg_cnt << '\n';
		auto msg = std::make_unique<chat::msg::ChatMessage>(std::move(msg_cnt));
		auto type = chat::MessageType::undefined;
		msg->GetParamI32(chat::attr::MESSAGE_TYPE_paramName, type);
		if (type == chat::MessageType::undefined)
			return true;
		if(msg_for_me)
			m_onMsgArrived(std::make_unique<chat::msg::ChatMessage>(msg->GetContainer()));
		if(!dst.empty())
			Send(std::move(msg), std::move(dst));
	}
	return true;
}
bool ChatV2Service::Init(
	const char *our_endpoint,
	const char *our_service,
	const bool permittedAll)
{
	m_atp.Start();
	m_isStarted = true;
	return true;
}
void ChatV2Service::Send(
	chat::msg::ChatMessagePtr && msg,
	std::vector<chat::CallID>&& endpoints)
{
	dstream4 << "Out:\n" << msg->GetContainer();
	for (const auto& i : endpoints)
	{
		bool to_server = VS_IsBrokerFormat(i);
		PostRequest(
			to_server ? i.c_str() : nullptr,
			to_server ? nullptr : i.c_str(),
			msg->GetContainer(),
			nullptr,
			OurService());
	}
}
bool ChatV2Service::ConstructChatLayout(const vs::ResolverPtr& resolver)
{
	chat::SetUUIDGeneratorFunc([]() {
		return boost::uuids::to_string(boost::uuids::random_generator()()); });
	m_resolver = resolver;
	auto account = std::make_shared<chat::AccountInfo>(
		OurEndpoint(),
		OurEndpoint(),
		vs::CallIDType::server,
		OurEndpoint(),
		[this](chat::CallIDRef call_id)
			{ return std::vector<chat::CallID>{ OurEndpoint() }; });

	auto send_f = [this] (
		chat::msg::ChatMessagePtr &&m,
		std::vector<chat::CallID> &&endpoints)
		{
			Send(std::move(m), std::move(endpoints));
		};
	using channel_gen = detail::Channel<decltype(send_f)>;
	auto ch = std::make_unique<channel_gen>(std::move(send_f));
	m_onMsgArrived = [ch_p = ch.get()](chat::msg::ChatMessagePtr&&m)
	{
		ch_p->OnMsgArrived(std::move(m));
	};
	ch->ConnectToOnDie([this]()
	{
		m_onMsgArrived = {};
	});
	auto cfg = vs::MakeShared<detail::ChatEnv>(m_atp.get_io_service());
	m_cfg = cfg;
	m_app = cfg->MakeLayout(
		account,
		resolver,
		std::move(ch));
	return !!m_app;
}
void ChatV2Service::DeconstructChatLayout()
{
	if(m_app)
		m_app->ShutDown();
	m_onMsgArrived = {};
	m_cfg.reset();
}