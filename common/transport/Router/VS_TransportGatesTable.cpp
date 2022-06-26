#if defined(_WIN32) // Not ported yet

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
/// \file VS_TransportGatesTable.cpp
/// \brief Реализация динамической таблицы маршрутов удаленных Endpoints.
/// \note
///

#include <Windows.h>
#include <stdio.h>

#include "VS_TransportGatesTable.h"
#include "../../acs/VS_AcsDefinitions.h"
#include "../../std/debuglog/VS_Debug.h"

#define DEBUG_CURRENT_MODULE VS_DM_LOGS

//////////////////////////////////////////////////////////////////////////////////////////

static inline void *Alloc( HANDLE hp, unsigned long size )
{	return HeapAlloc( hp, HEAP_NO_SERIALIZE | HEAP_ZERO_MEMORY, size );		}
// end EndpointsGraph::Alloc

static inline void Free( HANDLE hp, void *block )
{	HeapFree( hp, HEAP_NO_SERIALIZE, block );	}
// end EndpointsGraph::Free

//////////////////////////////////////////////////////////////////////////////////////////

#pragma pack( 1 )

//////////////////////////////////////////////////////////////////////////////////////////

struct Gates
{
	static inline Gates *Alloc( HANDLE hp ) {	return (Gates *)::Alloc( hp, sizeof(Gates) );	}
	// end Gates::Alloc

	inline void Free( HANDLE hp ) {		::Free( hp, (void *)this );		}
	// end Gates::Free

	inline bool Set( HANDLE hp, EndpointsList *el, const unsigned long gate, const unsigned char hops )
	{
		if (hops <= hops1) {	if (el2)	VS_TransportGatesTable::EndpointListFree( hp, el2 );
								el2 = el1;	gate2 = gate1;	hops2 = hops1;
								el1 = el;	gate1 = gate;	hops1 = hops;
								return true;	}
		if (!el2) {				el2 = el;	gate2 = gate;	hops2 = hops;
								return true;	}
		if (hops <= hops2) {	VS_TransportGatesTable::EndpointListFree( hp, el2 );
								el2 = el;	gate2 = gate;	hops2 = hops;
								return true;	}
		return false;
	}
	// end Gates::Set

	inline void SetStart( EndpointsList *el, const unsigned long gate, const unsigned char hops )
	{	el1 = el;	gate1 = gate;	hops1 = hops;	}
	// end Gates::SetStart

	inline bool Reset( HANDLE hp, const unsigned long gate )
	{
		if (gate1 == gate) {			VS_TransportGatesTable::EndpointListFree( hp, el1 );
										if (!el2)	return false;
										el1 = el2;	gate1 = gate2;	hops1 = hops2;
										el2 = 0;	gate2 = 0;	hops2 = 0;		return true;	}
		if (el2 && gate2 == gate) {		VS_TransportGatesTable::EndpointListFree( hp, el2 );
										el2 = 0;	gate2 = 0;	hops2 = 0;	}
		return true;
	}
	// end Gates::Reset

	inline bool Reset( HANDLE hp, const EndpointsList *el )
	{
		if (el1 == el) {	VS_TransportGatesTable::EndpointListFree( hp, el1 );
							if (!el2)	return false;
							el1 = el2;	gate1 = gate2;	hops1 = hops2;
							el2 = 0;	gate2 = 0;		hops2 = 0;		return true;	}
		if (el2 == el) {	VS_TransportGatesTable::EndpointListFree( hp, el2 );
							el2 = 0;	gate2 = 0;		hops2 = 0;		}
		return true;
	}
	// end Gates::Reset

	inline void Reset( HANDLE hp )
	{
		if (el1)	VS_TransportGatesTable::EndpointListFree( hp, el1 );
		if (el2)	VS_TransportGatesTable::EndpointListFree( hp, el2 );
		el1 = el2 = 0;		gate1 = gate2 = 0;		hops1 = hops2 = 0;
	}
	// end Gates::Reset

	EndpointsList   *el1, *el2;
	unsigned long   gate1, gate2;
	unsigned char   hops1, hops2;
};
// end Gates struct

//////////////////////////////////////////////////////////////////////////////////////////

struct EndpointsList
{
	inline void SetPrvNxtEg( EndpointsList *prv, EndpointsList *nxt, struct EndpointsGraph *eg )
	{
		EndpointsList::prv = prv;	prv->nxt = this;
		EndpointsList::nxt = nxt;	nxt->prv = this;
		EndpointsList::eg = eg;
	}
	// end EndpointsList::SetPrvNxtEg

	inline void RespecifyPrvNxt( void ) {		prv->nxt = nxt;		nxt->prv = prv;		}
	// end EndpointsList::RespecifyPrvNxt

