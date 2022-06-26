#include <stdlib.h>
#include <math.h>

#include "VS_XCCParser.h"

// get frame resolution from bitstream
// return:
// 0 - successfully decode frame resolution
// < 0 - try next compressed frame
int ResolutionFromBitstream_XCC(unsigned char *in, int insize, int& width, int& height)
{
	int res = -1;
	unsigned char *pb = in;

    if (insize < 2)
        return -1;

	unsigned short hdr = pb[0] + ((int)pb[1] << 8);
	bool bSynch = (((hdr >> 13) & 7) == 0);

	if (bSynch) {
		width  = (((hdr >> 7) & 63) + 1) * 16;
		height = (((hdr >> 1) & 63) + 1) * 16;
		res = 0;
	}

    return res;
}