#ifndef _MEDIUM_DELETER_HH
#define _MEDIUM_DELETER_HH

#include <Media.hh>

struct Medium_deleter
{
	void operator()(Medium* p) const
	{
		Medium::close(p);
	}
};

#endif