	static inline EndpointsList *Alloc( HANDLE hp, EndpointsList *prv, EndpointsList *nxt, EndpointsGraph *eg )
	{
		EndpointsList   *ret = (EndpointsList *)::Alloc( hp, sizeof(EndpointsList) );
		if (ret) 	ret->SetPrvNxtEg( prv, nxt, eg );
		return ret;
	}
	// end EndpointsList::Alloc

	inline void Free( HANDLE hp ) {		RespecifyPrvNxt();	::Free( hp, (void *)this );		}
	// end EndpointsList::Free

	EndpointsList   *prv, *nxt;		struct EndpointsGraph   *eg;
};
// end EndpointsList struct

//////////////////////////////////////////////////////////////////////////////////////////

struct EndpointsListStart
{
	inline void SpecifyStartSelHel( void ) {	sel.nxt = &hel;		hel.prv = &sel;		}
	// end EndpointsListStart::SpecifyStartSelHel

	static inline EndpointsListStart *Alloc( HANDLE hp, const unsigned long maxLocalEndpoints )
	{
		EndpointsListStart   *ret = (EndpointsListStart *)::Alloc( hp, sizeof(EndpointsListStart) * maxLocalEndpoints );
		if (ret)	for (unsigned long i = 0; i < maxLocalEndpoints; ++i)	ret[i].SpecifyStartSelHel();
		return ret;
	}
	// end EndpointsListStart::Alloc

	inline void Free( HANDLE hp ) {		::Free( hp, (void *)this );		}
	// end EndpointsListStart::Free

	inline EndpointsList *Add( HANDLE hp, EndpointsGraph *eg )
	{	return EndpointsList::Alloc( hp, hel.prv, &hel, eg );	}
	// end EndpointsListStart::Alloc

	inline void DeleteAll( HANDLE hp, const bool itself )
	{
		EndpointsList   *el = sel.nxt, *el_nxt;
		if (el != &hel)
		{	el_nxt = el->nxt;
			if (itself)		VS_TransportGatesTable::GatesReset( hp, el->eg, el );
			el = el_nxt;
		}
		while (el != &hel)
		{	el_nxt = el->nxt;
			VS_TransportGatesTable::GatesReset( hp, el->eg, el );
			el = el_nxt;
	}	}
	// end EndpointsListStart::DeleteAll

	EndpointsList   sel, hel;
};
// end EndpointsListStart struct

//////////////////////////////////////////////////////////////////////////////////////////

struct EndpointsGraph
{
	inline void ResetGates( void ) {	if (gts) {		free( (void *)gts );	gts = 0;	}}
	// end EndpointsGraph::ResetGates

	inline void SetEgIndex( EndpointsGraph *eg, const unsigned char index )
	{	pgr = eg;	EndpointsGraph::index = index;	}
	// end EndpointsGraph::SetEgIndex

	static inline EndpointsGraph *AllocStart( HANDLE hp )
	{
		EndpointsGraph   *ret = (EndpointsGraph *)::Alloc( hp, sizeof(EndpointsGraph) );
		return ret;
	}
	// end EndpointsGraph::AllocStart

	inline void FreeStart( HANDLE hp ) {	::Free( hp, (void *)this );		}
	// end EndpointsGraph::FreeStart

	static inline EndpointsGraph *Alloc( HANDLE hp, EndpointsGraph *eg, const unsigned char index )
	{
		EndpointsGraph   *ret = (EndpointsGraph *)::Alloc( hp, sizeof(EndpointsGraph) );
		if (ret)	ret->SetEgIndex( eg, index );
		return ret;
	}
	// end EndpointsGraph::Alloc

	inline void Free( HANDLE hp )
	{
		if (ngrs)	return;
		unsigned char   index = EndpointsGraph::index;
		EndpointsGraph   *eg = this, *neg = pgr;
		if (neg) {	while (neg) {	::Free( hp, (void *)eg );		neg->grs[index] = 0;
									if (--neg->ngrs)	return;
									::Free( hp, (void *)neg->grs );	neg->grs = 0;
									if (neg->gts)		return;
									eg = neg;	index = neg->index;		neg = neg->pgr;	}}
		else {		::Free( hp, (void *)eg->grs );	::Free( hp, (void *)eg );		}
	}
	// end EndpointsGraph::FreeEndpoint

	inline void CheckFree( HANDLE hp ) {	if (!gts)	Free( hp );		}
	// end EndpointsGraph::CheckFree

	EndpointsGraph   *pgr, **grs;	Gates   *gts;
	unsigned char   ngrs, index;

