#pragma once

#include "VS_SIPAuthScheme.h"

#include <boost/optional.hpp>

// For NTLM, Kerberos, TLS-DSK
class VS_SIPAuthGSS : public VS_SIPAuthScheme
{
public:
	explicit VS_SIPAuthGSS(eSIP_AUTH_SCHEME scheme);
	~VS_SIPAuthGSS() {}

	TSIPErrorCodes Decode(VS_SIPBuffer &aBuffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const override;

	TSIPErrorCodes Init(const VS_SIPGetInfoInterface& call) override;
	eSIP_AUTH_SCHEME scheme() const override { return m_scheme; }
	std::string scheme_str() const;
	const boost::optional<std::string> &gssapi_data() const { return m_gssapi_data; }
	void gssapi_data(const char *p)
		{ if (p) { m_gssapi_data = std::string(p); } else { m_gssapi_data.reset(); } }
	void gssapi_data(const char *p, size_t len) { m_gssapi_data = std::string(p, len); }

	const std::string &crand() const { return m_crand; }
	void crand(std::string rand) { m_crand = std::move(rand); }
	std::uint32_t cnum() const { return m_cnum; }
	void cnum(std::uint32_t num) { m_cnum = num; }
protected:
	eSIP_AUTH_SCHEME m_scheme;
	boost::optional<std::string> m_gssapi_data;
	std::string m_crand, m_srand;
	std::uint32_t m_cnum;
};