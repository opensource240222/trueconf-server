#include "VS_SDPCodecMPEG4ES.h"

#include "../MediaParserLib/VS_AACParser.h"
#include "../tools/Server/CommonTypes.h"
#include "std-generic/cpplib/string_view.h"
#include "std/cpplib/boost_range_boost_sub_match.hpp"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/regex.hpp>

#include <cstdlib>
#include <iomanip>
#include "std-generic/compat/memory.h"
#include <sstream>

namespace
{

	inline int hex_to_number(char c)
	{
		if (c >= '0' && c <= '9')
			return c - '0';
		if (c >= 'A' && c <= 'F')
			return c - 'A' + 10;
		if (c >= 'a' && c <= 'f')
			return c - 'a' + 10;
		return -1;
	}

}


static const boost::regex parameter_re(
	"([^= ]+)" // name
	" *= *"
	"([^; ]+)" // value
	";?",
	boost::regex::optimize);

VS_SDPCodecMPEG4ES::VS_SDPCodecMPEG4ES(const char* mode)
	: m_mode(mode)
	, m_size_length(0)
	, m_index_length(0)
	, m_index_delta_length(0)
	, m_constant_size(0)
{
}

VS_SDPCodecMPEG4ES::~VS_SDPCodecMPEG4ES()
{
}

TSIPErrorCodes VS_SDPCodecMPEG4ES::Decode(VS_SIPBuffer &_buffer)
{
	TSIPErrorCodes err = TSIPErrorCodes::e_null;
	std::string data;
	{
		std::unique_ptr<char[]> p;
		err = _buffer.GetAllDataAllocConst(p);
		data = p.get();
	}

	if (TSIPErrorCodes::e_ok != err)
	{
		SetValid(false);
		SetError(err);
		return err;
	}

	for (auto param_it = boost::sregex_iterator(data.begin(), data.end(), parameter_re); param_it != boost::sregex_iterator(); ++param_it)
	{
		if      (boost::iequals((*param_it)[1], string_view("mode")))
			m_mode = param_it->str(2);
		else if (boost::iequals((*param_it)[1], string_view("sizeLength")))
			m_size_length = atoi(param_it->str(2).c_str());
		else if (boost::iequals((*param_it)[1], string_view("indexLength")))
			m_index_length = atoi(param_it->str(2).c_str());
		else if (boost::iequals((*param_it)[1], string_view("indexDeltaLength")))
			m_index_delta_length = atoi(param_it->str(2).c_str());
		else if (boost::iequals((*param_it)[1], string_view("constantSize")))
			m_constant_size = atoi(param_it->str(2).c_str());
		else if (boost::iequals((*param_it)[1], string_view("config")) && (*param_it)[2].length() % 2 == 0)
		{
			m_config.clear();
			for (auto it = (*param_it)[2].first; it < (*param_it)[2].second;)
			{
				auto hi = hex_to_number(*it++);
				auto lo = hex_to_number(*it++);
				if (hi == -1 || lo == -1)
					break;
				m_config.push_back(hi * 16 + lo);
			}
		}
	}
	boost::to_lower(m_mode);

	if (m_mode == "aac-lbr" || m_mode == "aac-hbr")
	{
		SetCodecType(e_rcvAAC);
		if (m_config.empty())
		{
			size_t config_size = 5;
			m_config.resize(config_size);
			if (!MakeMPEG4AudioConfig(m_config.data(), config_size, 2/*AAC Low Complexity*/, MPEG4FrequencyIndexFromSamplingRate(GetClockRate()), GetNumChannels(), GetClockRate()))
				m_config.clear();
		}
	}
	else
	{
		SetValid(false);
		SetError(TSIPErrorCodes::e_match);
		return TSIPErrorCodes::e_match;
	}

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SDPCodecMPEG4ES::Encode(VS_SIPBuffer &_buffer) const
{
	if ( !IsValid() )
		return GetLastClassError();

	if (m_mode.empty())
		return TSIPErrorCodes::e_InputParam;

	const TSIPErrorCodes err = VS_SDPCodec::Encode(_buffer);
	if ( err != TSIPErrorCodes::e_ok )
	{
		return err;
	}

	std::ostringstream s;
	s << "a=fmtp:" << m_pt << " profile-level-id=1;mode=" << m_mode;
	if (m_size_length > 0)
		s << ";sizeLength=" << m_size_length;
	if (m_index_length > 0)
		s << ";indexLength=" << m_index_length;
	if (m_index_delta_length > 0)
		s << ";indexDeltaLength=" << m_index_delta_length;
	if (m_constant_size > 0)
		s << ";constantSize=" << m_constant_size;
	if (!m_config.empty())
	{
		s << ";config=" << std::hex << std::setfill('0');
		for (auto x: m_config)
			s << std::setw(2) << static_cast<unsigned>(x);
		s << std::dec;
	}


	_buffer.AddData(s.str());
	return TSIPErrorCodes::e_ok;
}

void VS_SDPCodecMPEG4ES::FillRcvAudioMode(VS_GatewayAudioMode &mode) const
{
	mode.mpeg4es.size_length = m_size_length;
	mode.mpeg4es.index_length = m_index_length;
	mode.mpeg4es.index_delta_length = m_index_delta_length;
	mode.mpeg4es.constant_size = m_constant_size;
	mode.mpeg4es.config = m_config;
}

std::unique_ptr<VS_SDPCodec> VS_SDPCodecMPEG4ES::Clone() const
{
	return vs::make_unique<VS_SDPCodecMPEG4ES>(*this);
}