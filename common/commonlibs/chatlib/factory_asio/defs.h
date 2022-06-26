#pragma once
#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include <memory>

namespace chat
{
template<typename LayersTraits, typename MainStoragePtr>
class AppLayer;
template<typename LayersTraits>
class DeliveryLayer;
template<typename LayersTraits>
class IntegrityLayer;
template<typename LayersTraits>
class MainStorageLayer;
template<typename LayersTraits>
class TransportLayer;
template<typename LayersTraits>
class SystemChatLayer;
template<
	typename StrandT,
	typename TimerT>
struct LayersTraits;
namespace asio_sync
{
class SteadyTimerBoost;
using LayersTraitsAsioSteadyTimer = LayersTraits<
	boost::asio::io_service::strand,
	SteadyTimerBoost
>;
using LayersTraitsAsioNoTimer = LayersTraits<
	boost::asio::io_service::strand,
	void
>;
using MainStorageLayer = chat::MainStorageLayer<LayersTraitsAsioNoTimer>;
using MainStorageLayerPtr = std::shared_ptr<MainStorageLayer>;
using AppLayer = chat::AppLayer<
	LayersTraitsAsioNoTimer,
	MainStorageLayerPtr>;
using AppLayerPtr = std::shared_ptr<AppLayer>;
using DeliveryLayer = chat::DeliveryLayer<LayersTraitsAsioSteadyTimer>;
using IntegrityLayer = chat::IntegrityLayer<LayersTraitsAsioSteadyTimer>;
using TransportLayer = chat::TransportLayer<LayersTraitsAsioNoTimer>;
using SystemChatLayer = chat::SystemChatLayer<LayersTraitsAsioNoTimer>;
}
}