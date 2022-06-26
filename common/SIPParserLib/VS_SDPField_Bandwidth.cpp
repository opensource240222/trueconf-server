#include "VS_SDPField_Bandwidth.h"
#include "VS_SDPObjectFactory.h"
#include "std/debuglog/VS_Debug.h"
#include "VS_SIPGetInfoInterface.h"
#include <string>
#include "std-generic/compat/memory.h"

#define DEBUG_CURRENT_MODULE VS_DM_SIPPARSER

const boost::regex VS_SDPField_Bandwidth::e("(?i) *b *= *(\\w+) *: *(\\d+) *(?-i)");

VS_SDPField_Bandwidth::VS_SDPField_Bandwidth()
{
	VS_SDPField_Bandwidth::Clean();
}

VS_SDPField_Bandwidth::~VS_SDPField_Bandwidth()
{
}

TSIPErrorCodes VS_SDPField_Bandwidth::Decode(VS_SIPBuffer &_buffer)
{
	Clean();

	std::unique_ptr<char[]> ptr;
	std::size_t ptr_sz = 0;

	const TSIPErrorCodes err = _buffer.GetNextBlockAllocConst(ptr, ptr_sz);
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
		dstream1 << "VS_SDPField_Bandwidth::Decode error " << ex.what() << "\n";
		regex_match_res = false;
	}
	if ( !regex_match_res)
	{
		dstream3 << "[SIPParserLib::SDPError] Bandwidth Field: buffer not match, dump |" << ptr.get() << "|";
		_buffer.SkipHeader();
		SetValid(false);
		SetError(TSIPErrorCodes::e_match);
		return TSIPErrorCodes::e_match;
	}

	std::string type = m[1];
	const std::string &bandwidth = m[2];

	if ( type.empty() || bandwidth.empty() )
	{
		SetValid(false);
		SetError(TSIPErrorCodes::e_InputParam);
		return TSIPErrorCodes::e_InputParam;
	}

	// UpperCase
	std::transform(type.begin(), type.end(), type.begin(), toupper);

	m_bandwidth = atoi( bandwidth.c_str() );

	m_type = VS_SDPObjectFactory::GetBandwidthType( type.c_str() );

	if ( (m_type == SDP_BANDWIDTH_CT) || (m_type == SDP_BANDWIDTH_AS) )
		m_bandwidth = m_bandwidth * 1024;

	_buffer.SkipHeader();

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SDPField_Bandwidth::Encode(VS_SIPBuffer &_buffer) const
{
	if ( !IsValid() )
		return GetLastClassError();

	const auto res_type = VS_SDPObjectFactory::GetBandwidthType( m_type );
	if ( !res_type )
		return TSIPErrorCodes::e_bad;

	std::string str = "b=";

	str += res_type;
	str += ":";

	unsigned int bw = m_bandwidth;

	if ( (m_type == SDP_BANDWIDTH_CT) || (m_type == SDP_BANDWIDTH_AS) )
		bw = bw / 1024;

	char bandwidth[std::numeric_limits<unsigned int>::digits10 + 1 + 1 /*0-terminator*/] = { 0 };
	snprintf(bandwidth, sizeof bandwidth, "%u", bw);

	str += bandwidth;
	str += "\r\n";

	return _buffer.AddData(str.c_str(), str.length());
}

TSIPErrorCodes VS_SDPField_Bandwidth::Init(const VS_SIPGetInfoInterface& call)
{
	m_type = SDP_BANDWIDTH_AS;

	const int bw = call.GetLocalBandwidth();
	m_bandwidth = (bw)? bw: 768*1024;

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

std::unique_ptr<VS_BaseField> VS_SDPField_Bandwidth_Instance()
{
	return vs::make_unique<VS_SDPField_Bandwidth>();
}

void VS_SDPField_Bandwidth::SetBandwidth(const unsigned int _bandwidth)
{
	m_bandwidth = _bandwidth;
}

unsigned int VS_SDPField_Bandwidth::GetBandwidth() const
{
	return m_bandwidth;
}

void VS_SDPField_Bandwidth::SetType(const eSDP_Bandwidth _type)
{
	m_type = _type;
}

eSDP_Bandwidth VS_SDPField_Bandwidth::GetType() const
{
	return m_type;
}

void VS_SDPField_Bandwidth::Clean() noexcept
{
	VS_SIPError::Clean();

	m_type = SDP_BANDWIDTH_INVALID;
	m_bandwidth = 0;
}

#undef DEBUG_CURRENT_MODULE