	inline char *GetName( char *name, const unsigned sz_name, unsigned &ret_size )
	{
		name += sz_name - 1;	*name = 0;	ret_size = 0;	EndpointsGraph   *eg = this;
		do {	--name;		++ret_size;		*name = (char)eg->index + 1;	eg = eg->pgr;	}
		while (eg);		return ++name;
	}
	// end EndpointsGraph::GetName

	static inline EndpointsGraph *Add( HANDLE hp, EndpointsGraph *eg, const char *point )
	{
		if (!eg && !point)
			return 0;
		unsigned long   level = 0;
		unsigned char	index(0);
		while (1)
		{
			index = point[level++];
			if (index)
			{
				if (!eg->grs)
				{
					eg->grs = (EndpointsGraph **)::Alloc( hp, sizeof(EndpointsGraph *) * 255 );
					if (!eg->grs) {		eg->CheckFree( hp );	return 0;	}
				}
				EndpointsGraph   *&neg = eg->grs[--index];
				if (!neg) {		neg = Alloc( hp, eg, index );
								if (!neg) {		eg->CheckFree( hp );	return 0;	}
								++eg->ngrs;		}
				eg = neg;
			}
			else	return eg;
	}	}
	// end EndpointsGraph::AddEndpoint

	static inline EndpointsGraph *Get( EndpointsGraph *eg, const char *point )
	{
		if (!eg && !point)
			return 0;

		unsigned long   level = 0;
		unsigned char index(0);
		while (1)
		{	index = point[level++];
			if (index) {	if (!eg->grs)	return 0;
							eg = eg->grs[--index];
							if (!eg)	return 0;	}
			else	return eg;
	}	}
	// end EndpointsGraph::GetEndpoint
};
// end EndpointsGraph struct

//////////////////////////////////////////////////////////////////////////////////////////

#pragma pack(   )

//////////////////////////////////////////////////////////////////////////////////////////

VS_TransportGatesTable::VS_TransportGatesTable( void )
	: maxLocalEndpoints(0), hp(0), eg(0), el(0) {}
// end VS_TransportGatesTable::VS_TransportGatesTable

VS_TransportGatesTable::~VS_TransportGatesTable( void ) {}
// end VS_TransportGatesTable::~VS_TransportGatesTable

bool VS_TransportGatesTable::InitializeRouterGatesTable( const unsigned long maxLocalEndpoints )
{
	hp = HeapCreate( HEAP_NO_SERIALIZE, sizeof(EndpointsGraph), 0 );
	if (!hp)	return false;
	el = EndpointsListStart::Alloc( hp, maxLocalEndpoints );
	if (!el)	return false;
	eg = EndpointsGraph::AllocStart( hp );
	if (!eg)	return false;
	VS_TransportGatesTable::maxLocalEndpoints = maxLocalEndpoints;
	return true;
}
// end VS_TransportGatesTable::InitializeRouterGatesTable

void VS_TransportGatesTable::DestroyRouterGatesTable( void )
{
	if (hp) {	HeapDestroy( hp );	hp = 0;		}
	eg = 0;		el = 0;		maxLocalEndpoints = 0;
}
// end VS_TransportGatesTable::DestroyRouterGatesTable



void VS_TransportGatesTable::AddGraphPoint( const char *point, const unsigned long indexLocalEndpoint, const unsigned char hops )
{
	dprint4("\n\t * TR_Gates: addpoint %s *\n",point);
	EndpointsGraph   *eg = EndpointsGraph::Add( hp, VS_TransportGatesTable::eg, point );
	if (!eg)	return;
	EndpointsList   *el = VS_TransportGatesTable::el[indexLocalEndpoint].Add( hp, eg );
	if (!el) {		eg->CheckFree( hp );	return;		}
	Gates   *&gts = eg->gts;
	if (!gts) {		gts = Gates::Alloc( hp );
					if (!gts) {		eg->Free( hp );		el->Free( hp );		return;		}
					gts->SetStart( el, indexLocalEndpoint, hops );	return;		}
	if (gts->Set( hp, el, indexLocalEndpoint, hops ))	return;
	el->Free( hp );
}
// end VS_TransportGatesTable::AddGraphPoint

bool VS_TransportGatesTable::IsGraphPoint( const char *point )
{
	const EndpointsGraph   *eg = EndpointsGraph::Get( VS_TransportGatesTable::eg, point );
	return eg && eg->gts;
}
// end VS_TransportGatesTable::IsGraphPoint

bool VS_TransportGatesTable::GetGraphPoint( const char *point, unsigned long &indexLocalEndpoint, unsigned char &hops )
{
	const EndpointsGraph   *eg = EndpointsGraph::Get( VS_TransportGatesTable::eg, point );
	if (eg && eg->gts) {	indexLocalEndpoint = eg->gts->gate1;
							hops = eg->gts->hops1;	return true;	}
	return false;
}
// end VS_TransportGatesTable::GetGraphPoint

