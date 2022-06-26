#include "VS_RTSPObjectFactory.h"
#include "VS_RTSP_CSeq.h"
#include "VS_RTSP_Accept.h"
#include "VS_RTSP_Session.h"
#include "VS_RTSP_UserAgent.h"
#include "VS_RTSP_Public.h"
#include "VS_RTSP_Transport.h"
#include "VS_RTSP_Server.h"
#include "VS_RTSP_StartLine.h"
#include "VS_RTSP_Content_type.h"
#include "VS_RTSP_ContentSize.h"
#include "VS_RTSP_Date.h"
#include "VS_SIPField_Auth.h"
#include "std/debuglog/VS_Debug.h"

#include <boost/regex.hpp>

#include <algorithm>

#define DEBUG_CURRENT_MODULE VS_DM_SIPPARSER

const std::pair<std::string, eRequestType> methods_tbl[] = {
	{ "ANNOUNCE", REQUEST_ANNOUNCE },
	{ "DESCRIBE", REQUEST_DESCRIBE },
	{ "GET_PARAMETER", REQUEST_GET_PARAMETER },
	{ "OPTIONS", REQUEST_OPTIONS },
	{ "PAUSE", REQUEST_PAUSE },
	{ "PLAY", REQUEST_PLAY },
	{ "RECORD", REQUEST_RECORD },
	{ "REDIRECT", REQUEST_REDIRECT },
	{ "SETUP", REQUEST_SETUP },
	{ "SET_PARAMETER", REQUEST_SET_PARAMETER },
	{ "TEARDOWN", REQUEST_TEARDOWN }
};

struct less_first
{
	template <class T1, class T2, class U>
	bool operator()(const std::pair<T1, T2>& l, const U& r)
	{
		return l.first < r;
	}
	template <class T, class U1, class U2>
	bool operator()(const T& l, const std::pair<U1, U2>& r)
	{
		return l < r.first;
	}
	template <class T1, class T2, class U1, class U2>
	bool operator()(const std::pair<T1, T2>& l, const std::pair<U1, U2>& r)
	{
		return l.first < r.first;
	}
};

struct less_second
{
	template <class T1, class T2, class U>
	bool operator()(const std::pair<T1, T2>& l, const U& r)
	{
		return l.second < r;
	}
	template <class T, class U1, class U2>
	bool operator()(const T& l, const std::pair<U1, U2>& r)
	{
		return l < r.second;
	}
	template <class T1, class T2, class U1, class U2>
	bool operator()(const std::pair<T1, T2>& l, const std::pair<U1, U2>& r)
	{
		return l.second < r.second;
	}
};

VS_RTSPObjectFactory * VS_RTSPObjectFactory::iThis = nullptr;

VS_ObjectFactory::CreateFieldResult VS_RTSPObjectFactory::CreateField(VS_SIPBuffer & sbInput) const
{
	boost::regex brRTSPCommand("((play|options|describe|get_parameter|setup|teardown)\\srtsp)", boost::regex::icase);
	std::unique_ptr <const char[]> data = 0;
	std::size_t iHeaderSize = 0;

	const TSIPErrorCodes err = sbInput.GetHeaderAllocConst(":", data, iHeaderSize);
	const char *cHeader = data.get();
	if (err != TSIPErrorCodes::e_ok)
		return CreateFieldResult(nullptr, err);
	try
	{
		if (boost::regex_match(cHeader, brRTSPCommand))
		{
				cHeader = "STARTLINE";
		}
	}
	catch (const std::runtime_error &ex)
	{
		dstream1 << "VS_RTSPObjectFactory::CreateField error " << ex.what() << "\n";
		return CreateFieldResult(nullptr, TSIPErrorCodes::e_match);
	}
	const VS_ObjectFactory::GetInstanceResult get_inst_res = GetInstance(cHeader);

	if (get_inst_res.error_code != TSIPErrorCodes::e_ok)
		return CreateFieldResult(nullptr, get_inst_res.error_code);

	return CreateFieldResult((*get_inst_res.instance)(), TSIPErrorCodes::e_ok);
}

VS_RTSPObjectFactory * VS_RTSPObjectFactory::Instance()
{
	if (!iThis)
	{
		iThis = new VS_RTSPObjectFactory;
		const TSIPErrorCodes res = iThis->Init();

		if (res != TSIPErrorCodes::e_ok)
		{
			iThis->SetError(res);
			iThis->SetValid(false);
			return iThis;
		}
	}
	iThis->SetValid(true);
	return iThis;
}

