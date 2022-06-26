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
/// \file VS_Acs64BitsMsg.h
/// \brief
/// \note
///

#ifndef VS_ACS_64_BITS_MSG_H
#define VS_ACS_64_BITS_MSG_H

#include <Windows.h>

inline unsigned __int64 VS_Get64Bits( WPARAM wParam, LPARAM lParam )
{	return (unsigned __int64)lParam | ((unsigned __int64)wParam << 32);		}
// end VS_Get64Bits

inline unsigned __int64 VS_Get64Bits( MSG &msg )
{	return (unsigned __int64)msg.lParam | ((unsigned __int64)msg.wParam << 32);		}
// end VS_Get64Bits

inline void VS_Set64Bits( unsigned __int64 bits, WPARAM &wParam, LPARAM &lParam )
{	lParam = (LPARAM)(bits & (unsigned __int64)0xFFFFFFFF);
	wParam = (WPARAM)(bits >> 32);	}
// end VS_Set64Bits

inline void VS_Set64Bits( unsigned __int64 bits, MSG &msg )
{	msg.lParam = (LPARAM)(bits & (unsigned __int64)0xFFFFFFFF);
	msg.wParam = (WPARAM)(bits >> 32);	}
// end VS_Set64Bits

#endif  // VS_ACS_64_BITS_MSG_H
