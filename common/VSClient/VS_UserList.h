/**
 **************************************************************************
 * \file VS_UserList.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief User List managment
 *
 * \b Project Client
 * \author SmirnovK
 * \date 08.05.2003
 *
 * $Revision: 5 $
 *
 * $History: VS_UserList.h $
 *
 * *****************  Version 5  *****************
 * User: Dront78      Date: 29.10.09   Time: 18:18
 * Updated in $/VSNA/VSClient
 * - default container map size enlarged to 1000
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 21.11.08   Time: 12:58
 * Updated in $/VSNA/VSClient
 * - ban removed
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 2.04.08    Time: 13:19
 * Updated in $/VSNA/VSClient
 * - ban list added
 * - message containers removed
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 6.03.08    Time: 16:05
 * Updated in $/VSNA/VSClient
 * - chat rewrited
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/VSClient
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 20.04.07   Time: 20:50
 * Updated in $/VS2005/VSClient
 * - UMC - added roles
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/VSClient
 *
 * *****************  Version 39  *****************
 * User: Smirnov      Date: 18.12.06   Time: 17:38
 * Updated in $/VS/VSClient
 * - removed messages storing in map
 *
 * *****************  Version 38  *****************
 * User: Smirnov      Date: 2.08.06    Time: 13:35
 * Updated in $/VS/VSClient
 * - address book and statuses processing removed
 * - added additional interface to receive container by GUI
 * - added hash param in SearchAB
 *
 * *****************  Version 37  *****************
 * User: Smirnov      Date: 3.05.06    Time: 11:39
 * Updated in $/VS/vsclient
 * - added AB container query from GUI
 *
 * *****************  Version 36  *****************
 * User: Smirnov      Date: 12.04.06   Time: 17:31
 * Updated in $/VS/VSClient
 * - conference list added (send container other then list)
 *
 * *****************  Version 35  *****************
 * User: Smirnov      Date: 6.04.06    Time: 20:03
 * Updated in $/VS/vsclient
 * - detail info
 *
 * *****************  Version 34  *****************
 * User: Smirnov      Date: 27.02.06   Time: 19:07
 * Updated in $/VS/VSClient
 * - update address book
 * - new storing concept for user information
 *
 * *****************  Version 33  *****************
 * User: Smirnov      Date: 20.02.06   Time: 17:58
 * Updated in $/VS/VSClient
 * - container in detailed user info
 *
 * *****************  Version 32  *****************
 * User: Smirnov      Date: 31.10.05   Time: 18:54
 * Updated in $/VS/VSClient
 * - Zoomfriends detailed info (copied from 4.2.6 release branch)
 *
 ****************************************************************************/
#ifndef VS_USER_LIST_H
#define VS_USER_LIST_H

/******************************************************************************
* Includes
******************************************************************************/
#include "../std/cpplib/VS_SimpleStr.h"
#include "../std/cpplib/VS_Map.h"
#include "../std/cpplib/VS_Lock.h"
class VS_Container;

/**
****************************************************************************
* \brief process user list
******************************************************************************/
class VS_UserList
{
	long				m_seq_id;
	DWORD				m_seq_time;
	DWORD				m_period;
public:
	VS_UserList(DWORD period = 30000);
	/// check if seq_id != seq_id_prev++
	bool TrackSequence(long seq_id, bool reset);
};

/**
****************************************************************************
* \brief Store VS_Container objects for GUI post requests
******************************************************************************/
class VS_ContainersMap: VS_Lock
{
	VS_Map				m_cntMap;
	int					m_CntKey;
	unsigned int		m_MaxSize;
public:
	/// Set Containers Map
	VS_ContainersMap(int MaxSize = 200);
	/// Container Copy Constructor
	static void* CntFactory(const void* cnt);
	/// Container destructor
	static void CntDestructor(void* cnt);
	/// Return adress container by it key in container queue.
	VS_Container* GetList(int Key);
	/// Add container to queue. Erase first if size of queue rise to MaxSize. return Key;
	int AddList(VS_Container *cnt);
};

#endif
