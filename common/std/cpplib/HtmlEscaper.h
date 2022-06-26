#pragma once

#include <vector>
#include <string>
//#include <regex>
#include <boost/regex.hpp>

namespace vs {
	class HtmlEscaper {
	public:
		class Tag {
		public:
			Tag(void);
			virtual ~Tag(void);

			bool Init(const char *open_tag, const char *closing_tag, const bool allow_subtags);

			size_t MatchOpen(const char *data) const;
			size_t MatchClosing(const char *data) const;

			bool AreSubtagsAllowed(void) const;
			bool IsSingle(void) const;
		private:
			size_t Match(const boost::regex &r, const char *data) const;
		private:

			bool m_subtags;
			bool m_is_single;
			boost::regex m_regex_open;
			boost::regex m_regex_closing;

			bool m_initialized;
		};
	public:
		HtmlEscaper(void);
		virtual ~HtmlEscaper(void);

		bool EscapeString(const char *in, std::string &out) const; // After the object finalization this method is reentrable.
		bool EscapeString(const std::string &in, std::string &out) const;

		// IMPORTANT: the order, in which the tags are added, is important.
		// Please consider adding longer matching tags first.
		// In the case of lexcial conflicts try to reorder the calls to AddTags to resolve them.
		// Also, please do not forget to prepend '^' character to the regular expression (I am certain you *DO* want it).
		bool AddAllowedTag(const char *opening_tag_regex, const char *closing_tag_regex = nullptr, const bool allow_subtags = false);
		bool Clear(void);
		void Finalize(void);
	private:
		size_t FindTag(size_t &tag, const char *in) const;

		// Parser methods:
		static void EscapeChar(const char *&in, std::string &out);
		static void EscapeNonTagCharacters(const char *&in, std::string &out);
		void EscapeTagSequence(const char *&in, std::string &out, bool subsequence = false) const;
		bool EscapeTag(const char *&in, std::string &out) const;
	private:

		bool m_finalized;
		std::vector<Tag> m_tags;
	};
}
