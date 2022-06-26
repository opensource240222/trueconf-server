/**
 *************************************************
 * $Revision: 18 $
 * $History: VS_ConfigurationService.cpp $
 *
 * *****************  Version 18  *****************
 * User: Mushakov     Date: 16.07.12   Time: 16:29
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - g_dbStorage was wrapped to shared_ptr
 *
 * *****************  Version 17  *****************
 * User: Ktrushnikov  Date: 11.04.12   Time: 16:11
 * Updated in $/VSNA/Servers/BaseServer/Services
 * #11566: operator==(const wchar_t* sz) const didn't check sz for null
 * - VS_ConfigurationService::SetEpProperties_Method() back to 15
 * revision, because fix at VS_WideStr
 *
 * *****************  Version 16  *****************
 * User: Ktrushnikov  Date: 11.04.12   Time: 14:45
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - #11566: BS crash at 1st call prop with null length and 2nd call prop
 * not null
 * - ep_prop_set value size set 8192 (OpenGL prop at android can be long)
 * - ep_group_prop_set Processor value size set 8192 (long string from
 * Android)
 *
 * *****************  Version 15  *****************
 * User: Smirnov      Date: 5.05.09    Time: 18:33
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - cache for ep props
 *
 * *****************  Version 14  *****************
 * User: Ktrushnikov  Date: 1.12.08    Time: 13:49
 * Updated in $/VSNA/Servers/BaseServer/Services
 * [#5231] param CallId added
 *
 * *****************  Version 13  *****************
 * User: Mushakov     Date: 17.09.08   Time: 16:55
 * Updated in $/VSNA/Servers/BaseServer/Services
 *  - properties encoding corrected
 *
 * *****************  Version 12  *****************
 * User: Smirnov      Date: 26.05.08   Time: 21:47
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - bugfix with set_group_properties
 *
 * *****************  Version 11  *****************
 * User: Mushakov     Date: 26.05.08   Time: 19:37
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - params added to SetAllEpProperties method
 *
 * *****************  Version 10  *****************
 * User: Ktrushnikov  Date: 16.04.08   Time: 21:28
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - "wan_ip" param is set alone, not with SetAllEpProperties
 * (sp_set_group_edpoint_properties)
 *
 * *****************  Version 9  *****************
 * User: Smirnov      Date: 13.03.08   Time: 16:47
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - memory leaks
 *
 * *****************  Version 8  *****************
 * User: Ktrushnikov  Date: 17.02.08   Time: 10:58
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - HardwareTest: fix output cause of funcs have other names
 * - Stored Procedure: call SetAllEpProperties() for known params
 * - split Capabilities into: is_audio, is_mic, is_camera
 *
 * *****************  Version 7  *****************
 * User: Smirnov      Date: 14.02.08   Time: 13:30
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - endpoInt properties
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 13.02.08   Time: 13:43
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - properties corrected
 * - sturtup sequence of server connects rewrited
 * - code redused
 *
 * *****************  Version 5  *****************
 * User: Mushakov     Date: 12.02.08   Time: 19:13
 * Updated in $/VSNA/Servers/BaseServer/Services
 * GetAppProperties method realized
 *
 * *****************  Version 4  *****************
 * User: Dront78      Date: 23.01.08   Time: 17:51
 * Updated in $/VSNA/Servers/BaseServer/Services
 * _malloca redefinition warning fixed.
 *
 * *****************  Version 3  *****************
 * User: Stass        Date: 12.12.07   Time: 20:40
 * Updated in $/VSNA/Servers/BaseServer/Services
 * fixed init
 *
 * *****************  Version 2  *****************
 * User: Stass        Date: 5.12.07    Time: 11:09
 * Updated in $/VSNA/Servers/BaseServer/Services
 * BS - new iteration
 *
 * *****************  Version 1  *****************
 * User: Stass        Date: 28.11.07   Time: 18:33
 * Created in $/VSNA/Servers/BaseServer/Services
 * first bs ver
 *
 *************************************************/
#ifdef _WIN32	// not ported
#include "../../ServerServices/Common.h"
#include "VS_ConfigurationService.h"
#include "storage/VS_DBStorage.h"
#include "std/cpplib/StrFromVariantT.h"
#include "../../common/std/cpplib/VS_MemoryLeak.h"

#define DEBUG_CURRENT_MODULE VS_DM_CONFIGS


bool VS_ConfigurationService::Init( const char *our_endpoint, const char *our_service, const bool permittedAll )
{
	m_propcache.SetPredicate(VS_SimpleStr::Predicate);
	m_propcache.SetKeyFactory(VS_SimpleStr::Factory, VS_SimpleStr::Destructor);
	m_propcache.SetDataFactory(VS_WideStr::Factory, VS_WideStr::Destructor);
	return true;
}


