#if !defined(_WIN32)

#include "ldap_bind.h"
#include <sys/types.h>
#include <errno.h>
#include <ldap.h>
#include <sasl.h>
#include "std-generic/cpplib/scope_exit.h"

BerElement  * ldap_build_bind_req(const char *dn, const char *mech, const struct berval *cred, int* msgID)
{
	if (!msgID)
		return NULL;

	if (dn == NULL)
		dn = "";
	BerElement  *ber = ber_alloc_t(LBER_USE_DER);
	if (!ber) {
		return NULL;
	}

	int rc = ber_printf(ber, "{it{ist{sON}N}" /*}*/,
		*msgID, LDAP_REQ_BIND,
		3/* version */, dn, LDAP_AUTH_SASL,
		mech, cred);

	++(*msgID);

	if (rc == -1) {
		ber_free(ber, 1);
		return NULL;
	}

	if (ber_printf(ber, /*{*/ "N}") == -1) {
		ber_free(ber, 1);
		return NULL;
	}

	return ber;
}

int verify_ber_reply(BerElement *ber) {
	if (!ber)
		return SASL_BADPARAM;

	/* message id */
	ber_int_t   id;
	if (ber_get_int(ber, &id) == LBER_ERROR || id < 0)
		return SASL_FAIL;

	ber_len_t bLen;
	unsigned tag = ber_peek_tag(ber, &bLen);
	if (tag == LBER_ERROR)
		return SASL_FAIL;

	if (tag == LDAP_RES_SEARCH_REFERENCE)	// do not support for now
		return SASL_FAIL;

	if (tag != LDAP_RES_SEARCH_ENTRY && tag != LDAP_RES_INTERMEDIATE)
	{
		BerElement  *tmpber = ber_dup(ber);
		ber_int_t   lderr;
		char    *lr_res_error = NULL;
		char    *lr_res_matched = NULL;

		if (ber_scanf(tmpber, "{eAA", &lderr,
			&lr_res_matched, &lr_res_error)
			!= LBER_ERROR)
		{
			if (tag != LDAP_RES_BIND)
				return SASL_FAIL;
		}
	}

	return SASL_OK;
}

int parse_bind_resp(BerElement *resp, struct berval  **servercredp) {
	struct berval* scred = NULL;
	ber_int_t errcode;
	char* ld_matched = NULL, *ld_error = NULL;
	BerElement  *ber = ber_dup(resp);

	ber_len_t len;
	ber_tag_t tag;
	tag = ber_scanf(ber, "{eAA" /*}*/, &errcode, &ld_matched, &ld_error);
	if (tag == LBER_ERROR) {
		ber_free(ber, 0);
		return LDAP_DECODING_ERROR;
	}

	tag = ber_peek_tag(ber, &len);
	if (tag == LDAP_TAG_REFERRAL) {
		/* skip 'em */
		if (ber_scanf(ber, "x") == LBER_ERROR) {
			ber_free(ber, 0);
			return LDAP_DECODING_ERROR;
		}
		tag = ber_peek_tag(ber, &len);
	}

	if (tag == LDAP_TAG_SASL_RES_CREDS) {
		if (ber_scanf(ber, "O", &scred) == LBER_ERROR) {
			ber_free(ber, 0);
			return LDAP_DECODING_ERROR;
		}
	}
	if (servercredp != NULL) {
		*servercredp = scred;
	}
	else if (scred != NULL) {
		ber_bvfree(scred);
	}

	ber_free(ber, 0);
	return LDAP_SUCCESS;
}

int send_ber(BerElement *b, Sockbuf *sb) {
	ber_sockbuf_add_io(sb, &ber_sockbuf_io_tcp, LBER_SBIOD_LEVEL_PROVIDER, NULL);
	return ber_flush2(sb, b, LBER_FLUSH_FREE_NEVER);
}

int make_ntlm_auth_step(Sockbuf *sb, const struct berval *in_ccred, int* msgID, struct berval **out_scred) {
	if (!sb || !in_ccred || !out_scred || !msgID)
		return LDAP_PARAM_ERROR;

	BerElement * ldapBindReq = ldap_build_bind_req("", "GSS-SPNEGO", in_ccred, msgID);
	VS_SCOPE_EXIT{ ber_free(ldapBindReq, 1); };
	if (!ldapBindReq)
		return LDAP_ENCODING_ERROR;
	send_ber(ldapBindReq, sb);

	ber_len_t bLen;
	BerElement *result = ber_alloc_t(LBER_USE_DER);
	VS_SCOPE_EXIT{ ber_free(result, 1); };

	ber_tag_t	tag;
	while (true) {
		tag = ber_get_next(sb, &bLen, result);
		if (tag != LBER_ERROR) break;

		if (errno == EWOULDBLOCK) continue;
		if (errno == EAGAIN) continue;
		return LDAP_NO_RESULTS_RETURNED;
	}
	if (verify_ber_reply(result) != SASL_OK)
		return LDAP_DECODING_ERROR;

	int rc = parse_bind_resp(result, out_scred);
	if (rc != LDAP_SUCCESS)
		return LDAP_DECODING_ERROR;

	return LDAP_SUCCESS;
}
#endif