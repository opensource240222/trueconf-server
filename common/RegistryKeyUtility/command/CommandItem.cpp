#include "CommandItem.h"
#include "../constants/Constants.h"
#include "../entity/ErrorHolder.h"

#include <cassert>
#include <cstring>

ErrorHolder CommandItem::ParseCommand(command_t first)
{
	assert(IsValid());

	return ParseCommandImpl(first);
}

CommandItem::NameCommand CommandItem::StrToNameCommand(const char* nameCommand)
{
	int i = 0;
	auto it = std::begin(NAME_COMMAND);
	for (; it != std::end(NAME_COMMAND); it = std::next(it), i++)
	{
		if (strcmp(nameCommand, *it) == 0)
		{
			break;
		}
	}
	if (it != std::end(NAME_COMMAND))
	{
		return static_cast<CommandItem::NameCommand>(i);
	}
	return CommandItem::NameCommand::NON_COMMAND;
}

const char* CommandItem::NameCommandToStr(const CommandItem::NameCommand nameCommand)
{
	const auto index = static_cast<std::underlying_type<CommandItem::NameCommand>::type>(nameCommand);
	if (index > -1)
	{
		return NAME_COMMAND[index];
	}
	return EMPTY_STR;
}
