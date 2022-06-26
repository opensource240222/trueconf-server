#pragma once

#include "std-generic/cpplib/string_view.h"
#include "std-generic/cpplib/numerical.h"

#include <cassert>
#include <cstring>
#include <limits>
#include <string>

// Stream parser for RTSP requests.
// It can parse the requests coming in arbitrary small chunks. Users of the
// class don't need to store/accumulate the raw request data, parser does that
// itself when it is required.
class VS_RTSPRequestParser
{
public:
	enum class Action
	{
		parse, // Parse parts on the object (request, header) and pass them to appropriate handlers.
		skip, // Skip parsing the object, forward all its bytes to DataHandler.
		skip_line, // Skip parsing current line, forward rest of the bytes on this line to DataHandler.
	};

	// Parses input 'data' invoking the following handlers when appropriate:
	// * MethodNameHandler, URIHandler, VersionHandler, HeaderNameHandler are called when the whole corresponding object is parsed (and reassembled), signature: Action (string_view part)
	// * HeaderValueHandler is called when the value of a header is parsed (and reassembled), signature: void (string_view part)
	// * HeadersEndHandler is called after all headers are parsed (at the start of a line containing only CRLF), signature: void ()
	// * DataHandler is called for parts of the message that don't require parsing, including all separators, signature: void (string_view part)
	// It is guaranteed that all bytes of the input will be (eventually) passed (and only once) to one of the handlers above.
	// So if all parts are concatenated the result will be equal to the input 'data'.
	//
	// ErrorHandler is called when incorrect message format in detected, signature: void (string_view::size_type pos)
	// Parsing will stop after calling this handler at the position 'pos' in the 'data' passed to Parse.
	// Returns true is parsing was successful, and false otherwise.
	template <class MethodNameHandler, class URIHandler, class VersionHandler, class HeaderNameHandler, class HeaderValueHandler, class HeadersEndHandler, class DataHandler, class ErrorHandler>
	bool Parse(string_view data, MethodNameHandler&& on_method, URIHandler&& on_uri, VersionHandler&& on_version, HeaderNameHandler&& on_header_name, HeaderValueHandler&& on_header_value, HeadersEndHandler&& on_headers_end, DataHandler&& on_data, ErrorHandler&& on_error);

private:
	enum class State : unsigned short
	{
		// Request line
		method, // In method name
		request_line_SP1, // On the space after method name
		uri, // In the URI
		request_line_SP2, // On the space after URI
		version, // In the RTSP version
		request_line_CR, // After '\r'
		// Headers
		header_name, // In header name
		header_value, // In header value
		header_CR, // After '\r'
		// Other
		initial, // At the start of a new request or interleaved data block
		newline, // At the start of a new line (except the first line)
		newline_CR, // After '\r' that is at the start of a new line
		payload, // In the message body or interleaved data payload
		// Interleaved data
		interleaved_marker, // After the marker ('$')
		interleaved_channel, // After the channel identifier
		interleaved_size_hi, // After the first byte of payload size
	};

	string_view GetAssembled(string_view part);
	static bool IsContentLength(string_view name);
	static int GetContentLength(string_view value);

	State m_state = State::initial;
	unsigned short m_skip_request : 1; // Don't parse the rest of the request
	unsigned short m_skip_request_line : 1; // Don't parse the rest of the request line
	unsigned short m_skip_header : 1; // Don't parse the rest of the header
	unsigned short m_is_content_length : 1; // True if current header is Content-Length
	int m_remaining_payload_size = 0;
	std::string m_buffer; // Used to reassemble parts of the message (method name, uri, version, header name, header value).
};

