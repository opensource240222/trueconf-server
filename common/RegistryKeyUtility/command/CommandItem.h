#pragma once

#include "Command.h"

#include <string>
#include <vector>

class CommandParams;
class ErrorHolder;

typedef std::vector<std::string>::const_iterator command_t;

class CommandItem : public Command
{
public:

	enum class NameCommand
	{
		NON_COMMAND = -1,
		SET_COMMAND,
		GET_COMMAND,
		RENAME_COMMAND,
		REMOVE_COMMAND,
		STORE_COMMAND,
		LOAD_COMMAND
	};

	CommandItem() = default;
	ErrorHolder ParseCommand(command_t first);
	virtual void CompletedExecute() = 0;
	virtual const char *GetNameCommnad() const = 0;
public:
	static NameCommand StrToNameCommand(const char *nameCommand);
	static const char *NameCommandToStr(const CommandItem::NameCommand nameCommand);

protected:
	virtual ErrorHolder ParseCommandImpl(command_t first) = 0;
};