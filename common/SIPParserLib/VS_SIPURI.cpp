#include "VS_SIPURI.h"
#include "VS_SIPObjectFactory.h"
#include "std-generic/clib/strcasecmp.h"
#include "std/debuglog/VS_Debug.h"
#include "std/cpplib/curl_deleters.h"


#include "utils/escape.h"
#include "exceptions/SIPURIEscapeException.h"
#include <tuple>
#include "net/Address.h"
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>

#define DEBUG_CURRENT_MODULE VS_DM_SIPPARSER

namespace
{
	constexpr struct
	{
		const net::protocol value;
		const char *str;
	} TRANSPORTS[]
	{
		{ net::protocol::TCP, "tcp" },
		{ net::protocol::UDP, "udp" },
		{ net::protocol::TLS, "tls" }
	};

	inline std::string MakeUrlUnEscape(const string_view s)
	{
		int len(0);
		std::unique_ptr<CURL, CURL_deleter> curl(::curl_easy_init());
		if (curl)
		{
			std::unique_ptr<char, curl_free_deleter> unescaped{ ::curl_easy_unescape(curl.get(), s.data(), s.length(), &len) };
			if (unescaped) return { unescaped.get() };
		}
		return {};
	}
}


const boost::regex VS_SIPURI::e1(".*; *(?i)(?:maddr)(?-i) *= *([-A-Za-z0-9\\.:]+) *.*");
const boost::regex VS_SIPURI::e2(R"(.*; *(?i)(?:transport)(?-i) *= *(?i)([-A-Za-z0-9\.!%\*_\+`'~]+)(?-i)(?: *; *.*)*)");
const boost::regex VS_SIPURI::e3(".*; *(?i)(?:lr)(?-i).*");
const boost::regex VS_SIPURI::e4(".*; *(?i)(?:epid)(?-i) *= *([-\\w\\d]*) *.*");
const boost::regex VS_SIPURI::e5(R"(.*; *(?i)(?:tag)(?-i) *= *([-A-Za-z0-9,=\.!%\*_\+`'"~ \[\]\(\)\<\>\/:\$]*) *.*)");
const boost::regex VS_SIPURI::e7(".*; *(?i)(?:gr)(?-i) *= *([-:\\w\\d]+) *.*");
const boost::regex VS_SIPURI::e6(
		"(?:"
			" *(?:\"?([^\"]*)\"?)" // display name
			" *<"
				" *(?i)((?:(?:sips?)|(?:mailto))+)(?-i) *:"		// "sip || sips"
				" *(?:@?([-A-Za-z0-9#\\.\\\\\\/]+|(?:\\[[0-9A-Fa-f:]+\\]))|(?:([-A-Za-z0-9#_,=!%`'~& \"\\.\\*\\+\\[\\]\\(\\)\\/\\\\\\$\\?]+)(?:[-A-Za-z0-9;=:\\.\\\\ ]*) *@? *([-A-Za-z0-9\\.]+|(?:\\[[0-9A-Fa-f:]+\\]))))"	// "host || user(;tag|:password)@host"
				" *(?:: *([0-9]*))*"		// ":port"
				" *([-A-Za-z0-9,;=\\.!%\\*_\\+`'\"~ \\[\\]\\/:&\\$\\?]*)"		// host's ";param=value"
			" *>"
			" *([-A-Za-z0-9,;=\\.!%\\*_\\+`'\"~ \\[\\]\\/:&\\$\\?><@]*) *"		// header's ";param='value'"
		")|(?:"
			" *(?i)((?:(?:sips?)|(?:mailto))+)(?-i):"		// "sip || sips"
			" *(?:@?(?:([-A-Za-z0-9#\\.\\\\\\/]+|(?:\\[[0-9A-Fa-f:]+\\])))|(?:([-A-Za-z0-9#_,;=!%`'~& \\.\\*\\+\\[\\]\\(\\)\\/\\\\\\$\\?]+)(?:[-A-Za-z0-9;=:\\.\\\\ ]*)@([-A-Za-z0-9\\.]+|(?:\\[[0-9A-Fa-f:]+\\]))))"	// "host || user;tag@host"
			" *(?:: *([0-9]*))*"		// ":port"
			" *([-A-Za-z0-9,;=\\.!%\\*_\\+`'\"~ \\[\\]\\/:&\\$\\?><]*)"		// header's ";param='value'"
			" *(?i)(?:sip/2\\.0)*(?-i) *"		// signature " SIP/2.0"
		")"
	);

