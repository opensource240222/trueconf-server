#pragma once

#include "ServerServices/Common.h"
#include "streams/fwd.h"
#include "transport/Router/VS_TransportRouter.h"
#include "tools/Watchdog/VS_RoutersWatchdog.h"
#include "tools/Watchdog/VS_Testable.h"
#include "tools/Server/VS_ServerComponentsInterface.h"
#include "std-generic/asio_fwd.h"

class VS_TlsHandler;
namespace vs { struct RegistrationParams; }

class VS_BaseServerServices : VS_Testable, public VS_ServerComponentsInterface
{
public:
	VS_BaseServerServices( void );
	virtual ~VS_BaseServerServices( void );
	struct VS_BaseServerServices_Implementation* imp;

	bool						IsValid( void ) const;
	stream::ConferencesConditions* GetConferencesConditions();
	bool						Init(vs::RegistrationParams&& rp, boost::asio::io_service& ios,
										VS_TransportRouter *tr, VS_RoutersWatchdog *watchdog,
										VS_AccessConnectionSystem *acs,
										VS_TlsHandler* tlsHandler, const char* ver, const char* server_id);
	void						Destroy( void );
	virtual bool				Test( void );

	//VS_ServerComponentsInterface
	VS_AccessConnectionSystem	* GetAcs() const override;
	stream::Router			* GetStreamsRouter() const override;
	VS_TransportRouter			* GetTransportRouter() const override;
	std::shared_ptr<VS_TranscoderLogin>   GetTranscoderLogin() const override;
};


