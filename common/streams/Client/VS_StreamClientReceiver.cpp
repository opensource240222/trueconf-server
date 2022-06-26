#if defined(_WIN32) // Not ported yet

/////////////////////////////////////////////////////////////////////////////////////////
//
//  (c) 2002 Visicron Inc.  http://www.visicron.net/
//
//  Project: Реализация клиента протокола медиа потоков, принимающего медиа фреймы
//
//  Created: 12.11.02     by  A.Slavetsky
//
/////////////////////////////////////////////////////////////////////////////////////////
///
/// \file VS_StreamClientReceiver.cpp
/// \brief Реализация клиента протокола медиа потоков, принимающего медиа фреймы
/// \note
///

#include "VS_StreamClientReceiver.h"
#include "VS_StreamClientTypes.h"

VS_StreamClientReceiver::VS_StreamClientReceiver( void ) :
	VS_StreamClient(stream::ClientType::receiver) {}
// end VS_StreamClientReceiver::VS_StreamClientReceiver

VS_StreamClientReceiver::~VS_StreamClientReceiver( void ) {}
// end VS_StreamClientReceiver::~VS_StreamClientReceiver

///
/// \brief Реализация функции приема медиа фреймов для базового интерфейса.
///
int VS_StreamClientReceiver::ReceiveFrame( void *buffer, const int s_buffer,
								stream::Track* track, unsigned long *milliseconds)
{	return !imp ? -1 : imp->ReceiveFrame( buffer, s_buffer, track, milliseconds );	}
// end VS_StreamClientReceiver::ReceiveFrame

void *VS_StreamClientReceiver::GetReceiveEvent( void )
{	return !imp ? 0 : imp->GetReceiveEvent();	}
// end VS_StreamClientReceiver::GetReceiveEvent

#endif
