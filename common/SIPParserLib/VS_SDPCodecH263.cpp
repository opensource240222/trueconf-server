#include "VS_SDPCodecH263.h"
#include "../tools/Server/CommonTypes.h"
#include "std/debuglog/VS_Debug.h"
#include "std-generic/compat/memory.h"

#define DEBUG_CURRENT_MODULE VS_DM_SIPPARSER

const boost::regex VS_SDPCodecH263::e1(R"(.*(?i)(?:\W+(?:sqcif)\W*) *= *(\d+)(?-i).*)");
const boost::regex VS_SDPCodecH263::e2(R"(.*(?i)(?:\W+(?:cif)\W*) *= *(\d+)(?-i).*)");
const boost::regex VS_SDPCodecH263::e3(R"(.*(?i)(?:\W+(?:cif4)\W*) *= *(\d+)(?-i).*)");
const boost::regex VS_SDPCodecH263::e4(R"(.*(?i)(?:\W+(?:cif16)\W*) *= *(\d+)(?-i).*)");
const boost::regex VS_SDPCodecH263::e5(R"(.*(?i)(?:\W+(?:qcif)\W*) *= *(\d+)(?-i).*)");
const boost::regex VS_SDPCodecH263::e6(".*(?i)(?:\\W+(F)(?: *= *(0|1))?(?: |;|$))(?-i).*");
const boost::regex VS_SDPCodecH263::e7(".*(?i)(?:\\W+(I)(?: *= *(0|1))?(?: |;|$))(?-i).*");
const boost::regex VS_SDPCodecH263::e8(".*(?i)(?:\\W+(J)(?: *= *(0|1))?(?: |;|$))(?-i).*");
const boost::regex VS_SDPCodecH263::e9(".*(?i)(?:\\W+(T)(?: *= *(0|1))?(?: |;|$))(?-i).*");

VS_SDPCodecH263::VS_SDPCodecH263():
m_SQCIF(-1), m_QCIF(-1), m_CIF(1), m_CIF4(1), m_CIF16(-1),
m_AnnexF(-1), m_AnnexI(-1), m_AnnexJ(-1), m_AnnexT(-1)
{

}

VS_SDPCodecH263::VS_SDPCodecH263(const VS_SDPCodecH263& src)
	:VS_SDPCodec(src)
	,m_SQCIF (src.m_SQCIF )
	,m_QCIF  (src.m_QCIF  )
	,m_CIF   (src.m_CIF   )
	,m_CIF4  (src.m_CIF4  )
	,m_CIF16 (src.m_CIF16 )
	,m_AnnexF(src.m_AnnexF)
	,m_AnnexI(src.m_AnnexI)
	,m_AnnexJ(src.m_AnnexJ)
	,m_AnnexT(src.m_AnnexT)
{}


TSIPErrorCodes VS_SDPCodecH263::Decode(VS_SIPBuffer &_buffer)
{
	std::unique_ptr<char[]> data;
	const TSIPErrorCodes err = _buffer.GetAllDataAllocConst(data);
	if (TSIPErrorCodes::e_ok != err )
	{
		SetValid(false);
		SetError(err);
		return err;
	}
	FindParam_SQCIF(data.get());
	FindParam_QCIF(data.get());
	FindParam_CIF(data.get());
	FindParam_CIF4(data.get());

	FindParam_AnnexF(data.get());
	FindParam_AnnexI(data.get());
	FindParam_AnnexJ(data.get());
	FindParam_AnnexT(data.get());

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}
TSIPErrorCodes VS_SDPCodecH263::Encode(VS_SIPBuffer &_buffer) const
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

	_buffer.AddData(" ");

	bool isAny = false;


	// Temporaly off (for comments ask ksmirnov) 19.09
	//if (m_CIF16 != -1)
	//{
	//	_buffer.AddData(";CIF16=");
	//	char cif16[11];		memset(cif16, 0, 11);
	//	_itoa(m_CIF16, cif16, 10);
	//	_buffer.AddData(cif16, (unsigned int) strlen(cif16));
	//}

	if (m_CIF4 != -1)
	{
		if (isAny)
			_buffer.AddData(";");

		_buffer.AddData("CIF4=");
		char cif4[std::numeric_limits<int>::digits10 + 1 /*sign*/ + 1 + 1 /*0-terminator*/] = { 0 };
		snprintf(cif4, sizeof cif4, "%d", m_CIF4);
		_buffer.AddData(cif4, strlen(cif4));
		isAny = true;
	}

	if (m_CIF != -1)
	{
		if (isAny)
			_buffer.AddData(";");

		_buffer.AddData("CIF=");
		char cif[std::numeric_limits<int>::digits10 + 1 /*sign*/ + 1 + 1 /*0-terminator*/] = { 0 };
		snprintf(cif, sizeof cif, "%d", m_CIF);
		_buffer.AddData(cif, strlen(cif));
		isAny = true;
	}

	if (m_QCIF != -1)
	{
		if (isAny)
			_buffer.AddData(";");

		_buffer.AddData("QCIF=");
		char qcif[std::numeric_limits<int>::digits10 + 1 /*sign*/ + 1 + 1 /*0-terminator*/] = { 0 };
		snprintf(qcif, sizeof qcif, "%d", m_QCIF);
		_buffer.AddData(qcif, strlen(qcif));
		isAny = true;
	}

	if (m_SQCIF != -1)
	{
		if (isAny)
			_buffer.AddData(";");

		_buffer.AddData("SQCIF=");
		char sqcif[std::numeric_limits<int>::digits10 + 1 /*sign*/ + 1 + 1 /*0-terminator*/] = { 0 };
		snprintf(sqcif, sizeof sqcif, "%d", m_SQCIF);
		_buffer.AddData(sqcif, strlen(sqcif));
		isAny = true;
	}

	if (m_AnnexF != -1)
	{
		if (isAny)
			_buffer.AddData(";");

		_buffer.AddData("F");
		isAny = true;
	}

	if (m_AnnexI != -1)
	{
		if (isAny)
			_buffer.AddData(";");

		_buffer.AddData("I");
		isAny = true;
	}

	if (m_AnnexJ != -1)
	{
		if (isAny)
			_buffer.AddData(";");

		_buffer.AddData("J");
		isAny = true;
	}

	if (m_AnnexT != -1)
	{
		if (isAny)
			_buffer.AddData(";");

		_buffer.AddData("T");
	}

	_buffer.AddData("\r\n");

	return TSIPErrorCodes::e_ok;
}

