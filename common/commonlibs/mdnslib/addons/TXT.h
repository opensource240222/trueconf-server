#pragma once

#include <vector>

#include "std-generic/cpplib/string_view.h"

namespace mdns
{
namespace addons
{

class TXT
{
// This class is used to form correct TXT rData for DNS-SD standard (RFC 6763:6)
public:
//	Takes key and value strings, puts them in array as ([string_size]key=value)
	template<class CharContainer1, class CharContainer2>
	bool addPair(const CharContainer1& key, const CharContainer2& value)
	{
		size_t size = key.size() + value.size() + 1;
		if (size > 255)
			return false;
		char charSize = static_cast<char>(size);
		data_.push_back(charSize);
		data_.insert(data_.end(), key.begin(), key.end());
		data_.push_back('=');
		data_.insert(data_.end(), value.begin(), value.end());
		return true;
	}

//	Clears stored data
	void clear() {data_.clear();}

//	Returns a pointer to TXT data
	const char* data() {return data_.data();}
//	Returns size of formed TXT data
	size_t size() const {return data_.size();}
//	Returns container, which contains(!) formed data
	const std::vector<char>& container() const {return data_;}

//	std::pair<key, value>
	std::pair<std::string, std::string> getEntry(unsigned int index) const
	{
		if (data_.empty())
			return {};
		auto iter = data_.begin();
		for (unsigned int i = 0; i < index; ++i)
		{
			iter += *iter + 1;
			if (iter == data_.end())
				return {};
		}
		if (iter + *iter == data_.end())// Make sure i can make a correct string_view out of this
			return {};
		string_view keyValuePair(&(*(iter + 1)), *iter);
		size_t indexEqual;
		if (string_view::npos == (indexEqual = keyValuePair.find('=', 0)))
			return {};
		return {{keyValuePair.begin(), keyValuePair.begin() + indexEqual},
		        {keyValuePair.begin() + indexEqual + 1, keyValuePair.end()}};
	}

	size_t getPairCount() const
	{
		size_t i = 0;
		for (auto iter = data_.begin(); iter != data_.end(); ++iter)
		{
			iter += *iter;
			++i;
		}
		return i;
	}

//	Constructs an object and fills the data_ field for further parsing
	template<class CharContainer>
	explicit TXT(const CharContainer& rData): data_(rData.begin(), rData.end())
		{}
	TXT() {}
private:
	std::vector<char> data_;
};

}
}
