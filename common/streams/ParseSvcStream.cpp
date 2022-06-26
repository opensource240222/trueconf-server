#include "ParseSvcStream.h"

unsigned char* ParseSvcStream(unsigned char *in, int size, bool svc, int *lsize, stream::SVCHeader *h, int *shift_size)
{
	if (in == 0 || size <= 0) return 0;
	unsigned char *out = in;
	*lsize = size;
	*shift_size = size;
	if (svc) {
		int hdrsize = sizeof(int) + sizeof(stream::SVCHeader);
		if (size <= hdrsize) return 0;
		*lsize = *(int*)out;
		*shift_size = *lsize + sizeof(int);
		*lsize -= sizeof(stream::SVCHeader);
		if (*lsize > size - hdrsize) return 0;
		out += sizeof(int);
		memcpy(h, out, sizeof(stream::SVCHeader));
		out += sizeof(stream::SVCHeader);
	}
	return out;
}
