#include "VS_SIPField_Contact.h"
#include "VS_SIPObjectFactory.h"
#include "VS_SIPAuthScheme.h"
#include "VS_SIPURI.h"
#include "std-generic/cpplib/string_view.h"
#include "std/debuglog/VS_Debug.h"
#include <string>
#include "VS_SIPGetInfoInterface.h"
#include "std-generic/compat/memory.h"
#include <cinttypes>

#define DEBUG_CURRENT_MODULE VS_DM_SIPPARSER

const boost::regex VS_SIPField_Contact::e("(?i)(Contact|m)(?-i)( *):.*");
const boost::regex VS_SIPField_Contact::e1(".*;(?i)(?:expires)(?-i) *(?:= *([\\d]+))? *.*");
const boost::regex VS_SIPField_Contact::e2(".*;(?i)(?:gruu)(?-i) *=\" *([-:;+.@=\\w\\d]*)\" *.*");
const boost::regex VS_SIPField_Contact::base_uri_e(
	"(([^,<]*<[^>]*>)|(\\w[\\w\\d\\+-.]*:[^;]*))"				// display name and sip uri
	"(?:\\s*;[^=]*=[^,]*)*");	// all info-params

// from here https://tools.ietf.org/html/rfc3261#section-20.10
// the URI including all URI parameters is enclosed in "<" and ">"
// If no "<"and ">" are present, all parameters after the URI are header parameters, not URI parameters.
// it means when no "<"and ">" are present we can skip all after semicolon if it present beacause we need only URI params
const boost::regex VS_SIPField_Contact::name_addr_param_e(
	"([^,<]*<[^>]*>)"				// name-addr =  [ display-name ] LAQUOT addr-spec RAQUOT
	"|(\\w[\\w\\d\\+-.]*:[^;]*)");	// addr-spec = SIP-URI / SIPS-URI / absoluteURI
									// absoluteURI    =  scheme ":" ( hier-part / opaque-part )
									// scheme         =  ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )

namespace
{
	inline void find_param_expires(string_view str, boost::optional<std::chrono::seconds> &outExpires) noexcept
	{
		boost::cmatch m;
		try
		{
			if (boost::regex_match(str.cbegin(), str.cend(), m, VS_SIPField_Contact::e1))
			{
				const std::string &expires_value = m[1];
				outExpires.emplace(::atoi(expires_value.c_str()));
			}
		}
		catch (const std::runtime_error &ex)
		{
			dstream1 << "VS_SIPField_Contact::FindParam_expires error " << ex.what() << "\n";
		}
	}

	inline void find_param_gruu(string_view str, std::string& outGruu) noexcept
	{
		boost::cmatch m;
		try
		{
			if (boost::regex_match(str.cbegin(), str.cend(), m, VS_SIPField_Contact::e2))
				outGruu = m[1];
		}
		catch (const std::runtime_error &ex)
		{
			dstream1 << "VS_SIPField_Contact::FindParam_gruu error " << ex.what() << "\n";
		}
	}

	inline TSIPErrorCodes add_uri(const VS_SIPField_Contact::ContactParam& param, VS_SIPBuffer &aBuffer) noexcept
	{
		if (!param.gruu.empty()) {	// if already have ready for use global routable uri, use it
			aBuffer.AddData("<");
			aBuffer.AddData(param.gruu);
			aBuffer.AddData(">");
			return TSIPErrorCodes::e_ok;
		}

		// othervise make uri ourselves
		return param.uri->Encode(aBuffer);
	}

	inline std::shared_ptr<VS_SIPURI> match_uri(const std::string &input)
	{
		boost::smatch m;
		if (!boost::regex_search(input, m, VS_SIPField_Contact::name_addr_param_e))
			return nullptr;
		if (m.empty())
			return nullptr;
		auto uriStr = std::string(m[0].first, m[0].second);
		VS_SIPBuffer b(uriStr); b.AddData("\r\n");

		auto uri = std::make_shared<VS_SIPURI>();
		if (uri->Decode(b) != TSIPErrorCodes::e_ok)
			return nullptr;
		return uri;
	}

} //anonymous namespace

VS_SIPField_Contact::VS_SIPField_Contact() :compact(false)
{
	VS_SIPError::Clean();
}

