#pragma once

#include <memory>
#include <list>
#include "CommandItem.h"

class Items
{
public:
	Items() = default;
	virtual ~Items() {}
	Items(const Items& other) = delete;
	Items(Items&& other) noexcept;
	Items& operator=(const Items& other) = delete;
	Items& operator=(Items&& other) noexcept;

	using collection_item = std::unique_ptr<CommandItem>;
	using collection = std::list<collection_item>;

	void AddItem(collection_item &&item);
	const collection& GetItems() const;
private:
	collection items_;
};