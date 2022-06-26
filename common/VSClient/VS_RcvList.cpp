/**
 **************************************************************************
 * \file VS_RcvList.cpp
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief VS_RcvList Implementation
 *
 * \b Project Client
 * \author SmirnovK
 * \date 09.10.2003
 *
 * $Revision: 2 $
 *
 * $History: VS_RcvList.cpp $
 *
 * *****************  Version 2  *****************
 * User: Dront78      Date: 18.01.08   Time: 17:02
 * Updated in $/VSNA/VSClient
 * - bigfixes from old architecture.
 * - ambiguous code marked as "old code"
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 24.12.07   Time: 17:55
 * Updated in $/VS2005/VSClient
 * - Not Clear receiver actions after "conf deleted" mesage
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 17.10.07   Time: 16:57
 * Updated in $/VS2005/VSClient
 * - bugfix #3422
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 26.09.07   Time: 14:46
 * Updated in $/VS2005/VSClient
 * - get receiver action user
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 13.08.07   Time: 19:23
 * Updated in $/VS2005/VSClient
 * - fixed bug with 2 network adapters while connecting to Intercom
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 4.07.07    Time: 19:25
 * Updated in $/VS2005/VSClient
 * - device statuses added
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/VSClient
 *
 * *****************  Version 12  *****************
 * User: Smirnov      Date: 21.10.05   Time: 13:28
 * Updated in $/VS/VSClient
 * - bugfix #1004
 *
 * *****************  Version 11  *****************
 * User: Smirnov      Date: 1.08.05    Time: 19:25
 * Updated in $/VS/VSClient
 * - multicast, last interation
 *
 * *****************  Version 10  *****************
 * User: Smirnov      Date: 27.07.05   Time: 20:49
 * Updated in $/VS/VSClient
 * - multicast beta 3
 *
 * *****************  Version 9  *****************
 * User: Smirnov      Date: 20.07.05   Time: 17:12
 * Updated in $/VS/VSClient
 * - added join task
 * - added lock in streams deleting
 *
 * *****************  Version 8  *****************
 * User: Smirnov      Date: 17.12.04   Time: 20:22
 * Updated in $/VS/VSClient
 * header to files
 *
 * *****************  Version 7  *****************
 * User: Smirnov      Date: 15.03.04   Time: 13:24
 * Updated in $/VS/VSClient
 * added multiconf multiformat
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 12.03.04   Time: 20:06
 * Updated in $/VS/VSClient
 * new access to rcv flrt
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 6.11.03    Time: 12:36
 * Updated in $/VS/VSClient
 * new rcv action intreface
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 3.11.03    Time: 18:48
 * Updated in $/VS/VSClient
 * added INTERCom support
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 29.10.03   Time: 19:15
 * Updated in $/VS/VSClient
 * multi connects scheme
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 24.10.03   Time: 17:39
 * Updated in $/VS/VSClient
 * MULTI test client and other files
 *
 * *****************  Version 1  *****************
 * User: Smirnov      Date: 9.10.03    Time: 19:44
 * Created in $/VS/VSClient
 * new methods in client
 * new files in std...
 *
 ****************************************************************************/

/****************************************************************************
 * Includes
 ****************************************************************************/
#include "VS_RcvList.h"
#include "..\std\cpplib\VS_RcvFunc.h"
#include "..\std\cpplib\VS_Protocol.h"
#include <stdio.h>


/****************************************************************************
 * Defines
 ****************************************************************************/
#ifdef _DEBUG
#define dprint if (1) printf
#else
#define dprint if (0) printf
#endif

/****************************************************************************
 * VS_RcvList
 ****************************************************************************/
/**
****************************************************************************
 * \brief Set Map parametrs
 * \date    08-10-2003
 ******************************************************************************/