VS_SIPField_Contact & VS_SIPField_Contact::operator=(const VS_SIPField_Contact &src)
{
	if (this == &src)
	{
		return *this;
	}
	if (*this != src)
	{
		Clean();
		this->VS_BaseField::operator=(src);

		this->compact = src.compact;
		this->iContacts = src.iContacts;
	}
	return *this;
}

bool VS_SIPField_Contact::operator!=(const VS_SIPField_Contact &src)const
{
	if (this->VS_BaseField::operator!=(src))
	{
		return true;
	}

	return (this->compact != src.compact) || (this->iContacts != src.iContacts);
}

bool VS_SIPField_Contact::operator==(const VS_SIPField_Contact &src)const
{
	return !(*this != src);
}
TSIPErrorCodes VS_SIPField_Contact::Decode(VS_SIPBuffer &aBuffer)
{
	Clean();

	std::unique_ptr<char[]> ptr;
	std::size_t ptr_sz = 0;

	try{

		TSIPErrorCodes err = aBuffer.GetNextBlockAlloc(ptr, ptr_sz);
		if (TSIPErrorCodes::e_ok != err) throw err;
		if (!ptr || !ptr_sz) throw TSIPErrorCodes::e_buffer;

		boost::cmatch m;
		try
		{
			if (!boost::regex_match(ptr.get(), m, e))
			{
				dstream3 << "[SIPParserLib::Error] Contact Field: buffer not match, dump |" << ptr.get() << "|";
				throw TSIPErrorCodes::e_match;
			}
		}
		catch (const std::runtime_error &ex)
		{
			dstream1 << "VS_SIPField_Contact::Decode error " << ex.what() << "\n";
			throw TSIPErrorCodes::e_match;
		}

		const std::string &header = m[1];
	 	const std::string &spaces = m[2];

		// ("Contact" || "m") + ":" + spaces
		const char *contacts_items = ptr.get() + (header.length() + spaces.length() + 1);

		string_view new_input(contacts_items);
		boost::cregex_token_iterator iter(new_input.begin(), new_input.end(), base_uri_e, 0);
		const boost::cregex_token_iterator end;

		for (; iter != end; ++iter){
			const std::string &contact_item = *iter;
			boost::optional<std::chrono::seconds> expires;
			std::string gruu;

			find_param_expires(contact_item, expires);
			find_param_gruu(contact_item, gruu);

			auto uri = match_uri(contact_item);
			if (uri != nullptr)
				iContacts.emplace_back(std::move(uri), std::move(expires), "", std::move(gruu));
		}

		if (iContacts.empty())
		{
			dstream3 << "[SIPParserLib::Error] Contact Field: URI not decoded";
			throw err;
		}

	}
	catch (const TSIPErrorCodes& err_code){
		ptr_sz = 0;

		SetError(err_code);
		return err_code;
	}

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SIPField_Contact::Encode(VS_SIPBuffer &aBuffer) const
{
	if ( !IsValid() )
		return GetLastClassError();

	if (compact)
		aBuffer.AddData("m: ");
	else
		aBuffer.AddData("Contact: ");
	TSIPErrorCodes err = TSIPErrorCodes::e_UNKNOWN;

	const auto end = iContacts.end();
	for (auto contact_it = iContacts.begin(); contact_it != end; ++contact_it){

		err = add_uri(*contact_it, aBuffer);
		if (contact_it->expires.is_initialized())
		{
			aBuffer.AddData(";expires=");
			const uint64_t expires = contact_it->expires.get().count();
			char val[std::numeric_limits<uint64_t>::digits10 + 1 + 1 /*0-terminator*/] = { 0 };
			::snprintf(val, sizeof val, "%" PRIu64, expires);
			aBuffer.AddData(string_view{ val });
		}

		if (!contact_it->sip_instance.empty())
		{
			aBuffer.AddData(";+sip.instance=\"");
			aBuffer.AddData(contact_it->sip_instance);
			aBuffer.AddData("\"");
		}
		if (contact_it != end - 1)	aBuffer.AddData(",");	// contacts separated by COMMA
	}

	return err;
}

VS_SIPURI* VS_SIPField_Contact::GetLastURI()
{
	return (iContacts.rbegin()!=iContacts.rend()) ? iContacts.rbegin()->uri.get(): nullptr;
}

void VS_SIPField_Contact::Clean() noexcept
{
	VS_SIPError::Clean();
	iContacts.clear();
}

void VS_SIPField_Contact::SetLastURI(VS_SIPURI* uri)
{
	if ( !uri )
		return ;

	auto it = iContacts.rbegin();
	if (it != iContacts.rend()){
		it->uri.reset(uri);
	}
}

boost::optional<std::chrono::seconds> VS_SIPField_Contact::GetExpires() const
{
	return (iContacts.rbegin() != iContacts.rend()) ? iContacts.rbegin()->expires : boost::none;
}

std::string VS_SIPField_Contact::LastGruu() const
{
	return (iContacts.rbegin() != iContacts.rend()) ? iContacts.rbegin()->gruu : std::string{};
}

TSIPErrorCodes VS_SIPField_Contact::Init(const VS_SIPGetInfoInterface& call)
{
	//unsigned long ip = 0;
	//unsigned short port = 0;
	//eConnectionType conn = CONNECTIONTYPE_INVALID;

	std::unique_ptr<VS_SIPURI> uri;
 	boost::optional<std::chrono::seconds> expires;
	std::string sip_instance;

	if (call.IsRequest() || call.GetMessageType() != TYPE_REGISTER || !call.GetSipContact()) {
		auto &&addr = call.GetMyCsAddress();

		if (addr.is_unspecified()) return TSIPErrorCodes::e_InputParam;

		std::string host;

		// use listen port, not the one used for sending the request
		net::port port = call.GetListenPort();

		if (call.GetMessageType() == TYPE_REGISTER) {
			expires.emplace(call.GetExpires());
		}

		if (call.GetContactHost().empty()) {
			host = addr.to_string();
		}
		else
		{
			host = call.GetContactHost();
		}
		if (call.GetMyCsType() == net::protocol::TLS)
		{                                // We are waiting for sip over tls traffic on our Call Signaling port
			port = call.GetMyCsPort(); // Temporary fix for s4b; In future we should listen 5061 port for sip over tls traffic
		}

		uri = vs::make_unique<VS_SIPURI>();
		uri->AngleBracket(true);
		uri->URIType(SIPURI_SIP);
		const auto auth = call.GetAuthScheme();

		uri->Name(call.IsRequest()? call.GetDisplayNameSip(): call.GetDisplayNameMy());

		if (auth && !call.IsRequest())
		{
			uri->User(auth->login());
		} else if (!call.GetUser().empty())
		{
			uri->User(std::string(call.GetUser()));
		}

		const std::string &externalHost = call.GetMyExternalCsAddress();
		if (externalHost.empty())
		{
			uri->Host(host);
		} else
		{
			uri->Host(externalHost);
		}
		//iSIPURI->Host( !externalHost ? host : externalHost , true);
		//	iSIPURI->Host( "193.39.72.35" );
		uri->Port(port);
		uri->Transport(call.GetMyCsType());

		uri->SetError(TSIPErrorCodes::e_ok);
		uri->SetValid(true);
	} else {
		uri = vs::make_unique<VS_SIPURI>();
		VS_SIPField_Contact *c = call.GetSipContact();
		if (!c)
			return TSIPErrorCodes::e_InputParam;
		auto last_uri = c->GetLastURI();
		if (!last_uri)
			return TSIPErrorCodes::e_InputParam;
			*uri = *last_uri;
		}

	compact = call.IsCompactHeaderAllowed();

	std::string gruu;
	if (call.IsRequest() && call.GetMessageType() == TYPE_REGISTER) {
		string_view instance = call.GetSipInstance();
		if (!instance.empty()) {
			sip_instance = std::string(instance);
		}
	} else {
		gruu = call.GetContactGruu();
	}

	iContacts.emplace_back(uri.release(), std::move(expires), std::move(sip_instance), std::move(gruu));

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

void VS_SIPField_Contact::GetURIs(std::vector<std::string> &OUT_contacts) const{
	auto end = iContacts.end();
	for (auto it = iContacts.begin(); it != end; ++it){
		if (it->uri){
			std::string request_uri;
			it->uri->GetRequestURI(request_uri);
			OUT_contacts.emplace_back(request_uri);
		}
	}
}

void VS_SIPField_Contact::AddContacts(const VS_SIPField_Contact* pContact){
	if (!pContact) return;
	iContacts.insert(iContacts.end(), pContact->iContacts.begin(), pContact->iContacts.end());
}

std::unique_ptr<VS_BaseField> VS_SIPField_Contact_Instance()
{
	return vs::make_unique<VS_SIPField_Contact>();
}

#undef DEBUG_CURRENT_MODULE
