#include "VS_SIPField_Via.h"
#include "VS_SIPObjectFactory.h"
#include "std/debuglog/VS_Debug.h"
#include "VS_SIPGetInfoInterface.h"
#include <boost/algorithm/string.hpp>
#include "std-generic/compat/memory.h"

#define DEBUG_CURRENT_MODULE VS_DM_SIPPARSER

const std::string VS_SIPField_Via::exp =	".*;(?i)(?:branch)(?-i) *= *("\
											BRANCH_MAGIC_NUMBER\
											"[^;]*)+ *.*";
const boost::regex VS_SIPField_Via::e1(exp);
const boost::regex VS_SIPField_Via::e2(
				"(?i)"
			"(?:via|v)+ *: *"						// Via:
			"(?:sip/2\\.0/(tcp|udp|tls)+)+ +"		// SIP/2.0
			"([\\w\\d\\.]+|(?:\\[[0-9A-Fa-f:]+\\])) *(?:: *([\\d]*))? *"	// host:port
			" *(.*?) *(?:\r\n)?"		// headers ";param=value"
		"(?-i) *"
	);
const boost::regex VS_SIPField_Via::e3(".*;(?i)(?:keep)(?-i) *(?:= *([\\d]+))? *.*");

VS_SIPField_Via::VS_SIPField_Via():
m_keep_alive_interval(0), m_port(0), m_conn_type(net::protocol::TCP),
m_keep_alive_response(false), m_keep_alive(false), m_compact(false)
{
	VS_SIPError::Clean();
}

VS_SIPField_Via::~VS_SIPField_Via()
{
}

bool VS_SIPField_Via::FindParam_branch(string_view aInput)
{
	boost::cmatch m;
	try
	{
		if (boost::regex_match(aInput.cbegin(), aInput.cend(), m, e1))
		{
			m_branch = m[1];
			return true;
		}
	}
	catch (const std::runtime_error &ex)
	{
		dstream1 << "VS_SIPField_Via::FindParam_branch error " << ex.what() << "\n";
	}
	return false;
}
bool VS_SIPField_Via::FindParam_keep_alive(string_view aInput)
{
	boost::cmatch m;
	try
	{
		if (boost::regex_match(aInput.cbegin(), aInput.cend(), m, e3))
		{
			const std::string &timeout = m[1];
			m_keep_alive_interval = atoi(timeout.c_str());
			m_keep_alive = true;

			return true;
		}
	}
	catch (const std::runtime_error &ex)
	{
		dstream1 << "VS_SIPField_Via::FindParam_keep_alive error " << ex.what() << "\n";
	}
	return false;
}

TSIPErrorCodes VS_SIPField_Via::Decode(VS_SIPBuffer &aBuffer)
{
	Clean();

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
		regex_match_res = boost::regex_match(ptr.get(), m, e2);
	}
	catch (const std::runtime_error &ex)
	{
		dstream1 << "VS_SIPField_Via::Decode error " << ex.what() << "\n";
		regex_match_res = false;
	}
	if (!regex_match_res)
	{
		dstream3 << "[SIPParserLib::Error] Via Field: buffer not match, dump |" << ptr.get() << "|";
		SetError(TSIPErrorCodes::e_match);
		return TSIPErrorCodes::e_match;
	}

	std::string conn = m[1];
	std::string host = m[2];
	const std::string &port = m[3];
	m_params = m[4];

	// UpperCase
	std::transform(conn.begin(), conn.end(), conn.begin(), toupper);

	if (conn == "TCP") {
		m_conn_type = net::protocol::TCP;
	}
	else if (conn == "UDP") {
		m_conn_type = net::protocol::UDP;
	} else if (conn == "TLS") {
		m_conn_type = net::protocol::TLS;
	} else {
		m_conn_type = net::protocol::none;

		SetError(TSIPErrorCodes::e_InputParam);
		return TSIPErrorCodes::e_InputParam;
	}

	// we take ipv6 address with '[' and ']'
	// so we must remove brackets
	if (host[0] == '[' && host.length() >= 3 && net::is_ipv6( { host.data() + 1, host.length() - 2 } ))
	{
		// for ipv6
		host = host.substr(1, host.length() - 2);
	}

	m_host = std::move(host);
	m_port = atoi( port.c_str() );

	FindParam_branch(m_params);
	FindParam_keep_alive(m_params);

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SIPField_Via::Encode(VS_SIPBuffer &aBuffer) const
{
	if ( !IsValid() )
		return GetLastClassError();

	std::string out = m_compact ? "v: SIP/2.0" : "Via: SIP/2.0";

	switch (m_conn_type)
	{
	case net::protocol::TCP:
			out += "/TCP ";
		break;

	case net::protocol::UDP:
			out += "/UDP ";
		break;

	case net::protocol::TLS:
			out += "/TLS ";
		break;
	default:
			return TSIPErrorCodes::e_UNKNOWN;
		break;
	}

	if (m_host.empty()) return TSIPErrorCodes::e_UNKNOWN;


	if (net::is_ipv6(m_host))
	{
		out += '[';
		out += m_host;
		out += '[';

	}
	else
	{
		out += m_host;
	}


	if ( m_port && m_port != 5060 )
	{
		char port[std::numeric_limits<net::port>::digits10 + 1 + 1 /*0-terminator*/] = { 0 };
		snprintf(port, sizeof port, "%hu", m_port);

		out = out + ":" + port;
	}

	if (!m_params.empty())
		out += m_params;
	else if ( !m_branch.empty() )
	{
		out = out + ";branch=" + m_branch;
	}

	if (m_keep_alive)
		out += ";keep";
	else if (m_keep_alive_response)
	{
		const size_t pos = out.find(";keep");
		if (pos != std::string::npos)
			out.insert(pos + 5, "=0");
		else
			out += ";keep=0";
	}

	return aBuffer.AddData(out);
}

