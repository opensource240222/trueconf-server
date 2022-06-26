#include "TransportLayer.h"
#include "chatlib/helpers/AccountInfo.h"
#include "chatlib/helpers/ExternalComponentsInterface.h"
#include "chatlib/helpers/ResolverInterface.h"
#include "chatlib/interface/GlobalConfigInterface.h"
#include "chatlib/interface/TransportChannel.h"
#include "chatlib/msg/attr.h"
#include "chatlib/msg/ChatMessage.h"

namespace chat
{
template<typename LayersTraits>
template<typename ...Args>
std::shared_ptr<TransportLayer<LayersTraits>>
TransportLayer<LayersTraits>::MakeTransportLayer(
	const GlobalConfigPtr&cfg,
	std::unique_ptr<TransportChannel>&&channel,
	Args&&...args)
{
	auto res = std::make_shared<TransportLayer<LayersTraits>>(
		cfg, std::move(channel),std::forward<Args>(args)...);
	res->Init();
	return res;
}
template<typename LayersTraits>
template<typename... Args>
TransportLayer<LayersTraits>::TransportLayer(
	const GlobalConfigPtr&cfg,
	std::unique_ptr<TransportChannel> &&channel,
	Args&& ...args)
	: LayerHelper<LayersTraits>(std::forward<Args>(args)...)
	, channel_(std::move(channel))
	, cfg_(cfg)
{
}
template<typename LayersTraits>
void TransportLayer<LayersTraits>::Init()
{
	channel_->SetOnMessageRecvCallBack(
		[this, self_weak = this->weak_from_this()](msg::ChatMessagePtr &&m, CallIDRef sender)
		{
			auto self = self_weak.lock();
			if (!self)
				return;
			this->PostCall(vs::move_handler(
				[
					self = std::move(self),
					m = std::move(m),
					sender = CallID(sender)
				]() mutable
			{
				self->OnChatMessageArrived(std::move(m), sender);
			}));
		});
}
template<typename LayersTraits>
void TransportLayer<LayersTraits>::ForwardBelowMessage(msg::ChatMessagePtr&&m,
	std::vector<ParticipantDescr>&& /*dstParts*/)
{
	this->PostCall(vs::move_handler([this, m = std::move(m)]() mutable
	{
		auto call_id = m->GetParamStr(attr::DST_CALLID_paramName);
		auto endpoint = m->GetParamStr(attr::DST_ENDPOINT_paramName);
		auto cfg = GetCfg();
		if (!cfg)
			return;
		m->SetParam(
			attr::SRC_ENDPOINT_paramName,
			cfg->GetCurrentAccount()->GetExactCallID());
		if (!endpoint.empty())
		{
			channel_->Send(std::move(m), { endpoint });
			return;
		}
		if (call_id.empty())
			return;
		auto self_weak = this->weak_from_this(); // MSVC gets confused when self_weak is used directly in the capture list and complains that there is no weak_from_this() in the class generated for parent lambda.
		auto resolver = cfg->GetResolver();
		resolver->ResolveCallIdType(
			call_id,
			[
				this,
				self_weak = std::move(self_weak),
				call_id,
				m = m.release(),
				resolver
			]
		(vs::CallIDType type) mutable
		{
			auto msg = msg::ChatMessagePtr(m);
			m = nullptr;
			auto self = self_weak.lock();
			if (!self)
				return;
			switch (type)
			{
			case vs::CallIDType::server:
				this->PostCall(vs::move_handler(
					[
						this,
						self = std::move(self),
						msg = std::move(msg),
						call_id = std::move(call_id)
					]() mutable
				{
					channel_->Send(std::move(msg), { call_id });
				}));
				break;
			case::vs::CallIDType::client:
			{
				resolver->Resolve(
					call_id,
					[this, self_weak = std::move(self_weak), m = msg.release()]
				(bool res, vs::ResolverInterface::ResolveInfo &&info) mutable
				{
					if (!res)
						return;
					auto msg = msg::ChatMessagePtr(m);
					m = nullptr;
					auto self = self_weak.lock();
					if (!self)
						return;
					auto cfg = GetCfg();
					assert(cfg);
					auto current_account = cfg->GetCurrentAccount();
					assert(current_account);
					auto it = std::remove(
						info.epList.begin(),
						info.epList.end(),
						current_account->GetExactCallID());
					info.epList.erase(it, info.epList.end());
					this->PostCall(vs::move_handler(
						[
							this,
							self = std::move(self),
							msg = std::move(msg),
							info =std::move(info)
						]() mutable
					{
						channel_->Send(
							std::move(msg),
							std::move(info.epList));
					}));
				});
			}
			break;
			case vs::CallIDType::undef:
				break;
			}
		});
	}));
}
template<typename LayersTraits>
void TransportLayer<LayersTraits>::OnChatMessageArrived(
	msg::ChatMessagePtr &&msg,
	CallIDRef sender)
{
	auto src_ep = msg->GetParamStrRef(attr::SRC_ENDPOINT_paramName);
	ForwardAboveMessage(std::move(msg), src_ep.empty() ? sender : src_ep);
}
template<typename LayersTraits>
void TransportLayer<LayersTraits>::ShutDown()
{
	LayerHelper<LayersTraits>::ShutDown();
}
template<typename LayersTraits>
GlobalConfigPtr TransportLayer<LayersTraits>::GetCfg() const
{
	auto cfg = cfg_.lock();
	assert(cfg);
	return cfg;
}
}
