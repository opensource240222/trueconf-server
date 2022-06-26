#pragma once

#include "transport/Router/VS_TransportRouter.h"
#include "tools/Watchdog/VS_RoutersWatchdog.h"
#include "tools/Watchdog/VS_Testable.h"
#include "ServerServices/Common.h"
#include "std-generic/asio_fwd.h"

class VS_TlsHandler;

class VS_ServerManagerServices : VS_Testable
{
public:
	VS_ServerManagerServices(boost::asio::io_service& ios);
	virtual ~VS_ServerManagerServices( void );
	struct VS_ServerManagerServices_Implementation* imp;

	bool						IsValid( void ) const;
	bool						Init( VS_TransportRouter *tr, VS_AccessConnectionSystem* acs,
										VS_TlsHandler* tlsHandler,
										VS_RoutersWatchdog* watchdog, const char* ver);
	void						Destroy( void );
	virtual bool				Test( void );
};


