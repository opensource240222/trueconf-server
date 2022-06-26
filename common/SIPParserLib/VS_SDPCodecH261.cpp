#include "VS_SDPCodecH261.h"
#include "../tools/Server/CommonTypes.h"
#include "std/debuglog/VS_Debug.h"
#include "std-generic/compat/memory.h"

#define DEBUG_CURRENT_MODULE VS_DM_SIPPARSER

const boost::regex VS_SDPCodecH261::e1(R"(.*(?i)(?:\W+(?:cif)\W*) *= *(\d+)(?-i).*)");
const boost::regex VS_SDPCodecH261::e2(R"(.*(?i)(?:\W+(?:qcif)\W*) *= *(\d+)(?-i).*)");
const boost::regex VS_SDPCodecH261::e3(".*(?i)(?:D) *= *(\\d+)(?-i).*");

VS_SDPCodecH261::VS_SDPCodecH261(const VS_SDPCodecH261& src) : VS_SDPCodec(src), m_CIF(src.m_CIF), m_QCIF(src.m_QCIF), m_D(src.m_D)
{
}
VS_SDPCodecH261::VS_SDPCodecH261(): m_CIF(-1), m_QCIF(-1), m_D(-1)
{

}

VS_SDPCodecH261::~VS_SDPCodecH261()
{

}

TSIPErrorCodes VS_SDPCodecH261::Decode(VS_SIPBuffer &_buffer)
{
	std::unique_ptr<char[]> data;
	const TSIPErrorCodes err = _buffer.GetAllDataAllocConst(data);
	if (TSIPErrorCodes::e_ok != err )
	{
		SetValid(false);
		SetError(err);
		return err;
	}

	FindParam_CIF(data.get());
	FindParam_QCIF(data.get());
	FindParam_D(data.get());

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SDPCodecH261::Encode(VS_SIPBuffer &_buffer) const
{
	if ( !IsValid() )
		return GetLastClassError();

	const TSIPErrorCodes err = VS_SDPCodec::Encode(_buffer);
	if ( err != TSIPErrorCodes::e_ok )
	{
		return err;
	}

	if ( (m_CIF != -1) || (m_QCIF != -1) )
	{
		_buffer.AddData("a=fmtp:");

		// payload type
		char pt[std::numeric_limits<int>::digits10 + 1 /*sign*/ + 1 + 1 /*0-terminator*/] = { 0 };
		snprintf(pt, sizeof pt, "%d", m_pt);

		_buffer.AddData(pt, strlen(pt));

		_buffer.AddData(" ");

		bool isAny = false;
		if (m_CIF != -1)
		{
			if (isAny)
				_buffer.AddData("; ");

			_buffer.AddData("CIF=");
			char cif[std::numeric_limits<int>::digits10 + 1 /*sign*/ + 1 + 1 /*0-terminator*/] = { 0 };
			snprintf(cif, sizeof cif, "%d", m_CIF);
			_buffer.AddData(cif, strlen(cif));
			isAny = true;
		}

		if (m_QCIF != -1)
		{
			if (isAny)
				_buffer.AddData("; ");

			_buffer.AddData("QCIF=");
			char qcif[std::numeric_limits<int>::digits10 + 1 /*sign*/ + 1 + 1 /*0-terminator*/] = { 0 };
			snprintf(qcif, sizeof qcif, "%d", m_QCIF);
			_buffer.AddData(qcif, strlen(qcif));
		}
		_buffer.AddData("\r\n");
	}
	return TSIPErrorCodes::e_ok;
}

bool VS_SDPCodecH261::FindParam_CIF(string_view _in)
{
	boost::cmatch m;
	try
	{
		if (boost::regex_match(_in.cbegin(), _in.cend(), m, e1))
		{
			const std::string &value = m[1];
			m_CIF = atoi(value.c_str());
			return true;
		}
	}
	catch (const std::runtime_error &ex)
	{
		dstream1 << "VS_SDPCodecH261::FindParam_CIF error " << ex.what() << "\n";
	}
	return false;
}

bool VS_SDPCodecH261::FindParam_QCIF(string_view _in)
{
	boost::cmatch m;
	try
	{
		if ( boost::regex_match(_in.cbegin(), _in.cend(), m, e2) )
		{
			const std::string &value = m[1];
			m_QCIF = atoi( value.c_str() );
			return true;
		}
	}
	catch (const std::runtime_error &ex)
	{
		dstream1 << "VS_SDPCodecH261::FindParam_QCIF error " << ex.what() << "\n";
	}
	return false;
}

bool VS_SDPCodecH261::FindParam_D(string_view _in)
{
	boost::cmatch m;
	try
	{
		if (boost::regex_match(_in.cbegin(), _in.cend(), m, e3))
		{
			const std::string &value = m[1];
			m_D = atoi(value.c_str());
			return true;
		}
	}
	catch (const std::runtime_error &ex)
	{
		dstream1 << "VS_SDPCodecH261::FindParam_D error " << ex.what() << "\n";
	}
	return false;
}

void VS_SDPCodecH261::SetCIF(int cif)
{
	m_CIF = cif;
}
int VS_SDPCodecH261::GetCIF() const
{
	return (m_CIF == -1)? 0: m_CIF;
}
void VS_SDPCodecH261::SetQCIF(int qcif)
{
	m_QCIF = qcif;
}
int VS_SDPCodecH261::GetQCIF() const
{
	return (m_QCIF == -1)? 0: m_QCIF;
}
void VS_SDPCodecH261::SetD(int d)
{
	m_D = d;
}
int VS_SDPCodecH261::GetD() const
{
	return m_D == 1;
}

void VS_SDPCodecH261::FillRcvVideoMode(VS_GatewayVideoMode &mode) const
{
	mode.Mode = (GetQCIF() != 0) * 0x02
	          | (GetCIF() != 0) * 0x04;
}

void VS_SDPCodecH261::FillSndVideoMode(VS_GatewayVideoMode &mode) const
{
	mode.Mode = (GetQCIF() != 0) * 0x02
	          | (GetCIF() != 0) * 0x04;
}

VS_SDPCodecH261 & VS_SDPCodecH261::operator=(const VS_SDPCodecH261 & src)
{
	if (this == &src)
	{
		return *this;
	}
	if (*this != src)
	{
		VS_SDPCodec::operator=(src);
		m_CIF = src.m_CIF;
		m_QCIF = src.m_QCIF;
		m_D = src.m_D;

	}
	return *this;

}

bool VS_SDPCodecH261::operator!=(const VS_SDPCodecH261 & src)const
{
	if (VS_SDPCodec::operator!=(src))
	{
		return true;
	}

	return (m_CIF != src.m_CIF) || (m_QCIF != src.m_QCIF) || (m_D != src.m_D);
}

std::unique_ptr<VS_SDPCodec> VS_SDPCodecH261::Clone() const
{
	return vs::make_unique<VS_SDPCodecH261>(*this);
}

#undef DEBUG_CURRENT_MODULE
