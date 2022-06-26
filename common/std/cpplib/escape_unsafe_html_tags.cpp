#include "escape_unsafe_html_tags.h"
#include "HtmlEscaper.h"

#include <memory>
#include <mutex>

#include <cassert>

static std::unique_ptr<vs::HtmlEscaper> escaper;
static std::mutex lock;

static void init(void)
{
	std::lock_guard<std::mutex> l(lock);
	if (escaper == nullptr)
	{
		escaper.reset(new vs::HtmlEscaper);
		// NOTE: the order could be important, especially when adding tags whose names start with the same letter (like <BR> and <B>).
		// Consider adding longer tags first.
		auto res = escaper->AddAllowedTag("^(a|A)[[:space:]]+(h|H)(r|R)(e|E)(f|F)[[:space:]]*=[[:space:]]*(\"([[:alnum:]]+:?(//)?)?[^\"]*\"|'([[:alnum:]]+:?(//)?)?[^']*')", "^(a|A)"); // <a href="http://link.to">...</a>
		assert(res == true);
		res = escaper->AddAllowedTag("^(b|B)(r|R)");     // <br>
		assert(res == true);
		//escaper->AddAllowedTag("^(p|P)", "^(p|P)", true); //<p>...</p>
		//assert(res == true);
		res = escaper->AddAllowedTag("^(i|I)", "^(i|I)", true); //<i>...</i>
		assert(res == true);
		res = escaper->AddAllowedTag("^(u|U)", "^(u|U)", true); //<u>...</u>
		assert(res == true);
		res = escaper->AddAllowedTag("^(b|B)", "^(b|B)", true); //<b>...</b>
		assert(res == true);
		escaper->Finalize();
	}
}

bool vs::escape_unsafe_html_tags(const char *in, std::string &out)
{
	init();
	return escaper->EscapeString(in, out);
}

bool vs::escape_unsafe_html_tags(const std::string &in, std::string &out)
{
	init();
	return escaper->EscapeString(in, out);
}
