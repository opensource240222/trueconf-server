#pragma once
#include "std/cpplib/VS_PerformanceMonitor.h"
#include "ServerServices/Common.h"
#include "transport/Router/VS_TransportRouter.h"
#include "streams/fwd.h"
#include "tools/Watchdog/VS_RoutersWatchdog.h"
#include "tools/Watchdog/VS_Testable.h"
#include "acs/AccessConnectionSystem/VS_TlsHandler.h"
#include "tools/Server/VS_ServerComponentsInterface.h"
#include "std-generic/asio_fwd.h"

#define   CONFERENCES_MAX_CNT   400		// Maximum alowed number of conferences on Stream Router
class VS_AccessConnectionSystem;
namespace vs { struct RegistrationParams; }

class VS_VCSServices : VS_Testable, public VS_ServerComponentsInterface
{
public:
	VS_VCSServices(boost::asio::io_service& ios);
	virtual ~VS_VCSServices( void );
	struct VS_VCSServices_Implementation* imp;

	bool						IsValid( void ) const;
	stream::ConferencesConditions* GetConferencesConditions();
	bool						Init(vs::RegistrationParams&& rp, VS_TransportRouter* tr, stream::Router* sr, VS_AccessConnectionSystem* acs,
										VS_TlsHandler* handler, VS_RoutersWatchdog* watchdog, const char* ver, VS_Container& UCAZA);
	void						Destroy( void );
	virtual bool				Test( void );

	//VS_ServerComponentsInterface
	virtual VS_AccessConnectionSystem	* GetAcs() const;
	virtual stream::Router			* GetStreamsRouter() const;
	virtual VS_TransportRouter			* GetTransportRouter() const;
	virtual std::shared_ptr<VS_TranscoderLogin>   GetTranscoderLogin() const;
};
