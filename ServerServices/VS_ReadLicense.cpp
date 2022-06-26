/**
 ****************************************************************************
 * (c) 2002-2003 Visicron Inc.  http://www.visicron.net/
 *
 * Project: Visicron Media Broker
 *
 * \file VS_ReadLicense.cpp
 *
 * License handling implementation
 *
 * $Revision: 41 $
 * $History: VS_ReadLicense.cpp $
 *
 * *****************  Version 41  *****************
 * User: Smirnov      Date: 24.08.12   Time: 23:18
 * Updated in $/VSNA/Servers/ServerServices
 * - 6 tribune for KZ
 *
 * *****************  Version 40  *****************
 * User: Smirnov      Date: 24.08.12   Time: 17:53
 * Updated in $/VSNA/Servers/ServerServices
 * - 6 tribune for smirnov
 *
 * *****************  Version 39  *****************
 * User: Smirnov      Date: 9.08.12    Time: 20:10
 * Updated in $/VSNA/Servers/ServerServices
 * - 4 podium in role conf
 *
 * *****************  Version 38  *****************
 * User: Smirnov      Date: 24.07.12   Time: 19:44
 * Updated in $/VSNA/Servers/ServerServices
 * - check role base on 255 parts
 *
 * *****************  Version 37  *****************
 * User: Mushakov     Date: 24.07.12   Time: 18:20
 * Updated in $/VSNA/Servers/ServerServices
 *  - flag MULTIGATEWAY_ALLOWED = 0x8000 supported
 *
 * *****************  Version 36  *****************
 * User: Smirnov      Date: 23.07.12   Time: 20:00
 * Updated in $/VSNA/Servers/ServerServices
 * - check role base on 400 parts
 *
 * *****************  Version 35  *****************
 * User: Ktrushnikov  Date: 25.04.12   Time: 16:17
 * Updated in $/VSNA/Servers/ServerServices
 * #9715
 * - MaxParts fix at Join with conf_id at short and full formats
 * #7086
 * - Join to SpecConf at remote server is forbidden
 * - Join from remote server to SpecConf is declined
 *
 * *****************  Version 34  *****************
 * User: Smirnov      Date: 25.10.11   Time: 18:07
 * Updated in $/VSNA/Servers/ServerServices
 * role conf up to 120 parts
 *
 * *****************  Version 33  *****************
 * User: Mushakov     Date: 29.08.11   Time: 16:43
 * Updated in $/VSNA/Servers/ServerServices
 * -  Dont remove not yet valid lic
 *
 * *****************  Version 32  *****************
 * User: Mushakov     Date: 25.08.11   Time: 19:38
 * Updated in $/VSNA/Servers/ServerServices
 * - some comments remooved
 * - update lic fixed
 *
 * *****************  Version 31  *****************
 * User: Ktrushnikov  Date: 8/23/11    Time: 1:30p
 * Updated in $/VSNA/Servers/ServerServices
 * [#9697]
 * - if role or asymmetric conf are forbidden by lic set tarif_opt = 0 for
 * this type of conf
 *
 * *****************  Version 30  *****************
 * User: Dront78      Date: 25.05.11   Time: 18:53
 * Updated in $/VSNA/Servers/ServerServices
 * - armadillo optimizations disabled totally
 *
 * *****************  Version 29  *****************
 * User: Ktrushnikov  Date: 10.05.11   Time: 20:14
 * Updated in $/VSNA/Servers/ServerServices
 * - HQ_ALLOWED renamed to HD_ALLOWED
 *
 * *****************  Version 28  *****************
 * User: Ktrushnikov  Date: 10.05.11   Time: 19:52
 * Updated in $/VSNA/Servers/ServerServices
 * #8710: support for HD video flag
 * - set UR_COMM_HDVIDEO if HQ_ALLOW in license
 *
 * *****************  Version 27  *****************
 * User: Mushakov     Date: 2.03.11    Time: 17:43
 * Updated in $/VSNA/servers/serverservices
 *  - roaming
 *
 * *****************  Version 26  *****************
 * User: Ktrushnikov  Date: 11.02.11   Time: 16:58
 * Updated in $/VSNA/Servers/ServerServices
 * VCS 3.2
 * - License: add Roaming users to OnlineUsers of our server
 * - Join & ReqInvite check for license for Roaming users
 *
 * *****************  Version 25  *****************
 * User: Ktrushnikov  Date: 12.11.10   Time: 11:54
 * Updated in $/VSNA/Servers/ServerServices
 * - VS_CheckLicense_MultiConferences() added
 *
 * *****************  Version 24  *****************
 * User: Smirnov      Date: 24.10.10   Time: 19:03
 * Updated in $/VSNA/Servers/ServerServices
 * - tarif restrictions
 * - fixed error whith no more than 80-85 participants in role conf
 *
 * *****************  Version 23  *****************
 * User: Ktrushnikov  Date: 11.10.10   Time: 14:26
 * Updated in $/VSNA/Servers/ServerServices
 * #7924: VIDEORECORDING bit in Rights by License
 * #7923: TarifRestrictions in VCS
 * - restrick in VCSConfRestrick
 * - send TARIFRESTR_PARAM to client
 *
 * *****************  Version 22  *****************
 * User: Mushakov     Date: 23.06.10   Time: 17:40
 * Updated in $/VSNA/Servers/ServerServices
 * - invalid lic removed from registry (except valid reg connected lic)
 *
 * *****************  Version 21  *****************
 * User: Mushakov     Date: 23.06.10   Time: 15:49
 * Updated in $/VSNA/Servers/ServerServices
 * - invalid lic removed from registry
 *
 * *****************  Version 20  *****************
 * User: Mushakov     Date: 29.04.10   Time: 20:07
 * Updated in $/VSNA/Servers/ServerServices
 * - memory leaks removed;
 * - descktop sharing supported (server)
 *
 * *****************  Version 19  *****************
 * User: Mushakov     Date: 27.02.10   Time: 18:43
 * Updated in $/VSNA/Servers/ServerServices
 * unlimited mobile users supported
 *
 * *****************  Version 18  *****************
 * User: Mushakov     Date: 26.02.10   Time: 15:48
 * Updated in $/VSNA/Servers/ServerServices
 *  - mobile client supported (AuthService)
 * - new cert
 *
 * *****************  Version 17  *****************
 * User: Smirnov      Date: 15.02.10   Time: 20:02
 * Updated in $/VSNA/Servers/ServerServices
 * - enh #6964
 *
 * *****************  Version 16  *****************
 * User: Melechko     Date: 1.02.10    Time: 16:29
 * Updated in $/VSNA/Servers/ServerServices
 *
 * *****************  Version 15  *****************
 * User: Melechko     Date: 21.01.10   Time: 18:18
 * Updated in $/VSNA/Servers/ServerServices
 * fix nanomites
 *
 * *****************  Version 14  *****************
 * User: Dront78      Date: 18.01.10   Time: 13:04
 * Updated in $/VSNA/Servers/ServerServices
 *
 * *****************  Version 13  *****************
 * User: Mushakov     Date: 3.01.10    Time: 15:16
 * Updated in $/VSNA/Servers/ServerServices
 * - VCS refactoried
 *
 * *****************  Version 12  *****************
 * User: Mushakov     Date: 3.01.10    Time: 13:51
 * Updated in $/VSNA/Servers/ServerServices
 * - LicenseEvents added
 * - restricted build in restrict interface included
 * - RESTRICTED_BUILD removed
 * - Reading "group allowed" flag from licenses added
 * - guest autorization in LDAP mode added
 *
 * *****************  Version 11  *****************
 * User: Mushakov     Date: 18.12.09   Time: 18:04
 * Updated in $/VSNA/Servers/ServerServices
 * - Removed VCS_BUILD somewhere
 * - Add new field to license
 * - Chat service for bsServer renamed
 *
 * *****************  Version 10  *****************
 * User: Mushakov     Date: 26.11.09   Time: 19:09
 * Updated in $/VSNA/Servers/ServerServices
 *  - init g_vcs_storage by getting Licenses
 *
 * *****************  Version 9  *****************
 * User: Mushakov     Date: 9.11.09    Time: 16:15
 * Updated in $/VSNA/Servers/ServerServices
 *
 * *****************  Version 8  *****************
 * User: Ktrushnikov  Date: 6.11.09    Time: 19:55
 * Updated in $/VSNA/Servers/ServerServices
 * VS_ReadLicense
 * - fix with break
 * - LE_GUEST_LOGIN=9 added
 * VS_Storage
 * - m_num_guest - count of loggedin guests
 * VCSAuthService
 * - DeleteUser: guest from map ("registry")
 * VCSStorage
 * - DeleteUser added to interface
 * VS_RegistryStorage
 * - Guest login supported
 * - Check Guest password
 * - Check Count of Guests by License
 * - Read Shared Key from registry
 *
 * *****************  Version 7  *****************
 * User: Mushakov     Date: 4.11.09    Time: 21:13
 * Updated in $/VSNA/Servers/ServerServices
 *  - new names
 *
 * *****************  Version 6  *****************
 * User: Mushakov     Date: 4.11.09    Time: 20:07
 * Updated in $/VSNA/Servers/ServerServices
 * - quick update license
 * - #regs suffix added
 *
 * *****************  Version 5  *****************
 * User: Mushakov     Date: 4.11.09    Time: 18:49
 * Updated in $/VSNA/Servers/ServerServices
 *
 * *****************  Version 4  *****************
 * User: Mushakov     Date: 28.10.09   Time: 17:59
 * Updated in $/VSNA/Servers/ServerServices
 * - vs_ep_id removed
 * - registration corrected
 *
 * *****************  Version 3  *****************
 * User: Mushakov     Date: 23.10.09   Time: 15:05
 * Updated in $/VSNA/Servers/ServerServices
 *  - VCS 3
 *
 * *****************  Version 2  *****************
 * User: Mushakov     Date: 7.08.09    Time: 15:14
 * Updated in $/VSNA/Servers/ServerServices
 * - SBSv3 added by Matvey (SBSv3_m)
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 31.03.09   Time: 19:11
 * Created in $/VSNA/Servers/ServerServices
 * Read Licenses added
 *
 * *****************  Version 2  *****************
 * User: Avlaskin     Date: 23.04.07   Time: 15:33
 * Updated in $/VS2005/Servers/MediaBroker/BrokerServices
 * +added roaming
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/Servers/MediaBroker/BrokerServices
 *
 * *****************  Version 20  *****************
 * User: Stass        Date: 21.12.06   Time: 16:45
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 *
 * *****************  Version 19  *****************
 * User: Smirnov      Date: 15.11.06   Time: 14:40
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 * - new endpoint id
 *
 * *****************  Version 18  *****************
 * User: Stass        Date: 30.10.06   Time: 18:46
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 * added gateway counter
 *
 * *****************  Version 17  *****************
 * User: Stass        Date: 29.08.06   Time: 13:30
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 * added license start time
 *
 * *****************  Version 16  *****************
 * User: Smirnov      Date: 14.08.06   Time: 15:53
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 * - memory leaks in transport
 *
 * *****************  Version 15  *****************
 * User: Stass        Date: 25.07.06   Time: 16:29
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 * added roaming control in licenses
 *
 * *****************  Version 14  *****************
 * User: Stass        Date: 20.04.06   Time: 13:28
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 * store license info
 *
 * *****************  Version 13  *****************
 * User: Stass        Date: 30.03.06   Time: 17:12
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 * added switchable AB to reg storage
 *
 * *****************  Version 12  *****************
 * User: Stass        Date: 27.04.04   Time: 17:19
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 * allowed creation of license reg key
 *
 * *****************  Version 11  *****************
 * User: Stass        Date: 29.01.04   Time: 14:54
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 * added grace period for licensing
 *
 * *****************  Version 10  *****************
 * User: Stass        Date: 28.01.04   Time: 20:09
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 * added license update logic
 *
 * *****************  Version 9  *****************
 * User: Stass        Date: 26.01.04   Time: 15:40
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 * moved license things to lib
 *
 * *****************  Version 8  *****************
 * User: Smirnov      Date: 6.01.04    Time: 20:35
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 * module based debug print
 *
 * *****************  Version 7  *****************
 * User: Stass        Date: 23.12.03   Time: 16:59
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 * added license reload
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 22.12.03   Time: 16:40
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 * license limits reported to client
 *
 * *****************  Version 5  *****************
 * User: Stass        Date: 22.12.03   Time: 15:22
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 * added New Conf license event
 *
 * *****************  Version 4  *****************
 * User: Stass        Date: 20.12.03   Time: 14:22
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 * added new license fields and ID control
 *
 * *****************  Version 3  *****************
 * User: Stass        Date: 19.12.03   Time: 17:12
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 * moved license key to global root
 *
 * *****************  Version 2  *****************
 * User: Stass        Date: 9.12.03    Time: 13:39
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 * licenses now are read from registry
 *
 * *****************  Version 1  *****************
 * User: Stass        Date: 25.11.03   Time: 18:59
 * Created in $/VS/Servers/MediaBroker/BrokerServices
 * added licensing
 *
 ****************************************************************************/

