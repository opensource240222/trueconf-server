#pragma once
#include "chatlib/chat_defs.h"
#include "chatlib/factory_asio/make_layers_asio.h"
#include "std/cpplib/ASIOThreadPool.h"
#include "transport/Router/VS_TransportRouterServiceHelper.h"
#include "AppServer/Services/VS_PresenceService.h"

class ChatV2Service
	: public VS_TransportRouterServiceHelper
	, public VS_PresenceServiceMember
{
public:
	ChatV2Service()
		: m_atp(1)
	{ }
	~ChatV2Service()
	{
		for (auto &i : m_signalConns)
		{
			i.disconnect();
		}
		DeconstructChatLayout();
		m_atp.Stop();
	}
	bool ConstructChatLayout(const vs::ResolverPtr& resolver);
	void DeconstructChatLayout();
	bool IsStarted() const noexcept
	{
		return m_isStarted;
	}
	static bool IsChatV2Enabled();
private:
	bool Processing(std::unique_ptr<VS_RouterMessage> &&recvMess) override;
	bool Init(
		const char *our_endpoint,
		const char *our_service,
		const bool permittedAll = false) override;
	void Send(chat::msg::ChatMessagePtr &&msg, std::vector<chat::ChatID> &&endpoints);

	std::function<void(chat::msg::ChatMessagePtr &&)> m_onMsgArrived;
	chat::asio_sync::AppLayerPtr m_app;
	chat::GlobalConfigPtr m_cfg;
	vs::ASIOThreadPool m_atp;
	std::vector<vs::SubscribeConnection> m_signalConns;
	std::weak_ptr<vs::ResolverInterface> m_resolver;
	bool m_isStarted = false;
};
