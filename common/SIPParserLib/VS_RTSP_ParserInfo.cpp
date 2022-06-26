#include "VS_RTSP_ParserInfo.h"
#include "VS_SIPAuthScheme.h"

std::uint32_t VS_RTSP_ParserInfo::GetCseq() const
{
	return CSeq;
}
void VS_RTSP_ParserInfo::SetCSeq(std::uint32_t iCseq)
{
	CSeq=iCseq;
}
const std::string& VS_RTSP_ParserInfo::GetContentType() const
{
	return ContentType;
}
void VS_RTSP_ParserInfo::SetContentType(std::string input)
{
	ContentType = std::move(input);
}
std::size_t VS_RTSP_ParserInfo::GetContentLength() const
{
	return ContentLength;
}
void VS_RTSP_ParserInfo::SetContentLength(unsigned int input)
{
	ContentLength = input;
}
void VS_RTSP_ParserInfo::SetServer(std::string sServer)
{
	Server = std::move(sServer);
}
const std::string& VS_RTSP_ParserInfo::GetServer() const
{
	return Server;
}
void VS_RTSP_ParserInfo::SetSessionsID(std::string iSessionsId)
{
	SessionsId = std::move(iSessionsId);
}
const std::string& VS_RTSP_ParserInfo::GetSessionsID() const
{
	return SessionsId;
}
CommandsSet VS_RTSP_ParserInfo::GetSupportedCommands() const
{
	return SupportedCommands;
}
void VS_RTSP_ParserInfo::SetSupportedCommands(const CommandsSet& cmd_set)
{
	SupportedCommands = cmd_set;
}
bool VS_RTSP_ParserInfo::IsCommandSupported(eRequestType cmd) const
{
	return SupportedCommands.count(cmd) != 0;
}

VS_RTSP_ParserInfo::VS_RTSP_ParserInfo()
	: CSeq(0)
	, ContentLength(0)
	, bResponseOrRequest(false)
	, RequestType(eRequestType::REQUEST_invalid)
	, ResponseType(eResponseType::RESPONSE_invalid)
	, auth_attempts(0)
{
}

VS_RTSP_ParserInfo::~VS_RTSP_ParserInfo()
{

}

bool VS_RTSP_ParserInfo::IsRequest() const
{
	return !bResponseOrRequest;
}
void VS_RTSP_ParserInfo::IsRequest(bool bIsRequest)
{
	bResponseOrRequest = !bIsRequest;
}
void VS_RTSP_ParserInfo::IncreaseCSeq()
{
	CSeq++;
}
void VS_RTSP_ParserInfo::SetRequestType(eRequestType in)
{
	RequestType = in;
}
eRequestType VS_RTSP_ParserInfo::GetRequestType() const
{
	return RequestType;
}
const std::string& VS_RTSP_ParserInfo::GetUrl() const
{
	return Url;
}
void VS_RTSP_ParserInfo::SetUrl(std::string iUrl)
{
	Url = std::move(iUrl);
}
eResponseType VS_RTSP_ParserInfo::GetResponseType() const
{
	return ResponseType;
}
void VS_RTSP_ParserInfo::SetResponseType(eResponseType resp)
{
	ResponseType = resp;
}
const std::string& VS_RTSP_ParserInfo::GetUserAgent() const
{
	return UserAgent;
}
void VS_RTSP_ParserInfo::SetUserAgent(std::string iUserAgent)
{
	UserAgent = std::move(iUserAgent);
}
const std::string& VS_RTSP_ParserInfo::GetAccept() const
{
	return Accept;
}
void VS_RTSP_ParserInfo::SetAccept(std::string iAccept)
{
	Accept = std::move(iAccept);
}

void VS_RTSP_ParserInfo::SetControlUrl(std::string url)
{
	controlUrl = std::move(url);
}

const std::string& VS_RTSP_ParserInfo::GetControlUrl() const
{
	return controlUrl;
}

void VS_RTSP_ParserInfo::SetUser(std::string iUser)
{
	user = std::move(iUser);
}

void VS_RTSP_ParserInfo::SetPassword(std::string iPassword)
{
	password = std::move(iPassword);
}

const std::string& VS_RTSP_ParserInfo::GetUser() const
{
	return user;
}

const std::string& VS_RTSP_ParserInfo::GetPassword() const
{
	return password;
}

void VS_RTSP_ParserInfo::SetAuthScheme(const std::shared_ptr<VS_SIPAuthScheme>& s)
{
	authScheme = s;
}

std::shared_ptr<VS_SIPAuthScheme> VS_RTSP_ParserInfo::GetAuthScheme() const
{
	return authScheme;
}

void VS_RTSP_ParserInfo::SetAuthAttempts(unsigned n)
{
	auth_attempts = n;
}

unsigned VS_RTSP_ParserInfo::GetAuthAttempts() const
{
	return auth_attempts;
}

const std::string &VS_RTSP_ParserInfo::GetRemoteUserAgent(void) const
{
	return remote_UserAgent;
}

void VS_RTSP_ParserInfo::SetRemoteUserAgent(const std::string &ua)
{
	remote_UserAgent = ua;
}

net::port VS_RTSP_ParserInfo::GetPort() const { return m_endpoint.port(); }
net::address VS_RTSP_ParserInfo::GetIp() const { return m_endpoint.address(); };

void VS_RTSP_ParserInfo::SetEndpoint(const boost::asio::ip::udp::endpoint& ep) {
	m_endpoint = ep;
}
boost::asio::ip::udp::endpoint VS_RTSP_ParserInfo::GetEndpoint() const {
	return m_endpoint;
}

bool VS_RTSP_ParserInfo::UseRemoteTransceiver() const
{
	return useRemoteTransceiver;
}

void VS_RTSP_ParserInfo::UseRemoteTransceiver(bool val)
{
	useRemoteTransceiver = val;
}
