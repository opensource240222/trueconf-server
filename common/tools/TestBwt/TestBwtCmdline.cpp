#include "TestBwtCmdline.h"
#include "../../Bwt/RunTest.h"
#include <iostream>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/errors.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/bind.hpp>

using namespace boost;
namespace bwt
{

	std::string test_mode_to_string(unsigned mode)
	{
		if (mode == VS_BWT_MODE_IN)
			return "IN";
		if (mode == VS_BWT_MODE_OUT)
			return "OUT";
		if (mode == VS_BWT_MODE_HALFDUPLEX)
			return "HALFDUPLEX";
		if (mode == VS_BWT_MODE_DUPLEX)
			return "DUPLEX";
		throw std::logic_error("wrong mode in test_mode_to_string()");
	}

		bwt::cmdline::cmdline(unsigned default_mode, unsigned default_duration, unsigned default_frame_size, unsigned default_period) :
			m_default_mode(default_mode),
			m_default_duration(default_duration),
			m_default_frame_size(default_frame_size),
			m_default_period(default_period)

		{

			m_options_description.add_options()
				("help,H", "produce this help message")
				("target-endpoint,E", program_options::value<std::string>()->default_value(""), "tested server, default server from registry")
				("test-type,T", program_options::value<std::string>()->default_value(test_mode_to_string(m_default_mode)), "I[N], O[UT], D[UPLEX], H[ALFDUPLEX]")
				("duration,D", program_options::value<unsigned>()->default_value(m_default_duration), "test duration in seconds, min 1, max 120")
				("frame-size,S", program_options::value<unsigned>()->default_value(m_default_frame_size), "frame size in bytes")
				("period,P", program_options::value<unsigned>()->default_value(m_default_period), "period between frames in milliseconds, 0 - max bandwidth")
				("generate-files,F", "generate *.CSV files with all results")
				("registry-backend", program_options::value<std::string>()
#if defined(_WIN32)
					->default_value("registry:")
#endif
					, "registry backend specification"
				)
				("registry-root,R", program_options::value<std::string>(), "registry key for application, default \"Online\",\"Client\",\"OnlineTwin\",\"ClientTwin\"")
				;
		}

		unsigned bwt::cmdline::get_test_mode()
		{
			std::string test_type = m_var_map["test-type"].as<std::string>();
			if (boost::iequals(test_type, "I") || boost::iequals(test_type, "IN"))
				return VS_BWT_MODE_IN;
			if (boost::iequals(test_type, "O") || boost::iequals(test_type, "OUT"))
				return VS_BWT_MODE_OUT;
			if (boost::iequals(test_type, "D") || boost::iequals(test_type, "DUPLEX"))
				return VS_BWT_MODE_DUPLEX;
			if (boost::iequals(test_type, "H") || boost::iequals(test_type, "HALFDUPLEX"))
				return VS_BWT_MODE_HALFDUPLEX;
			throw std::logic_error("wrong test-type");
		}

		unsigned bwt::cmdline::get_duration()
		{
			return m_var_map["duration"].as<unsigned>();
		}

		unsigned bwt::cmdline::get_frame_size()
		{
			return m_var_map["frame-size"].as<unsigned>();
		}

		unsigned bwt::cmdline::get_period()
		{
			return m_var_map["period"].as<unsigned>();
		}

		bool cmdline::get_generate_files()
		{
			return m_var_map.find("generate-files") != m_var_map.end();
		}

		std::string cmdline::get_target_endpoint()
		{
			return m_var_map["target-endpoint"].as<std::string>();
		}

		std::string cmdline::get_registry_backend()
		{
			return m_var_map["registry-backend"].as<std::string>();
		}

		std::vector<std::string>& cmdline::unrecognized()
		{
			return m_unrecognized;
		}

		bool cmdline::get_help()
		{
			return m_var_map.find("help") != m_var_map.end();
		}

		void cmdline::usage()
		{
			std::cout << m_options_description;
		}

		void cmdline::parse(int argc, char *argv[])
		{
			program_options::command_line_parser parser(argc, argv);
			parser.style(program_options::command_line_style::case_insensitive | program_options::command_line_style::default_style /*| command_line_style::allow_slash_for_short*/);
			auto parsed_options = parser.options(m_options_description).allow_unregistered().extra_parser(std::bind(&cmdline::parse_old_style_params, this, std::placeholders::_1)).run();
			program_options::store(parsed_options, m_var_map);
			for (size_t i = 0; i < parsed_options.options.size(); i++)
			{
				if (parsed_options.options[i].string_key.empty() && !parsed_options.options[i].value.empty())
				{
					m_unrecognized.push_back(parsed_options.options[i].value[0]);
				}
			}
		}

		std::pair<std::string, std::string> cmdline::parse_old_style_params(const std::string& token)
		{
			auto& options = m_options_description.options();
			if (token[0] == '/')
			{
				size_t i;
				for (i = 0; i < token.size() && token[i] != ':' && token[i] != '='; i++);
				std::string key = token.substr(0, i);
				for (auto& option : options)
				{
					if (iequals(key, option->canonical_display_name(program_options::command_line_style::allow_slash_for_short)))
					{
						if (i == token.size())
						{
							if (option->semantic()->max_tokens()==0)
							{
								return std::make_pair(option->long_name(), std::string());
							}
							else
							{
								throw program_options::invalid_command_line_syntax(program_options::invalid_command_line_syntax::missing_parameter, token, " ");
							}
						}
						return std::make_pair(option->long_name(), token.substr(i + 1, std::string::npos));
					}
				}
				throw program_options::validation_error(program_options::validation_error::invalid_option, token, " "); //if third param is empty program_options crashes
			}
			return std::make_pair(std::string(), std::string());
		}
}

