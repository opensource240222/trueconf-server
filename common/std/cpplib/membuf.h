#pragma once

#include <streambuf>

template <class charT, class traits = std::char_traits<charT> >
class basic_omembuf : public std::basic_streambuf<charT, traits>
{
public:
	typedef std::basic_streambuf<charT, traits> base_t;
	typedef typename base_t::char_type char_type;
	typedef typename base_t::traits_type traits_type;
	typedef typename base_t::int_type int_type;
	typedef typename base_t::pos_type pos_type;
	typedef typename base_t::off_type off_type;

	basic_omembuf()
	{
	}
	basic_omembuf(char_type* p, size_t size, size_t data_size = 0)
	{
		this->reset(p, size, data_size);
	}
	basic_omembuf(char_type* begin, char_type* end, char_type* data_end = nullptr)
	{
		this->reset(begin, end, data_end);
	}

	void reset(char_type* p, size_t size, size_t data_size = 0)
	{
		this->reset(p, p + size, p + data_size);
	}
	void reset(char_type* begin, char_type* end, char_type* data_end = nullptr)
	{
		this->setp(begin, end);
		if (data_end)
			this->pbump(data_end - begin);
	}

protected:
	base_t* setbuf(char_type* s, std::streamsize n) override
	{
		this->setp(s, s + n);
		return this;
	}

	pos_type seekoff(off_type off, std::ios_base::seekdir way, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out) override
	{
		if (which & std::ios_base::out)
		{
			switch (way)
			{
			case std::ios_base::beg: break;
			case std::ios_base::cur: off += this->pptr() - this->pbase(); break;
			case std::ios_base::end: off += this->epptr() - this->pbase(); break;
			default: off = pos_type(off_type(-1)); break;
			}
		}
		else
			off = pos_type(off_type(-1));
		return basic_omembuf::seekpos(off, which);
	}

	pos_type seekpos(pos_type sp, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out) override
	{
		if (which & std::ios_base::out)
		{
			if (sp >= 0 && sp <= this->epptr() - this->pbase())
				this->pbump((int)(sp - off_type(this->pptr() - this->pbase())));
			else
				sp = pos_type(off_type(-1));
		}
		else
			sp = pos_type(off_type(-1));
		return sp;
	}
};

typedef basic_omembuf<char> omembuf;
typedef basic_omembuf<wchar_t> owmembuf;
