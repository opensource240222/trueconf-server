/****************************************************************************
 * (c) 2002 Visicron Inc.  http://www.visicron.net/
 *
 * Project: Server Services
 *
 * $History: VS_StreamPartDesc.h $
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 10.02.10   Time: 15:26
 * Updated in $/VSNA/Servers/ServerServices/Types
 * - ssl streams for group conf
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:50
 * Created in $/VSNA/Servers/ServerServices/types
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 4.07.07    Time: 19:25
 * Updated in $/VS2005/Servers/ServerServices/types
 * - device statuses added
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/Servers/ServerServices/types
 *
 * *****************  Version 7  *****************
 * User: Smirnov      Date: 1.08.05    Time: 19:25
 * Updated in $/VS/Servers/ServerServices/Types
 * - multicast, last interation
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 27.07.05   Time: 19:57
 * Updated in $/VS/Servers/ServerServices/Types
 * -multicast beta 2
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 15.03.04   Time: 20:09
 * Updated in $/VS/Servers/ServerServices/Types
 * multi conference multiforat in broker
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 3.11.03    Time: 18:46
 * Updated in $/VS/Servers/ServerServices/Types
 * added INTERCom support
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 28.10.03   Time: 21:26
 * Updated in $/VS/Servers/ServerServices/Types
 * new disconnect scheme
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 9.10.03    Time: 19:44
 * Updated in $/VS/Servers/ServerServices/Types
 * new methods in client
 * new files in std...
 *
 * *****************  Version 1  *****************
 * User: Smirnov      Date: 2.10.03    Time: 19:18
 * Created in $/VS/Servers/ServerServices/Types
 * added coonections in multistream conf
 *
 ****************************************************************************/
/****************************************************************************
 * \file VS_StreamPartDesc.h
 * \brief contain stream part
 ****************************************************************************/

#ifndef VS_STREAM_PART_DESC_H
#define VS_STREAM_PART_DESC_H

#include "../../common/std/cpplib/VS_IDTypes.h"
#include "../../common/std/cpplib/VS_RcvFunc.h"

class VS_StreamPartDesc
{
public:
	VS_SimpleStr		m_name;		// id
	vs_user_id			m_snd;		// primary key
	vs_user_id			m_rcv;		// slave key
	vs_conf_id			m_conf;
	VS_SimpleStr		m_host;
	VS_SimpleStr		m_mcast_ip;
	VS_SimpleStr		m_sym_key;
	long				m_confKey;
	long				m_port;
	long				m_fltr;
	long				m_type;
	long				m_dvs;
	VS_BinBuff			m_caps;

	VS_StreamPartDesc(): m_confKey(0), m_port(0), m_fltr(0), m_type(RCT_UNKNOWN), m_dvs(0) {}
	~VS_StreamPartDesc(){};
	bool IsIam(){return m_snd==m_rcv;}
	static void* Factory(const void* pd){return new VS_StreamPartDesc(*(VS_StreamPartDesc*)pd);}
	static void Destructor(void* pd) {delete (VS_StreamPartDesc*) pd;}

};

class VS_StreamPartMap
{
public:
	static void* Factory(const void* /*Map*/) { // only one instanse for one key!
		VS_Map* sPartMap = new VS_Map;
		sPartMap->SetPredicate(VS_SimpleStr::Predicate);
		sPartMap->SetKeyFactory(VS_SimpleStr::Factory, VS_SimpleStr::Destructor);
		sPartMap->SetDataFactory(VS_StreamPartDesc::Factory, VS_StreamPartDesc::Destructor);
		return sPartMap;
	}
	static void Destructor(void* Map) { delete (VS_Map*) Map;}
};


#endif
