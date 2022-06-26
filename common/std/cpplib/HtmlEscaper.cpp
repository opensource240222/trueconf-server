#include "HtmlEscaper.h"
#include "std-generic/clib/strcasecmp.h"

#include <memory>
#include <cctype>
#include <cassert>

/*

Artem Boldarev (08.08.2018):

The code below is, in fact, a core of HTML-parser with a
reconfigurable lexical analyser. It uses Recursive Descent Parser
approach with backtracking to properly handle tree-like structure of
HTML - this approach allows it to detect and properly handle allowed,
but misused (unbalanced) tags. Backtracking is used as a part of the
error recovery mechanism.

The implementation of the parser is tailored to the needs of removing
unnecessary HTML-tags from the text string. This fact thoroughly
influenced the design. It is designed to preserve as much text
formatting as possible. The correct text strings are not affected at
all.

The fact that one can configure acceptable HTML tags at runtime
introduced some context sensitivity to the parser as it had to
distinguish between multiple types of the tags.

The overall structure of the parser can be (roughly) described using
the following grammar in EBNF notation:

Start = TagSequence.
TagSequence = { {NonTagCharacter} Tag}.
NonTagCharacter = < all characters except '<' >.
WhiteSpace = < white space characters (isspace()) >.
Tag = '<'TagOpening {WhiteSpace}'>' [TagBody '</'TagClosing {WhiteSpace}'>'].
TagBody = {NonTagCharacter} | TagSequence.

The acceptable tags can be configured using AddAllowedTag()
method. Currently, this method accepts regular expressions which
describe the representation of the opening (<...>) and closing
(</...>) part of the tag (without '<', '>', and '</').

Of course, HTML is a complex formatting language and using such a
crude approach is not enough to properly parse HTML according to the
specification. Nevertheless, the code below should be good enough for
the task of removing all but allowed HTML tags from the text
string. Moreover, this approach is more effective than adopting a
full-featured HTML parser because it is tailored to the specific
needs.

Here we shall follow the long-established tradition:

"You are not expected to understand this.", D. Ritchie.

*/

vs::HtmlEscaper::HtmlEscaper(void)
	: m_finalized(false)
{
}

vs::HtmlEscaper::~HtmlEscaper(void)
{
}

// 'Start' syntax rule definition.
bool vs::HtmlEscaper::EscapeString(const char *in, std::string &out) const
{
	if (!m_finalized)
	{
		return false;
	}

	if (in == nullptr)
		return false;

	if (*in == '\0') // there is nothing to do
	{
		return true;
	}

	const char *&p = in;
	EscapeTagSequence(in, out);

	return true;
}

bool vs::HtmlEscaper::EscapeString(const std::string &in, std::string &out) const
{
	return EscapeString(in.c_str(), out);
}

// 'NonTagCharacters' syntax rule definition.
void vs::HtmlEscaper::EscapeNonTagCharacters(const char *&p, std::string &out)
{
	while (*p != '\0' && *p != '<')
	{
		EscapeChar(p, out);
	}
}

// 'TagSequence' syntax rule definition.
void vs::HtmlEscaper::EscapeTagSequence(const char *&in, std::string &out, bool subsequence) const
{
	for (;;)
	{
		auto old_in = in;
		auto old_len = out.length();
		EscapeNonTagCharacters(in, out);
		auto res = EscapeTag(in, out);
		if (!res)
		{
			// backtrack - try to reparse
			in = old_in;
			out.resize(old_len);
			if (subsequence) // stop on error when parsing subsequence
			{
				break;
			}
			EscapeChar(in, out); // repair after error - skip problematic character.
		}

		if (*in == '\0') // match end of line
		{
			break;
		}
	}
}

// 'Tag' syntax rule definition.
bool vs::HtmlEscaper::EscapeTag(const char *&p, std::string &out) const
{
	bool inside_tag = false;
	size_t tag;

	auto advance = [&, this]() -> void {
		p++;
	};

	auto match = [&, this](int ch) -> bool {
		return ch == (*p);
	};

	auto advance_spaces = [&, this]() -> void {
		while (isspace(*p))
			advance();
	};

	if (match('\0'))
	{
		return true;
	}

	if (!match('<'))
	{
		return false;
	}

	auto start = p;
	advance();

	// match beginning of a tag
	{
		auto matched = FindTag(tag, p); // try to find start of a tag
		if (matched > 0)
		{
			p += matched;
			inside_tag = true;
			advance_spaces();
			if (match('>'))
			{
				advance();
				out.append(&start[0], &p[0]);
			}
			else
			{
				return false;
			}

		}
		else
		{
			return false;
		}
	}

	assert(inside_tag == true);
	if (m_tags[tag].IsSingle())
	{
		return true;
	}

	// match tag body
	if (m_tags[tag].AreSubtagsAllowed())
	{
		EscapeTagSequence(p, out, true);
	}
	else
	{
		EscapeNonTagCharacters(p, out);
	}

	// match end of a tag (greedy)
	while (*p)
	{
		auto escape_starting_char = [&, this]() -> void {
			p = start;
			EscapeChar(p, out);
		};
		start = p;

		if (match('\0'))
		{
			break;
		}

		if (match('<'))
		{
			advance();
		}
		else
		{
			escape_starting_char();
			continue;
		}

		if (match('/'))
		{
			advance();

			auto matched = m_tags[tag].MatchClosing(p);
			if (matched > 0)
			{
				p += matched;
				advance_spaces();
				if (match('>'))
				{
					advance();
					out.append(&start[0], &p[0]);
					inside_tag = false;
					break;
				}
				else
				{
					escape_starting_char();
					continue;
				}

			}
			else
			{
				escape_starting_char();
				continue;
			}
		}
		else
		{
			escape_starting_char();
			continue;
		}
	}

	if (inside_tag)
		return false;
	return true;
}


