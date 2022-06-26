#ifndef _WIN32

#include "VS_LDAPCoreImp_OpenLDAP.h"

#include "std/debuglog/VS_Debug.h"
#include "std/cpplib/VS_Errors.h"
#include "std/cpplib/base64.h"
#include "std-generic/cpplib/scope_exit.h"
#include "ldap_core/liblutil/tc_ldap.h"

// for DIGEST-MD5 bind
#include "ldap_core/liblutil/lutil_ldap.h"

#include <sys/time.h>	// for struct timeval



#define DEBUG_CURRENT_MODULE VS_DM_RSTOR

namespace tc{

bool VS_LDAPCoreImp_OpenLDAP::MakePageControl(std::shared_ptr<RAII_SearchHandle>& h, const long page_size_to_use, LDAPControl*& pageControl) {
	if (!h || !h->ctx()) {
		dstream4 << "VS_LDAPCoreImp_OpenLDAP::MakePageControl Error! Empty ctx!\n";
		return false;
	}

	auto sys_cookie = h->handle();
	return ldap_create_page_control(h->ctx(), page_size_to_use, &sys_cookie, false, &pageControl) == LDAP_SUCCESS;
}

ldap_error_code_t VS_LDAPCoreImp_OpenLDAP::LDAPSearchPagedImp(LDAP* ld, const std::string& dn, const long& scope, const std::string& filter, const char** attrs, std::vector<attrs_t>& out, page_cookie_t& cookie, const long page_size, const std::pair<std::string, bool>* sort_attr, const bool changed_ctx)
{
	dstream4 << "begin\tLDAPSearchPagedImp: filter=" << filter << ", dn=" << dn << "\n";
	VS_SCOPE_EXIT{ dstream4 << "end\tLDAPSearchPagedImp: filter=" << filter << ", dn=" << dn << "\n"; };
	long page_size_to_use = page_size;
	bool all_pages = false;
	if (page_size_to_use != -1) {
		all_pages = page_size == 0;
		if (all_pages)
			page_size_to_use = 1000;
	}

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

				if (changed_ctx) {
					berval empty_cookie = { 0, nullptr };
					search_handle->handle(empty_cookie);
				}
				else
				{
					berval empty_cookie = { 0, nullptr };
					search_handle = std::make_shared<RAII_SearchHandle>(ld, empty_cookie, dn);
					static page_cookie_t new_cookie;
					++new_cookie;
					m_page_map[new_cookie] = search_handle;
					cookie = new_cookie;
				}
			}
		} // end of lock

		if (!search_handle)
			throw VSS_LDAP_ERROR;

		uint32_t total_count(0);
		unsigned long page_num(0);
		long last_page_results(0);

		std::vector<referral_t> r;	// external referrals

		bool has_next_cookie(false);
		do
		{
			std::vector<LDAPControl*> ctrls = {};
			LDAPControl* pageControl = nullptr;
			VS_SCOPE_EXIT{ if (pageControl) ldap_control_free(pageControl); };

			if (page_size_to_use != -1) {
				auto res = MakePageControl(search_handle, page_size_to_use, pageControl);
				if(!res)
					throw VSS_LDAP_ERROR;
				ctrls.emplace_back(pageControl);
			}

			LDAPControl* sortControl = nullptr;
			VS_SCOPE_EXIT{ if (sortControl) ldap_control_free(sortControl); };
			if (sort_attr)
			{
				LDAPSortKey sortKey;
				sortKey.attributeType = (char*)sort_attr->first.c_str();
				sortKey.orderingRule = nullptr;
				sortKey.reverseOrder = sort_attr->second;

				LDAPSortKey* sortKeyList[] = { &sortKey, nullptr };
				auto sys_cookie = search_handle->handle();
				err = ldap_create_sort_control(search_handle->ctx(), sortKeyList, false, &sortControl);
				if (err != LDAP_SUCCESS)
					throw VSS_LDAP_ERROR;
				ctrls.emplace_back(sortControl);
			}
			ctrls.emplace_back(nullptr);

			struct timeval tv;
			tv.tv_sec = m_ldap_timeout.count();
			tv.tv_usec = 0;

			err = ldap_search_ext_s(search_handle->ctx(), dn.c_str(), scope, filter.c_str(), const_cast<char**>(attrs), false, ctrls.data(), nullptr,
				&tv,
				LDAP_NO_LIMIT,
				&lmsg);
			auto ds_results = dstream4;
			if (err != LDAP_SIZELIMIT_EXCEEDED &&
				err != LDAP_TIMELIMIT_EXCEEDED &&
				err != LDAP_SUCCESS &&
				err != LDAP_REFERRAL)
				throw VSS_LDAP_ERROR;
			unsigned long old_sz = out.size();

			for (LDAPMessage* iter = ldap_first_message(search_handle->ctx(), lmsg); iter; iter = ldap_next_message(search_handle->ctx(), iter))
			{
				char** refs = nullptr;
				VS_SCOPE_EXIT{ if (refs) ber_memvfree((void **)refs); };

				auto msg_type = ldap_msgtype(iter);
				if (msg_type == LDAP_RES_SEARCH_ENTRY) {
					auto dn = ldap_get_dn(search_handle->ctx(), iter);
					VS_SCOPE_EXIT{ if (dn) ldap_memfree(dn); };
					ds_results << "ResEntry[" << dn << "]\n";

					attrs_t v;
					BerElement* ber = nullptr;
					VS_SCOPE_EXIT{ if (ber != nullptr) ber_free(ber, 0); };;
					for (char* attribute = ldap_first_attribute(search_handle->ctx(), iter, &ber);
						attribute != NULL;
						attribute = ldap_next_attribute(search_handle->ctx(), iter, ber))
					{
						VS_SCOPE_EXIT{ ldap_memfree(attribute); };
						struct berval** ldap_value = ldap_get_values_len(search_handle->ctx(), iter, attribute);
						if (!ldap_value)
							continue;
						VS_SCOPE_EXIT{ ldap_value_free_len(ldap_value); };
						int num_values = ldap_count_values_len(ldap_value);

						for (int i = 0; i < num_values; ++i) {
							ds_results << "\tattr: " << attribute << "=";
							if (m_a_objectSid == attribute)
								v.emplace_back(attribute, ConvectSIDtoString(string_view(ldap_value[i]->bv_val, ldap_value[i]->bv_len)));
							else if (ldap_value[i]->bv_val)
							{
								ds_results << ldap_value[i]->bv_val;

								if (IsAvatarsAttr(attribute))
								{
									std::size_t len = 0;
									base64_encode(ldap_value[i]->bv_val, ldap_value[i]->bv_len, nullptr, len);

									attr_value_t base64;
									base64.resize(len);

									if (base64_encode(ldap_value[i]->bv_val, ldap_value[i]->bv_len, &base64[0]/*data()*/, len))
									{
										v.emplace_back(attribute, std::move(base64));
									}
								}
								else
								{
									v.emplace_back(attribute, ldap_value[i]->bv_val);
								}
							}
							ds_results << "\n";
						}
					}

					if (dn && *dn)
						v.emplace_back(m_a_distinguishedName, dn);
					if (!v.empty())
						out.push_back(v);
				}
				else if (msg_type == LDAP_RES_SEARCH_RESULT) {	// SearchResDone, get pagedControl value
					LDAPControl** responce_ctrls = nullptr;
					VS_SCOPE_EXIT{ if (responce_ctrls) ldap_controls_free(responce_ctrls); };

					err = ldap_parse_result(search_handle->ctx(), iter,
						/*&server_err*/nullptr, /*&matcheddn*/nullptr, /*error_string*/nullptr, &refs, &responce_ctrls, 0);
					if (responce_ctrls)
					{
						for (unsigned int i = 0; responce_ctrls[i] != NULL; i++) {
							if (!responce_ctrls[i]->ldctl_oid)
								continue;
							if (strcasecmp(responce_ctrls[i]->ldctl_oid, LDAP_CONTROL_PAGEDRESULTS) == 0)
							{
								ber_int_t estimate;
								struct berval c = { 0, nullptr };
								if (ldap_parse_pageresponse_control(search_handle->ctx(), responce_ctrls[i], &estimate, &c) == LDAP_SUCCESS && c.bv_len)
								{
									search_handle->handle(c);
									has_next_cookie = true;
								}
								else {
									if (c.bv_val) ldap_memfree(c.bv_val);
									has_next_cookie = false;
								}
							}
						}
					}
				}
				else if (msg_type == LDAP_RES_SEARCH_REFERENCE) {
					ldap_parse_reference(search_handle->ctx(), iter, &refs, nullptr, false);
				}

				if (refs)
					for (int i = 0; i < 32 && refs[i]; ++i)
					{
						LDAPURLDesc* d = nullptr;
						//	if (ParseReferral(*refs, new_ref) && !new_ref.host.empty() && !new_ref.baseDN.empty() && !isUselessDN(new_ref.baseDN))
						if (ldap_url_parse(refs[0], &d) == LDAP_SUCCESS)
						{
							referral_t new_ref;
							new_ref.host = d->lud_host;
							new_ref.port = d->lud_port;
							new_ref.baseDN = d->lud_dn;
							new_ref.IsSecure = strcasecmp(d->lud_scheme, "ldaps") == 0;
							ds_results << "\tgot ref: " << new_ref.baseDN << "\n";
							r.emplace_back(std::move(new_ref));
							ldap_free_urldesc(d);
						}
					}
			}
			last_page_results = out.size() - old_sz;
			++page_num;
			dprint4("LDAP: page %lu sz = %ld\n", page_num, last_page_results);
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
		} while (all_pages && err == LDAP_SUCCESS && has_next_cookie);

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
#ifdef _WIN32
							static l_timeval check_time;
							check_time.tv_sec = 1;
							check_time.tv_usec = 0;
							//unsigned long t1 = GetTickCount();
							long conn_res = ldap_connect(it->second, &check_time);
