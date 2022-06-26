#pragma once

#include "../../streams/fwd.h"

#include <boost/shared_ptr.hpp>

class VS_AccessConnectionSystem;
class VS_TransportRouter;
class VS_RoutersWatchdog;
class VS_TranscoderLogin;

namespace vs_relaymodule
{
	class VS_StreamsRelay;
};

namespace ts { struct IPool; }
class VS_TorrentService;

class VS_ServerComponentsInterface
{
public:
	virtual ~VS_ServerComponentsInterface(){}
	virtual VS_AccessConnectionSystem	* GetAcs() const = 0;
	virtual stream::Router			* GetStreamsRouter() const = 0;
	virtual VS_TransportRouter			* GetTransportRouter() const = 0;
	virtual std::shared_ptr<VS_TranscoderLogin>			GetTranscoderLogin() const = 0;
};
