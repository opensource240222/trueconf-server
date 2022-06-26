#pragma once

#include <string>
#include <vector>

class CommandParams
{
public:

	struct Options
	{
		std::string help;
		std::string backend;
		std::string root;
	};

	struct CommandParamsItem
	{
		std::string nameCommand;
		typedef std::vector<std::string> args_t;
		args_t commandArgs;
	};

	typedef std::vector<CommandParamsItem> container_command;

	const Options& GetOptions() const;
	const container_command &GetCommands() const;
	void AddCommand(CommandParamsItem item);
	void SetHelp(const std::string &message);
	void SetBackend(const std::string &backend);
	void SetRoot(const std::string &root);

private:
	Options options_;
	container_command commands_;
};