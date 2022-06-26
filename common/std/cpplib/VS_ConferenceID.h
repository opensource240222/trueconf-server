/**
 **************************************************************************
 * \file VS_ConferenceID.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief Conference identifier class definition
 *
 * \b Project Standart Libraries
 * \author Stass
 * \date 21.07.03
 *
 * $Revision: 1 $
 *
 * $History: VS_ConferenceID.h $
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/std/cpplib
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/std/cpplib
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 29.12.04   Time: 15:11
 * Updated in $/VS/std/cpplib
 * files headers
 *
 * *****************  Version 5  *****************
 * User: Stass        Date: 25.11.03   Time: 15:23
 * Updated in $/VS/std/cpplib
 * fixed operator ==,!=,! in SimpleStr to treat 0 length string as zero
 *
 * *****************  Version 4  *****************
 * User: Stass        Date: 20.11.03   Time: 11:35
 * Updated in $/VS/std/cpplib
 * fixed ConfID operators !,==,!= to treat 0 length ID as null (bug in
 * Join)
 *
 * *****************  Version 3  *****************
 * User: Stass        Date: 7.10.03    Time: 19:21
 * Updated in $/VS/std/cpplib
 * fixed C++ id syntax
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 5.08.03    Time: 19:45
 * Updated in $/VS/std/cpplib
 * broker readresing from conference name
 *
 * *****************  Version 1  *****************
 * User: Stass        Date: 21.07.03   Time: 14:13
 * Created in $/VS/std/cpplib
 * Moved vs_conf_id to StdLib, removed null(), added GetEndpoint function
 ****************************************************************************/
#ifndef VC_STD_CONF_ID_H
#define VC_STD_CONF_ID_H

/****************************************************************************
 * Includes
 ****************************************************************************/
#include "VS_SimpleStr.h"

#include <cstring>

typedef VS_SimpleStr vs_conf_id;

/**
 **************************************************************************
 * \brief Retrive endpoint name from conference Id
 ****************************************************************************/
inline const char* VS_GetConfEndpoint(const char * conf)
{
		if(conf==NULL)	return NULL;
		const char* p=strchr(conf,'@');
		if(p==NULL || p[1]==0)	return NULL;
		return p+1;
}

#endif // VC_STD_CONF_ID_H