template <class MethodNameHandler, class URIHandler, class VersionHandler, class HeaderNameHandler, class HeaderValueHandler, class HeadersEndHandler, class DataHandler, class ErrorHandler>
bool VS_RTSPRequestParser::Parse(string_view data, MethodNameHandler&& on_method, URIHandler&& on_uri, VersionHandler&& on_version, HeaderNameHandler&& on_header_name, HeaderValueHandler&& on_header_value, HeadersEndHandler&& on_headers_end, DataHandler&& on_data, ErrorHandler&& on_error)
{
	string_view::size_type pos = 0;
	while (pos < data.size())
	{
		switch (m_state)
		{
		case State::method:
		{
			const auto space_pos = data.find(' ', pos);
			const auto match = data.substr(pos, space_pos - pos);
			if (space_pos == data.npos)
			{
				m_buffer += match;
				return true;
			}

			const auto method = GetAssembled(match);
			const auto action = on_method(method);
			m_skip_request = action == Action::skip;
			m_skip_request_line = action == Action::skip_line;
			m_buffer.clear();

			m_state = State::request_line_SP1;
			pos = space_pos;
			break;
		}
		case State::request_line_SP1:
		{
			// As an extension we accept multiple consecutive spaces between parts of the request line.
			const auto non_space_pos = data.find_first_not_of(' ', pos);
			const auto match = data.substr(pos, non_space_pos - pos);
			on_data(match);
			if (non_space_pos == data.npos)
				return true;

			m_state = State::uri;
			pos = non_space_pos;
			break;
		}
		case State::uri:
		{
			const auto space_pos = data.find(' ', pos);
			const auto match = data.substr(pos, space_pos - pos);
			const bool skip = m_skip_request || m_skip_request_line;
			if (skip)
				on_data(match);
			if (space_pos == data.npos)
			{
				if (!skip)
					m_buffer += match;
				return true;
			}

			if (!skip)
			{
				const auto uri = GetAssembled(match);
				const auto action = on_uri(uri);
				m_skip_request = action == Action::skip;
				m_skip_request_line = action == Action::skip_line;
				m_buffer.clear();
			}

			m_state = State::request_line_SP2;
			pos = space_pos;
			break;
		}
		case State::request_line_SP2:
		{
			// As an extension we accept multiple consecutive spaces between parts of the request line.
			const auto non_space_pos = data.find_first_not_of(' ', pos);
			const auto match = data.substr(pos, non_space_pos - pos);
			on_data(match);
			if (non_space_pos == data.npos)
				return true;

			m_state = State::version;
			pos = non_space_pos;
			break;
		}
		case State::version:
		{
			// As an extension we accept lines terminated only by LF.
			const auto cr_pos = data.find_first_of("\r\n", pos);
			const auto match = data.substr(pos, cr_pos - pos);
			const bool skip = m_skip_request || m_skip_request_line;
			if (skip)
				on_data(match);
			if (cr_pos == data.npos)
			{
				if (!skip)
					m_buffer += match;
				return true;
			}

			if (!skip)
			{
				const auto version = GetAssembled(match);
				const auto action = on_version(version);
				m_skip_request = action == Action::skip;
				m_skip_request_line = action == Action::skip_line;
				m_buffer.clear();
			}

			on_data(data.substr(cr_pos, 1)); // Forward '\r' (or '\n')
			m_state = data[cr_pos] == '\r' ? State::request_line_CR : State::newline;
			pos = cr_pos + 1;
			break;
		}
		case State::request_line_CR:
		{
			if (data[pos] != '\n')
			{
				// As an extension we accept lines terminated only by CR.
				m_state = State::newline;
				break;
			}

			on_data(data.substr(pos, 1));
			m_state = State::newline;
			++pos;
			break;
		}
		case State::header_name:
		{
			const auto colon_pos = data.find(':', pos);
			const auto match = data.substr(pos, colon_pos - pos);
			const bool skip = m_skip_request;
			if (skip)
				on_data(match);
			if (colon_pos == data.npos)
			{
				// We always reassemble header names because we need to find and read Content-Length to be able to skip the body.
				m_buffer += match;
				return true;
			}

			const auto header_name = GetAssembled(match);
			m_is_content_length = IsContentLength(header_name);
			if (!skip)
			{
				const auto action = on_header_name(header_name);
				m_skip_header = action == Action::skip || action == Action::skip_line;
			}
			m_buffer.clear(); // Outside of the if because we reassemble unconditionally.

			on_data(data.substr(colon_pos, 1)); // Forward ':'
			m_state = State::header_value;
			pos = colon_pos + 1;
			break;
		}
		case State::header_value:
		{
			// As an extension we accept lines terminated only by LF.
			const auto cr_pos = data.find_first_of("\r\n", pos);
			const auto match = data.substr(pos, cr_pos - pos);
			const bool skip = m_skip_request | m_skip_header;
			if (skip)
				on_data(match);
			if (cr_pos == data.npos)
			{
				// We reassemble Content-Length value because we need to be able to skip the body.
				if (!skip || m_is_content_length)
					m_buffer += match;
				return true;
			}

			const auto header_value = GetAssembled(match);
			if (m_is_content_length)
			{
				m_remaining_payload_size = GetContentLength(header_value);
				if (m_remaining_payload_size == -1)
				{
					on_error(pos);
					return false;
				}
			}
			if (!skip)
				on_header_value(header_value);
			m_buffer.clear(); // Outside of the if because we reassemble unconditionally.

			on_data(data.substr(cr_pos, 1)); // Forward '\r' (or '\n')
			m_state = data[cr_pos] == '\r' ? State::header_CR : State::newline;
			pos = cr_pos + 1;
			break;
		}
		case State::header_CR:
		{
			if (data[pos] != '\n')
			{
				// As an extension we accept lines terminated only by CR.
				m_state = State::newline;
				break;
			}

			on_data(data.substr(pos, 1));
			m_state = State::newline;
			++pos;
			break;
		}
		case State::initial:
		{
			if (data[pos] == '$')
			{
				on_data(data.substr(pos, 1));
				m_state = State::interleaved_marker;
				++pos;
			}
			else
				m_state = State::method;
			break;
		}
		case State::newline:
		{
			const bool skip = m_skip_request;
			if (data[pos] == '\r' || data[pos] == '\n')
			{
				if (!skip)
					on_headers_end();

				on_data(data.substr(pos, 1));
				m_state = State::newline_CR;
				if (data[pos] == '\r')
					++pos;
			}
			else
				m_state = State::header_name;
			break;
		}
		case State::newline_CR:
		{
			if (data[pos] != '\n')
			{
				// As an extension we accept lines terminated only by CR.
				m_state = m_remaining_payload_size == 0 ? State::initial : State::payload;
				break;
			}

			on_data(data.substr(pos, 1));
			m_state = m_remaining_payload_size == 0 ? State::initial : State::payload;
			++pos;
			break;
		}
		case State::payload:
		{
			assert(m_remaining_payload_size >= 0);
			if (data.size() - pos < static_cast<size_t>(m_remaining_payload_size))
			{
				m_remaining_payload_size -= static_cast<int>(data.size() - pos);
				on_data(data.substr(pos));
				return true;
			}
			on_data(data.substr(pos, m_remaining_payload_size));

			m_state = State::initial;
			pos += m_remaining_payload_size;
			m_remaining_payload_size = 0;
			break;
		}
		case State::interleaved_marker:
		{
			on_data(data.substr(pos, 1));
			m_state = State::interleaved_channel;
			++pos;
			break;
		}
		case State::interleaved_channel:
		{
			m_remaining_payload_size = static_cast<unsigned char>(data[pos]) * 256;
			on_data(data.substr(pos, 1));
			m_state = State::interleaved_size_hi;
			++pos;
			break;
		}
		case State::interleaved_size_hi:
		{
			m_remaining_payload_size += static_cast<unsigned char>(data[pos]);
			on_data(data.substr(pos, 1));
			m_state = State::payload;
			++pos;
			break;
		}
		}
	}
	return true;
}

