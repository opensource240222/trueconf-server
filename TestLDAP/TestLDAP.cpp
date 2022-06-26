// TestLicense.cpp : Defines the entry point for the console application.
//

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <windns.h>
#include <winldap.h>

#ifdef _DEBUG
#define PAUSE getchar();
#else
#define PAUSE 
#endif


void print_usage(const char* exe_name)
{
	printf("\nUsage:\n\t%s \n",exe_name);
}

int ldap_err(const char* action)
{
	printf("LDAP %s error %s\n",action,ldap_err2string(LdapGetLastError()));
	PAUSE;
	return -1;
}

int main(int argc, char* argv[])
{
	puts("(C) Visicron 04");

	if(argc<3)
	{
		print_usage(argv[0]);
		return -1;
	};

	char buf[256];
	DWORD len=256;
	long result=DnsQueryConfig(DnsConfigPrimaryDomainName_UTF8,false,0,0,buf,&len);

	if(result)
	{
		printf("dns config quiery falied %d\n",result);
		return -1;
	};

	PDNS_RECORD dns_reply;
	result=DnsQuery_A("_ldap._tcp",DNS_TYPE_SRV,DNS_QUERY_STANDARD,NULL,&dns_reply,NULL);

	if(!result)
		printf("found LDAP %s\n",dns_reply->Data.SRV.pNameTarget);
	else
	{
		printf("dns lookup error %d\n",result);
		return -1;
	};

	char* ldap_server=_strdup(dns_reply->Data.SRV.pNameTarget);
	WORD	ldap_port=dns_reply->Data.SRV.wPort;

	DnsRecordListFree(dns_reply, DnsFreeRecordList);

	LDAP* ldap=ldap_sslinit(ldap_server,ldap_port,0);
	if(!ldap)
		return ldap_err("init");

	if(ldap_bind_s(ldap,NULL,NULL,LDAP_AUTH_NTLM)!=LDAP_SUCCESS)
		return ldap_err("bind_s with current name");

	char filter[256];
	sprintf(filter,"(&(objectClass=user)(sAMAccountName=%s))",argv[1]);
	char* dn_attr[] ={"principalName",0};
	
	ULONG search=ldap_search(ldap,"dc=pca,dc=ru",LDAP_SCOPE_SUBTREE,filter,dn_attr,false);
	if(search==-1)
		return ldap_err("search");

	LDAPMessage* lmsg;

	l_timeval timeout={300,0};
	
  result=ldap_result(ldap,search,LDAP_MSG_ALL,&timeout,&lmsg);
	if(result==-1)
		return ldap_err("fetch");
	if(result==0)
		return ldap_err("fetch timeout");

	long num=ldap_count_entries(ldap,lmsg);

	char** val=ldap_get_values(ldap,lmsg,"distinguishedName");

	if(!val)
		return ldap_err("get value");

	printf("found DN: '%s'\n",val[0]);
	char* dn=_strdup(val[0]);

	ldap_value_free(val);
	ldap_msgfree(lmsg);

    // Set the version to 3.0 (default version is 2.0).

	ULONG version = LDAP_VERSION3;
  result	= ldap_set_option(ldap,
                           LDAP_OPT_PROTOCOL_VERSION,
                           (void*)&version);

    // Enable concurrent bind.

	/*const LDAP_OPT_FAST_CONCURRENT_BIND=0x41;
        result = ldap_set_option(ldap,
                               LDAP_OPT_FAST_CONCURRENT_BIND,
                               LDAP_OPT_ON);*/

	if(ldap_bind_s(ldap,dn,argv[2],LDAP_AUTH_SIMPLE)==LDAP_SUCCESS)
		printf("login OK\n");
	else
		printf("login FAILED\n");





	ldap_unbind_s(ldap);

	

	free(ldap_server);
	free(dn);
	PAUSE;

	return 1;
}