size_t vs::HtmlEscaper::FindTag(size_t &tag, const char *p) const
{
	for (size_t i = 0; i < m_tags.size(); i++)
	{
		auto matched = m_tags[i].MatchOpen(p);
		if (matched > 0)
		{
			tag = i;
			return matched;
		}

	}
	return 0;
}

bool vs::HtmlEscaper::AddAllowedTag(const char *open_tag, const char *closing_tag, const bool allow_subtags)
{
	if (m_finalized)
	{
		return false;
	}

	Tag new_tag;
	if (!new_tag.Init(open_tag, closing_tag, allow_subtags))
	{
		return false;
	}

	m_tags.push_back(std::move(new_tag));

	return true;
}

bool vs::HtmlEscaper::Clear(void)
{
	m_tags.clear();
	return true;
}

void vs::HtmlEscaper::Finalize(void)
{
	m_finalized = true;
}

void vs::HtmlEscaper::EscapeChar(const char *&in, std::string &out)
{
	assert(in);
	switch (*in)
	{
	case '&':
	{
		if (strncasecmp(in + 1, "amp;", 4) == 0)
		{
			out.append(&in[0], &in[5]);
			in += 5;
		}
		else if (strncasecmp(in + 1, "quot;", 5) == 0)
		{
			out.append(&in[0], &in[6]);
			in += 6;
		}
		else if (strncasecmp(in + 1, "apos;", 5) == 0)
		{
			out.append(&in[0], &in[6]);
			in += 6;
		}
		else if (strncasecmp(in + 1, "lt;", 3) == 0)
		{
			out.append(&in[0], &in[4]);
			in += 4;
		}
		else if (strncasecmp(in + 1, "gt;", 3) == 0)
		{
			out.append(&in[0], &in[4]);
			in += 4;
		}
		else
		{
			out.append("&amp;");
			in++;
		}
	}
		break;
	case '\"':
		out.append("&quot;");
		in++;
		break;
	case '\'':
		out.append("&apos;");
		in++;
		break;
	case '<':
		out.append("&lt;");
		in++;
		break;
	case '>':
		out.append("&gt;");
		in++;
		break;
	default:
		out.append(1, (char)(*in));
		in++;
		break;
	}
}

/// Tag SubClass
vs::HtmlEscaper::Tag::Tag()
	: m_subtags(true),
	m_is_single(false),
	m_initialized(false)
{
}

vs::HtmlEscaper::Tag::~Tag()
{
}

bool vs::HtmlEscaper::Tag::Init(const char *open_tag, const char *closing_tag, const bool allow_subtags)
{
	if (m_initialized)
		return false;

	if (open_tag == nullptr || *open_tag == '\0')
	{
		return false;
	}

	if (closing_tag == nullptr || *closing_tag == '\0')
	{
		m_is_single = true;
	}
	else
	{
		m_is_single = false;
	}

	m_regex_open.assign(open_tag, boost::regex_constants::extended | boost::regex_constants::optimize);

	if (!m_is_single)
	{
		m_regex_closing.assign(closing_tag, boost::regex_constants::extended | boost::regex_constants::optimize);
		m_subtags = allow_subtags;
	}

	m_initialized = true;
	return true;
}

bool vs::HtmlEscaper::Tag::AreSubtagsAllowed(void) const
{
	return m_subtags;
}

bool vs::HtmlEscaper::Tag::IsSingle(void) const
{
	return m_is_single;
}

// Lexical analysis routines
size_t vs::HtmlEscaper::Tag::MatchOpen(const char *data) const
{
	return Match(m_regex_open, data);
}

size_t vs::HtmlEscaper::Tag::MatchClosing(const char *data) const
{
	return Match(m_regex_closing, data);
}

size_t vs::HtmlEscaper::Tag::Match(const boost::regex &r, const char *data) const
{
	if (m_initialized == false || data == nullptr || *data == '\0')
	{
		return 0;
	}

	boost::cmatch m;
	if (!boost::regex_search(data, m, r, boost::regex_constants::format_first_only))
	{
		return 0;
	}

	if (m[0].first != data)
	{
		return 0;
	}

	return (m[0].second - m[0].first);
}
