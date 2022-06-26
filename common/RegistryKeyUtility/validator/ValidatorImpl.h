#pragma once

#include "Validator.h"
#include "../entity/CommandParams.h"

class ValidatorImpl : public Validator<CommandParams>
{
public:
	ErrorHolder Validation(const CommandParams& entity) const override;
};
