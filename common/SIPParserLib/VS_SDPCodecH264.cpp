#include "VS_SDPCodecH264.h"
#include "VS_SDPObjectFactory.h"
#include "../tools/Server/CommonTypes.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/debuglog/VS_Debug.h"
#include "std-generic/compat/memory.h"
#include <sstream>

#define DEBUG_CURRENT_MODULE VS_DM_SIPPARSER

const boost::regex VS_SDPCodecH264::e("(?i).*profile-level-id *= *([a-zA-Z0-9]{2})([a-zA-Z0-9]{2})([a-zA-Z0-9]{2}).*(?-i)");
const boost::regex VS_SDPCodecH264::e1("(?i).*max-mbps *= *(\\d+).*(?-i)");
const boost::regex VS_SDPCodecH264::e2("(?i).*max-fs *= *(\\d+).*(?-i)");
const boost::regex VS_SDPCodecH264::e3("(?i).*max-br *= *(\\d+).*(?-i)");
const char *VS_SDPCodecH264::profile_level_id_base = "4200";

namespace
{

	int GetMbpsForLevel(int level)
{
	if (level >= 40) return 245760;
	if (level >= 32) return 216000;
	if (level >= 31) return 108000;
	if (level >= 30) return 40500;
	if (level >= 22) return 20250;
	if (level >= 21) return 19800;
	if (level >= 13) return 11880;
	if (level >= 12) return 6000;
	if (level >= 11) return 3000;

	return 1485;
}

	int GetFsForLevel(int level)
{
	if (level >= 40) return 8192;
	if (level >= 32) return 5120;
	if (level >= 31) return 3600;
	if (level >= 22) return 1620;
	if (level >= 21) return 792;
	if (level >= 11) return 396;

	return 99;
}

	int GetBrForLevel(int level)
{
	if (level >= 32) return 20000;
	if (level >= 31) return 14000;
	if (level >= 30) return 10000;
	if (level >= 21) return 4000;
	if (level >= 20) return 2000;
	if (level >= 13) return 768;
	if (level >= 12) return 384;
	if (level >= 11) return 192;

	return 64;
}

}

VS_SDPCodecH264::VS_SDPCodecH264(): m_mbps(0), m_fs(0), m_br(0), m_level(video_presets::max_h264_level)
{
}


VS_SDPCodecH264::VS_SDPCodecH264(const VS_SDPCodecH264& src) : VS_SDPCodec(src)
{
	this->m_level = src.m_level;
	this->m_mbps  = src.m_mbps;
	this->m_fs    = src.m_fs;
	this->m_br    = src.m_br;
}

VS_SDPCodecH264::~VS_SDPCodecH264()
{

}

void VS_SDPCodecH264::GetCommonCodec(const VS_SDPCodecH264 &src1, const VS_SDPCodecH264 &src2, VS_SDPCodecH264 &result)
{
	const VS_SDPCodecH264 &upper = src1.m_level < src2.m_level ? src2 : src1;
	const VS_SDPCodecH264 &lower = src1.m_level < src2.m_level ? src1 : src2;

	result.m_level = lower.m_level;

	const auto mbpsUpper = upper.m_mbps ? upper.m_mbps : GetMbpsForLevel(upper.m_level);
	const auto mbpsLower = lower.m_mbps ? lower.m_mbps : GetMbpsForLevel(lower.m_level);
	const auto mbpsDefault = GetMbpsForLevel(lower.m_level);
	result.m_mbps = std::min(mbpsUpper, mbpsLower);
	if (result.m_mbps == mbpsDefault)
		result.m_mbps = 0;

	const auto fsUpper = upper.m_fs ? upper.m_fs : GetFsForLevel(upper.m_level);
	const auto fsLower = lower.m_fs ? lower.m_fs : GetFsForLevel(lower.m_level);
	const auto fsDefault = GetFsForLevel(lower.m_level);
	result.m_fs = std::min(fsUpper, fsLower);
	if (result.m_fs == fsDefault)
		result.m_fs = 0;

	const auto brUpper = upper.m_br ? upper.m_br : GetBrForLevel(upper.m_level);
	const auto brLower = lower.m_br ? lower.m_br : GetBrForLevel(lower.m_level);
	const auto brDefault = GetBrForLevel(lower.m_level);
	result.m_br = std::min(brUpper, brLower);
	if (result.m_br == brDefault)
		result.m_br = 0;
}

