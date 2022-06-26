#pragma once
#include "chatlib/helpers/ExternalComponentsInterface.h"

class ExternalComponentsStub : public vs::ExternalComponentsInterface
{
	vs::ResolverPtr resolver_;// = std::make_shared<ResolverStub>();
public:
	explicit ExternalComponentsStub(const vs::ResolverPtr &r) :resolver_(r)
	{}
	vs::ResolverPtr GetResolver() const override
	{
		return resolver_;
	}
};