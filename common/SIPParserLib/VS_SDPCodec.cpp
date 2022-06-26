#include "VS_SDPCodec.h"
#include "VS_SDPObjectFactory.h"
#include "../tools/Server/CommonTypes.h"
#include "std/debuglog/VS_Debug.h"
#include "std-generic/compat/memory.h"
#include <sstream>

#define DEBUG_CURRENT_MODULE VS_DM_SIPPARSER

const boost::regex VS_SDPCodec::e(
		" *((?:\\w|\\d|-)+) */ *"		// encoding name
		"(\\d+) *"						// clock rate
		"(?:/(\\d+))? *"				// may be number of channels
	);

const boost::regex VS_SDPCodec::e2(".*(?i)(?:bitrate *= *)([0-9]{1,6})+(?-i).*");

VS_SDPCodec::VS_SDPCodec(): m_pt(-1), m_codec_type(-1), m_media_type(SDPMediaType::invalid), m_clock_rate(0), m_num_channels(0)
{

}

VS_SDPCodec::~VS_SDPCodec()
{

}
VS_SDPCodec::VS_SDPCodec(const VS_SDPCodec& src) :
		VS_BaseField(src),
		m_pt(src.GetPT()),
		m_codec_type(src.m_codec_type),
		m_media_type(src.m_media_type),
		m_clock_rate(src.m_clock_rate),
		m_num_channels(src.m_num_channels),
		m_encoding_name(src.m_encoding_name)
{
}


TSIPErrorCodes VS_SDPCodec::Decode(VS_SIPBuffer& _buffer)
{
	std::unique_ptr<char[]> ptr;
	std::size_t ptr_sz = 0;

	const TSIPErrorCodes err = _buffer.GetNextBlockAlloc(ptr, ptr_sz);
	if ( (TSIPErrorCodes::e_ok != err) || !ptr_sz || !ptr )
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
		dstream1 << "VS_SDPCodec::Decode error " << ex.what() << "\n";
		regex_match_res = false;
	}
	if (!regex_match_res)
	{
		SetValid(false);
		SetError(TSIPErrorCodes::e_match);
		return TSIPErrorCodes::e_match;
	}

	std::string name = m[1];
	const std::string &clock_rate = m[2];
	const std::string &num_channels = m[3];

	// UpperCase
	std::transform(name.begin(), name.end(), name.begin(), toupper);

	m_clock_rate = atoi( clock_rate.c_str() );
	m_num_channels = num_channels.empty() ? 1 : atoi(num_channels.c_str());
	const int codec = vs::GetCodecByEncodingName( name, m_clock_rate);
	if (codec == -1)
	{
		SetValid(false);
		SetError(TSIPErrorCodes::e_UnknownPT);
		return TSIPErrorCodes::e_UnknownPT;
	}

	m_codec_type = codec;
	m_encoding_name = std::move(name);

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SDPCodec::Encode(VS_SIPBuffer &_buffer) const
{
	if ( !IsValid() )
		return GetLastClassError();

	if ( m_encoding_name.length() < 1 )
		return TSIPErrorCodes::e_InputParam;

	_buffer.AddData("a=rtpmap:");

// payload type
	char pt[std::numeric_limits<int>::digits10 + 1 /*sign*/ + 1 + 1 /*0-terminator*/] = { 0 };
	snprintf(pt, sizeof pt, "%d", m_pt);

	_buffer.AddData(pt, strlen(pt));

// encoding name
	_buffer.AddData(" ");
	_buffer.AddData(m_encoding_name);
	_buffer.AddData("/");

// clock rate
	char clock_rate[std::numeric_limits<unsigned int>::digits10 + 1 + 1 /*0-terminator*/] = { 0 };
	snprintf(clock_rate, sizeof clock_rate, "%u", m_clock_rate);
	_buffer.AddData(clock_rate, strlen(clock_rate));

	_buffer.AddData("\r\n");

	return TSIPErrorCodes::e_ok;
}

VS_SDPCodec& VS_SDPCodec::operator=(const VS_SDPCodec& codec)
{
	if (this == &codec)
	{
		return *this;
	}

	if (*this != codec)
	{
		VS_BaseField::operator=(codec);
		m_pt = codec.GetPT();
		m_codec_type = codec.GetCodecType();
	m_media_type = codec.GetMediaType();
		m_clock_rate = codec.GetClockRate();
		m_num_channels = codec.GetNumChannels();
		m_encoding_name = codec.GetEncodingName();
	}
	return *this;
}

bool VS_SDPCodec::operator!=(const VS_SDPCodec& codec) const
{

	if (VS_BaseField::operator!=(codec))
	{
		return true;
	}

	if (  m_pt != codec.GetPT()
	    ||m_codec_type != codec.GetCodecType()
	    ||m_clock_rate != codec.GetClockRate()
	    ||m_num_channels != codec.GetNumChannels()
	    ||m_encoding_name != codec.GetEncodingName() )
	{
		return true;
	}
	return false;
}

// **************** Ìועמהû Get/Set() ****************
void VS_SDPCodec::SetPT(int _pt)
{
	m_pt = _pt;
}
int VS_SDPCodec::GetPT() const
{
	return m_pt;
}
void VS_SDPCodec::SetClockRate(unsigned int _rate)
{
	m_clock_rate = _rate;
}
unsigned int VS_SDPCodec::GetClockRate() const
{
	return m_clock_rate;
}
void VS_SDPCodec::SetNumChannels(unsigned int _n)
{
	m_num_channels = _n;
}
unsigned int VS_SDPCodec::GetNumChannels() const
{
	return m_num_channels;
}
void VS_SDPCodec::SetCodecType(int _c)
{
	m_codec_type = _c;
}
int VS_SDPCodec::GetCodecType() const
{
	return m_codec_type;
}
void VS_SDPCodec::SetEncodingName(std::string _name)
{
	m_encoding_name = std::move(_name);
}
const std::string &VS_SDPCodec::GetEncodingName() const
{
	return m_encoding_name;
}

SDPMediaType VS_SDPCodec::GetMediaType() const
{
	return m_media_type;
}

void VS_SDPCodec::SetMediaType( SDPMediaType mt)
{
	m_media_type = mt;
}

bool VS_SDPCodec::FindParam_bitrate(string_view _in, unsigned long &bitrate) const
{
	boost::cmatch m;
	try
	{
		if ( boost::regex_match(_in.cbegin(), _in.cend(), m, e2) )
		{
			const std::string &value = m[1];
			if (value.length())
			{
				std::istringstream iss(value);
				iss >> bitrate;
			}
			return true;
		}
	}
	catch (const std::runtime_error &ex)
	{
		dstream1 << "VS_SDPCodec::FindParam_bitrate() error " << ex.what() << "\n";
	}
	return false;
}

TSIPErrorCodes VS_SDPRtpmapDecoder::Decode(VS_SIPBuffer &_buffer)
{
	return VS_SDPCodec::Decode(_buffer);
}

std::unique_ptr<VS_SDPCodec> VS_SDPRtpmapDecoder::Clone() const
{
	return vs::make_unique<VS_SDPRtpmapDecoder>(*this);
}

#undef DEBUG_CURRENT_MODULE
