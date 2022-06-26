#pragma once

#include "std/cpplib/VS_RegistryKey.h"
#include "std/debuglog/VS_Debug.h"

namespace test {

inline void InitRegistry()
{
	VS_RegistryKey::SetDefaultRoot("TrueConf\\Server");
	if (!VS_RegistryKey::InitDefaultBackend("memory:"))
		throw std::runtime_error("Can't initialize registry backend!");

	VS_RegistryKey cfg(false, "Configuration", false, true);
	cfg.SetValueI32(VS_DM_ALL_, "Debug Modules");
	cfg.SetValueI32(4, "Debug Level");

	VS_ReadDebugKeys();
}

inline void ClearRegistry() {
	std::string old_root(VS_RegistryKey::GetDefaultRoot());

	auto remove_path = old_root;
	auto pos = remove_path.find_last_of('\\');
	if (pos == std::string::npos) return;
	remove_path.erase(pos);

	VS_RegistryKey::SetDefaultRoot("");
	VS_RegistryKey root(false, remove_path, false, false);
	root.RemoveKey("Server");

	VS_RegistryKey::SetDefaultRoot(old_root);
}

}
