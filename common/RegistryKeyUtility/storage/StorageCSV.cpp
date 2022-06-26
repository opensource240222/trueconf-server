#define BOOST_SPIRIT_USE_PHOENIX_V3
#include "StorageCSV.h"

#include <boost/fusion/include/at_c.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/exception/diagnostic_information.hpp>

#include "../exceptions/StorageException.h"
#include "../command/RegistryKeyCommand.h"
#include "../constants/Constants.h"
#include "../entity/ValueKey.h"
#include "../utils/utils.h"

#include "std-generic/compat/iterator.h"
#include <cassert>

static const char COLSEP[] = ",";

StorageCSV::StorageCSV(const bool rewriteFile)
	:rewriteFile_(rewriteFile)
{
}

bool StorageCSV::OpenStorage(const char* filePath, bool read_only)
{
	assert(filePath != nullptr);
	std::ios_base::openmode mode;

	if (read_only)
	{
		mode = std::ios::in;
	}
	else
	{
		mode = std::ios::out | std::ios::in;
	}

	if (rewriteFile_)
	{
		stream_.open(filePath, mode | std::ios::out | std::ios::trunc);
	}
	else
	{
		stream_.open(filePath, mode);
	}
	return stream_.is_open();
}

bool StorageCSV::IsValidStorage()
{
	return stream_.is_open();
}

bool StorageCSV::CloseStorage()
{
	if (stream_.is_open())
	{
		stream_.close();
		return true;
	}
	return false;
}

namespace qi = boost::spirit::qi;
namespace phx = boost::phoenix;

enum LineItems { KNAME_ITEM, VNAME_ITEM, TYPE_ITEM, VALUE_ITEM, UNUSED };

using Column = std::string;
using Columns = std::vector<Column>;
using CsvFile = std::vector<ValueKey>;

template<typename It>
struct CsvGrammar : qi::grammar<It, CsvFile(), qi::locals<std::vector<LineItems>>, qi::blank_type>
{
	CsvGrammar() : CsvGrammar::base_type(start_) {
		using namespace qi;

		item_.add(KNAME, LineItems::KNAME_ITEM)
			(VNAME, LineItems::VNAME_ITEM)
			(TYPE, LineItems::TYPE_ITEM)
			(VALUE, LineItems::VALUE_ITEM);


		start_ = qi::omit[header_[_a = _1]] >> eol >> line_(_a) % eol;
		header_ = (item_ | omit[column_] >> attr(UNUSED)) % COLSEP;
		line_ = (column_ % COLSEP)[convert_];

		column_ = quoted_ | +(char_ - COLSEP - eol);
		quoted_ = '"' >> *(("\"\"" >> qi::attr('"')) | char_ - '"') >> '"';

		BOOST_SPIRIT_DEBUG_NODES((header_)(line_)(column_)(quoted_));

		qi::on_error<qi::fail>(column_, phx::ref(std::cout)
			<< "Error! Expecting " << _4
			<< " here: \"" << phx::construct<std::string>(_3, _2)
			<< "\"\n");
	}

	static const unsigned COUNT_COLUMNS{ 4 };
	std::size_t Line() const
	{
		return convert_.line;
	}

private:
	qi::rule<It, std::vector<LineItems>(), qi::blank_type> header_;
	qi::rule<It, CsvFile(), qi::locals<std::vector<LineItems>>, qi::blank_type> start_;
	qi::rule<It, ValueKey(std::vector<LineItems> const&), qi::blank_type> line_;
	qi::rule<It, Column(), qi::blank_type> column_;
	qi::rule<It, std::string()> quoted_;
	qi::rule<It, qi::blank_type> empty_;
	qi::symbols<char, LineItems> item_;


	struct final {
		using type_ctx = decltype(line_);
		using Ctx = typename type_ctx::context_type;

		mutable std::size_t line{ 0 };

