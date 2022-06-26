#pragma once
#include "chatlib/layers/ChatLayerAbstract.h"

#include "std-generic/compat/memory.h"

namespace chat
{
class TransportChannel;
template<typename LayersTraits>
class TransportLayer: public LayerInterface,
					  public LayerHelper<LayersTraits>,
					  public vs::enable_shared_from_this<TransportLayer<LayersTraits>>
{
	void ForwardBelowMessage(msg::ChatMessagePtr&& msg,
		std::vector<ParticipantDescr>&& dstParts) override;
	void OnChatMessageArrived(
		msg::ChatMessagePtr&&msg,
		CallIDRef sender) override;
	void ShutDown() override;
	GlobalConfigPtr GetCfg() const;

	std::unique_ptr<TransportChannel> channel_;
	std::weak_ptr<GlobalConfigInterface> cfg_;
public:
	template<typename ...Args>
	static std::shared_ptr<TransportLayer<LayersTraits>>
		MakeTransportLayer(
			const GlobalConfigPtr &cfg,
			std::unique_ptr<TransportChannel>&&channel,
			Args&&...args);
	template<typename ...Args>
	TransportLayer(
		const GlobalConfigPtr&cfg,
		std::unique_ptr<TransportChannel> &&channel,
		Args&&... args);
	void Init();
};
}