inline string_view VS_RTSPRequestParser::GetAssembled(string_view part)
{
	if (m_buffer.empty())
		return part;
	m_buffer += part;
	return m_buffer;
}

inline bool VS_RTSPRequestParser::IsContentLength(string_view name)
{
	return name.size() == 14
		&& (name[ 0] == 'C' || name[ 0] == 'c')
		&& (name[ 1] == 'O' || name[ 1] == 'o')
		&& (name[ 2] == 'N' || name[ 2] == 'n')
		&& (name[ 3] == 'T' || name[ 3] == 't')
		&& (name[ 4] == 'E' || name[ 4] == 'e')
		&& (name[ 5] == 'N' || name[ 5] == 'n')
		&& (name[ 6] == 'T' || name[ 6] == 't')
		&&  name[ 7] == '-'
		&& (name[ 8] == 'L' || name[ 8] == 'l')
		&& (name[ 9] == 'E' || name[ 9] == 'e')
		&& (name[10] == 'N' || name[10] == 'n')
		&& (name[11] == 'G' || name[11] == 'g')
		&& (name[12] == 'T' || name[12] == 't')
		&& (name[13] == 'H' || name[13] == 'h')
		;
}

inline int VS_RTSPRequestParser::GetContentLength(string_view value)
{
	while (!value.empty() && value.front() == ' ')
		value.remove_prefix(1);
	while (!value.empty() && value.back() == ' ')
		value.remove_suffix(1);

	char buffer[12];
	if (value.size() < 1 || value.size() > sizeof(buffer) - 1)
		return -1;

	memcpy(buffer, value.data(), value.size());
	buffer[value.size()] = '\0';

	char* p_end;
	const auto num = strtoul(buffer, &p_end, 10);
	if (vs::numeric_less(std::numeric_limits<int>::max(), num))
		return -1;
	if (*p_end != '\0')
		return -1;
	return static_cast<int>(num);
}
