#include <cstdint>
#include <vector>
#include <fstream>
#include <string>



namespace ConvertDump
{

	/**
		for convertinng C Array format dump (Wireshark can represent snif by this way) to packets dump. Here every packet is converted to len-data format.
		That is first 4 bytes are lenght of packet data. After the length follows packet data with size = legth bytes. And so on.

	*/

	void CArrayToLengthDataFormat(const std::string &input_file)
	{
		std::ifstream f(input_file, std::ios::in | std::ios::binary);
		std::string out_file_name = input_file + ".ldp";
		std::ofstream out(out_file_name, std::ios::out | std::ios::binary);
		if (!f.good() || !out.good())
			return;

		std::vector<char> buf;
		std::string str;
		while (!f.eof())
		{
			f.ignore(std::numeric_limits<std::streamsize>::max(), '{');
			if (f.eof())
				return;
			while (f.peek() != '}')
			{
				while (f.peek() != ' ')
					str.push_back(f.get());
				buf.push_back(static_cast<uint8_t>(::strtol(str.c_str(), nullptr, 16)));
				f.ignore();
				str.clear();
			}
			uint32_t size = buf.size();
			out.write(reinterpret_cast<char*>(&size), sizeof(size));
			out.write(buf.data(), buf.size());
			buf.clear();
		}
	}
}


int main(int argc, char *argv[])
{
	std::string file_in;
	if (argc < 2)
		return 1;
	file_in = argv[1];
	ConvertDump::CArrayToLengthDataFormat(file_in);
}