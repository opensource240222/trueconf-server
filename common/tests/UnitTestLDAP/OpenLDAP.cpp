#include "UnitTestLDAP.h"
#include "ldap_core/VS_LDAPCoreImp_Common.h"
#include "std-generic/cpplib/utf8.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std-generic/clib/strcasecmp.h"

#ifndef _WIN32	// linux only
#define LDAP_DEPRECATED 1
#include <ldap.h>
#endif

#ifndef _WIN32 // linux only
//#include "lutil.h"
#include "ldap_core/liblutil/lutil_ldap.h"
//#include "ldap_defaults.h"
#endif

#include <vector>

#ifndef _WIN32 // linux only
//#include "LDAPConnection.h"
//#include "LDAPConstraints.h"
//#include "LDAPSearchReference.h"
//#include "LDAPSearchResults.h"

//
//struct lutil_sasl_defaults_s
//{
//	char *mech = 0;
//	char *realm = 0;
//	char *authcid = 0;
//	char *passwd = 0;
//	char *authzid = 0;
//	char **resps = 0;
//	int nresps = 0;
//};

//TEST_F(OpenLDAPTest, DISABLED_CPPImp)
//{
//	LDAPConnection ctx("sub1.trust1.loc", 389);
//	try {
//		ctx.bind("tc_login_user", "qweASD123", /*cons*/nullptr);
//
//		//ldap_create_page_control_value(null, 3/*pageSize*/, &pr_cookie, &(c.ldctl_value));
//		struct berval	null_cookie = { 0, NULL };
//		struct berval* cookie = &null_cookie;
//		struct berval value = { 0, NULL };
//		BerElement* ber = ber_alloc_t(LBER_USE_DER);
//		std::cout << "ber is null: " << (ber == NULL) << "\n";
//		ber_int_t pagesize = 3;
//		ber_tag_t tag = ber_printf(ber, "{iO}", pagesize, cookie);
//		if (tag == LBER_ERROR) {
//			//ld->ld_errno = LDAP_ENCODING_ERROR;
//			//goto done;
//			int x = 3;
//		}
//		std::cout << "Got ber_printf\n";
//		std::cout << "ber is null: " << (ber == NULL) << "\n";
//		if (ber_flatten2(ber, &value, 0) == -1) {
//			//ld->ld_errno = LDAP_NO_MEMORY;
//			int x = 3;
//		}
//		std::cout << "Got ber_flatten2\n";
//		//VS_SCOPE_EXIT{
//		//if (ber != NULL) {
//		//	ber_free(ber, 1);
//		//}
//		//};
//
//
//		//struct berval pr_cookie;
//		//LDAPControl* c = nullptr;
//		//ldap_create_page_control(ctx.getSessionHandle(), 3/*pageSize*/, &pr_cookie, &c);
//		//VS_SCOPE_EXIT{ ldap_control_free(c); };
//
//
//
//		LDAPControlSet ctrls;
//		ctrls.add(LDAPCtrl(LDAP_CONTROL_PAGEDRESULTS, false, value.bv_val, value.bv_len));
//		//ctrls.add(LDAPCtrl(c));
//
//		LDAPConstraints cons;
//		cons.setServerControls(&ctrls);
//
//		LDAPSearchResults* entries = ctx.search("DC=sub1,DC=trust1,DC=loc", LDAPConnection::SEARCH_SUB, "(sAMAccountName=василий*)", {}/*attrs*/,false/*attrs_only*/, &cons);
//		if (entries != 0) {
//			LDAPEntry* entry = entries->getNext();
//			if (entry != 0) {
//				std::cout << *(entry) << std::endl;
//			}
//			while (entry) {
//				try {
//					entry = entries->getNext();
//					if (entry != 0) {
//						std::cout << *(entry) << std::endl;
//					}
//					delete entry;
//				}
//				catch (LDAPReferralException e) {
//					std::cout << "Caught Referral" << std::endl;
//				}
//			}
//		}
//
//		ctx.unbind();
//	}
//	catch (LDAPException &e) {
//		std::cout << "-------------- caught Exception ---------" << std::endl;
//		std::cout << e << std::endl;
//	}
//}
#endif

