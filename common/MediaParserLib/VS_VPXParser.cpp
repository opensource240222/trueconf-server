#include <stdlib.h>
#include <math.h>

#include "VS_VPXParser.h"

// get frame resolution from bitstream
// return:
// 0 - successfully decode frame resolution
// < 0 - try next compressed frame
int	ResolutionFromBitstream_VPX(const void *in, int insize, int& width, int& height, int& nthreads)
{
	int res = -1;
	const unsigned char *pb = static_cast<const unsigned char*>(in);

    if (insize < 10)
        return -1;

	bool bSynch = (pb[3] == 0x9d && pb[4] == 0x01 && pb[5] == 0x2a);

	if (bSynch) {
		width  = *(unsigned short*)(pb + 6) & 0x3fff;
		height = *(unsigned short*)(pb + 8) & 0x3fff;
		nthreads = 1;
		res = 0;
	} else {
		if (insize < 18) return -1;
		bSynch = (pb[11] == 0x9d && pb[12] == 0x01 && pb[13] == 0x2a);
		if (bSynch) {
			nthreads = *(int*)pb;
			if (nthreads < 2 || nthreads > 6) return -1;
			int size = 0, w = 0, h = 0;
			pb += 4;
			insize -= 4;
			if (nthreads == 2) {
				size = *(int*)pb;
				if (size + 4 > insize) return -1;
				pb += 4;
				w  = *(unsigned short*)(pb + 6) & 0x3fff;
				h += *(unsigned short*)(pb + 8) & 0x3fff;
			} else {
				for (int i = 0; i < nthreads; i++) {
					size = *(int*)pb;
					if (size + 4 > insize) return -1;
					pb += 4;
					w  = *(unsigned short*)(pb + 6) & 0x3fff;
					h += *(unsigned short*)(pb + 8) & 0x3fff;
					h -= 32;
					if (i > 0 && i < nthreads - 1) h -= 32;
					pb += size;
					insize -= size;
				}
			}
			width = w;
			height = h;
			res = 0;
		}
	}

    return res;
}