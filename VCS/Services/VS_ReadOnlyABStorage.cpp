/**
 ****************************************************************************
 * (c) 2005 Visicron Inc.  http://www.visicron.net/
 *
 * Project: Visicron server services
 * \brief In-memory read-only  AB storage
 *
 * $Revision: 2 $
 * $History: VS_ReadOnlyABStorage.cpp $
 *
 * *****************  Version 2  *****************
 * User: Ktrushnikov  Date: 17.02.11   Time: 18:31
 * Updated in $/VSNA/Servers/VCS/Services
 * was not checked-in
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 3.01.10    Time: 15:16
 * Created in $/VSNA/Servers/VCS/Services
 * - VCS refactoried
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 4.11.09    Time: 21:13
 * Created in $/VSNA/Servers/VCS/Storage
 *  - new names
 *
 * *****************  Version 3  *****************
 * User: Mushakov     Date: 2.11.09    Time: 15:03
 * Updated in $/VSNA/Servers/SBSv3_m/Storage
 * - store users by login only
 *
 * *****************  Version 2  *****************
 * User: Mushakov     Date: 28.10.09   Time: 17:58
 * Updated in $/VSNA/Servers/SBSv3_m/Storage
 * - vs_ep_id removed
 * - registration corrected
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 7.08.09    Time: 14:48
 * Created in $/VSNA/Servers/SBSv3_m/Storage
 * - SBSv3 added by Matvey (SBSv3_m)
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/Servers/MediaBroker/BrokerServices
 *
 * *****************  Version 3  *****************
 * User: Stass        Date: 28.07.06   Time: 16:21
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 * address book caching support
 *
 * *****************  Version 2  *****************
 * User: Stass        Date: 13.04.06   Time: 18:01
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 * group based permissions
 *
 * *****************  Version 1  *****************
 * User: Stass        Date: 30.03.06   Time: 17:12
 * Created in $/VS/Servers/MediaBroker/BrokerServices
 * added switchable AB to reg storage
 ****************************************************************************/

#include "VS_ReadOnlyABStorage.h"

int VS_ReadOnlyABStorage::ABFind(VS_Container& cnt, int& entries, VS_AddressBook ab,const char*  owner, const std::string& query, long client_hash, VS_Container* in_cnt)
{
	if (ab==AB_COMMON || ab==AB_PERSONS)
	{
		if(VS_CompareHash(client_hash,*m_hash))
		{
			entries=-1;
			return SEARCH_NOT_MODIFIED;
		}
		VS_StorageUserData* ud;
		VS_RegistryStorage::UserMap::Iterator i;
		unsigned long j = 0;

		{auto locked_ab_users = m_ab_users->lock();
		if (locked_ab_users->Size()<=0)
		{
			entries=0;
			return SEARCH_DONE;
		};
		cnt.AddValueI32(HASH_PARAM, *m_hash);
		for (i = locked_ab_users->Begin(); i != locked_ab_users->End(); ++i, j++)
		{
			ud = i->data;
			cnt.AddValue(USERNAME_PARAM, ud->m_name);
			cnt.AddValue(CALLID_PARAM,   ud->m_name);
			cnt.AddValue(DISPLAYNAME_PARAM, ud->m_displayName);
		}
		entries=(int)locked_ab_users->Size();
		}
		return SEARCH_DONE;
	}
	else
		return SEARCH_FAILED;
}
