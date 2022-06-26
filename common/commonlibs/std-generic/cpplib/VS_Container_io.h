#pragma once

#include "VS_Container.h"

#include <ostream>

template <class charT, class traits>
std::basic_ostream<charT, traits>& operator<<(std::basic_ostream<charT, traits>& s, const VS_Container& x)
{
	auto flags(s.flags());
	s.width(0);
	s << std::dec;

	x.Reset();
	while (x.Next())
	{
		switch (x.GetType())
		{
		case VS_CNT_BOOL_VT:
		{
			s << "VS_CNT_BOOL_VT " << x.GetName() << ' ';
			bool value;
			if (x.GetValue(value))
				s << (value ? "true" : "false");
			else
				s << "[error]";
			s << '\n';
		}
			break;
		case VS_CNT_INTEGER_VT:
		{
			s << "VS_CNT_INTEGER_VT " << x.GetName() << ' ';
			int32_t value;
			if (x.GetValue(value))
				s << value;
			else
				s << "[error]";
			s << '\n';
		}
			break;
		case VS_CNT_DOUBLE_VT:
		{
			s << "VS_CNT_DOUBLE_VT " << x.GetName() << ' ';
			double value;
			if (x.GetValue(value))
				s << value;
			else
				s << "[error]";
			s << '\n';
		}
			break;
		case VS_CNT_STRING_VT:
		{
			s << "VS_CNT_STRING_VT " << x.GetName() << ' ';
			const char* value = x.GetStrValueRef();
			if (value)
				s << value;
			else
				s << "[error]";
			s << '\n';
		}
			break;
		case VS_CNT_BINARY_VT:
		{
			s << "VS_CNT_BINARY_VT " << x.GetName() << ' ';
			size_t size;
			if (x.GetBinValueRef(size) != nullptr)
				s << size << " bytes long";
			else
				s << "[error]";
			s << '\n';
		}
			break;
		case VS_CNT_INT64_VT:
		{
			s << "VS_CNT_INT64_VT " << x.GetName() << ' ';
			int64_t value;
			if (x.GetValue(value))
				s << value;
			else
				s << "[error]";
			s << '\n';
		}
			break;
		}
	}

	s.flags(flags);
	return s;
}
