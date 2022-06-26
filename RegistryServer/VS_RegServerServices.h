#pragma once

#include "../../ServerServices/Common.h"

#include "../../common/transport/Router/VS_TransportRouter.h"
#include "tools/Watchdog/VS_RoutersWatchdog.h"
#include "std-generic/asio_fwd.h"

class VS_TlsHandler;

class VS_RegServerServices
{
public:
    VS_RegServerServices(boost::asio::io_service& ios);
    virtual ~VS_RegServerServices( void );

    struct VS_RegServerServices_Implementation   *imp;

    bool                        IsValid( void ) const;
    bool                        Init( VS_TransportRouter *tr, VS_AccessConnectionSystem* acs,
                                      VS_TlsHandler* tlsHandler, VS_RoutersWatchdog *watchdog = 0);
    void                        Destroy( void );

};
// end VS_DirServerServices