#else
							long conn_res = LDAP_SUCCESS;
#endif
							//unsigned long t2 = GetTickCount();
							// dprint4("LDAP Referral connect result:%d to %s, diff_tick:%d\n", conn_res, HostName, t2 - t1);
							if (conn_res == LDAP_SUCCESS) {
								new_ctx = it->second;
							}
							else {
								ldap_unbind_ext(it->second, nullptr, nullptr);
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
						auto err_refer = this->LDAPSearchPagedImp(new_ctx, ref.baseDN, scope, filter, attrs, out_from_ref, cookie, page_size_for_referral, sort_attr, true);
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
#ifdef _WIN32
		//		error_code = error;
		if (error == VSS_LDAP_ERROR)
		{
			if (err == LDAP_SUCCESS)
				err = LdapGetLastError();
			//			ProcessLDAPError();
		}
#endif
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

int VS_LDAPCoreImp_OpenLDAP::ConnectServer(LDAP*& ctx, const char* server_host, const unsigned long server_port)
{
	if (ctx)
	{
		ldap_unbind_ext(ctx, nullptr, nullptr);
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
	if (!ctx)
	{
		//		error_code=VSS_LDAP_INIT_ERROR;
		return VSS_LDAP_INIT_ERROR;
	};

	// Set the version to 3.0 (default version is 2.0).
	unsigned long ldresult = ldap_set_option(ctx, LDAP_OPT_PROTOCOL_VERSION, (void*)&m_ldap_version);
	if (ldresult != LDAP_SUCCESS)
	{
		dprint1("\t\tcan't set auth LDAP connection version to 3\n");
		//		if (DoCheckLDAPError)
		//			ProcessLDAPError(ldresult);
	}

	// tell OpenLDAP not to automatically chase refs with simple_bind
	// we will do it manually with sasl_bind
	ldresult = ldap_set_option(ctx, LDAP_OPT_REFERRALS, LDAP_OPT_OFF);
	if (ldresult != LDAP_SUCCESS)
	{
		dprint1("\t\tcan't set LDAP_OPT_REFERRALS=LDAP_OPT_OFF\n");
		//		if (DoCheckLDAPError)
		//			ProcessLDAPError(ldresult);
	}

	dstream3 << "ldap_bind " << host << ":" << std::to_string(port);
	ldresult = tc_ldap::Bind(ctx, m_auth_user, m_auth_password, m_auth_domain, m_auth_method);

	if (ldresult != LDAP_SUCCESS)
	{
		dstream0 << "LDAP bind failed to " << host << ":" << port << ": " << ldap_err2string(ldresult);
		if (ctx)
		{
			ldap_unbind_ext(ctx, nullptr, nullptr);
			ctx = 0;
		}
		return (ldresult == LDAP_SERVER_DOWN || ldresult == LDAP_TIMEOUT) ? VSS_LDAP_CONNECT_SERVER_ERROR : VSS_LDAP_INIT_ERROR;
	};

	return 0;		// 0 - no error
}

void VS_LDAPCoreImp_OpenLDAP::InitLib()
{
}

} // namespace tc

#endif // ifndef _WIN32