VS_RcvList::VS_RcvList()
{
	m_id = 0;
	m_actions.SetDataFactory(VSClientRcvDesc::Factory, VSClientRcvDesc::Destructor);
	m_recievers.SetPredicate(VS_SimpleStr::Predicate);
	m_recievers.SetKeyFactory(VS_SimpleStr::Factory, VS_SimpleStr::Destructor);
	m_recievers.SetDataFactory(VSClientRcvDesc::Factory, VSClientRcvDesc::Destructor);
}

/**
****************************************************************************
 * \brief Clean list
 * \date    08-10-2003
 ******************************************************************************/
VS_RcvList::~VS_RcvList()
{
	Empty();
}

/**
****************************************************************************
 * \brief Add action to map;
 *
 * \date    08-10-2003
 ******************************************************************************/
bool VS_RcvList::AddAction(VSClientRcvDesc &rd)
{
	bool res = false;
	Lock();
	m_id++;
	res = m_actions.Insert((void*)m_id, &rd);
	UnLock();
	return res;
}

/**
****************************************************************************
 * \brief Do action # Id
 * \return	-1 - wrong params
 *			-2 - action not found
 *			 0 - OK
 * \date    26-09-2007
 *****************************************************************************/
int VS_RcvList::GetActionUser(int Id, char *name)
{
	VS_AutoLock lock(this);
	if (!name )
		return -1;
	*name = 0;
	VS_Map::Iterator i = m_actions.Find((void*)Id);
	if (i!= m_actions.End()) {
		VSClientRcvDesc* rd = (VSClientRcvDesc*)(*i).data;
		strcpy(name, rd->m_name);
		return 0;
	}
	return -2;
}

/**
****************************************************************************
 * \brief Do action # Id
 * \return	-1 - wrong params
 *			-2 - action not found
 *			-3 - logic error
 *			-4 - new rcv connect error
 *			0 - action is empty
 *			1 - new rcv conected
 *			2 - rcv updated
 *			3 - rcv "name" must be deleted
 * \date    09-10-2003
 *****************************************************************************/
int VS_RcvList::GetAction(int Id, char *name, boost::shared_ptr<VS_StreamClientReceiver> &rcv, long &fltr)
{
	VS_Map::Iterator i;
	if (!name )
		return -1;
	*name = 0;
	int res = 0;
	Lock();
	i = m_actions.Find((void*)Id);
	if (i!= m_actions.End()) {
		VSClientRcvDesc* rd = (VSClientRcvDesc*)(*i).data;
		if (rd->m_state == rd->RDSTATE_NEW) {
			stream::Track tracks[64];
			unsigned nTracks = 0;
			VS_RcvFunc::SetTracks(VS_RcvFunc::FLTR_ALL_MEDIA, tracks, nTracks);
			rd->m_rcv.reset(new VS_StreamClientReceiver);
			bool bOK = false;
			if (rd->m_type==RCT_ROUTER) {
				bOK = rd->m_rcv->Connect(rd->m_conf, rd->m_part, rd->m_name, rd->m_endp, tracks, nTracks, 20000, 0, true);
				dprint("\nCONNECT %s %s %s %s %x %d %s\n", rd->m_conf.m_str, rd->m_part.m_str, rd->m_name.m_str, rd->m_endp.m_str, static_cast<unsigned>(tracks[0]) | static_cast<unsigned>(tracks[1])<<8 | static_cast<unsigned>(tracks[2])<<16, nTracks, bOK ? "OK": "FALSE");
			}
			else{
				bOK = rd->m_rcv->Connect((unsigned short)rd->m_port, rd->m_mcast, rd->m_ckey, 0, 0, 0, rd->m_srcip);
				dprint("\nCONNECT %ld %s %s\n", rd->m_port, rd->m_host.m_str, bOK ? "OK": "FALSE");
			}
			if (!bOK) {
				rd->m_rcv.reset();
				res = -4;
			}
			else {
				strcpy(name, rd->m_name);
			 	rcv = rd->m_rcv;
				fltr = rd->m_fltr;
				m_recievers.Assign(rd->m_name, rd);
				res = 1; // sucs
			}
		}
		else if (rd->m_state == rd->RDSTATE_CHANGED) {
			i = m_recievers.Find(rd->m_name);
			if (i != m_recievers.End()) {
				VSClientRcvDesc *Rd = (VSClientRcvDesc*)(*i).data;
				Rd->m_fltr = rd->m_fltr;
				Rd->m_dvs = rd->m_dvs;
				strcpy(name, Rd->m_name);
				fltr = Rd->m_fltr;
				res = 2;
			}
			else {
				res = -3;
			}
		}
		else if (rd->m_state == rd->RDSTATE_DVS) {
			i = m_recievers.Find(rd->m_name);
			if (i != m_recievers.End()) {
				VSClientRcvDesc *Rd = (VSClientRcvDesc*)(*i).data;
				Rd->m_dvs = (Rd->m_dvs&~(DVS_MASK_SND|DVS_MASK_SND<<16))|rd->m_dvs;
				strcpy(name, Rd->m_name);
				fltr = Rd->m_fltr;
			}
			else {
				strcpy(name, rd->m_name);
				fltr = rd->m_fltr;
				m_recievers.Assign(rd->m_name, rd);
			}
			res = 2;
		}
		else if (rd->m_state == rd->RDSTATE_REMOVED) {
			strcpy(name, rd->m_name);
			m_recievers.Erase(rd->m_name);
			res = 3;
		}
		rd->m_state = rd->RDSTATE_PROCESSED;
		m_actions.Erase((void*)Id);
	}
	else {
		res = -1;
	}
	UnLock();
	return res;
}

