#include "VS_SDPConnect.h"
#include "VS_SIPGetInfoInterface.h"
#include "net/Address.h"
#include "std/debuglog/VS_Debug.h"
#include <string>

#define DEBUG_CURRENT_MODULE VS_DM_SIPPARSER

const boost::regex VS_SDPConnect::e(
			"(?i) *(?:IN) +"			// IN
			"(?:"
				"(?:"
					"(?:IP4) +"					// IP4
						"([\\w\\d\\.]+)"		// host (ipv4)
				")|(?:"
					"(?:IP6) +"					// IP6
						"([\\w\\d\\.:]+)"		// host (ipv6)
				")"
			")"
					" */? *"
				"(\\d+)?"				// ttl
					" */? *"
				"(\\d+)?"				// addr_num
			"(?-i)"
	);

VS_SDPConnect::VS_SDPConnect():
m_ttl(0), m_addr_num(0)
{

}

VS_SDPConnect::~VS_SDPConnect()
{
}

TSIPErrorCodes VS_SDPConnect::Decode(VS_SIPBuffer &aBuffer)
{
	std::unique_ptr<char[]> ptr;
	std::size_t ptr_sz = 0;

	const TSIPErrorCodes err = aBuffer.GetNextBlockAlloc(ptr, ptr_sz);
	if (TSIPErrorCodes::e_ok != err )
	{
		SetValid(false);
		SetError(err);
		return err;
	}

	if ( !ptr || !ptr_sz )
	{
		SetValid(false);
		SetError(TSIPErrorCodes::e_buffer);
		return TSIPErrorCodes::e_buffer;
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


	// m[1] is ipv4 address, m[2] is ipv6 address
	const std::string &host = m[1].matched ? m[1] : m[2];
	const std::string &ttl = m[3];
	const std::string &addr_num = m[4];

	const bool isipv4 = m[1].matched;

	this->SetHost( host );

	if (ttl.length() != 0)
		m_ttl = atoi( ttl.c_str() );

	if (addr_num.length() != 0)
		m_addr_num = atoi( addr_num.c_str() );

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SDPConnect::Encode(VS_SIPBuffer &aBuffer) const
{
	if ( !IsValid() )
		return GetLastClassError();

	if ( !m_host.length() ) return TSIPErrorCodes::e_UNKNOWN;

	if (net::is_ipv6(m_host)) aBuffer.AddData("IN IP6 ");
	else aBuffer.AddData("IN IP4 ");

	aBuffer.AddData(m_host);
	if ( m_ttl > 1 )
	{
		aBuffer.AddData("/");

		char ttl[std::numeric_limits<unsigned int>::digits10 + 1 + 1 /*0-terminator*/] = { 0 };
		snprintf(ttl, sizeof ttl, "%u", m_ttl);

		TSIPErrorCodes err = aBuffer.AddData(ttl, strlen(ttl));

		if ( err != TSIPErrorCodes::e_ok )
		{
			return err;
		}
	}

	if ( (m_addr_num > 1) && (m_ttl > 1) )
	{
		aBuffer.AddData("/");

		char addr_num[std::numeric_limits<unsigned int>::digits10 + 1 + 1 /*0-terminator*/] = { 0 };
		snprintf(addr_num, sizeof addr_num, "%u", m_addr_num);

		TSIPErrorCodes err = aBuffer.AddData(addr_num, strlen(addr_num));
		if (err != TSIPErrorCodes::e_ok)
		{
			return err;
		}
	}

	return TSIPErrorCodes::e_ok;
}


TSIPErrorCodes VS_SDPConnect::Init(const VS_SIPGetInfoInterface& call)
{
	const std::string &ext_host = call.GetMyExternalCsAddress();
	if (!ext_host.empty()) {
		SetHost(ext_host);
	}else{
		const auto& media_addr = call.GetMyMediaAddress();
		if(media_addr.is_unspecified()) return TSIPErrorCodes::e_InputParam;
		m_host = media_addr.to_string();
	}
	m_ttl = 1;
	m_addr_num = 1;

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

VS_SDPConnect & VS_SDPConnect::operator=(const VS_SDPConnect &conn)
{

	if (this == &conn)
	{
		return *this;
	}
	if (*this != conn)
	{
		this->VS_BaseField::operator=(conn);
		Clean();

		this->m_ttl = conn.m_ttl;
		this->m_addr_num = conn.m_addr_num;
		this->m_host = conn.m_host;
	}
	return *this;
}

bool VS_SDPConnect::operator!=(const VS_SDPConnect &conn) const
{
	return this->VS_BaseField::operator!=(conn) ||
		(std::tie(this->m_ttl, this->m_addr_num, this->m_host) != std::tie(conn.m_ttl, conn.m_addr_num, conn.m_host));
}


void VS_SDPConnect::Clean() noexcept
{
	m_host.clear();
	m_ttl = 0;
	m_addr_num = 0;
}

const std::string &VS_SDPConnect::GetHost() const
{
	return m_host;
}

void VS_SDPConnect::SetHost(std::string host)
{
	m_host = std::move(host);
}

#undef DEBUG_CURRENT_MODULE
