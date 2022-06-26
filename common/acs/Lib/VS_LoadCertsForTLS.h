#pragma once

#include "acs/connection/VS_ConnectionTLS.h"
#include "std-generic/cpplib/VS_Container.h"
#include "std/cpplib/VS_Protocol.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/VS_RegistryKey.h"
#include <vector>

inline bool VS_LoadCertsForTLS(VS_ConnectionTLS& connection)
{
	// open registry key
	int size;
	std::unique_ptr<uint8_t, free_deleter> buf;
	VS_RegistryKey key(false, CONFIGURATION_KEY, true, true);

	// !!! Load certificate chain !!!
	size = key.GetValue(buf, VS_REG_BINARY_VT, SRV_CERT_CHAIN_KEY);

	// can't read from registry
	if (size == 0)
		return false;

	{
		VS_Container chain_cont;

		chain_cont.Deserialize(buf.get(), size);
		chain_cont.Reset();
		while (chain_cont.Next())
		{
			if (chain_cont.GetName() && (_stricmp(chain_cont.GetName(), CERTIFICATE_CHAIN_PARAM) == 0))
			{
				size_t sz = 0;
				const void *cert = chain_cont.GetBinValueRef(sz);

				if ((!cert || sz == 0) || !connection.UseEndCert(cert, sz))
				{
					return false;
				}
			}
		}
	}

	// !!! Load certificate !!!
	size = key.GetValue(buf, VS_REG_BINARY_VT, SRV_CERT_KEY);

	// can't read from registry
	if (size == 0)
		return false;

	if (!connection.UseEndCert(buf.get(), size))
		return false;

	// !!! Load private key !!!
	size = key.GetValue(buf, VS_REG_BINARY_VT, SRV_PRIVATE_KEY);

	// can't read from registry
	if (size <= 0)
		return false;

	if (!connection.UsePrivateKey(buf.get(), size))
		return false;

	return true;
}

