#if defined(_WIN32) // Not ported yet

/////////////////////////////////////////////////////////////////////////////////////////
//
//  (c) 2002 Visicron Inc.  http://www.visicron.net/
//
//  Project: Реализация клиента протокола медиа потоков,отсылающего медиа фреймы
//
//  Created: 12.11.02     by  A.Slavetsky
//
/////////////////////////////////////////////////////////////////////////////////////////
///
/// \file VS_StreamClientSender.cpp
/// \brief Реализация клиента протокола медиа потоков,отсылающего медиа фреймы
/// \note
///

#include "VS_StreamClientSender.h"
#include "VS_StreamClientTypes.h"

VS_StreamClientSender::VS_StreamClientSender( void ) :
	VS_StreamClient(stream::ClientType::sender) {}
// end VS_StreamClientSender::VS_StreamClientSender

VS_StreamClientSender::~VS_StreamClientSender( void ) {}
// end VS_StreamClientSender::~VS_StreamClientSender

///
/// \brief Реализация функции отсылки медиа фреймов для базового интерфейса.
///
int VS_StreamClientSender::SendFrame( const void *buffer, const int n_bytes,
							stream::Track track, unsigned long *milliseconds)
{	return !imp ? -1 : imp->SendFrame( buffer, n_bytes, track, milliseconds );	}
// end VS_StreamClientSender::SendFrame

void *VS_StreamClientSender::GetSendEvent( void )
{	return !imp ? 0 : imp->GetSendEvent();	}
// end VS_StreamClientSender::GetSendEvent

#endif
