#pragma once

#include <comutil.h>

class VS_DBNULL_OBJ
{
protected:
	_variant_t		db_null;
public:
	VS_DBNULL_OBJ() : db_null()
	{
		db_null.ChangeType(VT_NULL);
	}
};