bool VS_ConfigurationService::Processing( std::unique_ptr<VS_RouterMessage>&&recvMess)
{
	if (recvMess == 0)
		return true;
	switch (recvMess->Type())
	{
	case transport::MessageType::Invalid:
		break;
	case transport::MessageType::Request:
		m_recvMess = recvMess.get();
		{
			VS_Container cnt;
			if (cnt.Deserialize(recvMess->Body(), recvMess->BodySize())) {
				const char* method = 0;
				if ((method = cnt.GetStrValueRef(METHOD_PARAM)) != 0) {
					dprint4("Processing %20s; %s:%s \n", method, recvMess->SrcServer(), recvMess->SrcUser());
					// Process methods
					if (_stricmp(method,GETALLAPPPROPERTIES_METHOD) == 0){
						GetAllAppProperties_Method();
					}
					else if (_stricmp(method, SETPROPERTIES_METHOD) == 0) {
						SetEpProperties_Method(cnt);
					}
				}
			}
		}
		break;
	case transport::MessageType::Reply:
		break;
	case transport::MessageType::Notify:
		break;
	}
	m_recvMess = nullptr;
	return true;
}


void VS_ConfigurationService::GetAllAppProperties_Method()
{
	VS_AppPropertiesMap prop_map;
	auto dbStorage = g_dbStorage;
	if(!dbStorage)
		return;
	if(dbStorage->GetAllAppProperties(prop_map))
	{
		VS_Container *cnt(0);
		for(VS_AppPropertiesMap::iterator i = prop_map.begin();i!=prop_map.end();i++)
		{
			cnt = i->second;
			cnt->AddValue(METHOD_PARAM, SETPROPERTIES_METHOD);
			cnt->AddValue("server_protocol_version", SERVER_PROTOCOL_VERSION_STR);
			PostReply(*cnt);
			delete cnt;
		}
		prop_map.clear();
	}
}


void VS_ConfigurationService::SetEpProperties_Method(VS_Container &cnt)
{
	VS_SimpleStr ep = cnt.GetStrValueRef(HASH_PARAM);
	auto dbStorage = g_dbStorage;

	// special params
	const char* app_id = 0;

	int prot_version = -1;
	short int type = -1;
	VS_WideStr	version;
	VS_WideStr	app_name;
	VS_WideStr	sys_conf;
	VS_WideStr	processor;
	VS_WideStr	directX;
	VS_WideStr	hardwareConfig;
	VS_WideStr	audioCapture;
	VS_WideStr	videoCapture;
	VS_WideStr	audioRender;


	const char *name = 0, *prop = 0;
	cnt.Reset();
	while(cnt.Next()) {
		if (_stricmp(cnt.GetName(), NAME_PARAM)==0) {
			name = cnt.GetStrValueRef();
		}
		else if (_stricmp(cnt.GetName(), PROPERTY_PARAM)==0) {
			prop = cnt.GetStrValueRef();
			if ( !name || !prop )
				continue;

			if (_stricmp(name, EP_PROTOCOL_VERSION)==0) {
				prot_version = atoi(prop);
			} else if (_stricmp(name, DB_TYPE)==0) {
				type = atoi(prop);
			} else if (_stricmp(name, EP_VERSION)==0) {
				version = vs::WStrFromVariantT(prop);
			} else if (_stricmp(name, APP_NAME)==0) {
				app_name.AssignUTF8(prop);
			} else if (_stricmp(name, EP_SYS_CONF) ==0 ) {
				sys_conf.AssignUTF8(prop);
			} else if(_stricmp(name, EP_PROCESSOR) == 0) {
				processor.AssignUTF8(prop);
			} else if(_stricmp(name, EP_DIRECTX) == 0 ) {
				directX.AssignUTF8(prop);
			} else if(_stricmp(name, EP_HARDWARE_CONFIG) == 0 ) {
				hardwareConfig.AssignUTF8(prop);
			} else if(_stricmp(name, EP_AUDIO_CAPTURE) == 0 ) {
				audioCapture.AssignUTF8(prop);
			} else if(_stricmp(name, EP_VIDEO_CAPTURE) == 0) {
				videoCapture.AssignUTF8(prop);
			} else if(_stricmp(name, EP_AUDIO_RENDER) == 0){
				audioRender.AssignUTF8(prop);
			} else {
				VS_WideStr wprop;
				wprop.AssignUTF8(prop);
				VS_SimpleStr hash(ep); hash+=name;
				VS_Map::Iterator it = m_propcache.Find(hash);
				if (it == m_propcache.End() || wprop!=(const wchar_t*)(it->data))
				{
					if (!!dbStorage && dbStorage->SetEndpointProperty(ep, name, wprop))
						m_propcache.Assign(hash, wprop);
				}
			}
			name = 0; prop = 0;
		}
	}
	if (prot_version!=-1 || type!=-1 || version || app_name || sys_conf || processor
		|| directX || hardwareConfig || audioCapture || videoCapture || audioRender )
	{
		if(!!dbStorage)
			dbStorage->SetAllEpProperties(ep, prot_version, type, version, app_name, sys_conf, processor,
			directX, hardwareConfig, audioCapture, videoCapture, audioRender, m_recvMess->SrcUser());
	}
}

#endif