		void operator()(Columns &columns, Ctx &ctx, bool &pass) const
		{
			line++;
			if (columns.size() != COUNT_COLUMNS)
			{
				throw StorageException(__LINE__, __FILE__, std::string{ INVALID_DATA_STORAGE_ERROR_MESSAGE } +" line: " + std::to_string(line + 1));
			}
			auto& csv_line = boost::fusion::at_c<0>(ctx.attributes);
			auto& positions = boost::fusion::at_c<1>(ctx.attributes);
			int i = 0;
			ValueKey::ValueType type = ValueKey::ValueType::non_type;

			for (LineItems position : positions)
			{
				switch (position) {
				case KNAME_ITEM:
				{
					if (!is_valid_key_name(columns[i]))
					{
						pass = false;
						break;
					}
					csv_line.SetKeyName(std::string(cut_front_end_delimeter(columns[i])));
					break;
				}
				case VNAME_ITEM: csv_line.SetValueName(std::move(columns[i])); break;

				case TYPE_ITEM:
				{
					type = ValueKey::StrToValueType(columns[i].c_str());
					if (type == ValueKey::ValueType::non_type || type == ValueKey::ValueType::bin)
					{
						pass = false;
						break;
					}
					csv_line.SetType(type);
					break;
				}
				case VALUE_ITEM:
				{
					if (type == ValueKey::ValueType::i32 || type == ValueKey::ValueType::i64)
					{
						if (!is_int_numbers(columns[i].c_str()))
						{
							pass = false;
							break;
						}
					}
					else if (type == ValueKey::ValueType::b64)
					{
						if (!is_base64(columns[i].c_str()))
						{
							pass = false;
							break;
						}
					}
					csv_line.SetValue(std::move(columns[i]));
					break;
				}
				default: pass = false;
				}

				if (!pass)
				{
					throw StorageException(__LINE__, __FILE__,
						std::string(INVALID_DATA_STORAGE_ERROR_MESSAGE) + " line: " + std::to_string(line + 1));
				}
				i++;
			}
			pass = true; // returning false fails the `line` rule
		}
	} convert_;
};


static std::string escape_csv(std::string s)
{
	std::string::size_type pos = 0;
	while (true) {
		pos = s.find("\"", pos);
		if (pos == std::string::npos)break;
		s.replace(pos, 1, "\"\"");
		pos += 2;
	}
	return s;
}

void StorageCSV::SaveToStorage(const storage_args& obj)
{
	assert(IsValidStorage());

	stream_ << KNAME << COLSEP << VNAME << COLSEP << TYPE << COLSEP << VALUE << "\n";
	for (auto &item : obj)
	{
		stream_ << "\"" << (item.GetName().keyName.find("\"") != std::string::npos ? escape_csv(std::move(item.GetName().keyName))
			: item.GetName().keyName) << "\"" << COLSEP;

		stream_ << "\"" << (item.GetName().valueName.find("\"") != std::string::npos ? escape_csv(std::move(item.GetName().valueName))
			: item.GetName().valueName) << "\"" << COLSEP;

		stream_ << "\"" << ValueKey::ValueTypeToStr((item.GetType() != ValueKey::ValueType::bin ?
			item.GetType() : ValueKey::ValueType::b64)) << "\"" << COLSEP;

		stream_ << "\"";

		if (item.GetType() == ValueKey::ValueType::str)
		{
			std::stringstream sstream;
			item.StreamShowData(sstream);
			auto str = sstream.str();
			stream_ << (str.find("\"") != std::string::npos ? escape_csv(std::move(str)) : str);
		}
		else
		{
			item.StreamShowData(stream_);
		}

		stream_ << "\"" << "\n";

	}
}

Storage::storage_args StorageCSV::LoadFromStorage()
{
	assert(IsValidStorage());

	stream_.seekg(0, std::ios::end);
	const std::size_t size = stream_.tellg();
	if (size > 0)
	{
		std::string buffer;
		buffer.reserve(size);
		stream_.seekg(0, std::ios::beg);
		buffer.assign((std::istreambuf_iterator<char>(stream_)), std::istreambuf_iterator<char>());

		auto first(vs::cbegin(buffer));
		const auto last(vs::cend(buffer));
		const CsvGrammar<std::string::const_iterator> it;

		CsvFile parsed;
		try
		{
			if (!qi::phrase_parse(first, last, it, qi::blank, parsed))
			{
				throw StorageException(__LINE__, __FILE__, std::string{ INVALID_DATA_STORAGE_ERROR_MESSAGE }+" line: " + std::to_string(it.Line()));
			}
		}
		catch (const boost::exception &ex)
		{
			throw StorageException(__LINE__, __FILE__, boost::diagnostic_information(ex));
		}
		return parsed;
	}
	return {};
}
