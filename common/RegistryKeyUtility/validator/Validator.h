#pragma once
#include "../entity/ErrorHolder.h"

template <class T>
class Validator
{
public:
	Validator() = default;
	virtual ~Validator() = default;
	Validator(Validator&& other) noexcept = default;
	Validator& operator=(Validator&& other) noexcept = default;
	Validator(const Validator& other) = delete;
	Validator& operator=(Validator &other) = delete;
	virtual ErrorHolder Validation(const T& params) const = 0;
};
