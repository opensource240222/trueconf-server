#include "VS_RegistryKey.h"
#include "VS_RegistryKey_internal.h"
#include "VS_RegistryKey_registry.h"
#include "VS_RegistryKey_database.h"
#include "VS_RegistryKey_memory.h"

#include <boost/algorithm/string/predicate.hpp>

#include "std-generic/compat/memory.h"
#include <cstdlib>
#include <cstring>

namespace regkey {

void Backend::TEST_DumpData()
{
}

}

std::string VS_RegistryKey::s_default_root;
std::shared_ptr<regkey::Backend> VS_RegistryKey::s_backend;
std::string VS_RegistryKey::s_backend_configuration;

regkey::KeyStub key_stub;
regkey::Key* VS_RegistryKey::s_no_key = &key_stub;

std::shared_ptr<regkey::Backend> regkey::Backend::Create(string_view configuration)
{
	std::shared_ptr<Backend> backend;

	if (false) ;
#if defined(_WIN32)
	else if (boost::starts_with(configuration, string_view("registry:")))
		backend = std::make_shared<regkey::RegistryBackend>();
#endif
#if defined(HAVE_CPPDB)
	else if (boost::starts_with(configuration, string_view("postgresql:")))
		backend = std::make_shared<regkey::DBBackend>();
#endif
	else if (boost::starts_with(configuration, string_view("memory:")))
		backend = std::make_shared<regkey::MemoryBackend>();

	if (backend && !backend->Init(configuration))
		backend.reset();
	return backend;
}

bool VS_RegistryKey::InitDefaultBackend(string_view configuration)
{
	auto backend = regkey::Backend::Create(configuration);
	if (!backend)
		return false;

	s_backend = std::move(backend);
	s_backend_configuration = static_cast<std::string>(configuration);
	return true;
}

void VS_RegistryKey::TEST_DumpData()
{
	s_backend->TEST_DumpData();
}