void VS_TransportGatesTable::RemoveGraphPoint( const char *point, const unsigned long indexLocalEndpoint )
{
	EndpointsGraph   *eg = EndpointsGraph::Get( VS_TransportGatesTable::eg, point );
	if (!eg || !eg->gts)	return;
	Gates   *&gts = eg->gts;
	if (!gts->Reset( hp, indexLocalEndpoint ))
	{	gts->Free( hp );	gts = 0;	eg->Free( hp );
}	}
// end VS_TransportGatesTable::RemoveGraphPoint

void VS_TransportGatesTable::RemoveGraphPoint( const char *point )
{
	EndpointsGraph   *eg = EndpointsGraph::Get( VS_TransportGatesTable::eg, point );
	if (!eg || !eg->gts)	return;
	Gates   *&gts = eg->gts;
	gts->Reset( hp );	gts->Free( hp );	gts = 0;	eg->Free( hp );
}
// end VS_TransportGatesTable::RemoveGraphPoint

void VS_TransportGatesTable::RemoveListPoints( const unsigned long indexLocalEndpoint, const bool itself )
{
	el[indexLocalEndpoint].DeleteAll( hp, itself );
}
// end VS_TransportGatesTable::RemoveListPoints

void *VS_TransportGatesTable::SerializeListPoints( void *returned_point, const unsigned long indexLocalEndpoint, const unsigned char hops, void *buffer, const unsigned long buffer_size, unsigned long &serialize_size )
{
	EndpointsListStart   &els = el[indexLocalEndpoint];
	EndpointsList   *sel = (EndpointsList *)returned_point, *fel = &els.hel;
	if (!sel) {		sel = els.sel.nxt;
					if (!sel || sel == fel) {	serialize_size = 0;		return 0;	}}
	unsigned char   *bf = (unsigned char *)buffer;		unsigned long   size = 0;
	while (sel != fel)
	{	unsigned   sz_nm;		EndpointsGraph   *eg = sel->eg;
		char   name[VS_ACS_MAX_SIZE_ENDPOINT_NAME + 1];
		const char   *nm = eg->GetName( name, sizeof(name), sz_nm );
		const unsigned long   new_size = size + 1 + sz_nm;
		if (new_size > buffer_size) {	serialize_size = size;		return (void *)sel;		}
		unsigned char   hps = eg->gts->hops1;
		if (hps < hops) {	*bf = hps;					++bf;
							strcpy( (char *)bf, nm );	bf += sz_nm;
							size = new_size;	}
		sel = sel->nxt;
	}
	serialize_size = size;		return 0;
}
// end VS_TransportGatesTable::SerializeListPoints

void VS_TransportGatesTable::DeserializeListPoints( const unsigned long indexLocalEndpoint, const void *buffer, const unsigned long buffer_size )
{
	const unsigned char   *bf = (unsigned char *)buffer;	unsigned long   size = 0;
	if (bf[buffer_size - 1])	return;
	while (size < buffer_size)
	{	const unsigned char   hops = *bf + 1;	++bf;	if (++size >= buffer_size)	return;
		if (!hops)	return;		const char   *point = (char *)bf;
		AddGraphPoint( point, indexLocalEndpoint, hops );
		const unsigned long   sz = (unsigned long)strlen( point ) + 1;	bf += sz;	size += sz;
}	}
// end VS_TransportGatesTable::DeserializeListPoints

void VS_TransportGatesTable::SerializePoint( const char *point, const unsigned char hops, void *buffer, const unsigned long buffer_size, unsigned long &serialize_size )
{
	const unsigned long   sz = 1 + (unsigned long)strlen( point ) + 1;
	if (sz > buffer_size) {		serialize_size = 0;		return;		}
	unsigned char   *bf = (unsigned char *)buffer;
	*bf = hops;		++bf;		strcpy( (char *)bf, point );	serialize_size = sz;
}
// end VS_TransportGatesTable::SerializePoint

inline void VS_TransportGatesTable::EndpointListFree( HANDLE hp, EndpointsList *el )
{	el->Free( hp );		}
// end VS_TransportGatesTable::EndpointListFree

inline void VS_TransportGatesTable::GatesReset( HANDLE hp, EndpointsGraph *eg, EndpointsList *el )
{
	Gates   *&gts = eg->gts;
	if (!gts->Reset( hp, el )) {	gts->Free( hp );	gts = 0;	eg->Free( hp );		}
}
// end VS_TransportGatesTable::GatesReset

//////////////////////////////////////////////////////////////////////////////////////////

#endif
