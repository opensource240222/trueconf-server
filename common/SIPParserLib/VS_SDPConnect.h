#pragma once

#include "../SIPParserBase/VS_BaseField.h"

#include <boost/regex.hpp>

class VS_SDPConnect: public VS_BaseField
{
public:
	const static boost::regex e;

	TSIPErrorCodes Decode(VS_SIPBuffer &aBuffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const override;


	TSIPErrorCodes Init(const VS_SIPGetInfoInterface& call) override;
	VS_SDPConnect & operator=(const VS_SDPConnect &conn) ;
	bool operator!=(const VS_SDPConnect &conn) const;

	void Clean() noexcept override;

	VS_SDPConnect();
	VS_SDPConnect(const VS_SDPConnect &) = delete;
	~VS_SDPConnect();

	const std::string &GetHost() const;
	void SetHost(std::string host);

private:
	std::string							m_host;
	unsigned int						m_ttl;
	unsigned int						m_addr_num;
};