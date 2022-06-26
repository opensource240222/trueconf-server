#include "VS_SIPField_RecordRoute.h"
#include "VS_SIPObjectFactory.h"
#include "VS_SIPURI.h"
#include "std/debuglog/VS_Debug.h"
#include <string>
#include <vector>
#include "std-generic/compat/memory.h"
#include "VS_SIPGetInfoInterface.h"

#define DEBUG_CURRENT_MODULE VS_DM_SIPPARSER

const boost::regex VS_SIPField_RecordRoute::e(" *(?i)Record-Route(?-i)( *):.*");
const boost::regex VS_SIPField_RecordRoute::e2(".*?(<.*?>).*?");	// split many <uri>'s and decode them separately

VS_SIPField_RecordRoute::VS_SIPField_RecordRoute()
{
}

VS_SIPField_RecordRoute::~VS_SIPField_RecordRoute()
{
	VS_SIPField_RecordRoute::Clean();
}

VS_SIPField_RecordRoute::VS_SIPField_RecordRoute(const VS_SIPField_RecordRoute &rhs) : VS_BaseField(rhs)
{
	operator=(rhs);
}

VS_SIPField_RecordRoute & VS_SIPField_RecordRoute::operator=(const VS_SIPField_RecordRoute &rhs)
{
	if (this == &rhs)
	{
		return *this;
	}
	if (*this != rhs)
	{
		VS_BaseField::operator=(rhs);

		m_uri.clear();
		for (const auto& uri : rhs.m_uri)
		{
			auto ptr = std::make_shared<VS_SIPURI>();
			*ptr = *uri;
			m_uri.emplace_back(ptr);
		}
	}

	return *this;
}

bool VS_SIPField_RecordRoute::operator!=(const VS_SIPField_RecordRoute &rhs)const
{
	return VS_BaseField::operator!=(rhs) || !(this->m_uri == rhs.m_uri);
}


TSIPErrorCodes VS_SIPField_RecordRoute::Decode(VS_SIPBuffer &buffer)
{
	Clean();

	std::unique_ptr<char[]> ptr;
	std::size_t ptr_sz = 0;

	TSIPErrorCodes err = buffer.GetNextBlockAllocConst(ptr, ptr_sz);
	if (TSIPErrorCodes::e_ok != err )
	{
		SetValid(false);
		SetError(err);
		return err;
	}

	if ( !ptr || !ptr_sz )
	{
		SetValid(false);
		SetError(TSIPErrorCodes::e_buffer);
		return TSIPErrorCodes::e_buffer;
	}

	boost::cmatch m;
	bool regex_match_res;
	try
	{
		regex_match_res = boost::regex_match(ptr.get(), m, e);
	}
	catch (const std::runtime_error &ex)
	{
		dstream1 << "VS_SIPField_RecordRoute::Decode error " << ex.what() << "\n";
		regex_match_res = false;
	}
	if (!regex_match_res)
	{
		dstream3 << "[SIPParserLib::Error] Record-Route Field: buffer not match, dump |" << ptr.get() << "|";
		SetError(TSIPErrorCodes::e_match);
        return TSIPErrorCodes::e_match;
	}

	const std::string &spaces = m[1];
	const std::size_t nspaces = spaces.length();

	buffer.Skip(13 + nspaces);	// "Record-Route:" + spaces

	// iterate through all <uri>'s at one header
	{
		err = buffer.GetNextBlockAllocConst(ptr, ptr_sz);
		if (TSIPErrorCodes::e_ok != err)
		{
			SetValid(false);
			SetError(err);
			return err;
		}

		if (!ptr || !ptr_sz)
		{
			SetValid(false);
			SetError(TSIPErrorCodes::e_buffer);
			return TSIPErrorCodes::e_buffer;
		}

		std::string s(ptr.get(), ptr_sz);
		boost::sregex_iterator it(s.begin(), s.end(), e2);
		while (it != boost::sregex_iterator())
		{
			VS_SIPBuffer b((*it)[1].str());
			b.AddData("\r\n");
			++it;

			auto uri = std::make_shared<VS_SIPURI>();
			err = uri->Decode(b);
			if (err != TSIPErrorCodes::e_ok)
			{
				SetError(err);
				return err;
			}
			m_uri.push_back(uri);
		}
	}

	buffer.SkipHeader();

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SIPField_RecordRoute::Encode(VS_SIPBuffer &buffer) const
{
	if ( !IsValid() )
		return GetLastClassError();

	buffer.AddData("Record-Route: ");

	std::size_t i = 0;
	TSIPErrorCodes err = TSIPErrorCodes::e_badObjectState;
	for (const auto& uri: m_uri)
	{
		err = static_cast<TSIPErrorCodes>(uri->Encode(buffer));
		if (err != TSIPErrorCodes::e_ok)
			break;
		++i;
		if (i < m_uri.size())
			buffer.AddData(",");
	}
	return err;
}

TSIPErrorCodes VS_SIPField_RecordRoute::Init(const VS_SIPGetInfoInterface& call)
{
	const VS_SIPURI *uri = call.GetNextSipRouteFromSet();
	if (!uri)
	{
		SetValid(false);
		SetError(TSIPErrorCodes::e_InputParam);
		return TSIPErrorCodes::e_InputParam;
	}

	m_uri.clear();
	auto ptr = std::make_shared<VS_SIPURI>();
	*ptr = *uri;
	m_uri.emplace_back(ptr);

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

void VS_SIPField_RecordRoute::Clean() noexcept
{
	VS_SIPError::Clean();
	m_uri.clear();
}

const std::vector<std::shared_ptr<VS_SIPURI>> &VS_SIPField_RecordRoute::GetURIs() const
{
	return m_uri;
}

std::unique_ptr<VS_BaseField> VS_SIPField_RecordRoute_Instance()
{
	return vs::make_unique<VS_SIPField_RecordRoute>();
}

#undef DEBUG_CURRENT_MODULE
