#if !defined(_WIN32)
#include "LdapNTLMAuthorizer.h"

#include "net/Connect.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std/cpplib/event.h"
#include "ldap_core/liblutil/ldap_bind.h"
#include "std/cpplib/VS_Protocol.h"

#include "std/debuglog/VS_Debug.h"
#define DEBUG_CURRENT_MODULE VS_DM_RSTOR

namespace tc_ldap {
bool NTLMAuthorizer::Init(const std::string & ldap_hostname, const std::string& service)
{
	m_ldap_hostname = ldap_hostname;
	m_service = service;
	return !m_ldap_hostname.empty() && !m_service.empty();
}
int NTLMAuthorizer::DoAuthorize(const std::string & epID, const void * inCrededinals, const size_t credSize, VS_Container & outCnt)
{
	using Protocol = boost::asio::ip::tcp;
	CheckThread();
	if (m_ldap_hostname.empty() || m_service.empty())
		return LDAP_LOCAL_ERROR;

	if(!inCrededinals || credSize == 0)
		return LDAP_PARAM_ERROR;

	auto it = m_binders.find(epID);
	if (it == m_binders.end())
	{
		vs::event conneted(false);
		net::Connect<Protocol>(m_strand, m_ldap_hostname, m_service,
			[this, w_self = weak_from_this(), &conneted, &epID]
		(const boost::system::error_code& ec, typename Protocol::socket&& socket) {
			VS_SCOPE_EXIT{ conneted.set(); };
			auto self = w_self.lock();
			if (!self)
				return;
			if (ec)	{
				dstream4 << "ldap::NTLMAuthorizer:" << " connect failed: " << ec.message();
				return;
			}

			BindCtx<Protocol> ctx(std::move(socket));
			m_binders.emplace(epID, std::move(ctx));
		});
		conneted.wait();
	}

	int res = LDAP_PARAM_ERROR;
	it = m_binders.find(epID);
	if (it == m_binders.end())
		return LDAP_CONNECT_ERROR;

	VS_SCOPE_EXIT{
		if (res != LDAP_SASL_BIND_IN_PROGRESS)	// we need context only when progressing with authentication
			m_binders.erase(it);
	};
	const struct berval ccred { credSize , (char *)inCrededinals };
	struct berval* scred = nullptr;

	auto &ctx = it->second;
	res = make_ntlm_auth_step(ctx.sb, &ccred, &ctx.msgID, &scred);
	if (res != LDAP_SUCCESS)
		return res;

	// msgID == 1: Negotiate step	 => returning LDAP_SASL_BIND_IN_PROGRESS
	if (ctx.msgID == 1) {
		outCnt.AddValueI32(TYPE_PARAM, LA_NTLM);
		outCnt.AddValue(DATA_PARAM, scred->bv_val, scred->bv_len);
		return res = LDAP_SASL_BIND_IN_PROGRESS;
	}

	// msgID == 2: Authenticate step => returning LDAP_SUCCESS
	return res = LDAP_SUCCESS;
}
}
#endif