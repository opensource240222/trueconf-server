#include "ErrorHolder.h"
#include <algorithm>
#include "../constants/Constants.h"

void ErrorHolder::AddError(std::string nameError, std::string messageError)
{
	errors_.push_back({ std::move(nameError), std::move(messageError) });
}

void ErrorHolder::AddAll(ErrorHolder &&errors)
{
	std::move(std::begin(errors.errors_), std::end(errors.errors_), std::back_inserter(errors_));
}

bool ErrorHolder::IsEmpty() const
{
	return errors_.empty();
}


std::string ErrorHolder::ToString() const
{
	std::string str;
	for (auto&& item : errors_)
	{
		str += item.first + " " + MESSAGE_MSG + ": " + item.second + "\n";
	}
	return str;
}
