#pragma once
#include "defs.h"
#include "SteadyTimerWrap.h"
#include "chatlib/layers/AppLayer.h"
#include "chatlib/layers/DeliveryLayer.h"
#include "chatlib/layers/IntegrityLayer.h"
#include "chatlib/layers/MainStorageLayer.h"
#include "chatlib/layers/SystemChatLayer.h"
#include "chatlib/layers/TransportLayer.h"

namespace chat
{
namespace asio_sync
{
AppLayerPtr MakeAppLayer(const chat::GlobalConfigPtr &cfg,
	const std::shared_ptr<MainStorageLayer>& storageLayerPtr,
	boost::asio::io_service &ios);
std::shared_ptr<DeliveryLayer> MakeDeliveryLayer(const GlobalConfigPtr &cfg,
	boost::asio::io_service &ios);
std::shared_ptr<IntegrityLayer> MakeIntegrityLayer(const GlobalConfigPtr &cfg,
	boost::asio::io_service &ios);
std::shared_ptr<MainStorageLayer> MakeStorageLayer(const ChatStoragePtr &impl,
	boost::asio::io_service &ios);
std::shared_ptr<TransportLayer> MakeTransportLayer(const GlobalConfigPtr &cfg,
	std::unique_ptr<TransportChannel>&&channel,	boost::asio::io_service &ios);
std::shared_ptr<SystemChatLayer> MakeSystemChatLayer(const GlobalConfigPtr &cfg,
	boost::asio::io_service &ios);
}
}