TSIPErrorCodes VS_RTSPObjectFactory::Init()
{
	TSIPErrorCodes res = AddClass("CSEQ", &VS_RTSP_CSeq_Instance);
	if (TSIPErrorCodes::e_ok != res)
		return res;
	res = AddClass("TRANSPORT", &VS_RTSP_Transport_Instance);
	if (TSIPErrorCodes::e_ok != res)
		return res;
	res = AddClass("SESSION", &VS_RTSP_Session_Instance);
	if (TSIPErrorCodes::e_ok != res)
		return res;
	res = AddClass("SERVER", &VS_RTSP_Server_Instance);
	if (TSIPErrorCodes::e_ok != res)
		return res;
	res = AddClass("USER-AGENT", &VS_RTSP_UserAgent_Instanse);
	if (TSIPErrorCodes::e_ok != res)
		return res;
	res = AddClass("ACCEPT", &VS_RTSP_Accept_Instanse);
	if (TSIPErrorCodes::e_ok != res)
		return res;
	res = AddClass("PUBLIC", &VS_RTSP_Public_Instance);
	if (TSIPErrorCodes::e_ok != res)
		return res;
	res = AddClass("STARTLINE", &VS_RTSP_StartLine_Instance);
	if (TSIPErrorCodes::e_ok != res)
		return res;
	res = AddClass("CONTENT-TYPE", &VS_RTSP_Content_type_Instance);
	if (TSIPErrorCodes::e_ok != res)
		return res;
	res = AddClass("CONTENT-LENGTH", &VS_RTSP_ContentLength_Instance);
	if (TSIPErrorCodes::e_ok != res)
		return res;
	res = AddClass("DATE", &VS_RTSP_Date_Instance);
	if (TSIPErrorCodes::e_ok != res)
		return res;
	res = AddClass("WWW-AUTHENTICATE", &VS_SIPField_Auth_WWWAuthenticate_Instance);
	if (TSIPErrorCodes::e_ok != res)
		return res;
	res = AddClass("AUTHORIZATION", &VS_SIPField_Auth_Authorization_Instance);
	if (TSIPErrorCodes::e_ok != res)
		return res;

	return TSIPErrorCodes::e_null;
}

VS_ObjectFactory::CreateFieldResult VS_RTSPObjectFactory::CreateField(const VS_RTSPObjectFactory::RTSPHeader header) const
{
 	const char * cHeader = nullptr;
	switch (header)
	{
	case RTSPHeader::CSEQ:				cHeader = "CSEQ";			break;
	case RTSPHeader::ACCEPT:			cHeader = "ACCEPT";			break;
	case RTSPHeader::CONTENT_LENGTH:	cHeader = "CONTENT-LENGTH"; break;
	case RTSPHeader::CONTENT_TYPE:		cHeader = "CONTENT-TYPE";	break;
	case RTSPHeader::PUBLIC:			cHeader = "PUBLIC";			break;
	case RTSPHeader::STARTLINE:			cHeader = "STARTLINE";		break;
	case RTSPHeader::SESSION:			cHeader = "SESSION";		break;
	case RTSPHeader::USER_AGENT:		cHeader = "USER-AGENT";		break;
	case RTSPHeader::TRANSPORT:			cHeader = "TRANSPORT";		break;
	case RTSPHeader::DATE:				cHeader = "DATE";			break;
	case RTSPHeader::SERVER:			cHeader = "SERVER";			break;
	case RTSPHeader::AUTHORIZATION:		cHeader = "AUTHORIZATION";	break;
	case RTSPHeader::WWW_AUTHENTICATE:	cHeader = "WWW_AUTHENTICATE"; break;
	}
	if (cHeader == nullptr)
	{
		return CreateFieldResult(nullptr, TSIPErrorCodes::e_InputParam);
	}

	const VS_ObjectFactory::GetInstanceResult get_inst_res = GetInstance(cHeader);
	if (get_inst_res.error_code != TSIPErrorCodes::e_ok)
		return CreateFieldResult(nullptr, get_inst_res.error_code);

	return CreateFieldResult((*get_inst_res.instance)(), TSIPErrorCodes::e_ok);
}

eRequestType VS_RTSPObjectFactory::GetMethod(const std::string& method) const
{
	const auto it = std::lower_bound(std::begin(methods_tbl), std::end(methods_tbl), method, less_first());
	if (it == std::end(methods_tbl) || it->first != method)
		return REQUEST_invalid;
	return it->second;
}

const std::string& VS_RTSPObjectFactory::GetMethod(eRequestType method) const
{
	static const std::string empty_str;
	const auto it = std::lower_bound(std::begin(methods_tbl), std::end(methods_tbl), method, less_second());
	if (it == std::end(methods_tbl) || it->second != method)
		return empty_str;
	return it->first;
}

#undef DEBUG_CURRENT_MODULE