bool VS_SDPCodecH263::FindParam_SQCIF(string_view _in)
{
	boost::cmatch m;
	try
	{
		if (boost::regex_match(_in.cbegin(), _in.cend(), m, e1))
		{
			const std::string &value = m[1];
			m_SQCIF = atoi(value.c_str());
			return true;
		}
	}
	catch (const std::runtime_error &ex)
	{
		dstream1 << "VS_SDPCodecH263::FindParam_SQCIF error " << ex.what() << "\n";
	}
	return false;
}

bool VS_SDPCodecH263::FindParam_CIF(string_view _in)
{
	boost::cmatch m;
	try
	{
		if (boost::regex_match(_in.cbegin(), _in.cend(), m, e2))
		{
			const std::string &value = m[1];
			m_CIF = atoi(value.c_str());
			return true;
		}
	}
	catch (const std::runtime_error &ex)
	{
		dstream1 << "VS_SDPCodecH263::FindParam_CIF error " << ex.what() << "\n";
	}
	return false;
}

bool VS_SDPCodecH263::FindParam_CIF4(string_view _in)
{
	boost::cmatch m;
	try
	{
		if (boost::regex_match(_in.cbegin(), _in.cend(), m, e3))
		{
			const std::string &value = m[1];
			m_CIF4 = atoi(value.c_str());
			return true;
		}
	}
	catch (const std::runtime_error &ex)
	{
		dstream1 << "VS_SDPCodecH263::FindParam_CIF4 error " << ex.what() << "\n";
	}

	return false;
}

bool VS_SDPCodecH263::FindParam_QCIF(string_view _in)
{
	boost::cmatch m;
	try
	{
		if (boost::regex_match(_in.cbegin(), _in.cend(), m, e5))
		{
			const std::string &value = m[1];
			m_QCIF = atoi(value.c_str());
			return true;
		}
	}
	catch (const std::runtime_error &ex)
	{
		dstream1 << "VS_SDPCodecH263::FindParam_QCIF error " << ex.what() << "\n";
	}
	return false;
}

bool VS_SDPCodecH263::FindParam_AnnexF(string_view _in)
{
	boost::cmatch m;
	try
	{
		if (boost::regex_match(_in.cbegin(), _in.cend(), m, e6))
		{
			const std::string &value = m[2];
			if (value.length() > 0)
				m_AnnexF = atoi(value.c_str());
			else
				m_AnnexF = 1;

			return true;
		}
	}
	catch (const std::runtime_error &ex)
	{
		dstream1 << "VS_SDPCodecH263::FindParam_AnnexF error " << ex.what() << "\n";
	}
	return false;
}

bool VS_SDPCodecH263::FindParam_AnnexI(string_view _in)
{
	boost::cmatch m;
	try
	{
		if (boost::regex_match(_in.cbegin(), _in.cend(), m, e7))
		{
			const std::string &value = m[2];
			if (value.length() > 0)
				m_AnnexI = atoi(value.c_str());
			else
				m_AnnexI = 1;

			return true;
		}
	}
	catch (const std::runtime_error &ex)
	{
		dstream1 << "VS_SDPCodecH263::FindParam_AnnexI error " << ex.what() << "\n";
	}
	return false;
}

bool VS_SDPCodecH263::FindParam_AnnexJ(string_view _in)
{
	boost::cmatch m;
	try
	{
		if (boost::regex_match(_in.cbegin(), _in.cend(), m, e8))
		{
			const std::string &value = m[2];
			if (value.length() > 0)
				m_AnnexJ = atoi(value.c_str());
			else
				m_AnnexJ = 1;

			return true;
		}
	}
	catch (const std::runtime_error &ex)
	{
		dstream1 << "VS_SDPCodecH263::FindParam_AnnexJ error " << ex.what() << "\n";
	}
	return false;
}

