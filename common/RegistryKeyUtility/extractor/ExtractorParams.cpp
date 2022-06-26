#include "ExtractorParams.h"

#include <boost/program_options.hpp>
#include <fstream>
#include <assert.h>

#include "../constants/Constants.h"
#include "../exceptions/ExtractorException.h"
#include "../entity/CommandParams.h"
#include "../command/RenameCommand.h"

static const struct
{
	CommandItem::NameCommand nameCommand;
	const int lenArgs;
	const std::string messageError;
} EXTRACTOR_COMMAND[] =
{
	{ CommandItem::NameCommand::SET_COMMAND, LEN_ARGS_SET, INVALID_INPUT_LIST_MESSAGE + std::string{ SET_MESSAGE } },
	{ CommandItem::NameCommand::GET_COMMAND, LEN_ARGS_GET, INVALID_INPUT_LIST_MESSAGE + std::string{ GET_MESSAGE } },
	{ CommandItem::NameCommand::RENAME_COMMAND, LEN_ARGS_RENAME, INVALID_INPUT_LIST_MESSAGE + std::string{ RENAME_MESSAGE } },
	{ CommandItem::NameCommand::REMOVE_COMMAND, LEN_ARGS_REMOVE, INVALID_INPUT_LIST_MESSAGE + std::string{ REMOVE_MESSAGE } },
	{ CommandItem::NameCommand::STORE_COMMAND, LEN_ARGS_STORE, INVALID_INPUT_LIST_MESSAGE + std::string{ STORE_MESSAGE } },
	{ CommandItem::NameCommand::LOAD_COMMAND,  LEN_ARGS_LOAD, INVALID_INPUT_LIST_MESSAGE + std::string{ LOAD_MESSAGE } }
};

typedef std::vector<boost::program_options::option> args_command_t;


static CommandParams::CommandParamsItem extract_command(const CommandItem::NameCommand nameCommand, args_command_t::iterator &first, const args_command_t::iterator &last)
{
	for (auto &&var : EXTRACTOR_COMMAND)
	{
		if (var.nameCommand == nameCommand)
		{
			if (std::distance(first, last) >= var.lenArgs + 1)
			{
				CommandParams::CommandParamsItem::args_t item;
				item.reserve(var.lenArgs);
				std::for_each((first + 1), (first + var.lenArgs + 1), [&item](args_command_t::value_type &param)
				{
					item.push_back(std::move(param.value.front()));
				});
				auto res = CommandParams::CommandParamsItem{ std::move(first->value.front()), std::move(item) };
				first = first + var.lenArgs;
				return res;
			}
			throw ExtractorException(__LINE__, __FILE__, var.messageError);
		}
	}
	assert(false);
	return {};
}

CommandParams ExtractorParams::Extract(const int argc, const char * const argv[]) const
{
	namespace po = boost::program_options;
	CommandParams params;
	std::string config_file;

	po::options_description command_line("", 1000);
	command_line.add_options()
		(HELP_H, HELP_MESSAGE)
		(CONFIG_FILE_C, po::value<std::string>(&config_file)->value_name(VALUE_NAME_FILE)->
			default_value(DEFAULT_CONFIG_FILE), CONFIG_FILE_MESSAGE)
		(BACKEND_B, po::value<std::string>()->value_name(VALUE_NAME_STR)->
				notifier(std::bind(&CommandParams::SetBackend, &params, std::placeholders::_1)), BACKEND_MESSAGE);

	po::options_description command_line_root("", 1000);
	command_line_root.add_options()
		(ROOT_R, po::value<std::string>()->value_name(VALUE_NAME_STR)->default_value(DEFAULT_ROOT)->
			notifier(std::bind(&CommandParams::SetRoot, &params, std::placeholders::_1)), ROOT_MESSAGE);

	po::options_description command_line_commands("", 1000);
	command_line_commands.add_options()
		(COMMANDS, po::value<std::vector<std::string>>(),
		("[\n  " + std::string(SET_MESSAGE) + "\n" +
		 "  TYPE:\n   +" + TYPE_I32 + "\n   +" + TYPE_I64 + "\n   +" + TYPE_STR + "\n   +" + TYPE_BIN + "\n   +" + TYPE_B64 + "\n]\n" +
		 "[" + GET_MESSAGE + "]\n" +
		 "[" + RENAME_MESSAGE + "]\n" +
		 "[" + REMOVE_MESSAGE + "]\n" +
		 "[" + STORE_MESSAGE + "]\n" +
		 "[" + LOAD_MESSAGE + "]\n"
		).c_str());


	po::positional_options_description pos;
	pos.add(COMMANDS, -1);

	try {
		po::variables_map vm;
		po::options_description options;
		options.add(command_line);
		options.add(command_line_root);
		options.add(command_line_commands);

		po::parsed_options parsed = po::command_line_parser(argc, argv).
			options(options).
			positional(pos).
			style(po::command_line_style::case_insensitive | po::command_line_style::default_style).
			run();

		po::store(parsed, vm);

		if (vm.count(HELP) > 0 || argc == 1)
		{
			std::stringstream ss;
			ss << options;
			params.SetHelp(ss.str() + EXAMPLES);
		}
		else
		{
			po::notify(vm);

			if (params.GetOptions().backend.empty())
			{
				std::fstream in(config_file);
				if (in.is_open())
				{
					po::options_description regisrty_backend_options;
					regisrty_backend_options.add_options()
						(REGISTRY_BACKEND, po::value<std::string>()->value_name(VALUE_NAME_STR)
#if defined(_WIN32)
							->default_value(REGISTRY_BACKEND_DEFAUL_VALUE, "registry:")
#else
							->required()
#endif
							->notifier(std::bind(&CommandParams::SetBackend, &params, std::placeholders::_1)));
					regisrty_backend_options.add(command_line_root);

					po::store(po::parse_config_file(in, regisrty_backend_options), vm);
					po::notify(vm);
				}
				else
				{
#if defined(_WIN32)
					params.SetBackend(REGISTRY_BACKEND_DEFAUL_VALUE);
#else
					throw ExtractorException(__LINE__, __FILE__,
						std::string(FILE_NOT_FOUNT_OR_BAD_ERROR_MESSAGE) + ": " + config_file);
#endif
				}
			}
			for (auto it = parsed.options.begin(); it != parsed.options.end(); ++it)
			{
				if (it->position_key > -1)
				{
					const CommandItem::NameCommand name_command = CommandItem::StrToNameCommand(it->value.front().c_str());
					if (name_command != CommandItem::NameCommand::NON_COMMAND)
					{
						params.AddCommand(extract_command(name_command, it, parsed.options.end()));
					}
					else
					{
						throw ExtractorException(__LINE__, __FILE__, "Invalid command: " + it->value.front());
					}
				}
			}
		}
	}
	catch (po::error &e)
	{
		throw ExtractorException(__LINE__, __FILE__, "Error parse args", e);
	}
	return params;
}