std::unique_ptr<VS_BaseField> VS_SIPField_Via_Instance()
{
	return vs::make_unique<VS_SIPField_Via>();
}

void VS_SIPField_Via::Clean() noexcept
{
	VS_SIPError::Clean();

	m_conn_type = net::protocol::none;
	m_port = 0;

	m_host.clear();
	m_branch.clear();
}

void VS_SIPField_Via::Branch(std::string aBranch)
{
	m_branch = std::move(aBranch);
}

const std::string &VS_SIPField_Via::Branch() const
{
	return m_branch;
}

bool VS_SIPField_Via::IsBranch() const
{
	return !m_branch.empty();
}

bool VS_SIPField_Via::IsKeepAlive() const
{
	return m_keep_alive;
}

size_t VS_SIPField_Via::KeepAliveInterval() const
{
	return m_keep_alive_interval;
}

void VS_SIPField_Via::ConnectionType(const net::protocol conn_type)
{
	m_conn_type = conn_type;
}

net::protocol VS_SIPField_Via::ConnectionType() const
{
	return m_conn_type;
}

void VS_SIPField_Via::Host(std::string host)
{
	m_host = std::move(host);
}

const std::string &VS_SIPField_Via::Host() const
{
	return m_host;
}

void VS_SIPField_Via::Port(const net::port port)
{
	m_port = port;
}

net::port VS_SIPField_Via::Port() const
{
	return m_port;
}

void VS_SIPField_Via::Received(std::string received)
{
	m_received = std::move(received);
}


const std::string &VS_SIPField_Via::Received() const
{
	return m_received;
}

TSIPErrorCodes VS_SIPField_Via::Init(const VS_SIPGetInfoInterface& call)
{
// Branch
	if (call.IsRequest())
	{
		m_port = call.GetListenPort();
		if (!m_port) m_port = call.GetMyCsPort();
		m_conn_type = call.GetMyCsType();

		auto &&via_host = call.GetViaHost();
		if(via_host.empty())
		{
			auto &&addr = call.GetMyCsAddress();
			if(addr.is_unspecified())
			{
				SetValid(false);
				SetError(TSIPErrorCodes::e_InputParam);
				return TSIPErrorCodes::e_InputParam;
			}
			m_host = addr.to_string();
		}
		else
		{
			m_host = via_host;
		}

		auto &&branch = call.GetMyBranch();
		m_branch = make_request_branch(branch);

		m_keep_alive = call.GetMessageType() == TYPE_INVITE && call.IsKeepAliveEnabled();

	} else { // Response

		const VS_SIPField_Via* via = call.GetSipViaCurrent();
		if ( !via )
		{
			SetValid(false);
			SetError(TSIPErrorCodes::e_InputParam);
			return TSIPErrorCodes::e_InputParam;
		}

		*this = *via;

		const int msgType = call.GetMessageType();
		m_keep_alive_response = !call.IsDirectionToSip() && (msgType == TYPE_INVITE || msgType == TYPE_UPDATE) && call.IsKeepAliveEnabled();
	}

	m_compact = call.IsCompactHeaderAllowed();

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

VS_SIPField_Via & VS_SIPField_Via::operator=(const VS_SIPField_Via &via)
{
	if (this == &via)
	{
		return *this;
	}
	if (*this != via)
	{
		this->VS_BaseField::operator=(via);
		this->m_conn_type = via.m_conn_type;
		this->m_host = via.m_host;
		this->m_port = via.m_port;
		this->m_params = via.m_params;
		this->m_branch = via.m_branch;
		this->m_keep_alive = via.m_keep_alive;
		this->m_keep_alive_interval = via.m_keep_alive_interval;
		this->m_keep_alive_response = via.m_keep_alive_response;
		this->m_compact = via.m_compact;
	}
	return *this;
}


bool VS_SIPField_Via::operator!=(const VS_SIPField_Via &via) const
{
	if (this->VS_BaseField::operator!=(via))
	{
		return true;
	}

	if (this->ConnectionType() != via.ConnectionType()
		|| this->Port() != via.Port()
		|| this->m_params != via.m_params
		|| this->m_keep_alive != via.m_keep_alive
		|| this->m_keep_alive_interval != via.m_keep_alive_interval
		|| this->m_keep_alive_response != via.m_keep_alive_response
		|| this->m_compact != via.m_compact)
	{
		return true;
	}

	return (this->Host().length() != via.Host().length() || !boost::iequals(this->Host(), via.Host()) )
		|| (this->Branch().length() != via.Branch().length() || !boost::iequals(this->Branch(), via.Branch()) );
}


std::string VS_SIPField_Via::make_request_branch(string_view my_branch) {
	return BRANCH_MAGIC_NUMBER + std::string(my_branch);
}

#undef DEBUG_CURRENT_MODULE