#define PHONE_VISUAL_SEPARATORS "-.()"
static const boost::regex rx_phone("^\\+?[-0-9\\(\\)\\.]+"); // includes visual-eparators from RFC3966: "-" / "." / "(" / ")"
static const boost::regex rx_sip_version("(?i)^sip/2\\.0$");

// definition of pvalue expression from rfc 3261
#define PVALUE "([\\w\\-_.!~*'\\(\\)%\\[\\]\\/:&\\+\\$]+)"

VS_SIPURI::VS_SIPURI(): m_param_transport(net::protocol::none)
{
	VS_SIPURI::Clean();
}

VS_SIPURI::~VS_SIPURI()
{
	VS_SIPURI::Clean();
}

bool VS_SIPURI::FindParam_maddr(string_view aInput)
{
	boost::cmatch m;
	try
	{
		if (boost::regex_match(aInput.cbegin(), aInput.cend(), m, e1))
		{
			iParam_maddr = m[1];
			return true;
		}
	}
	catch (const std::runtime_error &ex)
	{
		dstream1 << "VS_SIPURI::FindParam_maddr error " << ex.what() << "\n";
		SetError(TSIPErrorCodes::e_match);
	}
	return false;
}

bool VS_SIPURI::FindParam_transport(string_view aInput)
{
	boost::cmatch m;
	try
	{
		if ( boost::regex_match(aInput.cbegin(), aInput.cend(), m, e2) )
		{
			const std::string &theParam_transport = static_cast<const std::string &>(m[1]);
			for(auto &&item : TRANSPORTS)
			{
				string_view tmp{ item.str };
				if(tmp.length() == theParam_transport.length() &&
					std::equal(theParam_transport.cbegin(), theParam_transport.cend(), tmp.cbegin(), [](char c1, char c2)
				{
					if (c1 == c2) return true;
					if (std::tolower(c1) == std::tolower(c2)) return true;
					return false;
				}))
				{
					m_param_transport = item.value;
					return true;
				}
			}
		}
		m_param_transport = net::protocol::none;
	}
	catch (const std::runtime_error &ex)
	{
		dstream1 << "VS_SIPURI::FindParam_transport error " << ex.what() << "\n";
		SetError(TSIPErrorCodes::e_match);
	}
	return false;
}

bool VS_SIPURI::FindParam_lr(string_view aInput)
{
	boost::cmatch m;
	try
	{
		if ( boost::regex_match(aInput.cbegin(), aInput.cend(), m, e3) )
		{
			iParam_lr = true;
			return true;
		}
	}
	catch (const std::runtime_error &ex)
	{
		dstream1 << "VS_SIPURI::FindParam_lr error " << ex.what() << "\n";
		SetError(TSIPErrorCodes::e_match);
	}
	return false;
}

bool VS_SIPURI::FindParam_epid(string_view aInput)
{
	boost::cmatch m;
	try
	{
		if ( boost::regex_match(aInput.cbegin(), aInput.cend(), m, e4) )
		{
			m_param_epid = m[1];

			return true;
		}
	}
	catch (const std::runtime_error &ex)
	{
		dstream1 << "VS_SIPURI::FindParam_epid error " << ex.what() << "\n";
		SetError(TSIPErrorCodes::e_match);
	}
	return false;
}

bool VS_SIPURI::FindParam_tag(string_view aInput)
{
	boost::cmatch m;
	try
	{
		if ( boost::regex_match(aInput.cbegin(), aInput.cend(), m, e5) )
		{
			iParam_tag = m[1];

			return true;
		}
	}
	catch (const std::runtime_error &ex)
	{
		dstream1 << "VS_SIPURI::FindParam_tag error " << ex.what() << "\n";
		SetError(TSIPErrorCodes::e_match);
	}
	return false;
}

