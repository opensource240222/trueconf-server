#include "make_layers_asio.h"
#include "chatlib/layers/AppLayerImpl.h"
#include "chatlib/layers/DeliveryLayerImpl.h"
#include "chatlib/layers/IntegrityLayerImpl.h"
#include "chatlib/layers/LayersTraits.h"
#include "chatlib/layers/MainStorageLayerImpl.h"
#include "chatlib/layers/SystemChatLayerImpl.h"
#include "chatlib/layers/TransportLayerImpl.h"

#include <chrono>
namespace chat
{
template class chat::AppLayer<
	asio_sync::LayersTraitsAsioNoTimer,
	std::shared_ptr<asio_sync::MainStorageLayer>>;
template class chat::DeliveryLayer<asio_sync::LayersTraitsAsioSteadyTimer>;
template class chat::IntegrityLayer<asio_sync::LayersTraitsAsioSteadyTimer>;
template class chat::MainStorageLayer<asio_sync::LayersTraitsAsioNoTimer>;
template class chat::TransportLayer<asio_sync::LayersTraitsAsioNoTimer>;
template class chat::SystemChatLayer<asio_sync::LayersTraitsAsioNoTimer>;

namespace asio_sync
{
AppLayerPtr MakeAppLayer(const chat::GlobalConfigPtr &cfg,
	const std::shared_ptr<MainStorageLayer>& storageLayerPtr,
	boost::asio::io_service &ios)
{
	return std::make_shared<AppLayer>(cfg, storageLayerPtr, boost::asio::io_service::strand(ios));
}
std::shared_ptr<DeliveryLayer> MakeDeliveryLayer(const GlobalConfigPtr &cfg,
	boost::asio::io_service &ios)
{
	return std::make_shared<DeliveryLayer>(cfg, boost::asio::io_service::strand(ios), ios);
}
std::shared_ptr<IntegrityLayer> MakeIntegrityLayer(const GlobalConfigPtr &cfg,
	boost::asio::io_service &ios)
{
	return std::make_shared<IntegrityLayer>(cfg,
		boost::asio::io_service::strand(ios), ios);
}
std::shared_ptr<MainStorageLayer> MakeStorageLayer(const ChatStoragePtr& impl,
	boost::asio::io_service &ios)
{
	return std::make_shared<MainStorageLayer>(impl, boost::asio::io_service::strand(ios));
}
std::shared_ptr<TransportLayer> MakeTransportLayer(const GlobalConfigPtr &cfg,
	std::unique_ptr<TransportChannel>&&channel, boost::asio::io_service &ios)
{
	return TransportLayer::MakeTransportLayer(cfg, std::move(channel), boost::asio::io_service::strand(ios));
}
std::shared_ptr<SystemChatLayer> MakeSystemChatLayer(const GlobalConfigPtr &cfg,
	boost::asio::io_service &ios)
{
	return std::make_shared<SystemChatLayer>(cfg, boost::asio::io_service::strand(ios));
}
}
}
