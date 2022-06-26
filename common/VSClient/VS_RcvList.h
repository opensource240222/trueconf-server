/**
 **************************************************************************
 * \file VS_RcvList.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief List for receiver description
 *
 * \b Project Client
 * \author SmirnovK
 * \date 09.10.2003
 *
 * $Revision: 3 $
 *
 * $History: VS_RcvList.h $
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 31.03.09   Time: 18:15
 * Updated in $/VSNA/VSClient
 * - stream symmetric crypt support
 *
 * *****************  Version 2  *****************
 * User: Dront78      Date: 18.01.08   Time: 17:02
 * Updated in $/VSNA/VSClient
 * - bigfixes from old architecture.
 * - ambiguous code marked as "old code"
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
 * *****************  Version 11  *****************
 * User: Smirnov      Date: 1.08.05    Time: 19:25
 * Updated in $/VS/VSClient
 * - multicast, last interation
 *
 * *****************  Version 10  *****************
 * User: Smirnov      Date: 27.07.05   Time: 19:57
 * Updated in $/VS/VSClient
 * -multicast beta 2
 *
 * *****************  Version 9  *****************
 * User: Smirnov      Date: 17.12.04   Time: 20:22
 * Updated in $/VS/VSClient
 * header to files
 *
 * *****************  Version 8  *****************
 * User: Smirnov      Date: 12.03.04   Time: 20:06
 * Updated in $/VS/VSClient
 * new access to rcv flrt
 *
 * *****************  Version 7  *****************
 * User: Smirnov      Date: 11.03.04   Time: 20:17
 * Updated in $/VS/VSClient
 * new caps scheme
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 3.03.04    Time: 16:26
 * Updated in $/VS/VSClient
 * media format changes
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 6.11.03    Time: 12:36
 * Updated in $/VS/VSClient
 * new rcv action intreface
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 3.11.03    Time: 18:48
 * Updated in $/VS/vsclient
 * added INTERCom support
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 29.10.03   Time: 19:15
 * Updated in $/VS/vsclient
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
#ifndef VS_RCV_LIST_H
#define VS_RCV_LIST_H


/****************************************************************************
 * Includes
 ****************************************************************************/
#include "..\std\cpplib\VS_ClientCaps.h"
#include "..\std\cpplib\VS_SimpleStr.h"
#include "..\std\cpplib\VS_Map.h"
#include "..\std\cpplib\VS_Lock.h"
#include "..\streams\Client\VS_StreamClientReceiver.h"
#include "boost\shared_ptr.hpp"


/**
**************************************************************************
 * \brief Describe recievrs parameters need for connect
 ****************************************************************************/
class VSClientRcvDesc
{
public:
	enum State {
		RDSTATE_UNKNOWN,
		RDSTATE_NEW,
		RDSTATE_CHANGED,
		RDSTATE_REMOVED,
		RDSTATE_PROCESSED,
		RDSTATE_DVS
	};
	VS_SimpleStr	m_name;	///id
	VS_SimpleStr	m_conf;
	VS_SimpleStr	m_user;
	VS_SimpleStr	m_part;
	VS_SimpleStr	m_endp;
	VS_SimpleStr	m_host;
	VS_SimpleStr	m_mcast;
	VS_SimpleStr	m_srcip;
	VS_SimpleStr	m_symkey;

	long			m_ckey;
	long			m_port;
	long			m_fltr;
	long			m_type;
	long			m_dvs;
	long			m_frameSizeMB;
	State			m_state;
	boost::shared_ptr<VS_StreamClientReceiver> m_rcv;

	VS_ClientCaps	m_caps;

	VSClientRcvDesc(){
		m_ckey = 0;
		m_port = 0;
		m_fltr = 0;
		m_type = 0;
		m_dvs = 1|1<<16;
		m_state = RDSTATE_UNKNOWN;
		m_frameSizeMB = 0;
	}

	static void* Factory(const void* rd){
		VSClientRcvDesc *p = new VSClientRcvDesc();
		*p = *(VSClientRcvDesc*)rd;
		return p;
	}
	static void Destructor(void* rd) {delete (VSClientRcvDesc*) rd;}
};


/**
**************************************************************************
 * \brief List for processing VS_ClientRcvDesc
 ****************************************************************************/
class VS_RcvList: public VS_Lock
{
	VS_Map	m_actions, m_recievers;
	int		m_id;
public:
	VS_RcvList();
	~VS_RcvList();
	/// add rd in queue
	bool AddAction(VSClientRcvDesc &rd);
	/// return action's user
	int  GetActionUser(int Id, char *name);
	/// connect new, update, disconnect; fiil name and param by Id
	int  GetAction(int Id, char *name, boost::shared_ptr<VS_StreamClientReceiver> &rcv, long &fltr);
	/// return VSClientRcvDesc by name if present
	bool GetRcvDesc(char *name, VSClientRcvDesc &rd);
	/// return list of receivers
	VS_Map GetRcvList() { return m_recievers; }
	/// return Id
	int GetId(){return m_id;}
	/// set user resolution
	bool SetFrameSizeMBUser(char *name, int frameSizeMB);
	/// get user resolution
	void GetFrameSizeMBUsersList(VS_Container& cnt);
	/// Empty all
	void Empty();
};

#endif
