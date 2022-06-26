/**
 **************************************************************************
 * \file VS_UserList.cpp
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief VS_UserList Implementation
 *
 * \b Project Client
 * \author SmirnovK
 * \date 08.05.2003
 *
 * $Revision: 4 $
 *
 * $History: VS_UserList.cpp $
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 21.11.08   Time: 12:58
 * Updated in $/VSNA/VSClient
 * - ban removed
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 19.06.08   Time: 16:49
 * Updated in $/VSNA/VSClient
 * - theoretical fix with from=0 in message
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 2.04.08    Time: 13:19
 * Updated in $/VSNA/VSClient
 * - ban list added
 * - message containers removed
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
 * *****************  Version 46  *****************
 * User: Smirnov      Date: 2.08.06    Time: 13:35
 * Updated in $/VS/VSClient
 * - address book and statuses processing removed
 * - added additional interface to receive container by GUI
 * - added hash param in SearchAB
 *
 * *****************  Version 45  *****************
 * User: Smirnov      Date: 3.05.06    Time: 11:39
 * Updated in $/VS/VSClient
 * - added AB container query from GUI
 *
 * *****************  Version 44  *****************
 * User: Smirnov      Date: 12.04.06   Time: 17:31
 * Updated in $/VS/VSClient
 * - conference list added (send container other then list)
 *
 * *****************  Version 43  *****************
 * User: Smirnov      Date: 6.04.06    Time: 20:03
 * Updated in $/VS/vsclient
 * - detail info
 *
 * *****************  Version 42  *****************
 * User: Smirnov      Date: 27.02.06   Time: 19:07
 * Updated in $/VS/VSClient
 * - update address book
 * - new storing concept for user information
 *
 * *****************  Version 41  *****************
 * User: Smirnov      Date: 20.02.06   Time: 17:58
 * Updated in $/VS/VSClient
 * - container in detailed user info
 *
 * *****************  Version 40  *****************
 * User: Smirnov      Date: 31.10.05   Time: 18:54
 * Updated in $/VS/VSClient
 * - Zoomfriends detailed info (copied from 4.2.6 release branch)
 *
 ****************************************************************************/

/******************************************************************************
* Includes
******************************************************************************/
#include "VS_UserList.h"
#include "std-generic/cpplib/VS_Container.h"
#include <stdio.h>


VS_UserList::VS_UserList(DWORD period) : m_seq_id(-1), m_period(period), m_seq_time(0)
{
}

/**
****************************************************************************
******************************************************************************/
bool VS_UserList::TrackSequence(long seq_id, bool reset)
{
	DWORD time = GetTickCount();
	if (reset) {
		m_seq_id = seq_id;
		m_seq_time = time;
		return false;
	}

	if (seq_id != -1) {
		if (m_seq_id == -1 || seq_id != ++m_seq_id) {
			if (time - m_seq_time < m_period)
				return false;
			else {
				m_seq_time = time;
				return true;
			}
		}
	}
	return false;
}


/**
****************************************************************************
******************************************************************************/
VS_ContainersMap::VS_ContainersMap(int MaxSize)
{
	m_MaxSize = MaxSize;
	m_CntKey = 0;
	m_cntMap.SetDataFactory(CntFactory, CntDestructor);
}


/**
****************************************************************************
******************************************************************************/
void* VS_ContainersMap::CntFactory(const void* cnt)
{
	VS_Container *oldcnt = (VS_Container *)cnt;
	VS_Container *newcnt = new VS_Container;
	oldcnt->CopyTo(*newcnt);
	return newcnt;
}

/**
****************************************************************************
******************************************************************************/
void VS_ContainersMap::CntDestructor(void* cnt)
{
	VS_Container *oldcnt = (VS_Container *)cnt;
	delete oldcnt;
}


/**
****************************************************************************
******************************************************************************/
VS_Container* VS_ContainersMap::GetList(int Key)
{
	VS_AutoLock lock(this);
	VS_Map::Iterator it = m_cntMap.Find((const void*)Key);
	if (it!= m_cntMap.End())
		return (VS_Container*)(*it).data;
	else
		return 0;
}


/**
****************************************************************************
******************************************************************************/
int VS_ContainersMap::AddList(VS_Container *cnt)
{
	VS_AutoLock lock(this);
	m_CntKey++;
	m_cntMap.Insert((const void*)m_CntKey, cnt);
	if (m_cntMap.Size() > m_MaxSize) {
		VS_Map::Iterator it = m_cntMap.LowerBound((const void*)0);
		if (it!= m_cntMap.End())
			m_cntMap.Erase(it);
	}
	return m_CntKey;
}

