#include "VS_RelayModulesMgr.h"
#include "VS_RelayModule.h"

VS_RelayModulesMgr::~VS_RelayModulesMgr() = default;

bool VS_RelayModulesMgr::RegisterModule(const std::shared_ptr<VS_RelayModule> &module, bool permanent)
{
	if (permanent)
	{
		const auto result = m_modules.emplace(module->GetModuleName(), ModulePair{ module, std::weak_ptr<VS_RelayModule>() });
		if (result.second)
			return true;
		if (result.first->second.permanent.expired())
		{
			result.first->second.permanent = module;
			return true;
		}
		else
			return false;
	}
	else
	{
		const auto result = m_modules.emplace(module->GetModuleName(), ModulePair{ std::weak_ptr<VS_RelayModule>(), module });
		if (result.second)
			return true;
		if (result.first->second.temporary.expired())
		{
			result.first->second.temporary = module;
			return true;
		}
		else
			return false;
	}
}

void VS_RelayModulesMgr::UnregisterModule(const std::shared_ptr<VS_RelayModule> &module)
{
	const auto it = m_modules.find(module->GetModuleName());
	if (it == m_modules.end())
		return;
	if (!it->second.temporary.expired())
		it->second.temporary.reset();
	else
		it->second.permanent.reset();
}

void VS_RelayModulesMgr::UnregisterAllTemporaryModules()
{
	for (auto it = m_modules.begin(); it != m_modules.end(); /**/)
	{
		if (it->second.permanent.expired())
			it = m_modules.erase(it);
		else
		{
			it->second.temporary.reset();
			++it;
		}
	}
}

std::shared_ptr<VS_RelayModule> VS_RelayModulesMgr::GetModule(string_view name)
{
	std::shared_ptr<VS_RelayModule>	module;
	const auto it = m_modules.find(name);
	if (it == m_modules.end())
		return module;
	module = it->second.temporary.lock();
	if (!module)
		module = it->second.permanent.lock();
	if (!module)
		m_modules.erase(it);
	return module;
}