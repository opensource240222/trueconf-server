#include <algorithm>

#include "CPartitionTree.h"

CPartitionTree::CPartitionTree(uint16_t number)
{
	BuildNode(number, number);
}

std::vector<std::vector<uint16_t>> CPartitionTree::GetPartitions()
{
	std::vector<std::vector<uint16_t>> result;
	std::vector<uint16_t> currPath;

	for (size_t i = 0; i < Nodes[0].ChildIdx.size(); i++)
	{
		DeepStep(Nodes[0].ChildIdx[i], currPath, result);
	}

	return result;
}

void CPartitionTree::DeepStep(size_t nodeIdx,
	std::vector<uint16_t>& currPath,
	std::vector<std::vector<uint16_t>>& result
	)
{
	currPath.push_back(Nodes[nodeIdx].Value);

	if (!Nodes[nodeIdx].ChildIdx.size())
	{
		result.push_back(currPath);
	}
	else
	{
		for (size_t i = 0; i < Nodes[nodeIdx].ChildIdx.size(); i++)
		{
			DeepStep(Nodes[nodeIdx].ChildIdx[i], currPath, result);
		}
	}

	currPath.pop_back();
}

size_t CPartitionTree::BuildNode(uint16_t value, uint16_t remainder)
{
	uint32_t hash = (uint32_t(value) << 16) + uint32_t(remainder);

	if (Cache.find(hash) != Cache.end())
		return Cache[hash];

	Nodes.emplace_back(value, remainder);

	size_t resIdx = Nodes.size() - 1;

	Cache.emplace(hash, resIdx);

	for (uint16_t i = 1; i <= std::min(value, remainder); i++)
	{
		Nodes[resIdx].ChildIdx.push_back(BuildNode(i, remainder - i));
	}

	return resIdx;
}

CPartitionTree::SNode::SNode(uint16_t value, uint16_t remainder)
{
	Value = value;
	Remainder = remainder;
}
