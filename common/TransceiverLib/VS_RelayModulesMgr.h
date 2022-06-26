#pragma once

#include "std-generic/cpplib/StrCompare.h"
#include "std-generic/cpplib/string_view.h"

#include "std-generic/compat/map.h"
#include <memory>
#include <string>
#include <utility>

class VS_RelayModule;

class VS_RelayModulesMgr
{
public:
	virtual ~VS_RelayModulesMgr();
	virtual bool RegisterModule(const std::shared_ptr<VS_RelayModule>& module, bool permanent = false);
	virtual void UnregisterModule(const std::shared_ptr<VS_RelayModule> &module);
	virtual void UnregisterAllTemporaryModules();

protected:
	std::shared_ptr<VS_RelayModule>	GetModule(string_view name);

private:
	struct ModulePair
	{
		std::weak_ptr<VS_RelayModule> permanent;
		std::weak_ptr<VS_RelayModule> temporary;
	};
	vs::map<std::string /*name*/, ModulePair, vs::str_less> m_modules;
};