bool VS_SIPURI::FindParam_gruu(string_view aInput)
{
	boost::cmatch m;
	try
	{
		if (boost::regex_match(aInput.cbegin(), aInput.cend(), m, e7))
		{
			m_param_gruu = m[1];
			return true;
		}
	}
	catch (const std::runtime_error &ex)
	{
		dstream1 << "VS_SIPURI::FindParam_gruu error " << ex.what() << "\n";
		SetError(TSIPErrorCodes::e_match);
	}
	return false;
}
bool FindPvalueParam(const std::string & input, const boost::regex& e, std::string &res) {
	boost::smatch m;
	if (!boost::regex_search(input, m, e)) return false;

	res = m[1];
	return true;
}

const boost::regex VS_SIPURI::opaque_e("(?i)opaque=" PVALUE, boost::regex_constants::optimize);
bool VS_SIPURI::FindParam_opaque(const std::string & input)
{
	return FindPvalueParam(input, opaque_e, m_opaque);
}

const boost::regex VS_SIPURI::ms_opaque_e("(?i)ms-opaque=" PVALUE, boost::regex_constants::optimize);
bool VS_SIPURI::FindParam_MsOpaque(const std::string & input)
{
	return FindPvalueParam(input, ms_opaque_e, m_ms_opaque);
}

const boost::regex VS_SIPURI::ms_route_sig_e("(?i)ms-route-sig=" PVALUE, boost::regex_constants::optimize);
bool VS_SIPURI::FindParam_MsRouteSig(const std::string & input)
{
	return FindPvalueParam(input, ms_route_sig_e, m_ms_route_sig);
}

const boost::regex VS_SIPURI::ms_key_info_e("(?i)ms-key-info=" PVALUE, boost::regex_constants::optimize);
bool VS_SIPURI::FindParam_MsKeyInfo(const std::string & input)
{
	return FindPvalueParam(input, ms_key_info_e, m_ms_key_info);
}

const boost::regex VS_SIPURI::ms_identity_e("(?i)ms-identity=" PVALUE, boost::regex_constants::optimize);
bool VS_SIPURI::FindParam_MsIdentity(const std::string & input)
{
	return FindPvalueParam(input, ms_identity_e, m_ms_identity);
}

const boost::regex VS_SIPURI::ms_fe_e("(?i)ms-fe=" PVALUE, boost::regex_constants::optimize);
bool VS_SIPURI::FindParam_MsFe(const std::string & input)
{
	return FindPvalueParam(input, ms_fe_e, m_ms_fe);
}

bool FindParam(const std::string & input, const boost::regex& e) {
	boost::smatch m;
	return boost::regex_search(input, m, e);
}

const boost::regex VS_SIPURI::ms_role_rs_to_e("(?i)ms-role-rs-to", boost::regex_constants::optimize);
bool VS_SIPURI::FindParam_MsRoleRsTo(const std::string & input)
{
	return m_ms_role_rs_to = FindParam(input, ms_role_rs_to_e);
}

const boost::regex VS_SIPURI::ms_role_rs_from_e("(?i)ms-role-rs-from", boost::regex_constants::optimize);
bool VS_SIPURI::FindParam_MsRoleRsFrom(const std::string & input)
{
	return m_ms_role_rs_from = FindParam(input, ms_role_rs_from_e);
}

const boost::regex VS_SIPURI::ms_ent_dest_e("(?i)ms-ent-dest", boost::regex_constants::optimize);
bool VS_SIPURI::FindParam_MsEntDest(const std::string & input)
{
	return m_ms_ent_dest = FindParam(input, ms_ent_dest_e);
}
void VS_SIPURI::FindParam_MsGruu(const std::string & input)
{
	m_ms_gruu = input.find(";gruu") != std::string::npos;
}
#undef PVALUE


