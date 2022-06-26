
#ifndef VS_BUFFER_H
#define VS_BUFFER_H

#include <string.h>

struct VS_Buffer
{
	unsigned long   length;
	void           *buffer;
};
// end VS_Buffer struct

inline unsigned long VS_BuffersLength( const VS_Buffer *buffers, const unsigned long n_buffers )
{
	unsigned long i, bytes;
	for (i = bytes = 0; i < n_buffers; ++i)		bytes += buffers[i].length;
	return bytes;
}
// end VS_BuffersLength

inline unsigned long VS_BuffersCopy( void *buffer, const VS_Buffer *buffers, const unsigned long n_buffers )
{
	unsigned long i, bytes;
	for (i = bytes = 0; i < n_buffers; ++i)
	{
		memcpy( (void *)&((char *)buffer)[bytes], buffers[i].buffer, (size_t)buffers[i].length );
		bytes += buffers[i].length;
	}
	return bytes;
}
// end VS_BuffersLength

#endif // VS_BUFFER_H
