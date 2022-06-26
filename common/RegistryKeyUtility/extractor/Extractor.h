#pragma once
#include "std-generic/cpplib/string_view.h"

typedef std::pair<string_view, string_view> name_extracrt;

template <typename  T, typename... Types>
class Extractor
{
public:
	Extractor() = default;
	virtual ~Extractor() = default;
	Extractor(const Extractor& other) = delete;
	Extractor(Extractor&& other) noexcept = default;
	Extractor& operator=(const Extractor& other) = delete;
	Extractor& operator=(Extractor&& other) noexcept = default;
	/**
	* @throw ExtractorException.
	*/
	virtual T Extract(Types... arg) const = 0;
};