#include <stdio.h>

#include "AppServer/Services/VS_Storage.h"
#include "../common/std/cpplib/VS_RegistryKey.h"
#include "../common/std/cpplib/VS_RegistryConst.h"
#include "ProtectionLib/Protection.h"
#include "VS_ReadLicense.h"
#include "../common/std/cpplib/VS_MemoryLeak.h"
#include "std/debuglog/VS_Debug.h"

#include <cinttypes>
#include <string>
#include <vector>

#define DEBUG_CURRENT_MODULE VS_DM_READLIC
#define EMPTY_TIME std::chrono::system_clock::time_point()

const char LIC_ONLINE_USERS_TAG[] = "Online Users";
const char LIC_CONFERENCES_TAG[] = "Conferences";
const char LIC_GATEWAYS_TAG[] = "Gateways";
const char LIC_TERMINAL_PRO_USERS_TAG[] = "TerminalProUsers";
const char LIC_SYMMETRIC_PARTICIPANTS_TAG[] = "SymmetricParticipants";
const char LIC_MAX_GUESTS_TAG[] = "MaxGuests";
const char LIC_RESTRICTION_TAG[] = "Restrictions";
const char LIC_LIMITED_TAG[] = "Limited";
const char LIC_TRIAL_TIME_TAG[] = "Trial Time";
const char LIC_VALID_UNTIL_TAG[] = "Valid Until";
const char LIC_RELOAD_DATE_TAG[] = "Reload Date";
const char SERVER_SHUTDOWN_TIME_TAG[] = "Shutdown Time";

