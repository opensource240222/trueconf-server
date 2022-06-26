#include "VS_SIPMetaContainer.h"
#include <memory>

VS_SIPMetaContainer::VS_SIPMetaContainer()
{
}

VS_SIPMetaContainer::~VS_SIPMetaContainer()
{}

void VS_SIPMetaContainer::GetSize(std::size_t& aSize) const
{
	aSize = iContainer.size();
}

void VS_SIPMetaContainer::AddField(std::unique_ptr<VS_BaseField>&& aBaseField)
{
	if (!aBaseField)
	{
		return;
	}

	if (aBaseField->order() == -1)
	{
		iContainer.push_back(std::move(aBaseField));
	}
	else
	{
		auto it = iContainer.begin();
		for (; it != iContainer.end(); ++it)
		{
			if ((*it)->order() == -1 || aBaseField->order() < (*it)->order())
			{
				it = iContainer.emplace(it, std::move(aBaseField));
				break;
			}
		}
		if (it == iContainer.end())
		{
			iContainer.push_back(std::move(aBaseField));
		}
	}

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
}

TSIPErrorCodes VS_SIPMetaContainer::GetField(std::size_t aIndex, VS_BaseField* & aBaseField) const
{
	if (aIndex >= iContainer.size())
	{
		return TSIPErrorCodes::e_InputParam;
	}

	aBaseField = iContainer[aIndex].get();

	return TSIPErrorCodes::e_ok;
}

std::size_t VS_SIPMetaContainer::EraseField(std::size_t aIndex)
{
	if (aIndex >= iContainer.size())
	{
		SetError(TSIPErrorCodes::e_InputParam);
		return -1;
	}

	iContainer[aIndex].reset();
	iContainer.erase(iContainer.begin() + aIndex);

	return aIndex;
}

void VS_SIPMetaContainer::Clear() noexcept
{
	iContainer.clear();
}
