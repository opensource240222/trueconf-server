#pragma once

#include "VS_RTSP_Const.h"

#include <string>
#include "net/Address.h"
#include "net/Port.h"
#include "tools/Server/CommonTypes.h"

//добавить локи

class VS_SIPAuthScheme;

class VS_RTSP_ParserInfo
{
public:
	//Headers
	std::uint32_t GetCseq() const;
	void SetCSeq(std::uint32_t iCseq);
	void SetContentType(std::string input);
	std::size_t GetContentLength() const;
	void SetContentLength(unsigned int input);
	const std::string& GetContentType() const;
	const std::string& GetSessionsID() const;
	void SetSessionsID(std::string sSessions);
	const std::string& GetServer() const;
	void SetServer(std::string sServer);
	CommandsSet GetSupportedCommands() const;
	void SetSupportedCommands(const CommandsSet& cmd_set);
	bool IsCommandSupported(eRequestType cmd) const;
	const std::string& GetUserAgent() const;
	void SetUserAgent(std::string sUserAgent);
	void SetUrl(std::string iUrl);
	const std::string& GetUrl() const;
	eResponseType GetResponseType() const;
	void SetResponseType(eResponseType responseT);
	void SetAccept(std::string iAccept);
	const std::string& GetAccept() const;

	net::port GetPort() const;
	net::address GetIp() const;

	//
	bool IsRequest() const;
	void IsRequest(bool bIsRequest);
	void IncreaseCSeq();
	//
	void SetRequestType(eRequestType ertIn);
	eRequestType GetRequestType() const;

	void SetControlUrl(std::string url);
	const std::string& GetControlUrl() const;

	void SetUser(std::string iUser);
	void SetPassword(std::string iPassword);
	const std::string& GetUser() const;
	const std::string& GetPassword() const;
	void SetAuthScheme(const std::shared_ptr<VS_SIPAuthScheme>& s);
	std::shared_ptr<VS_SIPAuthScheme> GetAuthScheme() const;
	void SetAuthAttempts(unsigned n);
	unsigned GetAuthAttempts() const;

	const std::string &GetRemoteUserAgent(void) const;
	void SetRemoteUserAgent(const std::string &ua);
	void SetEndpoint(const boost::asio::ip::udp::endpoint& ep);
	boost::asio::ip::udp::endpoint GetEndpoint() const;

	bool UseRemoteTransceiver() const;
	void UseRemoteTransceiver(bool val);

	VS_RTSP_ParserInfo();
	~VS_RTSP_ParserInfo();

private:
	std::string Accept;
	std::uint32_t CSeq;
	std::string ContentType;
	std::size_t ContentLength;
	std::string Server;
	std::string UserAgent;
	std::string SessionsId;
	CommandsSet SupportedCommands;
	bool bResponseOrRequest;
	eRequestType RequestType;
	eResponseType ResponseType;
	std::string Url;

	boost::asio::ip::udp::endpoint m_endpoint;

	std::string user;
	std::string password;
	std::shared_ptr<VS_SIPAuthScheme> authScheme;
	unsigned auth_attempts;
	std::string controlUrl;
	std::string remote_UserAgent;
	bool		useRemoteTransceiver = false;
};