VS_LicensesWrap	*p_licWrap(0);

SECURE_FUNC
bool VS_ReadLicense(const VS_SimpleStr& my_name,VS_License::Flags flags, VS_Container &cnt) /**копии параметров, 1 - ограничение на пользователей, 2 - на кол-во конференций 3 - содержит корректный valid_until*/
{
//NANOBEGIN;

	if(!p_licWrap)
		return false;

	char const2[] = "q&&3d;,;";
	char meth[9] = {0};
	for(int i=0;i<8;i++)
	{
		meth[i] = USERNAME_PARAM[i]^const2[i];
	}

	cnt.AddValue(METHOD_PARAM,meth);
	p_licWrap->Clear();
	VS_SimpleStr errStr = "";

	VS_License	allLicenseSum;
	allLicenseSum.m_error = VSS_LICENSE_NOT_VALID;

	int licenses=0;
	int valid_licenses = 0;
	char buf[128] = {0};
	std::chrono::system_clock::time_point nextNewLicense;
	auto now =  std::chrono::system_clock::now();
	auto maxLicense = now;
	bool is_limited = false; // True if at least one valid license requires a connection to reg server, and we don't yet have the connection.

	//VS_RegistryKey	l_root(VS_Server::RegistryKey(),false, LICENSE_KEY, false, true);
	VS_RegistryKey	l_root(false, LICENSE_KEY, false, true);
	VS_RegistryKey	l_reg;
	if (!l_root.IsValid())
	{
		dprint1("license root not valid\n");
	}
	else
	{
		l_root.ResetKey();

		std::vector<std::string> to_delete;
		while (l_root.NextKey(l_reg))
		{
			bool valid_in_some_cases = false;
			dprint1("\texamining license key %s\n", l_reg.GetName() );
			VS_License	lic (l_reg);
			if(lic.m_error)
			{
				dprint0("\tlicense key '%s' not valid, status: %d\n",l_reg.GetName(),lic.m_error);
			}
			else
			{
				dprint3("\t\tlicense is valid\n");
				if(lic.m_onlineusers==lic.TC_INFINITY)
				{	dprint3("\t\tno user restriction\n");}
				else
					dprint3("\t\tonline users: %d\n",lic.m_onlineusers);

				if(lic.m_conferences==lic.TC_INFINITY)
				{	dprint3("\t\tno conference restriction\n");}
				else
					dprint3("\t\tconferences:  %d\n",lic.m_conferences);

				if(lic.m_restrict & VS_License::ROAMING_ALLOWED)
					dprint3("\t\troaming\n");
				if(lic.m_restrict & VS_License::ENABLE_DIRECTORY)
					dprint3("\t\tdirectory is enabled\n");
				if(lic.m_restrict & VS_License::PAID_SERVER)
					dprint3("\t\tpaid_server\n");

				if(lic.m_gateways==lic.TC_INFINITY)
				{	dprint3("\t\tunlimited gateways\n");}
				else if(lic.m_gateways>0)
					dprint3("\t\tgateways:  %d\n",lic.m_gateways);
				if(!lic.m_serverName)
				{
					dprint3("\t\tno cluster/endpoint restriction\n");
				}
				else
				{
					dprint1("\t\tlicense restricted to server '%s'\n",(char*)lic.m_serverName);
					if(lic.m_serverName!=my_name)
					{
							dprint0("\tlicense key '%s' restricted to other server\n",l_reg.GetName());
							lic.m_error=VSS_LICENSE_NOT_VALID;
					}
				}
				if(lic.m_validuntil==EMPTY_TIME)
				{
					maxLicense = lic.m_validuntil;
					dprint3("\t\tvalid until forever\n");
				}
				else
				{
					tu::TimeToLStr(lic.m_validuntil, buf, 128);
					dprint1("\t\tvalid until %s\n", buf);
					if(lic.m_validuntil!=EMPTY_TIME && lic.m_validuntil<now)
					{
							dprint0("\tlicense key '%s' expired\n",l_reg.GetName());
							lic.m_error=VSS_LICENSE_NOT_VALID;
					}
					else if(maxLicense != EMPTY_TIME && maxLicense <lic.m_validuntil)
						maxLicense = lic.m_validuntil;

				};

				if(lic.m_validafter==EMPTY_TIME)
				{
					dprint3("\t\tvalid after 0\n");
				}
				else
				{
					tu::TimeToLStr(lic.m_validafter, buf, 128);
					dprint1("\t\tvalid after %s\n", buf);
					//VS_FileTime now_with_delta(now);
					//now_with_delta+=(LONGLONG)10000000*60*15; //+ 15 min
					if(lic.m_validafter!= EMPTY_TIME && lic.m_validafter>=now)
					{
							dprint1("\tlicense key '%s' not yet valid\n",l_reg.GetName());
							if (lic.IsValid())
								valid_in_some_cases = true;
							lic.m_error=VSS_LICENSE_NOT_VALID;
							if(nextNewLicense== EMPTY_TIME || nextNewLicense>lic.m_validafter)
								nextNewLicense=lic.m_validafter;
					}
				};
				if(lic.m_restrict & VS_License::REG_CONNECTED)
				{
					if (flags&VS_License::REG_CONNECTED)
					{	dprint1("\t\trestricted to connected mode\n");	}
					else
					{
						dprint1("\t\tlicense is not valid in disconnected mode\n");
						// Merge license into allLicenseSum now because it won't be merged at the end of the loop after we will invalidate it.
						if (!p_licWrap->HasLicense(lic.m_id))
							allLicenseSum.MergeLicense(lic);
						if (lic.IsValid())
						{
							valid_in_some_cases = true;
							is_limited = true;
						}
						lic.m_error=VSS_LICENSE_NOT_VALID;
					};
				};
				if (p_licWrap->HasLicense(lic.m_id))
				{
					lic.m_error=VSS_LICENSE_DUPLICATE;
					dprint0("\tlicense key '%s' is duplicate\n",l_reg.GetName());
				}
				else if (lic.IsValid() || valid_in_some_cases)
				{
					p_licWrap->AddLicense(lic.m_id, l_reg.GetName());
					++valid_licenses;
				}
				p_licWrap->MergeLicense(lic);
				allLicenseSum.MergeLicense(lic);
			};
			if (!lic.IsValid() && !(flags & VS_License::GRACE_PERIOD) && !valid_in_some_cases)
				to_delete.emplace_back(l_reg.GetName());

			licenses++;
		}
		for (const auto& x: to_delete)
			l_root.RemoveKey(x);
	}// license root not valid
	if (allLicenseSum.IsValid())
	{
		std::unique_ptr<uint8_t[]> lic_buf;
		size_t lic_buf_sz = 0;
		if(allLicenseSum.Serialize(lic_buf, lic_buf_sz))
		{
			cnt.AddValue(DATA_PARAM,lic_buf.get(),lic_buf_sz);
			cnt.AddValue(VALID_UNTIL_PARAM,maxLicense);
			dprint4("\tallLicenseSum added to cnt; err = %d\n",allLicenseSum.m_error);
		}
		int32_t limited = is_limited ? 1 : 0;;
		cnt.AddValueI32("limited", limited);
	}

	if (valid_licenses == 0)
	{
		if(licenses==0)
		{	errStr = "no licenses found!";
			dprint0("%s\n", errStr.m_str);

		}
		else
		{
			errStr = "no valid licenses available!";
			dprint0("%s\n", errStr.m_str);
		}

		if(!(flags&VS_License::GRACE_PERIOD))
		{
			VS_AutoLock _(p_licWrap);
			cnt.AddValue(RESULT_PARAM,errStr);
			auto lic_sum = p_licWrap->GetLicSum();
			return false;
		}
		dprint1( "broker will be shut down in 3 minutes\n" );
		auto valid_until = std::chrono::system_clock::now();
		p_licWrap->SetValidUntil(valid_until);
		tu::TimeToLStr(valid_until, buf, 128);
		dprint3("\tdown time %s\n", buf);

		VS_AutoLock _(p_licWrap);
		auto lic_sum = p_licWrap->GetLicSum();
		return true;
	};

	if(nextNewLicense!=EMPTY_TIME && (p_licWrap->GetValidUntil()== EMPTY_TIME || p_licWrap->GetValidUntil()>nextNewLicense))
		p_licWrap->SetValidUntil(nextNewLicense);

	VS_AutoLock _(p_licWrap);
	VS_License sum_lic = p_licWrap->GetLicSum();
	dprint1("\tResulting license: (%" PRIx64 ")\n",sum_lic.m_restrict);
	if(sum_lic.m_onlineusers!=sum_lic.TC_INFINITY)
		dprint1("\t\tonline users: %d\n",sum_lic.m_onlineusers);
	if(sum_lic.m_conferences!=sum_lic.TC_INFINITY)
		dprint1("\t\tconferences:  %d\n",sum_lic.m_conferences);

	if(sum_lic.m_conferences==sum_lic.TC_INFINITY)
	{
		dprint1("\t\tunlimited gateways\n");
	}
	else if(sum_lic.m_gateways>0)
		dprint1("\t\tgateways:  %d\n",sum_lic.m_gateways);


	if(sum_lic.m_validuntil!=EMPTY_TIME)
	{
		tu::TimeToLStr(sum_lic.m_validuntil, buf, 128);
		dprint3("\t\treload time %s\n", buf);
	}
	else
		dprint3("\t\tno reload\n");

	cnt.AddValue(RESULT_PARAM,"");
//NANOEND;
	return true;
}

