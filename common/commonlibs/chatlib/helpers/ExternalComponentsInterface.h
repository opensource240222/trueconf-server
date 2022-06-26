#pragma once
#include "vs_def.h"

namespace vs
{
class ExternalComponentsInterface
{
public:
	virtual ~ExternalComponentsInterface() = default;
	virtual ResolverPtr GetResolver() const = 0;

};
}