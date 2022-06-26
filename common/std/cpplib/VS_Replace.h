#pragma once

#include "std-generic/cpplib/string_view.h"

#include "std-generic/compat/type_traits.h"
#include <string>

template <class charT, class traits, class Allocator>
const charT* VS_ReplaceAll(std::basic_string<charT, traits, Allocator>& text, vs::type_identity_t<basic_string_view<charT, traits>> what, vs::type_identity_t<basic_string_view<charT, traits>> with)
{
	if (what.empty())
		return text.c_str();

	typename std::basic_string<charT, traits, Allocator>::size_type index = 0;
	while ((index = text.find(what.data(), index, what.size())) != text.npos)
	{
		text.replace(index, what.size(), with.data(), with.size());
		index += with.size();
	}
	return text.c_str();
}
