#pragma once

#include "Extractor.h"
#include "../entity/CommandParams.h"

class ExtractorParams : public Extractor<CommandParams, const int, const char *const[]>
{
public:
	CommandParams Extract(const int argc, const char *const argv[]) const override;
};
