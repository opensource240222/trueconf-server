#include "VS_SIPField_SessionExpires.h"
#include "VS_SIPObjectFactory.h"
#include "VS_TimerExtention.h"
#include "std/debuglog/VS_Debug.h"
#include <string>
#include "std-generic/compat/memory.h"
#include "std/debuglog/VS_Debug.h"
#include "VS_SIPGetInfoInterface.h"

#define DEBUG_CURRENT_MODULE VS_DM_SIPPARSER

const boost::regex VS_SIPField_SessionExpires::e(
		"(?i)"
		" *(?:session-expires|x) *: *"
		"(\\d+) *"
		"(.*)?"
		"(?-i)"
	);

const boost::regex VS_SIPField_SessionExpires::e1(".*; *(?i)(?:refresher) *= *(uas|uac)+ *.*(?-i)");

VS_SIPField_SessionExpires::VS_SIPField_SessionExpires()
{
	VS_SIPField_SessionExpires::Clean();
}

VS_SIPField_SessionExpires::~VS_SIPField_SessionExpires()
{
}

TSIPErrorCodes VS_SIPField_SessionExpires::Decode(VS_SIPBuffer &aBuffer)
{
	Clean();

	std::unique_ptr<char[]> ptr;
	std::size_t ptr_sz = 0;

	const TSIPErrorCodes err = aBuffer.GetNextBlockAlloc(ptr, ptr_sz);
	if ( (TSIPErrorCodes::e_ok != err) || !ptr || !ptr_sz )
	{
		SetValid(false);
		SetError(err);
		return err;
	}

	boost::cmatch m;
	bool regex_match_res;
	try
	{
		regex_match_res = boost::regex_match(ptr.get(), m, e);
	}
	catch (const std::runtime_error &ex)
	{
		dstream1 << "VS_SIPField_SessionExpires::Decode error " << ex.what() << "\n";
		regex_match_res = false;
	}
	if (!regex_match_res)
	{
		dstream3 << "[SIPParserLib::Error] Session-Expires Field: buffer not match, dump |" << ptr.get() << "|";
		SetValid(false);
		SetError(TSIPErrorCodes::e_match);
		return TSIPErrorCodes::e_match;
	}

	const std::string &value = m[1];
	const std::string &param = m[2];

	FindParam_refresher( param );
	m_value = std::chrono::seconds(atoi(value.c_str()));
	if (std::chrono::duration_cast<std::chrono::seconds>(m_value).count() == 0)
		m_value = std::chrono::seconds(90);

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SIPField_SessionExpires::Encode(VS_SIPBuffer &aBuffer) const
{
	if ( !IsValid() )
		return GetLastClassError();

	aBuffer.AddData("Session-Expires: ");

	auto&& val = std::to_string(std::chrono::duration_cast<std::chrono::seconds>(m_value).count());
	aBuffer.AddData(val.c_str(), val.length());


	if (m_refresher == REFRESHER::REFRESHER_UAC) {
		aBuffer.AddData("; refresher=uac");
	}else if (m_refresher == REFRESHER::REFRESHER_UAS) {
		aBuffer.AddData("; refresher=uas");
	}

	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SIPField_SessionExpires::Init(const VS_SIPGetInfoInterface& call)
{
	m_value = call.GetTimerExtention().refreshPeriod;

	if (call.GetTimerExtention().refresher == REFRESHER::REFRESHER_UAS)
		this->m_refresher = REFRESHER::REFRESHER_UAS;
	else this->m_refresher = REFRESHER::REFRESHER_UAC;

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

void VS_SIPField_SessionExpires::Clean() noexcept
{
	VS_SIPError::Clean();

	m_refresher = REFRESHER::REFRESHER_INVALID;
	m_value = std::chrono::seconds(90);
}

std::unique_ptr<VS_BaseField> VS_SIPField_SessionExpires_Instance()
{
	return vs::make_unique<VS_SIPField_SessionExpires>();
}

bool VS_SIPField_SessionExpires::FindParam_refresher(string_view aInput)
{
	boost::cmatch m;
	try {
		if (boost::regex_match(aInput.cbegin(), aInput.cend(), m, e1))
		{
			std::string param = m[1];

			// LowerCase
			std::transform(param.begin(), param.end(), param.begin(), tolower);

			if (param == "uac")
				m_refresher = REFRESHER::REFRESHER_UAC;
			else if (param == "uas")
				m_refresher = REFRESHER::REFRESHER_UAS;
			else
				m_refresher = REFRESHER::REFRESHER_INVALID;

			return true;
		}
	}
	catch (const std::runtime_error &ex)
	{
		dstream1 << "VS_SIPField_SessionExpires::FindParam_refresher error " << ex.what() << "\n";
		return false;
	}
	return false;
}

std::chrono::steady_clock::duration VS_SIPField_SessionExpires::GetRefreshInterval() const
{
	return m_value;
}

REFRESHER VS_SIPField_SessionExpires::GetRefresher() const
{
	return m_refresher;
}


#undef DEBUG_CURRENT_MODULE
