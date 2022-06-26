#pragma once

#include "std-generic/cpplib/string_view.h"
#include "std-generic/clib/strcasecmp.h"

#include <cstring>
#include <string>

namespace vs {

// Transparent comparison function for std::map, std::set, etc with std::string keys.
// Comparison is lexicographical, sort order is determined by strcmp() function.
struct str_less
{
	using is_transparent = void;

	// This function covers all combinations of std::string and string_view
	bool operator()(string_view l, string_view r) const
	{
		if (l.size() < r.size())
			return ::memcmp(l.data(), r.data(), l.size()) <= 0;
		else
			return ::memcmp(l.data(), r.data(), r.size()) < 0;
	}

	// If you've got "use of a deleted function" error here it's because you have passed const char* to .find().
	// This is inefficient. Standard comparison for std::string and a C-string calculates length of the C-string first, and it does so on every comparison.
	// These repeating length caclucations actually make performance of transparent comparison worse than non-transparent.
	// Not calculating length and using strcmp instead of memcmp improves the situation but that isn't the best solution.
	// Best solution is to calculate length just 1 time by creating string_view from C-string when calling .find():
	//     my_map_or_set.find(string_view(my_cstring));
	bool operator()(const std::string& l, const char* r) const = delete;
	bool operator()(const char* l, const std::string& r) const = delete;
};

// Transparent comparison function for std::map, std::set, etc with std::string keys.
// Comparison is lexicographical, sort order is determined by strcasecmp() function.
// Comparison ignores the case of ASCII characters.
struct istr_less
{
	using is_transparent = void;

	bool operator()(string_view l, string_view r) const
	{
#if defined(_WIN32)
		// On Windows this works faster than strncasecmp (including our version of strncasecmp).
		auto pl = l.data();
		auto pr = r.data();
		auto sz = l.size() < r.size() ? l.size() : r.size();
		while (sz--)
		{
			if (*pl != *pr)
			{
				const unsigned char cl = (*pl >= 'A' && *pl <= 'Z') ? *pl - 'A' + 'a' : *pl;
				const unsigned char cr = (*pr >= 'A' && *pr <= 'Z') ? *pr - 'A' + 'a' : *pr;
				if (cl != cr)
					return cl < cr;
			}
			++pl;
			++pr;
		}
		return l.size() < r.size();
#else
		if (l.size() < r.size())
			return ::strncasecmp(l.data(), r.data(), l.size()) <= 0;
		else
			return ::strncasecmp(l.data(), r.data(), r.size()) < 0;
#endif
	}

#if !defined(_WIN32)
	// Overload for std::string in which we can use strcasecmp which is faster than strncasecmp.
	// But since we are not using str*casecmp on Windows defining it there would make comparison for std::string slower.
	bool operator()(const std::string& l, const std::string& r) const
	{
		return ::strcasecmp(l.c_str(), r.c_str()) < 0;
	}
#endif

	// Unlike cese-sensetive comparison, for case-insensetive comparison precalculating length doesn't make a difference.
	bool operator()(const std::string& l, const char* r) const
	{
		return ::strcasecmp(l.c_str(), r) < 0;
	}

	bool operator()(const char* l, const std::string& r) const
	{
		return ::strcasecmp(l, r.c_str()) < 0;
	}
};

}