#ifndef _WIN32 // linux only
TEST_F(UnitTestLDAP, DISABLED_Debug)
{
	bool chaserefs(false);
	int version = LDAP_VERSION3;
	LDAP* ctx = nullptr;
	//ldap_create(&ctx);
	const char* conn_str = "ldap://trust1.loc/";
	const char* login = "kt1";
	const char* pass = "qweASD123";
	const char* base_dn = "DC=trust1,DC=loc";
	struct berval pass_ber = { strlen(pass), (char*)pass };
	//struct berval pass_ber = { 0, nullptr };


	auto res = ldap_initialize(&ctx, conn_str);
	ASSERT_EQ(res, LDAP_SUCCESS);
	//VS_SCOPE_EXIT{ if (ctx) ldap_destroy(ctx); };

	res = ldap_set_option(ctx, LDAP_OPT_PROTOCOL_VERSION, &version);
	ASSERT_EQ(res, LDAP_SUCCESS);
	res = ldap_set_option(ctx, LDAP_OPT_REFERRALS, chaserefs ? LDAP_OPT_ON : LDAP_OPT_OFF);
	ASSERT_EQ(res, LDAP_SUCCESS);

	//lutil_sasl_defaults_s d;
	//d.mech = "DIGEST-MD5";
	//d.realm = "trust1.loc";
	//d.authcid = "kt1";
	//d.passwd = "qweASD123";
	//d.authzid = "kt1";
	auto defaults = lutil_sasl_defaults(ctx,
		"DIGEST-MD5",
		"trust1.loc"/*sasl_realm*/,
		"kt2"/*sasl_authc_id*/,
		"qweASD123"/*passwd.bv_val*/,
		"kt2"/*sasl_authz_id*/);

	auto rc = ldap_sasl_interactive_bind_s(ctx, base_dn, "DIGEST-MD5", nullptr,
		nullptr/*sctrlsp*/, nullptr, 0/*sasl_flags*/, lutil_sasl_interact, defaults);

	lutil_sasl_freedefs(defaults);
	//return;
	//if (rc != LDAP_SUCCESS) {
	//	char *msg = NULL;
	//	ldap_get_option(ctx, LDAP_OPT_DIAGNOSTIC_MESSAGE, (void*)&msg);
	//	std::cout << "Error: ldap_sasl_interactive_bind_s: " << msg << "\n";
	//	//tool_perror("ldap_sasl_interactive_bind_s",
	//	//	rc, NULL, NULL, msg, NULL);
	//	ldap_memfree(msg);
	//	//tool_exit(ld, rc);
	//}

	////res = ldap_sasl_bind_s(ctx, nullptr/*login*/, "GSS-SPNEGO", nullptr/*&pass_ber*/, NULL, NULL, NULL);
	////res = ldap_bind_s(ctx, login, pass, LDAP_AUTH_NEGOTIATE);
	//res = ldap_sasl_interactive_bind_s(ctx, nullptr/*dn*/, "DIGEST-MD5", nullptr, nullptr, 0, nullptr, nullptr);
	//ASSERT_EQ(res, LDAP_SUCCESS);

	int pageSize = 3;
	struct berval pr_cookie = { 0, nullptr };
	///VS_SCOPE_EXIT{ for (auto const& i: ctrls)
	////	delete i;
	////};

	//-------------------
	LDAPControl* pageControl = nullptr;
	res = ldap_create_page_control(ctx, pageSize, &pr_cookie, false, &pageControl);
	VS_SCOPE_EXIT{ if (pageControl) ldap_control_free(pageControl); };
	ASSERT_EQ(res, LDAP_SUCCESS);
	ASSERT_NE(pageControl, nullptr);
	//if (pr_cookie.bv_val != NULL) {
	//	ber_memfree(pr_cookie.bv_val);
	//	pr_cookie.bv_val = NULL;
	//	pr_cookie.bv_len = 0;
	//}

	std::vector<LDAPControl*> ctrls = { pageControl, /*new LDAPControl,*/ nullptr };
//	ctrls[0]->ldctl_oid = LDAP_CONTROL_PAGEDRESULTS;
//	ctrls[0]->ldctl_iscritical = true;

	//-------------------
	//if (ldap_create_sort_control_value(ctx,	sss_keys, &c[i].ldctl_value))
	//{
	//	tool_exit(ld, EXIT_FAILURE);
	//}

	//c[i].ldctl_oid = LDAP_CONTROL_SORTREQUEST;
	//c[i].ldctl_iscritical = sss > 1;
	//i++;


	//res = ldap_set_option(ctx, LDAP_OPT_SERVER_CONTROLS, &ctrls[0]);
	//ASSERT_EQ(res, LDAP_SUCCESS);

	const char* filter = "(sAMAccountName=kt1)";
	char* srchattrs[] = { "cn", "sn", NULL };
	char** attrs = srchattrs;
	LDAPMessage* first_msg = nullptr;
	res = ldap_search_ext_s(ctx, base_dn, LDAP_SCOPE_SUBTREE,
		filter, attrs, 0, &ctrls[0], NULL, NULL, LDAP_NO_LIMIT, &first_msg);
	ASSERT_EQ(res, LDAP_SUCCESS);
	if (res == LDAP_SIZELIMIT_EXCEEDED ||
		res == LDAP_TIMELIMIT_EXCEEDED ||
		res == LDAP_SUCCESS)
	{
		auto num_entries = ldap_count_entries(ctx, first_msg);
		std::cout << "num_entries=" << num_entries << "\n";
		int	nvalues = 0;
		char** values = NULL;
		for (LDAPMessage* next_msg = ldap_first_message(ctx, first_msg); !!next_msg; next_msg = ldap_next_message(ctx, next_msg))
		{
			auto dn = ldap_get_dn(ctx, next_msg);
			//VS_SCOPE_EXIT{ if (dn) ldap_memfree(dn); };
			std::cout << "ResEntry[" << dn << "], type=" << ldap_msgtype(next_msg) <<"\n";

			int server_err = 0;
			LDAPControl** resp_ctrls = nullptr;
			switch (ldap_msgtype(next_msg))
			{
			case LDAP_RES_SEARCH_RESULT:
			{
				res = ldap_parse_result(ctx, next_msg,
					&server_err, /*&matcheddn*/0, /*&error_string*/0, /*&refs*/0, &resp_ctrls, 0);
				std::cout << "ldap_parse_result = " << server_err << "\n";
				if (resp_ctrls)
				{
					std::cout << "has controls\n";
					for (unsigned int i = 0; resp_ctrls[i] != NULL; i++) {
						std::cout << "oid: " << resp_ctrls[i]->ldctl_oid << "\n";
						if (!resp_ctrls[i]->ldctl_oid)
							continue;
						if (strcasecmp(resp_ctrls[i]->ldctl_oid, LDAP_CONTROL_PAGEDRESULTS) == 0)
						{
							ber_int_t estimate;
							res = ldap_parse_pageresponse_control(ctx, resp_ctrls[i], &estimate, &pr_cookie);
							ASSERT_EQ(res, LDAP_SUCCESS);
							//std::cout << "cookie:" << pr_cookie.bv_len << ", val=" << std::string(pr_cookie.bv_val, pr_cookie.bv_len) << "\n";
							//for (int j = 0; j < pr_cookie.bv_len; ++j)
							//	std::cout << std::to_string(pr_cookie.bv_val[j]);
							//std::cout << "\nestimate = " << estimate << "\n";

							res = ldap_create_page_control_value(ctx, pageSize+1, &pr_cookie, &(ctrls[0]->ldctl_value));
							ASSERT_EQ(res, LDAP_SUCCESS);

							//std::cout << "copy\n";
							//// todo(kt): copy cookie value
							////if (pr_cookie.bv_val != NULL) {
							////	std::cout << "del_old\n";
							////	ber_memfree(pr_cookie.bv_val);
							////	pr_cookie.bv_val = NULL;
							////	pr_cookie.bv_len = 0;
							////}
							////std::cout << "old:" << pr_cookie.bv_len << ", val=" << std::string(pr_cookie.bv_val, pr_cookie.bv_len) << "\n";
							//std::cout << "new:" << ctrls[i]->ldctl_value.bv_len  << "\n" << ", val=" << std::string(ctrls[i]->ldctl_value.bv_val, ctrls[i]->ldctl_value.bv_len) << "\n";
							//pr_cookie.bv_val = ctrls[i]->ldctl_value.bv_val;
							//pr_cookie.bv_len = ctrls[i]->ldctl_value.bv_len;
							////pr_cookie = ctrls[i]->ldctl_value;
						}
					}
				}
			}
			break;
			default:
				break;
			}

//			res = ldap_get_entry_controls(ctx, next_msg, &ctrls);
//			VS_SCOPE_EXIT{ ldap_controls_free(ctrls); };

//			ASSERT_EQ(res, LDAP_SUCCESS);




			//struct berval **v = ldap_get_values_len(ctx, next_msg, attr);

			//if (v != NULL) {
			//	int n = ldap_count_values_len(v);
			//	int j;

			//	values = realloc(values, (nvalues + n + 1) * sizeof(char *));
			//	for (j = 0; j < n; j++) {
			//		values[nvalues + j] = strdup(v[j]->bv_val);
			//	}
			//	values[nvalues + j] = NULL;
			//	nvalues += n;
			//	ldap_value_free_len(v);
			//}
		}

		ldap_msgfree(first_msg);
	}

	// retry with new cookie
	res = ldap_search_ext_s(ctx, base_dn, LDAP_SCOPE_SUBTREE,
		filter, attrs, 0, &ctrls[0], NULL, NULL, LDAP_NO_LIMIT, &first_msg);
	ASSERT_EQ(res, LDAP_SUCCESS);


	if (ctx != NULL) {
		ldap_unbind_ext(ctx, NULL, NULL);
	}

	//ldap_free(ctx);
	ASSERT_TRUE(true);
}
#endif
