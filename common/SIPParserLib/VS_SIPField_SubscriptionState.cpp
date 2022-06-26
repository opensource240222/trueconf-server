#include "VS_SIPField_SubscriptionState.h"
#include "VS_SIPObjectFactory.h"
#include "std-generic/compat/memory.h"

VS_SIPField_SubscriptionState::VS_SIPField_SubscriptionState():m_state(VS_SIPField_SubscriptionState::STATE::STATE_INVALID)
{
	VS_SIPField_SubscriptionState::Clean();
}

VS_SIPField_SubscriptionState::~VS_SIPField_SubscriptionState()
{
	VS_SIPField_SubscriptionState::Clean();
}

TSIPErrorCodes VS_SIPField_SubscriptionState::Decode(VS_SIPBuffer &aBuffer)
{
	Clean();

	//std::unique_ptr<char[]> ptr;
	//unsigned int ptr_sz = 0;
	//int err = e_null;

	//err = aBuffer.GetNextBlockAlloc(ptr, ptr_sz);
	//if ( (e_ok != err) || !ptr || !ptr_sz )
	//{
	//	if ( ptr ) { delete ptr; ptr = 0; } ptr_sz = 0;

	//	SetValid(false);
	//	SetError(err);
	//	return err;
	//}

	//boost::regex e(
	//	"(?i)"
	//	" *(?:session-expires|x) *: *"
	//	"(\\d+) *"
	//	"(.*)?"
	//	"(?-i)"
	//);
	//boost::cmatch m;

	//if ( !boost::regex_match(ptr.get(), m, e) )
	//{
	//	dstream3 << "[SIPParserLib::Error] Session-Expires Field: buffer not match, dump |" << ptr.get() << "|";

	//	if ( ptr ) { delete ptr; ptr = 0; } ptr_sz = 0;

	//	SetValid(false);
	//	SetError(e_match);
	//	return e_match;
	//}

	//std::string value = m[1];
	//std::string param = m[2];

	//FindParam_refresher( param.c_str() );

	//if ( ptr ) { delete ptr; ptr = 0; } ptr_sz = 0;

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SIPField_SubscriptionState::Encode(VS_SIPBuffer &aBuffer) const
{
	if ( !IsValid() )
		return GetLastClassError();

	if (m_state != STATE_ACTIVE)
	{
		return TSIPErrorCodes::e_InputParam;
	}

	aBuffer.AddData("Subscription-State: active");

	//char val[11];	memset(val, 0, 11);
	//_itoa(m_value, val, 10);
	//aBuffer.AddData(val, strlen(val));

	//if (m_refresher == REFRESHER_UAC) {
	//	aBuffer.AddData("; refresher=uac");
	//}else if (m_refresher == REFRESHER_UAS) {
	//	aBuffer.AddData("; refresher=uas");
	//}

	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SIPField_SubscriptionState::Init(const VS_SIPGetInfoInterface& call)
{
	m_state = STATE_ACTIVE;

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

void VS_SIPField_SubscriptionState::Clean() noexcept
{
	VS_SIPError::Clean();

//	m_refresher = REFRESHER_INVALID;
//	m_value = 90;
}

std::unique_ptr<VS_BaseField> VS_SIPField_SubscriptionState_Instance()
{
	return vs::make_unique<VS_SIPField_SubscriptionState>();
}

//bool VS_SIPField_SubscriptionState::FindParam_refresher(const char* aInput)
//{
//	if ( !aInput )
//		return false;
//
//	boost::regex e(".*; *(?i)(?:refresher) *= *(uas|uac)+ *.*(?-i)");
//	boost::cmatch m;
//
//	if ( boost::regex_match(aInput, m, e) )
//	{
//		std::string param = m[1];
//
//		// LowerCase
//		std::transform(param.begin(), param.end(), param.begin(), tolower);
//
//		if (param == "uac")
//			m_refresher = REFRESHER_UAC;
//		else if (param == "uas")
//			m_refresher = REFRESHER_UAS;
//		else
//			m_refresher = REFRESHER_INVALID;
//
//		return true;
//	}
//
//	return false;
//}