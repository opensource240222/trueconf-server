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
/// \file VS_StreamsRouter.h
/// \brief
/// \note
///

#ifndef VS_STREAMS_ROUTER_H
#define VS_STREAMS_ROUTER_H

#include "Router.h"

class VS_StreamsRouter : public stream::Router
{
public:
	static VS_StreamsRouter* Create();

	virtual bool Init(const char* endpointName, class VS_AccessConnectionSystem* acs,
		class VS_TlsHandler* tlsHandler, stream::ConferencesConditions* ccs, const unsigned long maxConferences = 200) = 0;
	virtual bool IsInit() = 0;
};
// end VS_StreamsRouter class

#endif  // VS_STREAMS_ROUTER_H