/**
****************************************************************************
 * \brief return VSClientRcvDesc in rd by name if present
 * \date    08-10-2003
 ******************************************************************************/
bool VS_RcvList::GetRcvDesc(char *name, VSClientRcvDesc &rd)
{
	bool ret = false;
	if (!name )
		return false;
	VS_Map::Iterator i;
	Lock();
	i = m_recievers.Find(name);
	if (i!= m_recievers.End()) {
		rd = *(VSClientRcvDesc *)(*i).data;
		ret = true;
	}
	UnLock();
	return ret;
}

/**
****************************************************************************
 * \brief set frame mbps by name if present
 * \date    21-01-2013
 ******************************************************************************/
bool VS_RcvList::SetFrameSizeMBUser(char *name, int frameSizeMB)
{
	bool ret = false;

	Lock();

	VS_Map::Iterator i = m_recievers.Find(name);
	if (i != m_recievers.End()) {
		VSClientRcvDesc *rd = (VSClientRcvDesc*)(*i).data;
		if (rd->m_frameSizeMB != frameSizeMB && frameSizeMB != 0) {
			rd->m_frameSizeMB = frameSizeMB;
			ret = true;
		}
	}

	UnLock();

	return ret;
}

/**
****************************************************************************
 * \brief return frame mbps by name if present
 * \date    21-01-2013
 ******************************************************************************/
void VS_RcvList::GetFrameSizeMBUsersList(VS_Container& cnt)
{
	Lock();

	for (VS_Map::Iterator i = m_recievers.Begin(), e = m_recievers.End(); i != e; ) {
		VSClientRcvDesc *rd = (VSClientRcvDesc*)(*i).data;
		if (rd->m_frameSizeMB > 0) {
			cnt.AddValue(USERNAME_PARAM, (const char*)i->key);
			cnt.AddValueI32(FRAMESIZEMBUSER_PARAM, rd->m_frameSizeMB);
		}
		i++;
	}

	UnLock();
}

/**
****************************************************************************
 * \brief Remove all
 * \date    08-10-2003
 ******************************************************************************/
void VS_RcvList::Empty()
{
	Lock();
	m_recievers.Clear();
	UnLock();
}
