#ifdef _WIN32

#include "VS_LDAPCoreImp_WinLDAP.h"
#include "std/debuglog/VS_Debug.h"
#include "std/cpplib/VS_Errors.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std/cpplib/curl_deleters.h"
#include "std/cpplib/base64.h"
#include "ldap_core/liblutil/tc_ldap.h"

#include <curl/curl.h>
#include <boost/algorithm/string.hpp>

#include <boost/function.hpp>
#include <boost/bind.hpp>

// todo(kt): delete this line
#define LDAP_UNICODE 1

#include <Sddl.h>
#include <WinBER.h>

#define UNICODE 1

#if !LDAP_UNICODE
#error "LDAP storage designed for unicode compilation"
#endif

#if !UNICODE
#error "LDAP storage designed for unicode compilation"
#endif

#define DEBUG_CURRENT_MODULE VS_DM_RSTOR

namespace tc {

boost::function<void(LDAP*&, PCHAR /*HostName*/, ULONG /*PortNumber*/)> g_ldap_referral_functor;

ULONG g_ldap_referral_qfc(PLDAP       PrimaryConnection,
	PLDAP       ReferralFromConnection,
	PWCHAR      NewDN,
	PCHAR       HostName,
	ULONG       PortNumber,
	PVOID       SecAuthIdentity,    // If NULL, use CurrentUser below
	PVOID       CurrentUserToken,   // pointer to current user LUID.
	PLDAP       *ConnectionToUse)
{
	LDAP* ldap(0);
	g_ldap_referral_functor(ldap, HostName, PortNumber);
	*ConnectionToUse = ldap;
	return 0;
}

BOOLEAN auto_verify_server_cert(PLDAP Connection, PCCERT_CONTEXT* pServerCert)
{
	return true;
}

void VS_LDAPCoreImp_WinLDAP::InitLib()
{
	if (m_referrals_enabled)
		g_ldap_referral_functor = bind(&VS_LDAPCoreImp_WinLDAP::LDAPReferral, this, ::_1, ::_2, ::_3);
}

ldap_error_code_t VS_LDAPCoreImp_WinLDAP::LDAPSearchPagedImp(LDAP* ld, const std::string& dn_, const long& scope, const std::string& filter_, const char** attrs_, std::vector<attrs_t>& out, page_cookie_t& cookie, const long page_size, const std::pair<std::string, bool>* sort_attr, const bool changed_ctx)
{
	dstream4 << "begin\tLDAPSearchPagedImp: filter=" << filter_ << ", dn=" << dn_ << "\n";
	VS_SCOPE_EXIT{ dstream4 << "end\tLDAPSearchPagedImp: filter=" << filter_ << ", dn=" << dn_ << "\n"; };
	long page_size_to_use = page_size;
	bool all_pages = false;
	if (page_size_to_use != -1) {
		all_pages = page_size == 0;
		if (all_pages)
			page_size_to_use = 1000;
	}

	// convert utf8 interface to wchar_t of WinLDAP
	auto dn = vs::UTF8ToWideCharConvert(dn_);
	auto filter = vs::UTF8ToWideCharConvert(filter_);
	auto sort_attr_w = vs::UTF8ToWideCharConvert(sort_attr ? sort_attr->first.c_str() : "");
	std::vector<std::wstring> tmp_attrs;
	if (attrs_)
	{
		for (unsigned int i = 0; attrs_[i]; ++i)
			tmp_attrs.emplace_back(vs::UTF8ToWideCharConvert(attrs_[i]));
	}
	std::vector<wchar_t*> attrs;
	for (auto const& s : tmp_attrs)
		 attrs.emplace_back(const_cast<wchar_t*>(s.c_str()));
	attrs.push_back(0);

	LDAPMessage* lmsg = 0;
	ldap_error_code_t err(LDAP_SUCCESS);

	std::shared_ptr<RAII_SearchHandle> search_handle;

	try
	{
		auto start = std::chrono::high_resolution_clock::now();

		{
			VS_AutoLock lock(&m_page_map_lock);
			auto it_search = m_page_map.find(cookie);
			if (it_search != m_page_map.end()) {
				search_handle = it_search->second;
			}
			if (!search_handle || changed_ctx)
			{
				if (!changed_ctx && cookie != page_cookie_t())		// cookie provided, but not found
					return LDAP_PARAM_ERROR;
				PLDAPSortKey sort_keys[2] = { 0, 0 };
				LDAPSortKey sk;
				if (sort_attr && m_is_server_sort_control_supported)
				{
					sk.sk_attrtype = (PWCHAR)sort_attr_w.c_str();
					sk.sk_matchruleoid = NULL;
					sk.sk_reverseorder = sort_attr->second;
					sort_keys[0] = &sk;
				}
				PLDAPSearch sys_search_handle = ldap_search_init_page(ld,
					(const PWSTR)dn.c_str(),
					scope,
					(PWCHAR)filter.c_str(),
					(wchar_t**)attrs.data(),
					false, 0, 0, 0, 0, (m_is_server_sort_control_supported) ? sort_keys : 0);
				if (sys_search_handle)
				{
					if (changed_ctx) {
						search_handle->handle(sys_search_handle);
					}
					else {
						search_handle = std::make_shared<RAII_SearchHandle>(ld, sys_search_handle, dn_);
						static page_cookie_t new_cookie;
						++new_cookie;
						m_page_map[new_cookie] = search_handle;
						cookie = new_cookie;
					}
				}
			}
		} // end of lock

		if (!search_handle)
			throw VSS_LDAP_ERROR;

		ULONG total_count(0);
		unsigned long page_num(0);
		long last_page_results(0);

		std::vector<referral_t> r;	// external referrals

									// todo(kt): skip while, if  (what specified?)
		do
		{
			l_timeval t;
			t.tv_sec = m_ldap_timeout.count();
			t.tv_usec = 0;
			if (page_size_to_use != -1)
				err = ldap_get_next_page_s(search_handle->ctx(), search_handle->handle(), &t, page_size_to_use, &total_count, &lmsg);
			else
				err = ldap_search_ext_s(search_handle->ctx(), (const PWSTR)dn.c_str(), scope, (PWCHAR)filter.c_str(), attrs.data(), 0, nullptr, nullptr, &t, 0, &lmsg);
			auto err_result = ldap_result2error(search_handle->ctx(), lmsg, 0);
			if (err_result != LDAP_SUCCESS && err_result != LDAP_REFERRAL && err_result != LDAP_OPERATIONS_ERROR)
				throw VSS_LDAP_ERROR;
			auto ds_results = dstream4;
			{ // get external referrals
				PWCHAR* pReferrals = 0;
				auto err_parse = ldap_parse_result(search_handle->ctx(), lmsg, /*err*/NULL, /*&matchedDNs*/NULL, /*&errorMsg*/NULL, &pReferrals, NULL, false);
				int ref_index(0);
				while (pReferrals && pReferrals[ref_index])
				{
					referral_t new_ref;
					if (ParseReferral(pReferrals[ref_index], new_ref) && !new_ref.host.empty() && !new_ref.baseDN.empty() && !isUselessDN(new_ref.baseDN))
					{
						ds_results << "\text_referral: " << new_ref.baseDN << "\n";
						r.emplace_back(std::move(new_ref));
					}
					++ref_index;
				}
				ldap_value_free(pReferrals);
			}
			unsigned long old_sz = out.size();

			for (LDAPMessage* iter = ldap_first_entry(search_handle->ctx(), lmsg); iter; iter = ldap_next_entry(search_handle->ctx(), iter))
			{
				wchar_t* dn = ldap_get_dn(search_handle->ctx(), iter);
				VS_SCOPE_EXIT{ if (dn) ldap_memfree(dn); };
				ds_results << "ResEntry[" << dn << "]\n";

				attrs_t v;
				BerElement* ber = nullptr;
				VS_SCOPE_EXIT{ if (ber != nullptr) ber_free(ber, 0); };
				for (wchar_t* attribute = ldap_first_attribute(search_handle->ctx(), iter, &ber);
					attribute != NULL;
					attribute = ldap_next_attribute(search_handle->ctx(), iter, ber))
				{
					VS_SCOPE_EXIT{ ldap_memfree(attribute); };
					struct berval** ldap_value = ldap_get_values_len(search_handle->ctx(), iter, attribute);
					if (!ldap_value)
						continue;
					VS_SCOPE_EXIT{ ldap_value_free_len(ldap_value); };
					int num_values = ldap_count_values_len(ldap_value);
					static auto a = vs::UTF8ToWideCharConvert(m_a_objectSid);
					for (int i = 0; i < num_values; ++i) {
						ds_results << "\tattr: " << attribute << "=";
						if (a == attribute)
							v.emplace_back(vs::WideCharToUTF8Convert(attribute),
								ConvectSIDtoString(string_view(ldap_value[i]->bv_val, ldap_value[i]->bv_len)));
						else if (ldap_value[i]->bv_val)
						{
							ds_results << ldap_value[i]->bv_val;

							std::string utf8_attr = vs::WideCharToUTF8Convert(attribute);

							if (IsAvatarsAttr(utf8_attr))
							{
								size_t len = 0;
								base64_encode(ldap_value[i]->bv_val, ldap_value[i]->bv_len, nullptr, len);
								attr_value_t base64;
								base64.resize(len);

								if (base64_encode(ldap_value[i]->bv_val, ldap_value[i]->bv_len, &base64[0]/*data()*/, len))
								{
									v.emplace_back(std::move(utf8_attr), std::move(base64));
								}
							}
							else
							{
								v.emplace_back(std::move(utf8_attr), ldap_value[i]->bv_val);
							}
						}
						ds_results << "\n";
					}
				}

				if (dn && *dn)
					v.emplace_back(m_a_distinguishedName, vs::WideCharToUTF8Convert(dn));

				if (!v.empty())
					out.push_back(v);
			}
			last_page_results = out.size() - old_sz;
			++page_num;
			dprint4("LDAP: page %lu sz = %ld\n", page_num, last_page_results);

			if (m_referrals_enabled)
			{
				for (LDAPMessage* iter = ldap_first_reference(search_handle->ctx(), lmsg); iter; iter = ldap_next_reference(search_handle->ctx(), iter))
				{
					PWCHAR* refs = 0;
					if (ldap_parse_reference(search_handle->ctx(), iter, &refs) == LDAP_SUCCESS)
					{
						referral_t new_ref;
						if (ParseReferral(*refs, new_ref) && !new_ref.host.empty() && !new_ref.baseDN.empty() && !isUselessDN(new_ref.baseDN))
						{
							ds_results << "\tlocal_referral: " << new_ref.baseDN << "\n";
							r.emplace_back(std::move(new_ref));
						}
					}
					if (refs)
						ldap_value_free(refs);
				}
			}
			if (lmsg)
				ldap_msgfree(lmsg);
			lmsg = 0;

			bool fail_by_sz = out.size() > m_max_results;
			bool fail_by_time = decltype(start)::clock::now() - start > m_ldap_timeout;
			if (fail_by_sz || fail_by_time)
			{
				dstream4 << "fail_by_sz=" << fail_by_sz << ", fail_by_time=" << fail_by_time;
				{
					VS_AutoLock lock(&m_page_map_lock);
					m_page_map.erase(cookie);
				}
				cookie = page_cookie_t();
				err = (fail_by_sz) ? LDAP_SIZELIMIT_EXCEEDED : LDAP_TIMELIMIT_EXCEEDED;
				throw VSS_LDAP_ERROR;
			}
		} while (all_pages && err == LDAP_SUCCESS);

		auto end = std::chrono::high_resolution_clock::now();
		dstream4 << "LDAPSearchPagedImp=" << out.size() << " (pages:" << page_num <<
			", mills:" << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() <<
			", dn=" << dn << ", filter=" << filter << "\n";

		if (m_referrals_enabled)
		{
			{
				VS_AutoLock lock(&m_page_map_lock);		 // kt: that lock or other or atomic??
				for (const auto& new_ref : r)
				{
					auto added = search_handle->add_refer(new_ref);
					dstream4 << "\treferral: " << new_ref.host << ":" << new_ref.port << " baseDN=" << new_ref.baseDN << " chased=" << added << "\n";
				}
			}

			std::vector<referral_t> refers_to_remove;
			int n_recursive(100);
			while (!search_handle->refs().empty() && --n_recursive && (search_handle->num_chased_refs() < m_referrals_hops))
			{
				long page_size_for_referral = (all_pages) ? page_size : page_size - last_page_results;
				if (page_size_for_referral < 0)
					page_size_for_referral = 0;
				if (!all_pages && page_size_for_referral <= 0)
					break;
				auto ref = search_handle->refs()[0];
				bool remove_refer(false);
				if (all_pages || last_page_results < page_size)		// chase referrals
				{
					search_handle->remove_refer(ref);

					std::pair<std::string, unsigned long> key(ref.host, ref.port);
					LDAP* new_ctx(0);
					{
						auto lock = m_ldap_refferal.lock();
						auto it = lock->find(key);
						if (it != lock->end())
						{
							static l_timeval check_time;
							check_time.tv_sec = 1;
							check_time.tv_usec = 0;
							//unsigned long t1 = GetTickCount();
							long conn_res = ldap_connect(it->second, &check_time);
							//unsigned long t2 = GetTickCount();
							//dprint4("LDAP Referral connect result:%d to %s, diff_tick:%d\n", conn_res, HostName, t2 - t1);
							if (conn_res == LDAP_SUCCESS) {
								new_ctx = it->second;
							}
							else {
								ldap_unbind(it->second);
								lock->erase(it);
							}
						}
					}
					if (!new_ctx && ConnectServer(new_ctx, ref.host.c_str(), ref.port) == 0)	// no error
						m_ldap_refferal->emplace(key, new_ctx);

					if (!!new_ctx) {
						{
							VS_AutoLock lock(&m_page_map_lock);
							auto it = m_page_map.find(cookie);
							if (it == m_page_map.end())
								break;
							it->second->new_ctx(new_ctx);
							it->second->base_dn(ref.baseDN);
						}
						auto amount_before_refer = out.size();
						std::vector<attrs_t> out_from_ref;
						auto err_refer = this->LDAPSearchPagedImp(new_ctx, ref.baseDN, scope, filter_, attrs_, out_from_ref, cookie, page_size_for_referral, sort_attr, true);
						for (auto&& i : out_from_ref)
						{
							i.emplace_back("trueconf_referral_host", ref.host);
							out.push_back(i);
						}
						if (err != LDAP_SUCCESS)
							remove_refer = true;
						else {
							long amount_from_refer = (out.size() - amount_before_refer);
							if (amount_from_refer < 0)
								amount_from_refer = 0;
							if (amount_from_refer >= page_size_for_referral)		// todo(kt): invert the condition?
								remove_refer = true;
							last_page_results += amount_from_refer;
						}
					}
					else {
						remove_refer = true;
					}
					if (remove_refer)
						refers_to_remove.push_back(ref);
				}
				else {
					break;
				}
			}	// while have any referrals

			{
				VS_AutoLock lock(&m_page_map_lock);
				auto it = m_page_map.find(cookie);
				if (it != m_page_map.end())
					for (const auto& ref : refers_to_remove)
						it->second->remove_refer(ref);
			}
		}	// process referrals
	}
	catch (int error)
	{
		//		error_code = error;
		if (error == VSS_LDAP_ERROR)
		{
			if (err == LDAP_SUCCESS)
				err = LdapGetLastError();
			//			ProcessLDAPError();
		}
	};

	if (lmsg)
		ldap_msgfree(lmsg);
	lmsg = 0;

	if (all_pages || err == LDAP_NO_RESULTS_RETURNED)
	{
		VS_AutoLock lock(&m_page_map_lock);
		auto it_search = m_page_map.find(cookie);
		if (it_search != m_page_map.end())
			m_page_map.erase(it_search);
	}

	if (err == LDAP_NO_RESULTS_RETURNED || err == LDAP_REFERRAL)		// previous call got all results, so that call just return empty results
		err = LDAP_SUCCESS;

	return err;
}

bool VS_LDAPCoreImp_WinLDAP::ParseReferral(const std::wstring& ref, referral_t& obj) const
{
	if (ref.empty())
		return false;
	auto pos = ref.find_first_of(L"://");
	if (pos == std::string::npos)
		return false;
	referral_t r;
	// kt: scheme check: ugly, case-sens, but should work most of times
	auto pos_sec = ref.find(L"ldaps");
	if (pos_sec != std::string::npos && pos_sec < pos)
		r.IsSecure = true;

	pos += wcslen(L"://");

	std::wstring hostport;
	auto pos2 = ref.find_first_of(L"/", pos);
	if (pos2 != std::string::npos) {
		hostport = ref.substr(pos, pos2 - pos);
		pos2 += wcslen(L"/");
	}
	else {
		hostport = ref.substr(pos, ref.length() - pos);
		pos2 = ref.length();
	}

	// parse hostport
	{
		std::vector<std::wstring> strs;
		boost::split(strs, hostport, boost::is_any_of(L":"), boost::token_compress_on);
		if (strs.size() > 1 && !strs[1].empty()) {
			std::string out = vs::WideCharToUTF8Convert(strs[1]);
			if (!out.empty())
				r.port = atoi(out.c_str());
		}
		if (strs.size() >= 1 && !strs[0].empty()) {
			r.host = vs::WideCharToUTF8Convert(strs[0]);
		}
	}

	if (!r.port)
		r.port = (r.IsSecure) ? LDAP_SECURE_PORT_INIT : LDAP_PORT_INIT;

	std::wstring rest = ref.substr(pos2, ref.length() - pos2);
	if (rest.empty())
	{
		obj = std::move(r);
		return true;
	}

	std::vector<std::wstring> strs;
	boost::split(strs, rest, boost::is_any_of(L"/?"), boost::token_compress_on);
	if (strs.size() >= 1 && !strs[0].empty())
	{
		auto str = vs::WideCharToUTF8Convert(strs[0]);
		std::unique_ptr<CURL, CURL_deleter> curl(::curl_easy_init());
		if (curl) {
			int new_len = 0;
			std::unique_ptr<char, curl_free_deleter> unescaped_baseDN(::curl_easy_unescape(curl.get(), str.c_str(), str.length(), &new_len));
			if (unescaped_baseDN)
				r.baseDN = unescaped_baseDN.get();
		}
	}

	obj = std::move(r);
	return true;
}

void VS_LDAPCoreImp_WinLDAP::LDAPReferral(LDAP*& ctx, PCHAR HostName, ULONG PortNumber)
{
	dprint3("LDAP Referral to %s:%lu\n", HostName, PortNumber);
	if (!HostName || !*HostName || !PortNumber)
		return;
	std::pair<std::string, unsigned long> key(HostName, PortNumber);
	auto lock = m_ldap_refferal.lock();
	LDAP* p_ldap(0);
	auto it = lock->find(key);
	if (it != lock->end())
		p_ldap = it->second;
	if (ConnectServer(p_ldap, HostName, PortNumber) == 0)
	{
		ctx = p_ldap;
		(*lock)[key] = p_ldap;
	}
	else
		lock->erase(key);
}

void VS_LDAPCoreImp_WinLDAP::ProcessLDAPError(tc::ldap_error_code_t& ldap_error)
{
	// todo(kt): check functor
}

int VS_LDAPCoreImp_WinLDAP::ConnectServer(LDAP*& ctx, const char* server_host, const unsigned long server_port)
{
	if (ctx)
	{
		ldap_unbind(ctx);
		ctx = 0;
	}
	std::string host;
	unsigned long port(0);
	if (server_host && server_port) {
		host = server_host;
		port = server_port;
	}
	else {
		host = m_ldap_server;
		port = m_ldap_port;
	}
	ctx = tc_ldap::Connect(host, port, m_ldap_secure);
	if (ctx) {
		ULONG err;
		ldap_get_option(ctx, LDAP_OPT_ERROR_NUMBER, &err);
		if (err != LDAP_SUCCESS) {
			ldap_unbind(ctx);
			dstream0 << "LDAP connect failed to " << host << ":" << port << ": " << ldap_err2string(err);
			return VSS_LDAP_INIT_ERROR;
		}	
	}
	else
		return VSS_LDAP_INIT_ERROR;

	// Set the version to 3.0 (default version is 2.0).
	unsigned long ldresult = ldap_set_option(ctx, LDAP_OPT_PROTOCOL_VERSION, (void*)&m_ldap_version);
	if (ldresult != LDAP_SUCCESS)
	{
		dprint1("\t\tcan't set auth LDAP connection version to 3\n");
		//		if (DoCheckLDAPError)
		//			ProcessLDAPError(ldresult);
	}

	// tell WinLDAP not to automatically chase refs with simple_bind
	// we will do it manually with sasl_bind
	ldresult = ldap_set_option(ctx, LDAP_OPT_REFERRALS, LDAP_OPT_OFF);
	if (ldresult != LDAP_SUCCESS)
	{
		dprint1("\t\tcan't set LDAP_OPT_REFERRALS=LDAP_OPT_OFF\n");
		//		if (DoCheckLDAPError)
		//			ProcessLDAPError(ldresult);
	}

	if (m_ldap_auto_verify_server_cert)
	{
		ldresult = ldap_set_option(ctx, LDAP_OPT_SERVER_CERTIFICATE, (void*)&auto_verify_server_cert);
		dprint3("Set LDAP_OPT_SERVER_CERTIFICATE res:%lu\n", ldresult);
	}

	dstream3 << "ldap_bind " << host << ":" << std::to_string(port);
	ldresult = tc_ldap::Bind(ctx, m_auth_user, m_auth_password, m_auth_domain, m_auth_method);
	if (ldresult != LDAP_SUCCESS)
	{
		dstream0 << "LDAP bind failed to: " << host << ':' << port << ": " << ldap_err2string(ldresult);
		if (ctx)
		{
			ldap_unbind(ctx);
			ctx = 0;
		}
		return (ldresult == LDAP_SERVER_DOWN) ? VSS_LDAP_CONNECT_SERVER_ERROR : VSS_LDAP_INIT_ERROR;
	};

#ifdef _WIN32
	// Set referrals callback
	LDAP_REFERRAL_CALLBACK cb;
	cb.SizeOfCallbacks = sizeof(LDAP_REFERRAL_CALLBACK);
	cb.QueryForConnection = g_ldap_referral_qfc;
	cb.NotifyRoutine = 0;
	cb.DereferenceRoutine = 0;
	ldresult = ldap_set_option(ctx, LDAP_OPT_REFERRAL_CALLBACK, (void*)&cb);
	dprint3("Set LDAP_OPT_REFERRAL_CALLBACK res:%lu\n", ldresult);

	// turn off auto-process referral by Windows (cause it rebind with Anonymous);
	// better we process callback from Windows and decide if we need referrals
	ldresult = ldap_set_option(ctx, LDAP_OPT_REFERRALS, LDAP_OPT_OFF);
	dprint3("Set LDAP_OPT_REFERRALS=%d res:%lu\n", m_referrals_enabled, ldresult);

	ldresult = ldap_set_option(ctx, LDAP_OPT_REFERRAL_HOP_LIMIT, (void*)&m_referrals_hops);
	dprint3("Set LDAP_OPT_REFERRAL_HOP_LIMIT=%u res:%lu\n", m_referrals_hops, ldresult);
#endif
	return 0;		// 0 - no error
}


} // namespace tc
#endif // ifdef _WIN32