bool VS_SDPCodecH263::FindParam_AnnexT(string_view _in)
{
	boost::cmatch m;
	try
	{
		if (boost::regex_match(_in.cbegin(), _in.cend(), m, e9))
		{
			const std::string &value = m[2];
			if (value.length() > 0)
				m_AnnexT = atoi(value.c_str());
			else
				m_AnnexT = 1;

			return true;
		}
	}
	catch (const std::runtime_error &ex)
	{
		dstream1 << "VS_SDPCodecH263::FindParam_AnnexT error " << ex.what() << "\n";
	}
	return false;
}

void VS_SDPCodecH263::SetSQCIF(int sqcif)
{
	m_SQCIF = sqcif;
}
int VS_SDPCodecH263::GetSQCIF() const
{
	return (m_SQCIF == -1)? 0: m_SQCIF;
}
void VS_SDPCodecH263::SetCIF(int cif)
{
	m_CIF = cif;
}
int VS_SDPCodecH263::GetCIF() const
{
	return (m_CIF == -1)? 0: m_CIF;
}
void VS_SDPCodecH263::SetCIF4(int cif4)
{
	m_CIF4 = cif4;
}
int VS_SDPCodecH263::GetCIF4() const
{
	return (m_CIF4 == -1)? 0: m_CIF4;
}
void VS_SDPCodecH263::SetCIF16(int cif16)
{
	m_CIF16 = cif16;
}
int VS_SDPCodecH263::GetCIF16() const
{
	return (m_CIF16 == -1)? 0: m_CIF16;
}
void VS_SDPCodecH263::SetQCIF(int qcif)
{
	m_QCIF = qcif;
}
int VS_SDPCodecH263::GetQCIF() const
{
	return (m_QCIF == -1)? 0: m_QCIF;
}
void VS_SDPCodecH263::SetAnnexF(bool is_true)
{
	m_AnnexF = is_true;
}
bool VS_SDPCodecH263::GetAnnexF() const
{
	return (m_AnnexF == 1);
}
void VS_SDPCodecH263::SetAnnexI(bool is_true)
{
	m_AnnexI = is_true;
}
bool VS_SDPCodecH263::GetAnnexI() const
{
	return (m_AnnexI == 1);
}
void VS_SDPCodecH263::SetAnnexJ(bool is_true)
{
	m_AnnexJ = is_true;
}
bool VS_SDPCodecH263::GetAnnexJ() const
{
	return (m_AnnexJ == 1);
}
void VS_SDPCodecH263::SetAnnexT(bool is_true)
{
	m_AnnexT = is_true;
}
bool VS_SDPCodecH263::GetAnnexT() const
{
	return (m_AnnexT == 1);
}

void VS_SDPCodecH263::FillRcvVideoMode(VS_GatewayVideoMode &mode) const
{
	mode.Mode = (GetSQCIF() != 0) * 0x01
	          | (GetQCIF() != 0) * 0x02
	          | (GetCIF() != 0) * 0x04
	          | (GetCIF4() != 0) * 0x08
	          /*| (GetCIF16() != 0) * 0x10*/; // <-- ksmirnov idea - ask him!

	if ( GetAnnexF() )
		mode.CodecAnexInfo |= VS_GatewayVideoMode::e_anexF;

	if ( GetAnnexI() )
		mode.CodecAnexInfo |= VS_GatewayVideoMode::e_anexI;

	if ( GetAnnexJ() )
		mode.CodecAnexInfo |= VS_GatewayVideoMode::e_anexJ;

	if ( GetAnnexT() )
		mode.CodecAnexInfo |= VS_GatewayVideoMode::e_anexT;
}

void VS_SDPCodecH263::FillSndVideoMode(VS_GatewayVideoMode &mode) const
{
	mode.Mode = (GetSQCIF() != 0) * 0x01
	          | (GetQCIF() != 0) * 0x02
	          | (GetCIF() != 0) * 0x04
	          | (GetCIF4() != 0) * 0x08
	          | (GetCIF16() != 0) * 0x10;

	if (!mode.Mode) // if no dimensions
		mode.Mode |= 0x04; // set CIF

	if ( GetAnnexF() )
		mode.CodecAnexInfo |= VS_GatewayVideoMode::e_anexF;

	if ( GetAnnexI() )
		mode.CodecAnexInfo |= VS_GatewayVideoMode::e_anexI;

	if ( GetAnnexJ() )
		mode.CodecAnexInfo |= VS_GatewayVideoMode::e_anexJ;

	if ( GetAnnexT() )
		mode.CodecAnexInfo |= VS_GatewayVideoMode::e_anexT;
}

std::unique_ptr<VS_SDPCodec> VS_SDPCodecH263::Clone() const
{
	return vs::make_unique<VS_SDPCodecH263>(*this);
}

#undef DEBUG_CURRENT_MODULE
