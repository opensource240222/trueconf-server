/////////////////////////////////////////////////////////////////////////////////////////
//
//  (c) 2002 Visicron Inc.  http://www.visicron.net/
//
//  Project:
//
//  Created: 13.04.04     by  A.Slavetsky
//
/////////////////////////////////////////////////////////////////////////////////////////
///
/// \file VS_AddLogHeadMedia.h
/// \brief
/// \note
///
/////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <stdio.h>

#pragma pack( 1 )

struct VS_AddLogHeadMedia
{
	/////////////////////////////////////////////////////////////////////////////////////

	VS_AddLogHeadMedia( void ) {	ZeroMemory( (void *)this, sizeof(*this) );	}
	// end of VS_AddLogHeadMedia constructor

	VS_AddLogHeadMedia( FILE *file ) {	ReadHead( file );	}
	// end of VS_AddLogHeadMedia constructor

	unsigned char	direction;	// 0 - send (to H323Terminal, from Bridge),
								// 1 - receive (from H323Terminal, to Bridge).
	unsigned char	channel;	// 0 - 3 number of media channel.
								// Video or Audio see RTP Heading - Payload Type field,
								// or Any Other Attributes in the data. :)
	unsigned long	mills;		// Milliseconds between last packages.
	unsigned short	length;		// Length without heading.

	inline bool ReadHead( FILE *file )
	{
		if (!file || fread( (void *)this, sizeof(*this), 1, file ) != 1
				|| direction > 1 || channel > 3)
		{	ZeroMemory( (void *)this, sizeof(*this) );	return false;	}
		return true;
	}
	// end of VS_AddLogHeadMedia::ReadHead

	inline bool ReadData( FILE *file, void *buffer, const unsigned size_buffer )
	{
		if (!file || !length || (unsigned)length > size_buffer)		return false;
		return fread( buffer, (size_t)length, 1, file ) == 1;
	}
	// end of VS_AddLogHeadMedia::ReadHead

	/////////////////////////////////////////////////////////////////////////////////////
};
// end of VS_AddLogHeadMedia

#pragma pack(   )

/////////////////////////////////////////////////////////////////////////////////////////
