#include <stdlib.h>
#include <math.h>

#include "VS_H263Parser.h"
#include "std-generic/cpplib/hton.h"

static const unsigned short h263_format_resolution[6][2] = {
    { 0, 0 },
    { 128, 96 },
    { 176, 144 },
    { 352, 288 },
    { 704, 576 },
    { 1408, 1152 },
};

const unsigned int bits_read[] =
{
    (((unsigned int)0x01 << (0)) - 1),
    (((unsigned int)0x01 << (1)) - 1),
    (((unsigned int)0x01 << (2)) - 1),
    (((unsigned int)0x01 << (3)) - 1),
    (((unsigned int)0x01 << (4)) - 1),
    (((unsigned int)0x01 << (5)) - 1),
    (((unsigned int)0x01 << (6)) - 1),
    (((unsigned int)0x01 << (7)) - 1),
    (((unsigned int)0x01 << (8)) - 1),
    (((unsigned int)0x01 << (9)) - 1),
    (((unsigned int)0x01 << (10)) - 1),
    (((unsigned int)0x01 << (11)) - 1),
    (((unsigned int)0x01 << (12)) - 1),
    (((unsigned int)0x01 << (13)) - 1),
    (((unsigned int)0x01 << (14)) - 1),
    (((unsigned int)0x01 << (15)) - 1),
    (((unsigned int)0x01 << (16)) - 1),
    (((unsigned int)0x01 << (17)) - 1),
    (((unsigned int)0x01 << (18)) - 1),
    (((unsigned int)0x01 << (19)) - 1),
    (((unsigned int)0x01 << (20)) - 1),
    (((unsigned int)0x01 << (21)) - 1),
    (((unsigned int)0x01 << (22)) - 1),
    (((unsigned int)0x01 << (23)) - 1),
    (((unsigned int)0x01 << (24)) - 1),
    (((unsigned int)0x01 << (25)) - 1),
    (((unsigned int)0x01 << (26)) - 1),
    (((unsigned int)0x01 << (27)) - 1),
    (((unsigned int)0x01 << (28)) - 1),
    (((unsigned int)0x01 << (29)) - 1),
    (((unsigned int)0x01 << (30)) - 1),
    (((unsigned int)0x01 << (31)) - 1),
    ((unsigned int)0xFFFFFFFF),
};

int GetBits(const unsigned int* &current_data, int &offset, unsigned int nbits)
{
    unsigned int x;
	unsigned int cdata0 = vs_ntohl(current_data[0]);
	unsigned int cdata1 = vs_ntohl(current_data[1]);

    offset -= (nbits);
    if(offset >= 0) {
        x = cdata0 >> (offset + 1);
    } else {
        offset += 32;
        x = cdata1 >> (offset);
        x >>= 1;
        x += cdata0 << (31 - offset);
        current_data++;
    }
    return (x & bits_read[nbits]);
}

int ResolutionFromBitstream_H263(const void* in, int insize, int &width, int &height)
{
    int format = 0, i = 0, offset = 31, codeNum = 0, ufep = 0;
    unsigned int startcode = 0;
	const unsigned int *ipb = static_cast<const unsigned int*>(in);

	if (insize < 4) return -1;

	startcode = GetBits(ipb, offset, 22-8);

    for(i = 8 * insize - 14; i > 24; i -= 8) {
        startcode = ((startcode << 8) | GetBits(ipb, offset, 8)) & 0x003FFFFF;
        if(startcode == 0x20)
            break;
    }

	if (startcode != 0x20) return -1;

	codeNum = GetBits(ipb, offset, 8); /* picture timestamp */
	codeNum = GetBits(ipb, offset, 1);
	codeNum = GetBits(ipb, offset, 1);
	codeNum = GetBits(ipb, offset, 1);
	codeNum = GetBits(ipb, offset, 1);
	codeNum = GetBits(ipb, offset, 1);

    format = GetBits(ipb, offset, 3);

	if (format != 7 && format != 6) {
		// h.263
        width = h263_format_resolution[format][0];
        height = h263_format_resolution[format][1];
		return 0;
	} else {
		// h.263p
		ufep = GetBits(ipb, offset, 3); /* Update Full Extended PTYPE */
		if (ufep > 1) return -1;
		if (ufep == 1) {
		    /* OPPTYPE */
            format = GetBits(ipb, offset, 3);
			codeNum = GetBits(ipb, offset, 1); /* custom pcf */
			codeNum = GetBits(ipb, offset, 1); /* Unrestricted Motion Vector */
			codeNum = GetBits(ipb, offset, 1); /* SAC*/
			codeNum = GetBits(ipb, offset, 1); /* Advanced prediction mode */
			codeNum = GetBits(ipb, offset, 1); /* Advanced Intra Coding (AIC) */
			codeNum = GetBits(ipb, offset, 1); /* loop filter */
			codeNum = GetBits(ipb, offset, 1); /* slice structure */
			codeNum = GetBits(ipb, offset, 1); /* referebce picture */
			codeNum = GetBits(ipb, offset, 1); /* Indepent Segment Decoding */
			codeNum = GetBits(ipb, offset, 1); /* inter VLC */
			codeNum = GetBits(ipb, offset, 1); /* modified quant */
			GetBits(ipb, offset, 1); /* Prevent start code emulation */
			GetBits(ipb, offset, 3); /* Reserved */
		}
		codeNum = GetBits(ipb, offset, 3); /* MPPTYPE */
		GetBits(ipb, offset, 2);
		codeNum = GetBits(ipb, offset, 1); /* no rounding */
		GetBits(ipb, offset, 4);
		/* Get the picture dimensions */
		if (ufep) {
			if (format == 6) {
				GetBits(ipb, offset, 4);
				codeNum = GetBits(ipb, offset, 9);
                width = (codeNum + 1) * 4;
                GetBits(ipb, offset, 1);
				codeNum = GetBits(ipb, offset, 9);
                height = codeNum * 4;
				return 0;
			} else {
				width = h263_format_resolution[format][0];
				height = h263_format_resolution[format][1];
				return 0;
			}
		}
	}

	return -1;
}