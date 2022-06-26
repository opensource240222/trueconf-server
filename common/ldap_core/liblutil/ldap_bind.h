#pragma once

#include <lber.h>

BerElement  * ldap_build_bind_req(const char *dn, const char *mech, const struct berval *cred, int* msgID);
int verify_ber_reply(BerElement *ber);
int parse_bind_resp(BerElement *resp, struct berval  **servercredp);
int make_ntlm_auth_step(Sockbuf *sb, const struct berval *in_ccred, int* msgID, struct berval **out_scred);