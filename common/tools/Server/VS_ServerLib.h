
#pragma once

inline bool MakeChangeDir( const wchar_t *dir )
{
	const int   st = _waccess( dir, 06 );
	if (!st) {	_wchdir( dir );	return true;	}
	if (st == -1 && errno == ENOENT)
		if (!_wmkdir( dir )) {	_wchdir( dir );	return true;	}
	return false;
}
// end MakeChangeDir
