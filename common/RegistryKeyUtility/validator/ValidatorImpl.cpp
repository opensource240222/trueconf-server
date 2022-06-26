#include "ValidatorImpl.h"

#include "std/cpplib/VS_RegistryKey.h"

#include "../constants/Constants.h"
#include "../entity/ErrorHolder.h"

inline ErrorHolder empty_validation(const char *keyError, const std::string& input)
{
	ErrorHolder error;
	if (input.empty())
	{
		error.AddError(keyError, NOT_EMPTY_ERROR_MESSAGE);
	}
	return error;
}

ErrorHolder ValidatorImpl::Validation(const CommandParams& entity) const
{
	auto &&options = entity.GetOptions();

	ErrorHolder errors;
	errors.AddAll(empty_validation(REGISTRY_BACKEND, options.backend));
	errors.AddAll(empty_validation(ROOT, options.root));
	return errors;
}
