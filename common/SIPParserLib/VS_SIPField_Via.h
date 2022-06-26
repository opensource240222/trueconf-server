#pragma once

#include "../SIPParserBase/VS_BaseField.h"
#include <boost/regex.hpp>
#include <string>
#include "net/Address.h"
#include "net/Port.h"
#include "net/Protocol.h"

class VS_SIPField_Via : public VS_BaseField
{
public:
	const static std::string exp;
	const static boost::regex e1;
	const static boost::regex e2;
	const static boost::regex e3;

	TSIPErrorCodes Decode(VS_SIPBuffer &aBuffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const override;

	TSIPErrorCodes Init(const VS_SIPGetInfoInterface& call) override;

	void Clean() noexcept override;

	VS_SIPField_Via();
	VS_SIPField_Via(VS_SIPField_Via &) = delete;
	~VS_SIPField_Via();

	VS_SIPField_Via & operator=(const VS_SIPField_Via &via);

	// Get/Set Methods
	void Branch(std::string aBranch);
	const std::string &Branch() const;
	bool IsBranch() const;

	bool IsKeepAlive() const;
	size_t KeepAliveInterval() const;

	void ConnectionType(const net::protocol conn_type);
	net::protocol ConnectionType() const;

	void Host(std::string host);

	const std::string &Host() const;

	void Port(const net::port port);
	net::port Port() const;

	void Received(std::string received);
	const std::string &Received() const;

	int order() const override
	{
		return 20;
	}

	static std::string make_request_branch(string_view my_branch);
private:
	std::string									m_host;
	std::string									m_branch;
	std::string									m_params;
	std::string									m_received;
	size_t										m_keep_alive_interval;
	net::port									m_port;
	net::protocol								m_conn_type;
	bool										m_keep_alive_response;
	bool										m_keep_alive;
	bool										m_compact;

	bool FindParam_branch(string_view aInput);
	bool FindParam_keep_alive(string_view aInput);
	bool operator!=(const VS_SIPField_Via &via) const;
};

std::unique_ptr<VS_BaseField> VS_SIPField_Via_Instance();