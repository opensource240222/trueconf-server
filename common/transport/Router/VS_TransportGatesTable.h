//////////////////////////////////////////////////////////////////////////////////////////
//
//  (c) 2002 Visicron Inc.  http://www.visicron.net/
//
//  Project:
//
//  Created: 24.06.03     by  A.Slavetsky
//
//////////////////////////////////////////////////////////////////////////////////////////
///
/// \file VS_TransportGatesTable.h
/// \brief Реализация динамической таблицы маршрутов удаленных Endpoints.
/// \note
///

#ifndef VS_TRANSPORT_ROUTER_GATES_TABLE_H
#define VS_TRANSPORT_ROUTER_GATES_TABLE_H

#include <Windows.h>

struct VS_TransportGatesTable
{
		VS_TransportGatesTable( void );
	virtual ~VS_TransportGatesTable( void );

	bool InitializeRouterGatesTable( const unsigned long maxLocalEndpoints );
	void DestroyRouterGatesTable( void );
	void AddGraphPoint( const char *point, const unsigned long indexLocalEndpoint, const unsigned char hops );
	bool IsGraphPoint( const char *point );
	bool GetGraphPoint( const char *point, unsigned long &indexLocalEndpoint, unsigned char &hops );
	void RemoveGraphPoint( const char *point, const unsigned long indexLocalEndpoint );
	void RemoveGraphPoint( const char *point );
	void RemoveListPoints( const unsigned long indexLocalEndpoint, const bool itself = true );
	void *SerializeListPoints( void *returned_point, const unsigned long indexLocalEndpoint, const unsigned char hops, void *buffer, const unsigned long buffer_size, unsigned long &serialize_size );
	void SerializePoint( const char *point, const unsigned char hops, void *buffer, const unsigned long buffer_size, unsigned long &serialize_size );
	void DeserializeListPoints( const unsigned long indexLocalEndpoint, const void *buffer, const unsigned long buffer_size );

	unsigned long   maxLocalEndpoints;
	HANDLE   hp;	struct EndpointsGraph   *eg;	struct EndpointsListStart   *el;

	static inline void EndpointListFree( HANDLE hp, struct EndpointsList *el );
	static inline void GatesReset( HANDLE hp, struct EndpointsGraph *eg, struct EndpointsList *el );
};
// end VS_TransportGatesTable struct

#endif  // VS_TRANSPORT_ROUTER_GATES_TABLE_H
