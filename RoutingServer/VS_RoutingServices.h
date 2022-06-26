#ifndef VS_ROUTER_SERVICES_H
#define VS_ROUTER_SERVICES_H

#include "ServerServices/Common.h"

#include "transport/Router/VS_TransportRouter.h"
#include "tools/Watchdog/VS_RoutersWatchdog.h"
#include "tools/Watchdog/VS_Testable.h"

namespace vs { struct RegistrationParams; }

class VS_RoutingServices : VS_Testable
{
public:
	VS_RoutingServices( void );
	virtual ~VS_RoutingServices( void );
	struct VS_RoutingServices_Implementation* imp;

	bool						IsValid( void ) const;
	//stream::ConferencesConditions* GetConferencesConditions();
	bool						Init(vs::RegistrationParams&& rp, VS_TransportRouter *tr, /*VS_StreamsRouter *sr,*/
										VS_RoutersWatchdog *watchdog,
										const char* ver);
	void						Destroy( void );
	virtual bool				Test( void );
};
// end VS_AppServices

#endif // VS_APP_SERVICES_H
