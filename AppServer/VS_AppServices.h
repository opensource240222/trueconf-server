#ifndef VS_APP_SERVICES_H
#define VS_APP_SERVICES_H

#include "ServerServices/Common.h"
#include "transport/Router/VS_TransportRouter.h"
#include "streams/fwd.h"
#include "tools/Watchdog/VS_Testable.h"
#include "tools/Server/VS_ServerComponentsInterface.h"
#include "TrueGateway/clientcontrols/VS_TranscoderLogin.h"
#include "std-generic/asio_fwd.h"

#define   CONFERENCES_MAX_CNT   400		// Maximum alowed number of conferences on Stream Router

namespace vs { struct RegistrationParams; }

class VS_AppServices : VS_Testable, public VS_ServerComponentsInterface
{
public:
	VS_AppServices(boost::asio::io_service& ios);
	virtual ~VS_AppServices( void );
	struct VS_AppServices_Implementation* imp;

	bool						IsValid( void ) const;
	stream::ConferencesConditions* GetConferencesConditions();
	bool						Init(vs::RegistrationParams&& rp, VS_TransportRouter* tr, stream::Router* sr,
										VS_AccessConnectionSystem* acs, VS_TlsHandler* tlsHandler,
										VS_RoutersWatchdog* watchdog,
										const char* ver);
	void						Destroy( void );
	virtual bool				Test( void );

	//VS_ServerComponentsInterface
	virtual VS_AccessConnectionSystem	* GetAcs() const;
	virtual stream::Router			* GetStreamsRouter() const;
	virtual VS_TransportRouter			* GetTransportRouter() const;
	virtual std::shared_ptr<VS_TranscoderLogin>   GetTranscoderLogin() const;
};
// end VS_AppServices

#endif // VS_APP_SERVICES_H
