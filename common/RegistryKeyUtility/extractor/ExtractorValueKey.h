#pragma once

#include "../command/RegistryKeyCommand.h"
#include "Extractor.h"


class ExtractorValueKey : public Extractor<ValueKey, const void*, const RegistryVT, name_extracrt, const std::size_t>
{
public:

	ExtractorValueKey() = default;
	ValueKey Extract(const void* value, const RegistryVT type,
		name_extracrt name, const std::size_t offset) const override;
};
