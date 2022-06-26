
#pragma once
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/options_description.hpp>


namespace bwt
{
	class cmdline
	{
		public:

			cmdline(unsigned default_mode, unsigned default_duration, unsigned default_frame_size, unsigned default_period);
			void parse(int argc, char *argv[]);
			void usage();
			unsigned get_test_mode();
			unsigned get_duration();
			unsigned get_frame_size();
			unsigned get_period();
			bool get_generate_files();
			std::string get_target_endpoint();
			std::string get_registry_backend();
			std::vector<std::string>& unrecognized();
			bool get_help();
	   private:
			std::pair<std::string, std::string> parse_old_style_params(const std::string& token);
			boost::program_options::variables_map m_var_map;
			boost::program_options::options_description m_options_description;
			std::vector<std::string> m_unrecognized;
			unsigned m_default_mode, m_default_duration, m_default_frame_size, m_default_period;
	};
}
