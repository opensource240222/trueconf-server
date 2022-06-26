#include "Command.h"
#include <assert.h>

Command::Command() : isValid_(true)
{
}

Command::Command(Command&& other) noexcept : isValid_{ other.isValid_ }
{
	other.isValid_ = false;
}

Command& Command::operator=(Command&& other) noexcept
{
	if (this == &other)
	{
		return *this;
	}
	isValid_ = other.isValid_;
	other.isValid_ = false;
	return *this;
}

bool Command::IsValid() const
{
	return isValid_;
}

void Command::Execute()
{
	assert(IsValid());

	ExecuteImpl();
}

void Command::SetValid(const bool isValid)
{
	isValid_ = isValid;
}
