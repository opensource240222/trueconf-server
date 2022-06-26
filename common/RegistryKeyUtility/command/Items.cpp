#include "Items.h"


Items::Items(Items&& other) noexcept
	: items_{ std::move(other.items_) }
{
}

Items& Items::operator=(Items&& other) noexcept
{
	if (this == &other)
	{
		return *this;
	}
	items_ = std::move(other.items_);
	return *this;
}

void Items::AddItem(collection_item &&item)
{
	items_.push_back(std::move(item));
}

const Items::collection& Items::GetItems() const
{
	return items_;
}