TSIPErrorCodes VS_SDPCodecH264::Decode(VS_SIPBuffer &_buffer)
{
	std::unique_ptr<char[]> data;
	const TSIPErrorCodes err = _buffer.GetAllDataAllocConst(data);
	if (TSIPErrorCodes::e_ok != err )
	{
		SetValid(false);
		SetError(err);
		return err;
	}

	FindParam_profile_level_id(data.get());
	FindParam_mbps(data.get());
	FindParam_fs(data.get());
	FindParam_br(data.get());

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SDPCodecH264::Encode(VS_SIPBuffer &_buffer) const
{
	if ( !IsValid() )
		return GetLastClassError();

	const TSIPErrorCodes err = VS_SDPCodec::Encode(_buffer);
	if ( err != TSIPErrorCodes::e_ok )
	{
		return err;
	}

	char buff[30 +
		std::numeric_limits<int>::digits10 + 1 /*sign*/ + 1 + 1 /*0-terminator*/] = { 0 };
	snprintf(buff, sizeof(buff), "a=fmtp:%d profile-level-id=", this->GetPT());
	std::string profile_level_id = buff;

	std::unique_ptr<char, free_deleter> str;
	VS_RegistryKey key(false, CONFIGURATION_KEY);
	key.GetValue(str, VS_REG_STRING_VT, "H.264 profile-level-id");
	if (str) {
		profile_level_id += str.get();
	} else {
		char buf[(sizeof(int) * 2) + 1] = {0}; //  1 {0000|0000} bytes in binary - FF hex: (count bytes * 2) + NULL-terminator
		snprintf(buf, sizeof(buf), "%02X", m_level);
		profile_level_id += profile_level_id_base;
		profile_level_id += buf;

		if (m_mbps)
		{
			snprintf(buf, sizeof(buf), "%d", m_mbps);
			profile_level_id += ";max-mbps=";
			profile_level_id += buf;
		}
		if (m_fs)
		{
			snprintf(buf, sizeof(buf), "%d", m_fs);
			profile_level_id += ";max-fs=";
			profile_level_id += buf;
		}
		if (m_br)
		{
			snprintf(buf, sizeof(buf), "%d", m_br);
			profile_level_id += ";max-br=";
			profile_level_id += buf;
		}
	}

	profile_level_id += "\r\n";

	_buffer.AddData(profile_level_id);

	return TSIPErrorCodes::e_ok;
}

void VS_SDPCodecH264::FindParam_profile_level_id(string_view in)
{
	boost::cmatch m;
	try
	{
		if (!boost::regex_match(in.cbegin(), in.cend(), m, e))
		{
			dstream3 << "[SIPParserLib::SDPError] VS_SDPCodecH264 Field: buffer not match, dump |" << in << "|";
			SetValid(false);
			SetError(TSIPErrorCodes::e_match);
			return;
		}
	}
	catch (const std::runtime_error &ex)
	{
		dstream1 << "VS_SDPCodecH264::FindParam_profile_level_id error " << ex.what() << "\n";
		SetValid(false);
		SetError(TSIPErrorCodes::e_match);
		return;
	}
	const std::string &str_level = m[3];
	std::istringstream iss(str_level);
	iss >> std::hex >> m_level;
}

void VS_SDPCodecH264::FindParam_mbps(string_view in)
{
	boost::cmatch m;
	try
	{
		if (boost::regex_match(in.cbegin(), in.cend(), m, e1))
		{
			const std::string &mbps(m[1]);
			m_mbps = atoi(mbps.c_str());
		}
		else
			m_mbps = 0;
	}
	catch (const std::runtime_error &ex)
	{
		dstream1 << "VS_SDPCodecH264::FindParam_mbps error " << ex.what() << "\n";
	}
}

void VS_SDPCodecH264::FindParam_fs(string_view in)
{
	boost::cmatch m;
	try
	{
		if (boost::regex_match(in.cbegin(),in.cend(), m, e2))
		{
			const std::string &fs(m[1]);
			m_fs = atoi(fs.c_str());
		}
		else
			m_fs = 0;
	}
	catch (const std::runtime_error &ex)
	{
		dstream1 << "VS_SDPCodecH264::FindParam_fs error " << ex.what() << "\n";
	}
}

void VS_SDPCodecH264::FindParam_br(string_view in)
{
	boost::cmatch m;
	try
	{
		if (boost::regex_match(in.cbegin(), in.cend(), m, e3))
		{
			const std::string &br = m[1];
			m_br = atoi(br.c_str());
		}
		else
			m_br = 0;
	}
	catch (const std::runtime_error &ex)
	{
		dstream1 << "VS_SDPCodecH264::FindParam_br error " << ex.what() << "\n";
	}
}

int VS_SDPCodecH264::Level() const
{
	return m_level;
}

void VS_SDPCodecH264::LimitLevel(int level)
{
	if(level<m_level)
		m_level = level;
}

int VS_SDPCodecH264::MaxMbps() const
{
	return m_mbps;
}

int VS_SDPCodecH264::MaxFs() const
{
	return m_fs;
}
int VS_SDPCodecH264::MaxBr() const
{
	return m_br;
}

void VS_SDPCodecH264::FillRcvVideoMode(VS_GatewayVideoMode &mode) const
{
	mode.Mode = Level();
	mode.MaxMbps = MaxMbps();
	mode.MaxFs = MaxFs();
}

void VS_SDPCodecH264::FillSndVideoMode(VS_GatewayVideoMode &mode) const
{
	mode.Mode = Level();
	mode.MaxFs = MaxFs();
	mode.MaxMbps = MaxMbps();
	if (MaxBr())
		mode.Bitrate = MaxBr() * 1200; // bitrate for NAL HRD. rfc6184
}

std::unique_ptr<VS_SDPCodec> VS_SDPCodecH264::Clone() const
{
	return vs::make_unique<VS_SDPCodecH264>(*this);
}

#undef DEBUG_CURRENT_MODULE
