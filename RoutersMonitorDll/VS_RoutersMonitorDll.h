
#pragma once

#include "../common/acs/Lib/VS_Acs64BitsMsg.h"
#include "../common/acs/AccessConnectionSystem/VS_AccessConnectionMonitor.h"
#include "../common/transport/Router/VS_TransportMonitor.h"

#ifdef __cplusplus
extern "C" {
#endif

/////////////////////////////////////////////////////////////////////////////////////////

bool __declspec( dllexport ) VS_WhatAtPresent( const char *computerName,
												const char *endpointName,
												bool *acs,
												bool *transportRouter,
												bool *streamsRouter );

/////////////////////////////////////////////////////////////////////////////////////////

void __declspec( dllexport ) VS_FreeMessage( void *wlparams );

/////////////////////////////////////////////////////////////////////////////////////////

char __declspec( dllexport ) *VS_CTime( const time_t time );

/////////////////////////////////////////////////////////////////////////////////////////

bool __declspec( dllexport ) VS_StartReadingAcs( const char *computerName,
													const char *endpointName,
													const HWND hwnd, const UINT msg );
void __declspec( dllexport ) VS_StopReadingAcs( void );

/////////////////////////////////////////////////////////////////////////////////////////

bool __declspec( dllexport ) VS_StartReadingTransportRouter( const char *computerName,
														const char *endpointName,
														const HWND hwnd, const UINT msg );
void __declspec( dllexport ) VS_StopReadingTransportRouter( void );

/////////////////////////////////////////////////////////////////////////////////////////

bool __declspec( dllexport ) VS_StartReadingStreamsRouter( const char *computerName,
														const char *endpointName,
														const HWND hwnd, const UINT msg );
void __declspec( dllexport ) VS_StopReadingStreamsRouter( void );

/////////////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif
