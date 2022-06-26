#ifdef _WIN32
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#endif

#include "SecureLib/OpenSSLCompat/tc_asn1.h"
#include "SecureLib/VS_CertMixins.h"

X509_EXTENSION *MakeExtension(SubjectAltNameType type, string_view value)
{
	std::vector<char> buf;// Literally have to make it a vector so that data() returns non-const value
	switch (type)
	{
	case SUBJ_ALT_NAME_DNS:
		buf = {'D', 'N', 'S', ':'};//"DNS:"
		break;
	case SUBJ_ALT_NAME_IP:
		buf = {'I', 'P', ':'};//"IP:"
		break;
	case SUBJ_ALT_NAME_EMAIL:
		buf = {'e', 'm', 'a', 'i', 'l', ':'};//"email:"
		break;
	default:
		return nullptr;
	}
	buf.insert(buf.end(), value.begin(), value.end());
	buf.push_back('\0');
	return X509V3_EXT_conf_nid(NULL, NULL, NID_subject_alt_name, buf.data());
}
// WARNING: Winsock will find you, Winsock will bite you! (if you actually decide to use this function)
void FetchExtensions(const STACK_OF(X509_EXTENSION) *exts, SubjAltNameExtensionsSet &out)
{
	size_t count = 0;
	char strbuf[128];

	count = sk_num(reinterpret_cast<const _STACK *>(exts));
	for (size_t i = 0; i < count; i++)
	{
		X509_EXTENSION *ext = static_cast<X509_EXTENSION *>(sk_value(reinterpret_cast<const _STACK *>(exts), i));
		GENERAL_NAMES *names = NULL;

		if (ext == NULL)
			continue;

		if (OBJ_obj2nid(X509_EXTENSION_get_object(ext)) != NID_subject_alt_name)
			continue;

		{
			const unsigned char *in = ASN1_STRING_get0_data(X509_EXTENSION_get_data(ext));

			names = d2i_GENERAL_NAMES(NULL, &in,
				ASN1_STRING_length(X509_EXTENSION_get_data(ext)));
			if (names == NULL)
			{
				continue;
			}
		}

		for (int i = 0; i < sk_num(reinterpret_cast<_STACK *>(names)); i++)
		{
			SubjectAltNameType type;
			char *value = NULL;
			GENERAL_NAME *name = static_cast<GENERAL_NAME *>(sk_value(reinterpret_cast<_STACK *>(names), i));

			switch (name->type)
			{
			case GEN_EMAIL:
				type = SUBJ_ALT_NAME_EMAIL;
				value = reinterpret_cast<char *>(ASN1_STRING_data(name->d.rfc822Name));
				break;
			case GEN_IPADD:
			{
				type = SUBJ_ALT_NAME_IP;
				// parse IP address
				{
					ASN1_OCTET_STRING *ip = name->d.iPAddress;

					if (ip->type == V_ASN1_OCTET_STRING)
					{
						// convert IP address binary data into string.
						// This code was honourably stolen from common/net/Lib.cpp:inet_ntop()
#if defined (_WIN32)
						sockaddr_storage srcaddr;
						DWORD srcaddr_sz;
						if (ip->length == 4) // IPv4
						{
							srcaddr_sz = sizeof(sockaddr_in);
							auto addr4 = reinterpret_cast<sockaddr_in*>(&srcaddr);
							addr4->sin_family = AF_INET;
							addr4->sin_port = 0;
							::memcpy(&addr4->sin_addr, ip->data, sizeof(addr4->sin_addr));
						}
						else if (ip->length == 16) // IPv6
						{
							srcaddr_sz = sizeof(sockaddr_in6);
							auto addr6 = reinterpret_cast<sockaddr_in6*>(&srcaddr);
							addr6->sin6_family = AF_INET6;
							addr6->sin6_port = 0;
							addr6->sin6_flowinfo = 0;
							::memcpy(&addr6->sin6_addr, ip->data, sizeof(addr6->sin6_addr));
							addr6->sin6_scope_id = 0;
						} else
							srcaddr_sz = 0;
						DWORD dst_sz = sizeof(strbuf);
						WSAAddressToStringA(reinterpret_cast<sockaddr*>(&srcaddr),
							srcaddr_sz, nullptr, strbuf, &dst_sz);
						value = strbuf;
#else
						if (ip->length == 4) // IPv4
							value = (char *)inet_ntop(AF_INET, ip->data, strbuf, sizeof(strbuf));
						else if (ip->length == 16) // IPv6
							value = (char *)inet_ntop(AF_INET6, ip->data, strbuf, sizeof(strbuf));
#endif
					}
				}
			}
				break;
			case GEN_DNS:
				type = SUBJ_ALT_NAME_DNS;
				value = reinterpret_cast<char *>(ASN1_STRING_data(name->d.dNSName));
				break;
			}

			if (!value)
				continue;

			out.emplace_back(type, value);
		}

		GENERAL_NAMES_free(names);
	}
}

