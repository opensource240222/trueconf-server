/////////////////////////////////////////////////////////////////////////////////////////
//
//  (c) 2002 Visicron Inc.  http://www.visicron.net/
//
//  Project: Непосредственная реализация поддержки транспортного протокола на сервере
//
//  Created: 12.11.02     by  A.Slavetsky
//
/////////////////////////////////////////////////////////////////////////////////////////
///
/// \file VS_TransportRouter.h
/// \brief Непосредственная реализация поддержки транспортного протокола на сервере
/// \note
///

#ifndef VS_TRANSPORT_ROUTER_H
#define VS_TRANSPORT_ROUTER_H

#include "TransportRouterInterface.h"
#include "transport/typedefs.h"
#include <functional>

class VS_TransportRouterServiceBase;
class VS_PointConditions;
struct VS_TransportRouterStatistics;
class VS_ServCertInfoInterface;
class VS_RouterMessExtHandlerInterface;
struct VS_TransportRouter_SetConnection;
class VS_RoutersWatchdog;

class VS_TransportRouter : public transport::IRouter
{
public:
	VS_TransportRouter(VS_RoutersWatchdog* watchDog, const char * private_key_pass = "");
	~VS_TransportRouter();
	struct VS_TransportRouter_Implementation   *imp;
	bool	IsValid( void ) const { return imp != 0; }
	bool	Init					(const std::string& endpointName,
										class VS_AccessConnectionSystem *acs,
										class VS_TlsHandler* tlsHandler,
										const unsigned long maxServices = 200,
										const unsigned long maxEndpoints = 5000,
										const unsigned long maxEndpointQueueMess = 200,
										const unsigned long maxEndpointLackMs = 30000,
										const unsigned long maxConnSilenceMs = 8000,
										const unsigned long fatalSilenceCoef = 3,
										const unsigned long maxPostMess = 6000,
										const unsigned long maxRecursionDepthSendMess = 20,
										const unsigned long routerTickSizeMs = 1000 );
	void	SetIsRoamingAllowedFunc(const std::function<bool(const char *)>&f);
	bool	IsInit					( void ) const;
	bool	AddService(const char *serviceName,
		VS_TransportRouterServiceBase *service,
		bool withOwnThread = true,
		const bool permittedForAll = false) override;
	bool	AddThreadedService				( const char *serviceName,
										VS_TransportRouterServiceBase *service,
										const bool permittedForAll = false);
	bool	AddCallService(const char *serviceName,
		VS_TransportRouterServiceBase *service, const bool permittedForAll = false);
	bool	RemoveService			( const char *serviceName );
	void	Shutdown				( void );
	void	PrepareToDie			( void );
	bool	GetStatistics			(VS_TransportRouterStatistics &stat);

	bool	SetServCertInfoInterface(VS_ServCertInfoInterface *pInterface);
	bool	AddRouterMessExtHandler(VS_RouterMessExtHandlerInterface *ext_handler);
	bool	RemoveRouterMessExtHandler(VS_RouterMessExtHandlerInterface *ext_handler);
	void	DisconnectAllEndpoints();
	void	DisconnectAllByCondition(const CheckEndpointPredT &f);

	VS_TransportRouter_SetConnection *GetSetConnectionHandler();
	const std::string& EndpointName() const override;

};
// end VS_TransportRouter class

#endif  // VS_TRANSPORT_ROUTER_H
