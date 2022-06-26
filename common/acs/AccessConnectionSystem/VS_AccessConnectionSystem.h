/////////////////////////////////////////////////////////////////////////////////////////
//
//  (c) 2002 Visicron Inc.  http://www.visicron.net/
//
//  Project:
//
//  Created: 12.11.02     by  A.Slavetsky
//
/////////////////////////////////////////////////////////////////////////////////////////
///
/// \file VS_AccessConnectionSystem.h
/// \brief
/// \note
///

#ifndef VS_ACCESS_CONNECTION_SYSTEM_H
#define VS_ACCESS_CONNECTION_SYSTEM_H

#include "../Lib/VS_AcsLog.h"
#include "../../std/cpplib/VS_SimpleStr.h"
#include "../connection/VS_Connection.h"
#include "VS_AccessConnectionHandler.h"
#include "../../tools/Watchdog/VS_Testable.h"

class VS_AccessConnectionSystem : public VS_Testable
{
public:
			VS_AccessConnectionSystem( void );
	~VS_AccessConnectionSystem();
	struct VS_AccessConnectionSystem_Implementation   *imp;
	bool	IsValid( void ) const { return imp != 0; };
	bool		Init				( const unsigned long maxHandlers = 10,
										const unsigned long maxListeners = VS_MAX_LISTENERS,
										const unsigned long maxConnections = 1024,//512
										const unsigned long connectionLifetime = 20000 );
	bool		IsInit				( void ) const;
	void		Shutdown			( void );
	bool		AddHandler			( const char *name,
										VS_AccessConnectionHandler *handler,
										const bool isFinal = true );
	void		RemoveHandler		( const char *name );
	void		RemoveAllHandlers	( void );
	unsigned	AddListeners		( const char *endpointName,const bool hidden = false,
										const unsigned maxConnections = 10  );
	bool		AddListener			( const char *host, const int port, const bool hidden = false,
										const unsigned maxConnections = 10 );
	bool		AddListenerByIP		( const char *ip, const int port, const bool hidden = false,
										const unsigned maxConnections = 10 );
	int			GetListeners		( std::string& str );
	void		RemoveListener		( const char *host, const int port );
	void		RemoveListenersIp	( const char *host );
	void		RemoveListenersPort	( const int port );
	void		RemoveAllListeners	( void );
	void		*HandleIOCP			( void );
	bool		Test				( void );
	void		PrepareToDie		( void );
};
// end VS_AccessConnectionSystem class

#endif // VS_ACCESS_CONNECTION_SYSTEM_H