TSIPErrorCodes VS_SIPURI::Decode(VS_SIPBuffer &aBuffer)
{
	Clean();

	std::unique_ptr<char[]> theBuffer;
	std::size_t theSize(0);

	TSIPErrorCodes res = aBuffer.GetNextBlockAlloc(theBuffer, theSize);
	if (TSIPErrorCodes::e_ok != res)
	{
		SetError(res);
		return res;
	}

	if ( !theBuffer || !theSize )
	{
		SetError(TSIPErrorCodes::e_buffer);
		return TSIPErrorCodes::e_buffer;
	}

	if(m_do_pre_decode_escaping)
	{
		try {
			// user@domain@server -> user%40domain@server
			std::string urlesc_buffer = theBuffer.get();
			urlesc_buffer.erase(0, 4); //sip:user@domain@server -> user@domain@server
			std::string result = vs::sip_uri_escape(urlesc_buffer);
			if (result != urlesc_buffer)
			{
				theBuffer.reset(new char[result.size() + 1 + 4]);
				memcpy(theBuffer.get(), "sip:", 4);
				memcpy(theBuffer.get() + 4, result.c_str(), result.size() + 1);
			}
		}catch(const vs::SIPURIEscapeException &ex)
		{
			dstream1 << "VS_SIPURI::Decode error" << ex.what() << "\n";
			return TSIPErrorCodes::e_UNKNOWN;
		}
		m_do_pre_decode_escaping = false;	// to be sure 'SetDoPreDecodeEscaping' will be called next time before 'Decode' when we need it
	}
	boost::cmatch m;
	try
	{
		bool l_brack = false;
		const char *p = theBuffer.get();
		auto skip_spaces = [&p]()
		{
			while (isspace(*p))
				p++;
		};

		// Try follow the general recommendation in RFC3966 section 9.
		// The parser is incomplete and crude though.
		auto parse_tel = [this, &p, &theBuffer, &skip_spaces, &l_brack]() -> TSIPErrorCodes
		{
			iURIType = SIPURI_TEL;
			std::string token;

			auto parse_error = [&]()
			{
				dstream3 << "[SIPParserLib::Error] SIPURI Class: buffer not match, dump |" << theBuffer.get() << "|";
			};

			auto try_match = [this, &p, &theBuffer, &parse_error](const boost::regex &pattern, std::string &res) -> TSIPErrorCodes {
				boost::cmatch m;
				res.clear();

				if (!boost::regex_search(p, m, pattern))
				{
					parse_error();
					SetError(TSIPErrorCodes::e_match);
					return TSIPErrorCodes::e_match;
				}

				res.assign(m[0].first, m[0].second);
				p += res.length();

				return TSIPErrorCodes::e_ok;
			};

			skip_spaces();
			auto res = try_match(rx_phone, token);
			if (res != TSIPErrorCodes::e_ok)
			{
				return res;
			}

			// get phone number
			{
				std::string phone;
				// skip separators
				for (size_t i = 0; i < token.length(); i++)
				{
					char c = token[i];
					if (strchr(PHONE_VISUAL_SEPARATORS, c) == NULL)
					{
						phone.push_back(c);
					}
				}

				iUser = std::move(phone);
			}

			// get phone context (lets treat it as hostname for now)
			{
				std::string phone_context;
				if (strncasecmp(";phone-context=", p, 15) == 0)
				{
					p += 15;
					while (isspace(*p) == 0)
					{
						phone_context.push_back(*p);
						p++;
					}

					iHost = std::move(phone_context);
				}
			}

			if (l_brack)
			{
				skip_spaces();
				if (p[0] == '>')
				{
					p++;
					return TSIPErrorCodes::e_ok;
				}
				else
				{
					parse_error();
					SetError(TSIPErrorCodes::e_match);
					return TSIPErrorCodes::e_match;
				}
			}

			skip_spaces();
			res = try_match(rx_sip_version, token);
			if (res != TSIPErrorCodes::e_ok)
			{
				return res;
			}

			return TSIPErrorCodes::e_ok;
		};

		auto try_sip_phone = [&p, &skip_spaces]() -> bool // sip(s):+123456789
		{
			size_t uri_prefix_len = 0;
			boost::cmatch m;
			if (strncasecmp("sip:", p, 4) == 0)
			{
				uri_prefix_len = 4;
			}
			else if (strncasecmp("sips:", p, 5) == 0)
			{
				uri_prefix_len = 5;
			}
			else
			{
				return false;
			}

			p += uri_prefix_len;
			skip_spaces();
			if (p[0] != '+') // allow only global numbers in SIP URI
			{
				return false;
			}

			if (!boost::regex_search(p, m, rx_phone))
			{
				return false;
			}

			if (strchr(p, '@') != nullptr)
			{
				return false;
			}

			return true;
		};

		skip_spaces();
		if (p[0] == '<')
		{
			l_brack = true;
			p++;
		}
		skip_spaces();
		if (strncasecmp("tel:", p, 4) == 0)
		{
			p += 4; // skip tel
			res = parse_tel();
			if (res != TSIPErrorCodes::e_ok)
			{
				return res;
			}
		}
		else if (try_sip_phone())
		{
			// It slightly violates the RFC3966 (section 9),
			// as we do not check for host-name, but I doubt it is possible to do something useful
			// in SIP without violating some specification.
			res = parse_tel();
			if (res != TSIPErrorCodes::e_ok)
			{
				return res;
			}
		}
		else
		{
		if ( !boost::regex_match(theBuffer.get(), m, e6) )
		{
			// workaround for "Contact: *" in REGISTER from PVX
			if (strncmp(theBuffer.get(), "*", theSize) == 0 || strncmp(theBuffer.get(), " *", theSize) == 0)
			{
				SetValid(true);
				SetError(TSIPErrorCodes::e_ok);
				return TSIPErrorCodes::e_ok;
			}

			dstream3 << "[SIPParserLib::Error] SIPURI Class: buffer not match, dump |" <<  theBuffer.get() << "|";
			SetError(TSIPErrorCodes::e_match);
			return TSIPErrorCodes::e_match;
		}

		const std::string &theProto = m[9];		// 2nd "sip"

		std::size_t theIndex;
		if ( theProto.length() == 0 )
		{
			// iName
			const std::string &theName = m[1];

			// ПРОВЕРКА: Если нет букв в имени - то оно не валидное
			static boost::regex e2(".*\\w.*");

			if ( boost::regex_match(theName, e2) && theName.length() )
			{
				iName = theName;
			}
			iIsAngleBracket = true;
			theIndex = 2;
			}
			else {
			iIsAngleBracket = false;
			theIndex = 9;
		}

		// iURIType
		const std::string &theURIType = m[theIndex];

		bool bURIType = false;
			if (theURIType == "sip")
		{
			iURIType = SIPURI_SIP;
			bURIType = true;
		}
			else if (theURIType == "sips")
		{
			iURIType = SIPURI_SIPS;
			bURIType = true;
		}
			else if (theURIType == "mailto")
		{
			iURIType = SIPURI_MAILTO;
			bURIType = true;
		}
			else if (theURIType == "tel")
			{
				iURIType = SIPURI_TEL;
				bURIType = true;
			}

		if ( !bURIType )
		{
			dstream3 << "[SIPParserLib::Error] SIPURI Class: URI type not supported";
			iURIType = SIPURI_INVALID;
			SetError(TSIPErrorCodes::e_InputParam);
			return TSIPErrorCodes::e_InputParam;
		}

		// Host
		std::string theHost = m[theIndex + 1];
		if ( theHost.length() == 0 )
			theHost = m[theIndex + 3];
		// we take ipv6 address with '[' and ']'
		// so we must remove brackets
		if(theHost[0] == '[' && theHost.length() >=3 && net::is_ipv6({ theHost.data() + 1, theHost.length() - 2 }))
		{
			// ipv6, have to remove [ and ]
			theHost = theHost.substr(1, theHost.length() - 2);
		}

		iHost = std::move(theHost);

		// User
		const std::string &theUser = m[theIndex + 2];
		if (theUser.length() > 0)
		{
			iUser = MakeUrlUnEscape(theUser);
		}

		// Port
		const std::string &thePort = m[theIndex + 4];
		if (thePort.length() > 0)
		{
			iPort = atoi( thePort.c_str() );
		}

		// Params
		const std::string &theHostParam = m[theIndex + 5];

		std::size_t idex_header_param;
		if (iIsAngleBracket)
		{
			idex_header_param = theIndex + 6;
		}
		else
		{
			idex_header_param = theIndex + 5;
		}

		FindParam_gruu(theHostParam );
		FindParam_maddr( theHostParam );
		FindParam_transport( theHostParam );
		FindParam_lr( theHostParam );
		FindParam_tag( static_cast<const std::string&>(m[idex_header_param]) );
		FindParam_epid( static_cast<const std::string &>(m[idex_header_param]) );
		FindParam_opaque(theHostParam);
		FindParam_MsOpaque(theHostParam);
		FindParam_MsRouteSig(theHostParam);
		FindParam_MsKeyInfo(theHostParam);
		FindParam_MsIdentity(theHostParam);
		FindParam_MsFe(theHostParam);
		FindParam_MsRoleRsTo(theHostParam);
		FindParam_MsRoleRsFrom(theHostParam);
		FindParam_MsEntDest(theHostParam);
		FindParam_MsGruu(theHostParam);
	}
	}
	catch (const std::runtime_error &ex)
	{
		dstream1 << "VS_SIPURI::Decode error" << ex.what() << "\n";
		SetError(TSIPErrorCodes::e_match);
		return TSIPErrorCodes::e_match;
	}

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SIPURI::Encode(VS_SIPBuffer &aBuffer) const
{
	if ( !IsValid() )
		return GetLastClassError();

	if ( !iURIType )
		return TSIPErrorCodes::e_InputParam;

	std::string theOutput;

	if(!iName.empty())
	{
		// https://tools.ietf.org/html/rfc3261#section-25.1
		//  A string of text is parsed as a single word if it is quoted using
		//	double - quote marks.In quoted strings, quotation marks(") and backslashes(\) need to be escaped.
		//	quoted - string = SWS DQUOTE *(qdtext / quoted - pair) DQUOTE
		//	qdtext = LWS / %x21 / %x23 - 5B / %x5D - 7E / UTF8 - NONASCII
		//	The backslash character("\") MAY be used as a single-character
		//		quoting mechanism only within quoted - string and comment constructs.
		//		Unlike HTTP / 1.1, the characters CR and LF cannot be escaped by this
		//		mechanism to avoid conflict with line folding and header separation.
		//		quoted - pair = "\" (%x00-09 / %x0B-0C / %x0E - 7F)
		std::string copy_iName = iName;
		boost::replace_all(copy_iName, "\\", "%5C");
		boost::replace_all(copy_iName, "\"", "%22");

		theOutput += '\"';
		theOutput += copy_iName;
		theOutput += '\"';
	}

	if ( iIsAngleBracket )
		theOutput += "<";

	switch(iURIType)
	{
	case SIPURI_SIP:
			theOutput += "sip:";
		break;
	case SIPURI_SIPS:
			theOutput += "sips:";
		break;
	default:
			return TSIPErrorCodes::e_UNKNOWN;
	}

	if(!iUser.empty())
	{
		try {
			const std::string str = std::string(iUser) + "@";
			theOutput += vs::sip_uri_escape(str);
		}catch(const vs::SIPURIEscapeException &ex)
		{
			dstream1 << "VS_SIPURI::Encode error " << ex.what() << "\n";
			return TSIPErrorCodes::e_UNKNOWN;
		}
	}

	if(!iHost.empty())
	{
		std::string host = MakeUri();
		if (host.empty())
		{
			return TSIPErrorCodes::e_UNKNOWN;
		}
		theOutput += host;
	}

	if ( iPort )
	{
		theOutput += ":";

		char thePort[std::numeric_limits<net::port>::digits10 + 1 + 1 /*0-terminator*/] = { 0 };
		snprintf(thePort, sizeof thePort, "%hu", iPort);
		theOutput += thePort;
	}

	if (!m_param_gruu.empty())
	{
		theOutput += ";gr=";
		theOutput += m_param_gruu;
	}

// maddr
	if (!iParam_maddr.empty())
	{
		theOutput += ";maddr=";
		theOutput += iParam_maddr;
	}

// transport
	if ( m_param_transport != net::protocol::none )
	{
        theOutput += ";transport=";

		switch ( m_param_transport )
		{
		case net::protocol::TCP:
				theOutput += "tcp";
			break;

		case net::protocol::UDP:
				theOutput += "udp";
			break;

		case net::protocol::TLS:
				theOutput += "tls";
			break;

		default:
				return TSIPErrorCodes::e_UNKNOWN;
		}
	}

// lr
	if ( iParam_lr )
		theOutput += ";lr";

	if (!m_opaque.empty()) {
		theOutput += ";opaque=" + m_opaque;
	}
	if (!m_ms_opaque.empty()) {
		theOutput += ";ms-opaque=" + m_ms_opaque;
	}
	if (!m_ms_route_sig.empty()) {
		theOutput += ";ms-route-sig=" + m_ms_route_sig;
	}
	if (!m_ms_key_info.empty()) {
		theOutput += ";ms-key-info=" + m_ms_key_info;
	}
	if (!m_ms_identity.empty()) {
		theOutput += ";ms-identity=" + m_ms_identity;
	}
	if (!m_ms_fe.empty()) {
		theOutput += ";ms-fe=" + m_ms_fe;
	}
	if (m_ms_role_rs_to) {
		theOutput += ";ms-role-rs-to";
	}
	if (m_ms_role_rs_from) {
		theOutput += ";ms-role-rs-from";
	}
	if (m_ms_ent_dest) {
		theOutput += ";ms-ent-dest";
	}
	if (m_ms_gruu) theOutput += ";gruu";

	if ( iIsAngleBracket )
		theOutput += ">";

	if ( !iParam_tag.empty() )
	{
		theOutput += ";tag=";
		theOutput += iParam_tag;
	}

	if (!m_param_epid.empty())
	{
		theOutput += ";epid=";
		theOutput += m_param_epid;
	}

	return aBuffer.AddData(theOutput);
}


bool VS_SIPURI::operator!=(const VS_SIPURI &aURI)const
{

	if (this->VS_BaseField::operator!=(aURI))
	{
		return true;
	}

	return (aURI.iName.length() != this->iName.length() || !boost::iequals(aURI.iName, this->iName) )
		|| (this->iUser.length() != aURI.iUser.length() || !boost::iequals(this->iUser, aURI.iUser) )
		|| (this->iHost.length() != aURI.iHost.length() || !boost::iequals(this->iHost, aURI.iHost) )
		|| (this->m_param_gruu.length() != aURI.m_param_gruu.length() || !boost::iequals(this->m_param_gruu, aURI.m_param_gruu))
		|| (this->iParam_maddr.length() != aURI.iParam_maddr.length() || !boost::iequals(this->iParam_maddr, aURI.iParam_maddr))
		|| (this->iParam_tag.length() != aURI.iParam_tag.length() || !boost::iequals(this->iParam_tag, aURI.iParam_tag))
		|| this->iURIType != aURI.iURIType
		|| this->iIsAngleBracket != aURI.iIsAngleBracket
		|| this->iParam_lr != aURI.iParam_lr
		|| this->m_param_transport != aURI.m_param_transport
		|| this->iPort != aURI.iPort
		|| this->m_opaque != aURI.m_opaque
		|| this->m_ms_opaque != aURI.m_ms_opaque
		|| this->m_ms_route_sig != aURI.m_ms_route_sig
		|| this->m_ms_key_info != aURI.m_ms_key_info
		|| this->m_ms_identity != aURI.m_ms_identity
		|| this->m_ms_fe != aURI.m_ms_fe
		|| this->m_ms_role_rs_to != aURI.m_ms_role_rs_to
		|| this->m_ms_role_rs_from != aURI.m_ms_role_rs_from
		|| this->m_ms_ent_dest != aURI.m_ms_ent_dest;
}

bool VS_SIPURI::operator==(const VS_SIPURI &aURI)const
{
	return !(*this != aURI);
}

void VS_SIPURI::Clean() noexcept
{
	VS_BaseField::Clean();

	iName.clear();
	iUser.clear();
	iHost.clear();
	iParam_maddr.clear();
	iParam_tag.clear();
	m_param_epid.clear();
	m_param_gruu.clear();

	iIsAngleBracket = false;
	iParam_lr = false;

	iURIType = SIPURI_INVALID;
	m_param_transport = net::protocol::none;
	iPort = 0;

	VS_SIPError(TSIPErrorCodes::e_null, false);
}

bool VS_SIPURI::GetRequestURI(std::string &uri) const
{
	uri.clear();

	VS_SIPBuffer uri_buff;
	if(Encode(uri_buff)!= TSIPErrorCodes::e_ok) return false;
	uri_buff.AddData("\r\n");

	char buff[1024] = { 0 };
	std::size_t buff_sz = 1024;
	if(uri_buff.GetNextBlock(buff, buff_sz) != TSIPErrorCodes::e_ok) return false;

	{
		uri = MakeUrlUnEscape({ buff, buff_sz });
	}

	const auto uri_begin = uri.find_first_of('<');
	const auto uri_end = uri.find_last_of('>');
	if (uri_begin != std::string::npos && uri_end != std::string::npos)
	{
		if (uri_end <= uri_begin)
			return false;
		uri = uri.substr(uri_begin + 1, uri_end - uri_begin - 1);
	}

	if (string_view(uri).substr(0, 4) == "sip:")
		uri = uri.substr(4);

	return true;
}

bool VS_SIPURI::Name(std::string aName)
{
	if ( aName.length() > 255 ) return false;
	iName = std::move(aName);
	return true;
}

const std::string &VS_SIPURI::Name() const
{
	return iName;
}

bool VS_SIPURI::IsName() const
{
	return !iName.empty();
}

int VS_SIPURI::URIType() const
{
	return iURIType;
}

void VS_SIPURI::URIType(int type)
{
	iURIType = type;
}

const std::string &VS_SIPURI::User() const
{
	return iUser;
}

void VS_SIPURI::User(std::string user)
{
	iUser = std::move(user);
}
const std::string &VS_SIPURI::Host() const
{
	return iHost;
}
void VS_SIPURI::Host(std::string host)
{
	iHost = std::move(host);
}

net::port VS_SIPURI::Port() const
{
	return iPort;
}

void VS_SIPURI::Port(net::port port)
{
	iPort = port;
}
bool VS_SIPURI::AngleBracket() const
{
	return iIsAngleBracket;
}
void VS_SIPURI::AngleBracket(bool IsAngleBracket)
{
	iIsAngleBracket = IsAngleBracket;
}
TSIPErrorCodes VS_SIPURI::Init(const VS_SIPGetInfoInterface& call)
{
	return TSIPErrorCodes::e_UNKNOWN;
}
const std::string &VS_SIPURI::Tag() const
{
	return iParam_tag;
}

void VS_SIPURI::Tag(std::string tag)
{
	iParam_tag = std::move(tag);
}

void VS_SIPURI::Transport(const net::protocol transport)
{
	m_param_transport = transport;
}

net::protocol VS_SIPURI::Transport() const
{
	return m_param_transport;
}

const std::string &VS_SIPURI::Epid() const
{
	return m_param_epid;
}

void VS_SIPURI::Epid(std::string epid)
{
	m_param_epid = std::move(epid);
}

bool VS_SIPURI::GetAlias_UserHost(char* alias) const
{
	if ( !alias ) return false;

	const std::size_t l1 = iUser.length();
	std::size_t l2;

	std::string host = MakeUri();
	if (!host.empty())
		l2 = host.length();
	else
		return false;

	if ( (l1 + l2 + 1) > 255 )
		return false;

	if ( !iUser.empty() )
	{
		memcpy(alias, iUser.data(), l1);
		memcpy(alias + l1, "@", 1);
		memcpy(alias + l1 + 1, host.c_str(), l2);
		alias[l1 + 1 + l2] = 0;
	}else
	{
		memcpy(alias, host.c_str(), l2);
		alias[l2] = 0;
	}

	return true;
}

void VS_SIPURI::maddr(std::string maddr)
{
	iParam_maddr = std::move(maddr);
}

const std::string &VS_SIPURI::maddr() const
{
	return iParam_maddr;
}

bool VS_SIPURI::lr() const
{
	return iParam_lr;
}

void VS_SIPURI::set_opaque(std::string s) {
	m_opaque = std::move(s);
}

std::string VS_SIPURI::MakeUri() const
{
	if(net::is_ipv6(iHost))
	{
		 return "[" + iHost + ']';
	}
	return iHost;
}

#undef DEBUG_CURRENT_MODULE
