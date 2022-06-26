/**
 **************************************************************************
 * \file VS_Endpoint.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief endpoint name different functions
 *
 * \b Project Standart Libraries
 * \author SlavetskyA
 * \author SMirnovK
 * \author StasS
 * \date 09.04.03
 *
 * $Revision: 5 $
 *
 * $History: VS_Endpoint.h $
 *
 * *****************  Version 5  *****************
 * User: Mushakov     Date: 29.04.08   Time: 18:45
 * Updated in $/VSNA/std/cpplib
 * fixed operator < for Full ID
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 21.02.08   Time: 14:47
 * Updated in $/VSNA/std/cpplib
 * - auto login from client with endpoints
 *
 * *****************  Version 3  *****************
 * User: Stass        Date: 20.11.07   Time: 22:10
 * Updated in $/VSNA/std/cpplib
 * moved full id here
 *
 * *****************  Version 2  *****************
 * User: Stass        Date: 13.11.07   Time: 17:34
 * Updated in $/VSNA/std/cpplib
 * new server
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/std/cpplib
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 15.02.07   Time: 19:30
 * Updated in $/VS2005/std/cpplib
 * - bugfix whith atol() and wtol() functions
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 5.02.07    Time: 21:36
 * Updated in $/VS2005/std/cpplib
 * - project configuration
 * - depricated functions warnings suppressed
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/std/cpplib
 *
 * *****************  Version 26  *****************
 * User: Smirnov      Date: 29.11.06   Time: 19:22
 * Updated in $/VS/std/cpplib
 * - anti reconnect param in registry
 *
 * *****************  Version 25  *****************
 * User: Smirnov      Date: 28.11.06   Time: 16:44
 * Updated in $/VS/std/cpplib
 * - more complex random generation functions
 *
 * *****************  Version 24  *****************
 * User: Smirnov      Date: 13.09.06   Time: 13:28
 * Updated in $/VS/std/cpplib
 * - temp enpoint more random
 * - test client repaired
 *
 * *****************  Version 23  *****************
 * User: Smirnov      Date: 29.12.04   Time: 15:11
 * Updated in $/VS/std/cpplib
 * files headers
 ****************************************************************************/
#ifndef VC_STD_ENDPOINT_H
#define VC_STD_ENDPOINT_H


/****************************************************************************
 * Includes
 ****************************************************************************/
#include "VS_SimpleStr.h"

#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

/// from numerical to string conversion
inline char* VS_ConvertEndpoint( uint64_t source, char* destination )
{
	const uint32_t lo = source & 0x00000000ffffffff;
	const uint32_t hi = source >> 32;
    *reinterpret_cast<uint32_t*>(destination) = hi;
    destination[4] = ':';
    sprintf(destination + 5, "%" PRIu32, lo);
    return destination;
}

/// from string to numerical conversion
inline uint64_t VS_ConvertEndpoint( const char* source )
{
	if (!source || strlen(source) < 6 || source[4] != ':')
		return 0;

	uint32_t lo = atol(source + 5);
	uint32_t hi = *reinterpret_cast<const uint32_t*>(source);
	return static_cast<uint64_t>(lo) | (static_cast<uint64_t>(hi) << 32);
}


/*/// Generate random string in endpoint name format
inline void VS_GenerateTempEndpoint(char *dest)
{
	unsigned char ep[8];
	unsigned long idx;
	while ((idx = VS_GenKeyByMD5()) <= VS_BROKER_ID_RESERVED);
	((unsigned long *)ep)[0] = idx;
    ((unsigned long *)ep)[1] = VS_ENDPOINT_TEMP_PREFIX;
    VS_ConvertEndpoint(*(vs_num_endpoint*)ep, dest);
}
*/

class VS_FullID
{
public:
	VS_SimpleStr m_serverID;
	VS_SimpleStr m_userID;

	VS_FullID() {};

	VS_FullID(const char* serverID,const char* userID)
		: m_serverID(serverID),m_userID(userID) {};

	VS_FullID(const VS_FullID& a)
		: m_serverID(a.m_serverID),m_userID(a.m_userID) {};

	bool operator < (const VS_FullID& a) const
	{
		if(a.m_serverID==m_serverID )
		{
			return a.m_userID<m_userID;
		}
		else
		{
			return a.m_serverID<m_serverID;
		}
	}

	bool operator == (const VS_FullID& a) const
	{
		return a.m_serverID==m_serverID && a.m_userID==m_userID;
	}

	bool operator != (const VS_FullID& a) const
	{
		return !operator ==(a);
	}

	VS_FullID& operator = (const VS_FullID& a)
	{
		m_serverID=a.m_serverID;
		m_userID=a.m_userID;

		return *this;
	}

	bool operator ! () const
	{
		return !m_serverID && !m_userID;
	}

};



#endif // VC_STD_ENDPOINT_H