SECURE_FUNC
bool VS_CheckLicense(VS_LicenseEvents evt)
{
//NANOBEGIN;
	if(!p_licWrap)
		return false;
	VS_License	lic_sum = p_licWrap->GetLicSum();

	if (!lic_sum.IsValid())
	{
		dprint3("server license not valid\n");
		return false;
	}

	bool result=false;
	switch(evt)
	{
	case LE_LOGIN:
		if(lic_sum.m_onlineusers==lic_sum.TC_INFINITY)
			{result=true;break;};
		if(g_storage->CountOnlineUsers()<lic_sum.m_onlineusers) {
			result=true;
		} else {
			p_licWrap->IncrFailCounter(LE_LOGIN);
			dprint0("licensed online user number exceeded\n");
		}
		break;
	case LE_NEWCONFERENCE:
		if(lic_sum.m_conferences==lic_sum.TC_INFINITY)
			{result=true;break;};
		if(g_storage->CountGroupConferences()<lic_sum.m_conferences)
			result=true;
		else {
			p_licWrap->IncrFailCounter(LE_NEWCONFERENCE);
			dprint0("licensed concurrent conference number exceeded\n");
		}
		break;
	case LE_ROAMING_ON:
		if(lic_sum.m_restrict & VS_License::ROAMING_ALLOWED)
			{result=true;break;};
		dprint3("roaming disabled\n");
		break;
	case LE_GATEWAYLOGIN:
		if(lic_sum.m_gateways==lic_sum.TC_INFINITY)
			{result=true;break;};
		if(g_storage->CountOnlineGateways()<lic_sum.m_gateways)
			result=true;
		else {
			p_licWrap->IncrFailCounter(LE_GATEWAYLOGIN);
			dprint0("licensed online gateway number exceeded\n");
		}
		break;
	case LE_TRIAL:
		if(lic_sum.m_trial_conf_minutes != 0)
			result = true;
		break;
	case LE_FILETRANSFER:
		if(lic_sum.m_restrict &VS_License::FILETRANSFER_ALLOWED)
			result = true;
		break;
	case LE_WHITEBOARD:
		if(lic_sum.m_restrict & VS_License::WHITEBOARD_ALLOWED)
			result = true;
		break;
	case LE_SLIDESHOW:
		if(lic_sum.m_restrict & VS_License::SLIDESHOW_ALLOWED)
			result = true;
		break;
	case LE_DSHARING:
		if(lic_sum.m_restrict & VS_License::DSHARING_ALLOWED)
			result = true;
		break;
	case LE_GUEST_LOGIN:
		if(lic_sum.m_max_guests==lic_sum.TC_INFINITY)
		{result=true;break;};
		if (lic_sum.m_max_guests > g_storage->CountOnlineGuests()) {
			result = true;
		} else {
			p_licWrap->IncrFailCounter(LE_GUEST_LOGIN);
		}
		break;
	case LE_TERMINAL_LOGIN:
		if (VS_CheckLicense(LE_PAID_SERVER))
		{
			if (lic_sum.m_terminal_pro_users == lic_sum.TC_INFINITY)
			{
				result = true; break;
			};
			if (lic_sum.m_terminal_pro_users > g_storage->CountOnlineTerminalPro())
				result = true;
		}
		break;
	case LE_LDAP_ALLOWED:
		if(lic_sum.m_restrict & VS_License::LDAP_ALLOWED)
			result = true;
		break;
	case LE_ASYMMETRICCONF_ALLOWED:
		if(lic_sum.m_restrict & VS_License::ASYMMETRICCONF_ALLOWED)
			result = true;
		break;
	case LE_ROLE_CONF_ALLOWED:
		if(lic_sum.m_restrict & VS_License::ROLECONF_ALLOWED)
			result = true;
		break;
	case LE_UDPMULTICAST_ALLOWED:
		if(lic_sum.m_restrict & VS_License::UDPMULTICAST_ALLOWED)
			result = true;
		break;
	case LE_USER_GROUPS_ALLOWED:
		if(lic_sum.m_restrict & VS_License::USER_GROUPS_ALLOWED)
			result = true;
		break;
	case LE_SSL_STREAMS:
		if(lic_sum.m_restrict & VS_License::ENCRYPT_ALLOWED)
			result = true;
		break;
	case LE_VIDEORECORDING:
		if(lic_sum.m_restrict & VS_License::VIDEORECORDING_ALLOWED)
			result = true;
		break;
	case LE_HDVIDEO:
		if(lic_sum.m_restrict & VS_License::HD_ALLOWED)
			result = true;
		break;
	case LE_MULTIGATEWAY:
		if(lic_sum.m_restrict & VS_License::MULTIGATEWAY_ALLOWED)
			result = true;
		break;
	case LE_WEBRTC_BROADCAST:
		if(lic_sum.m_restrict & VS_License::WEBRTC_BROADCAST_ALLOWED)
			result = true;
		break;
	case LE_IMPROVED_SECURITY:
		if(lic_sum.m_restrict & VS_License::IMPROVED_SECURITY)
			result = true;
		break;
	case LE_ENABLE_DIRECTORY:
		if(lic_sum.m_restrict & VS_License::ENABLE_DIRECTORY)
			result = true;
		break;
	case LE_PAID_SERVER:
		if(lic_sum.m_restrict & VS_License::PAID_SERVER)
			result = true;
		break;
	case LE_CONF_BROADCAST:
		if(lic_sum.m_restrict & VS_License::ENABLE_CONF_BROADCAST)
			result = true;
		break;
	case LE_WEBINARS:
		if (lic_sum.m_restrict & VS_License::ENABLE_WEBINARS)
			result = true;
		break;
	case LE_SDK_CLIENTS:
		if (lic_sum.m_restrict & VS_License::ENABLE_SDK_CLIENTS)
			result = true;
		break;
	case LE_ENTERPRISE_MASTER:
		if (lic_sum.m_restrict & VS_License::ENTERPRISE_MASTER)
			result = true;
		break;
	case LE_ENTERPRISE_SLAVE:
		if (lic_sum.m_restrict & VS_License::ENTERPRISE_SLAVE)
			result = true;
		break;
	}
//NANOEND;
	return result;
}
bool VS_CheckLicense_MultiConferences()
{
//NANOBEGIN;
	if(!p_licWrap)
		return false;
	VS_License	lic_sum = p_licWrap->GetLicSum();

	if (!lic_sum.IsValid())
	{
		dprint3("server license not valid\n");
		return false;
	}

	bool result=false;

	if (lic_sum.m_conferences>0 || lic_sum.m_conferences==lic_sum.TC_INFINITY)
		result = true;
	else
		result = false;

//NANOEND;
	return result;
}
void VS_GetLicence_TarifRestrictions(int32_t tarif_opts[4], long Podiums, bool IsUser, bool IsIntercom, bool IsPublic)
{
//NANOBEGIN;
	if(!p_licWrap)
		return ;
	VS_License	lic_sum = p_licWrap->GetLicSum();

	if (!lic_sum.IsValid())
	{
		dprint3("server license not valid\n");
		return;
	}
	const static int MAX_INTERCOM_ROLE_PARTS = 1599;// limit for planned UDP Multicast conferences -1
	const static int MAX_ROLE_PARTS_USER = 250;		// limit for hot conferenses, don't inrease it more than 255

	int online_users = lic_sum.m_onlineusers == VS_License::TC_INFINITY ? INT_MAX : lic_sum.m_onlineusers;
	int sym_users = lic_sum.m_symmetric_participants == VS_License::TC_INFINITY ? INT_MAX : lic_sum.m_symmetric_participants;
	int guests = lic_sum.m_max_guests == VS_License::TC_INFINITY ? INT_MAX : lic_sum.m_max_guests;
	int maxparts = IsPublic && !IsUser ? (online_users == INT_MAX || guests == INT_MAX ? INT_MAX : online_users + guests) : online_users;
	if (VS_CheckLicense(LE_ENTERPRISE_SLAVE))
		maxparts = INT_MAX;
	int PodLimit = (maxparts > 0) * 35 + 1;
	Podiums = Podiums > PodLimit ? PodLimit : (Podiums < 1 ? 1 : Podiums);
	int maxrole = MAX_INTERCOM_ROLE_PARTS;
	if (!IsIntercom) {
		if (Podiums > 1 && Podiums < 6)
			maxrole = 300 + (2 / Podiums) * 100;
		else
			maxrole = (240 / (Podiums + 2)) * 10; // restriction by router
	}
	else
		maxrole++;

	int N0 = std::min(49, sym_users);	N0 = std::min(N0, maxparts);
	int N1 = VS_CheckLicense(LE_ASYMMETRICCONF_ALLOWED) ? N0 : 0;
	int N2 = maxrole;
	if (IsUser)
		N2 = std::min(N2, MAX_ROLE_PARTS_USER);
	N2 = VS_CheckLicense(LE_ROLE_CONF_ALLOWED) ? std::min(N2, maxparts) : 0;
	int N3 = N2 > 0 ? Podiums : 1;

	unsigned long t=0;
	tarif_opts[0] = N0;
	tarif_opts[1] = N1;
	tarif_opts[2] = N2;
	tarif_opts[3] = N3;

//NANOEND;
}

bool VS_CheckLicense_CheckRoamingParticipant()
{
//NANOBEGIN;
	if(!p_licWrap)
		return false;
	VS_License	lic_sum = p_licWrap->GetLicSum();

	if (!lic_sum.IsValid())
	{
		dprint3("server license not valid\n");
		return false;
	}
	if(!VS_CheckLicense(LE_ROAMING_ON))
	{
		dprint3("roaming is not allowed by lic\n");
		return false;
	}
	if (VS_CheckLicense(LE_ENTERPRISE_SLAVE) || VS_CheckLicense(LE_ENTERPRISE_MASTER)) // bug#51028
		return true;

	bool result = false;
	int total = g_storage->CountOnlineUsers() + g_storage->CountRoamingParts();
	if(lic_sum.m_onlineusers==lic_sum.TC_INFINITY)
		result=true;
	else if(total<lic_sum.m_onlineusers)
		result=true;
	else
		dprint0("licensed online & roaming user number exceeded\n");
//NANOEND;
	return result;
}
