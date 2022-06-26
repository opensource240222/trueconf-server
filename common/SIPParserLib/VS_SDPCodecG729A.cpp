#include "VS_SDPCodecG729A.h"
#include "std/debuglog/VS_Debug.h"
#include "std-generic/compat/memory.h"

#define DEBUG_CURRENT_MODULE VS_DM_SIPPARSER

const boost::regex VS_SDPCodecG729A::e(".*(?i)(?:annexb *= *)(yes|no)+(?-i).*");

VS_SDPCodecG729A::VS_SDPCodecG729A(): m_AnnexB(false)
{
	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
}
VS_SDPCodecG729A::VS_SDPCodecG729A(const VS_SDPCodecG729A&src) : VS_SDPCodec(src), m_AnnexB(src.m_AnnexB)
{
}

VS_SDPCodecG729A::~VS_SDPCodecG729A()
{

}

TSIPErrorCodes VS_SDPCodecG729A::Decode(VS_SIPBuffer &_buffer)
{
	std::unique_ptr<char[]> data;
	const TSIPErrorCodes err = _buffer.GetAllDataAllocConst(data);
	if (TSIPErrorCodes::e_ok != err )
	{
		SetValid(false);
		SetError(err);
		return err;
	}

	FindParam_AnnexB(data.get());

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SDPCodecG729A::Encode(VS_SIPBuffer &_buffer) const
{
	if ( !IsValid() )
		return GetLastClassError();

	const TSIPErrorCodes err = VS_SDPCodec::Encode(_buffer);
	if ( err != TSIPErrorCodes::e_ok )
	{
		return err;
	}

	_buffer.AddData("a=fmtp:");

	// payload type
	char pt[std::numeric_limits<int>::digits10 + 1 /*sign*/ + 1 + 1 /*0-terminator*/] = { 0 };
	snprintf(pt, sizeof pt, "%d", m_pt);
	_buffer.AddData(pt, strlen(pt));

	_buffer.AddData(" annexb=");
	if (m_AnnexB)
		_buffer.AddData("yes");
	else
		_buffer.AddData("no");
	_buffer.AddData("\r\n");

	return TSIPErrorCodes::e_ok;
}

bool VS_SDPCodecG729A::FindParam_AnnexB(string_view _in)
{
	boost::cmatch m;
	try
	{
		if (boost::regex_match(_in.cbegin(), _in.cend(), m, e))
		{
			const std::string &value = m[1];

			if (value == "yes")
				m_AnnexB = true;
			else
				m_AnnexB = false;

			return true;
		}
	}
	catch (const std::runtime_error &ex)
	{
		dstream1 << "VS_SDPCodecG729A::FindParam_AnnexB error " << ex.what() << "\n";
	}
	return false;
}

std::unique_ptr<VS_SDPCodec> VS_SDPCodecG729A::Clone() const
{
	return vs::make_unique<VS_SDPCodecG729A>(*this);
}

#undef DEBUG_CURRENT_MODULE
