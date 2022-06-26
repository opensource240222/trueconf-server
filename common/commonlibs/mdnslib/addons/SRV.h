#pragma once

#include <vector>
#include <cstdint>

#include "std-generic/cpplib/string_view.h"
#include "std-generic/cpplib/hton.h"

namespace mdns
{
namespace addons
{

// This class is used to work with SRV resource record's rData. Can form one or get values from one.
class SRV
{
public:
//	Forms SRV rData
	template<class ContinuousCharContainer1, class ContinuousCharContainer2>
	static void form(ContinuousCharContainer1& dest, uint16_t priority, uint16_t weight,
		uint16_t port, const ContinuousCharContainer2& target)
	{
		dest.clear();
		dest.resize(6);
		reinterpret_cast<uint16_t*>(dest.data())[0] = vs_htons(priority);
		reinterpret_cast<uint16_t*>(dest.data())[1] = vs_htons(weight);
		reinterpret_cast<uint16_t*>(dest.data())[2] = vs_htons(port);
		dest.insert(dest.end(), target.begin(), target.end());
	}

//	Gets Target field of SRV rData
	/*
	static string_view getSrvTarget(const std::vector<char>& rData)
	{
		return {rData.data() + TARGET_OFFSET, rData.size() - TARGET_OFFSET};
	}*/
	static std::vector<char> copySrvTarget(const std::vector<char>& rData)
	{
		return {rData.begin() + TARGET_OFFSET, rData.end()};
	}

//	Gets Priority field of SRV rData
	static uint16_t priority(const std::vector<char>& rData)
	{
		if (rData.size() > 1)
			return vs_ntohs(reinterpret_cast<const uint16_t*>(rData.data())[0]);
		else
			return 0;
	}

//	Gets Weight field of SRV rData
	static uint16_t weight(const std::vector<char>& rData)
	{
		if (rData.size() > 3)
			return vs_ntohs(reinterpret_cast<const uint16_t*>(rData.data())[1]);
		else
			return 0;
	}

//	Gets Port field of SRV rData
	static uint16_t port(const std::vector<char>& rData)
	{
		if (rData.size() > 5)
			return vs_ntohs(reinterpret_cast<const uint16_t*>(rData.data())[2]);
		else
			return 0;
	}
private:
	static const unsigned int TARGET_OFFSET = 6;
};

}
}
