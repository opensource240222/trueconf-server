#pragma once

#include <vector>
#include <cinttypes>
#include <map>

class CPartitionTree
{
public:
	CPartitionTree(uint16_t number);

	std::vector<std::vector<uint16_t>> GetPartitions();

	void DeepStep(size_t nodeIdx,
		std::vector<uint16_t>& currPath,
		std::vector<std::vector<uint16_t>>& result);

	size_t BuildNode(uint16_t value, uint16_t remainder);

private:
	struct SNode
	{
		SNode(uint16_t value, uint16_t remainder);

		uint16_t Value;
		uint16_t Remainder;

		std::vector<size_t> ChildIdx;
	};

	std::vector<SNode> Nodes;
	std::map<uint32_t, size_t> Cache;
};
