
#ifndef VS_H263PARSER_TYPES_H
#define VS_H263PARSER_TYPES_H

#define mmax(a, b)  	((a) > (b) ? (a) : (b))
#define mmin(a, b)  	((a) < (b) ? (a) : (b))
#define mnint(a)        ((a) < 0 ? (int)(a - 0.5) : (int)(a + 0.5))
#define sign(a)         ((a) < 0 ? -1 : 1)

#ifndef PI
# ifdef M_PI
#  define PI M_PI
# else
#  define PI 3.14159265358979323846
# endif
#endif


#define TMN_ERROR (-1)

/* From sim.h */
#define PSC								1
#define PSC_LENGTH						17
#define SEC                             31

#define MODE_INTER                      0
#define MODE_INTER_Q                    1
#define MODE_INTER4V                    2
#define MODE_INTRA                      3
#define MODE_INTRA_Q                    4

#define PBMODE_NORMAL                   0
#define PBMODE_MVDB                     1
#define PBMODE_CBPB_MVDB                2

#define ESCAPE                          7167
#define ESCAPE_INDEX                    102

#define PCT_INTER                       1
#define PCT_INTRA                       0
#define ON                              1
#define OFF                             0

#define SF_SQCIF                        1  /* 001 */
#define SF_QCIF                         2  /* 010 */
#define SF_CIF                          3  /* 011 */
#define SF_4CIF                         4  /* 100 */
#define SF_16CIF                        5  /* 101 */

// Corresponds to CIF
#define MBC_MAX                         22
#define MBR_MAX                         18

#define NO_VEC                          999

#define T_YUV      0
#define T_SIF      1
#define T_TGA      2
#define T_PPM      3
#define T_X11      4
#define T_YUV_CONC 5


struct VLCtab{
	int val, len;
};

struct VLCtabI{
	int val, len;
};

struct DCTtab{
	char run, level, len;
};

struct RunCoef{
	int val, run, sign;
};

struct RdBuffer{
	/* bit input */
	unsigned char *large_buffer;
	unsigned char *plb;
	int lbsize;

	unsigned char rdbfr[2051];
	unsigned char *rdptr;
	unsigned char inbfr[16];
	int incnt;
	int bitcnt;
	/* block data */
	short block[12][64];
};

#endif
