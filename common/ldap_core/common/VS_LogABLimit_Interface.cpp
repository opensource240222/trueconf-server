#include "VS_LogABLimit_Interface.h"

#include "../../std/cpplib/VS_RegistryKey.h"
#include "../../std/cpplib/VS_RegistryConst.h"

unsigned long VS_LogABLimit_Interface::GetABLimitOrDefault(const char* key_name, const long def)
{
	if (!key_name || !*key_name)
		return 0;
	VS_RegistryKey key(false, CONFIGURATION_KEY);
	if (!key.IsValid())
		return 0;
	long ret(def);
	key.GetValue(&ret, sizeof(ret), VS_REG_INTEGER_VT, key_name);
	return ret;
}
