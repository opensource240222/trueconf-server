
#ifndef VS_H263PARSER_TABLES_H
#define VS_H263PARSER_TABLES_H

#include "types.h"

// vlc
static VLCtab TMNMVtab0[] = {
	{3,4}, {61,4}, {2,3}, {2,3}, {62,3}, {62,3},
	{1,2}, {1,2}, {1,2}, {1,2}, {63,2}, {63,2}, {63,2}, {63,2}
};

static VLCtab TMNMVtab1[] = {
	{12,10}, {52,10}, {11,10}, {53,10}, {10,9}, {10,9},
	{54,9}, {54,9}, {9,9}, {9,9}, {55,9}, {55,9},
	{8,9}, {8,9}, {56,9}, {56,9}, {7,7}, {7,7},
	{7,7}, {7,7}, {7,7}, {7,7}, {7,7}, {7,7},
	{57,7}, {57,7}, {57,7}, {57,7}, {57,7}, {57,7},
	{57,7}, {57,7}, {6,7}, {6,7}, {6,7}, {6,7},
	{6,7}, {6,7}, {6,7}, {6,7}, {58,7}, {58,7},
	{58,7}, {58,7}, {58,7}, {58,7}, {58,7}, {58,7},
	{5,7}, {5,7}, {5,7}, {5,7}, {5,7}, {5,7},
	{5,7}, {5,7}, {59,7}, {59,7}, {59,7}, {59,7},
	{59,7}, {59,7}, {59,7}, {59,7}, {4,6}, {4,6},
	{4,6}, {4,6}, {4,6}, {4,6}, {4,6}, {4,6},
	{4,6}, {4,6}, {4,6}, {4,6}, {4,6}, {4,6},
	{4,6}, {4,6}, {60,6}, {60,6},{60,6},{60,6},
	{60,6},{60,6},{60,6},{60,6},{60,6},{60,6},
	{60,6},{60,6},{60,6},{60,6},{60,6},{60,6}
};

static VLCtab TMNMVtab2[] = {
	{32,12}, {31,12}, {33,12}, {30,11}, {30,11}, {34,11},
	{34,11}, {29,11}, {29,11}, {35,11}, {35,11}, {28,11},
	{28,11}, {36,11}, {36,11}, {27,11}, {27,11}, {37,11},
	{37,11}, {26,11}, {26,11}, {38,11}, {38,11}, {25,11},
	{25,11}, {39,11}, {39,11}, {24,10}, {24,10}, {24,10},
	{24,10}, {40,10}, {40,10}, {40,10}, {40,10}, {23,10},
	{23,10}, {23,10}, {23,10}, {41,10}, {41,10}, {41,10},
	{41,10}, {22,10}, {22,10}, {22,10}, {22,10}, {42,10},
	{42,10}, {42,10}, {42,10}, {21,10}, {21,10}, {21,10},
	{21,10}, {43,10}, {43,10}, {43,10}, {43,10}, {20,10},
	{20,10}, {20,10}, {20,10}, {44,10}, {44,10}, {44,10},
	{44,10}, {19,10}, {19,10}, {19,10}, {19,10}, {45,10},
	{45,10}, {45,10}, {45,10}, {18,10}, {18,10}, {18,10},
	{18,10}, {46,10}, {46,10}, {46,10}, {46,10}, {17,10},
	{17,10}, {17,10}, {17,10}, {47,10}, {47,10}, {47,10},
	{47,10}, {16,10}, {16,10}, {16,10}, {16,10}, {48,10},
	{48,10}, {48,10}, {48,10}, {15,10}, {15,10}, {15,10},
	{15,10}, {49,10}, {49,10}, {49,10}, {49,10}, {14,10},
	{14,10}, {14,10}, {14,10}, {50,10}, {50,10}, {50,10},
	{50,10}, {13,10}, {13,10}, {13,10}, {13,10}, {51,10},
	{51,10}, {51,10}, {51,10}
};


static VLCtab MCBPCtab[] = {
	{TMN_ERROR,0},
	{255,9}, {52,9}, {36,9}, {20,9}, {49,9}, {35,8}, {35,8}, {19,8}, {19,8},
	{50,8}, {50,8}, {51,7}, {51,7}, {51,7}, {51,7}, {34,7}, {34,7}, {34,7},
	{34,7}, {18,7}, {18,7}, {18,7}, {18,7}, {33,7}, {33,7}, {33,7}, {33,7},
	{17,7}, {17,7}, {17,7}, {17,7}, {4,6}, {4,6}, {4,6}, {4,6}, {4,6},
	{4,6}, {4,6}, {4,6}, {48,6}, {48,6}, {48,6}, {48,6}, {48,6}, {48,6},
	{48,6}, {48,6}, {3,5}, {3,5}, {3,5}, {3,5}, {3,5}, {3,5}, {3,5},
	{3,5}, {3,5}, {3,5}, {3,5}, {3,5}, {3,5}, {3,5}, {3,5}, {3,5},
	{32,4}, {32,4}, {32,4}, {32,4}, {32,4}, {32,4}, {32,4}, {32,4}, {32,4},
	{32,4}, {32,4}, {32,4}, {32,4}, {32,4}, {32,4}, {32,4}, {32,4}, {32,4},
	{32,4}, {32,4}, {32,4}, {32,4}, {32,4}, {32,4}, {32,4}, {32,4}, {32,4},
	{32,4}, {32,4}, {32,4}, {32,4}, {32,4}, {16,4}, {16,4}, {16,4}, {16,4},
	{16,4}, {16,4}, {16,4}, {16,4}, {16,4}, {16,4}, {16,4}, {16,4}, {16,4},
	{16,4}, {16,4}, {16,4}, {16,4}, {16,4}, {16,4}, {16,4}, {16,4}, {16,4},
	{16,4}, {16,4}, {16,4}, {16,4}, {16,4}, {16,4}, {16,4}, {16,4}, {16,4},
	{16,4}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3},
	{2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3},
	{2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3},
	{2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3},
	{2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3},
	{2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3},
	{2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3},
	{2,3}, {2,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3},
	{1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3},
	{1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3},
	{1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3},
	{1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3},
	{1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3},
	{1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3},
	{1,3}, {1,3}, {1,3},
};



static VLCtab MCBPCtabintra[] = {
	{TMN_ERROR,0},
	{20,6}, {36,6}, {52,6}, {4,4}, {4,4}, {4,4},
	{4,4}, {19,3}, {19,3}, {19,3}, {19,3}, {19,3},
	{19,3}, {19,3}, {19,3}, {35,3}, {35,3}, {35,3},
	{35,3}, {35,3}, {35,3}, {35,3}, {35,3}, {51,3},
	{51,3}, {51,3}, {51,3}, {51,3}, {51,3}, {51,3},
	{51,3},
};



static VLCtab CBPYtab[48] =
{ {TMN_ERROR,0}, {TMN_ERROR,0}, {9,6}, {6,6}, {7,5}, {7,5}, {11,5}, {11,5},
{13,5}, {13,5}, {14,5}, {14,5}, {15,4}, {15,4}, {15,4}, {15,4},
{3,4}, {3,4}, {3,4}, {3,4}, {5,4},{5,4},{5,4},{5,4},
{1,4}, {1,4}, {1,4}, {1,4}, {10,4}, {10,4}, {10,4}, {10,4},
{2,4}, {2,4}, {2,4}, {2,4}, {12,4}, {12,4}, {12,4}, {12,4},
{4,4}, {4,4}, {4,4}, {4,4}, {8,4}, {8,4}, {8,4}, {8,4},
};


static VLCtabI DCT3Dtab0[] = {
	{4225,7}, {4209,7}, {4193,7}, {4177,7}, {193,7}, {177,7},
	{161,7}, {4,7}, {4161,6}, {4161,6}, {4145,6}, {4145,6},
	{4129,6}, {4129,6}, {4113,6}, {4113,6}, {145,6}, {145,6},
	{129,6}, {129,6}, {113,6}, {113,6}, {97,6}, {97,6},
	{18,6}, {18,6}, {3,6}, {3,6}, {81,5}, {81,5},
	{81,5}, {81,5}, {65,5}, {65,5}, {65,5}, {65,5},
	{49,5}, {49,5}, {49,5}, {49,5}, {4097,4}, {4097,4},
	{4097,4}, {4097,4}, {4097,4}, {4097,4}, {4097,4}, {4097,4},
	{1,2}, {1,2}, {1,2}, {1,2}, {1,2}, {1,2},
	{1,2}, {1,2}, {1,2}, {1,2}, {1,2}, {1,2},
	{1,2}, {1,2}, {1,2}, {1,2}, {1,2}, {1,2},
	{1,2}, {1,2}, {1,2}, {1,2}, {1,2}, {1,2},
	{1,2}, {1,2}, {1,2}, {1,2}, {1,2}, {1,2},
	{1,2}, {1,2}, {17,3}, {17,3}, {17,3}, {17,3},
	{17,3}, {17,3}, {17,3}, {17,3}, {17,3}, {17,3},
	{17,3}, {17,3}, {17,3}, {17,3}, {17,3}, {17,3},
	{33,4}, {33,4}, {33,4}, {33,4}, {33,4}, {33,4},
	{33,4}, {33,4}, {2,4}, {2,4},{2,4},{2,4},
	{2,4}, {2,4},{2,4},{2,4},
};


static VLCtabI DCT3Dtab1[] = {
	{9,10}, {8,10}, {4481,9}, {4481,9}, {4465,9}, {4465,9},
	{4449,9}, {4449,9}, {4433,9}, {4433,9}, {4417,9}, {4417,9},
	{4401,9}, {4401,9}, {4385,9}, {4385,9}, {4369,9}, {4369,9},
	{4098,9}, {4098,9}, {353,9}, {353,9}, {337,9}, {337,9},
	{321,9}, {321,9}, {305,9}, {305,9}, {289,9}, {289,9},
	{273,9}, {273,9}, {257,9}, {257,9}, {241,9}, {241,9},
	{66,9}, {66,9}, {50,9}, {50,9}, {7,9}, {7,9},
	{6,9}, {6,9}, {4353,8}, {4353,8}, {4353,8}, {4353,8},
	{4337,8}, {4337,8}, {4337,8}, {4337,8}, {4321,8}, {4321,8},
	{4321,8}, {4321,8}, {4305,8}, {4305,8}, {4305,8}, {4305,8},
	{4289,8}, {4289,8}, {4289,8}, {4289,8}, {4273,8}, {4273,8},
	{4273,8}, {4273,8}, {4257,8}, {4257,8}, {4257,8}, {4257,8},
	{4241,8}, {4241,8}, {4241,8}, {4241,8}, {225,8}, {225,8},
	{225,8}, {225,8}, {209,8}, {209,8}, {209,8}, {209,8},
	{34,8}, {34,8}, {34,8}, {34,8}, {19,8}, {19,8},
	{19,8}, {19,8}, {5,8}, {5,8}, {5,8}, {5,8},
};

static VLCtabI DCT3Dtab2[] = {
	{4114,11}, {4114,11}, {4099,11}, {4099,11}, {11,11}, {11,11},
	{10,11}, {10,11}, {4545,10}, {4545,10}, {4545,10}, {4545,10},
	{4529,10}, {4529,10}, {4529,10}, {4529,10}, {4513,10}, {4513,10},
	{4513,10}, {4513,10}, {4497,10}, {4497,10}, {4497,10}, {4497,10},
	{146,10}, {146,10}, {146,10}, {146,10}, {130,10}, {130,10},
	{130,10}, {130,10}, {114,10}, {114,10}, {114,10}, {114,10},
	{98,10}, {98,10}, {98,10}, {98,10}, {82,10}, {82,10},
	{82,10}, {82,10}, {51,10}, {51,10}, {51,10}, {51,10},
	{35,10}, {35,10}, {35,10}, {35,10}, {20,10}, {20,10},
	{20,10}, {20,10}, {12,11}, {12,11}, {21,11}, {21,11},
	{369,11}, {369,11}, {385,11}, {385,11}, {4561,11}, {4561,11},
	{4577,11}, {4577,11}, {4593,11}, {4593,11}, {4609,11}, {4609,11},
	{22,12}, {36,12}, {67,12}, {83,12}, {99,12}, {162,12},
	{401,12}, {417,12}, {4625,12}, {4641,12}, {4657,12}, {4673,12},
	{4689,12}, {4705,12}, {4721,12}, {4737,12}, {7167,7},
	{7167,7}, {7167,7}, {7167,7}, {7167,7}, {7167,7}, {7167,7},
	{7167,7}, {7167,7}, {7167,7}, {7167,7}, {7167,7}, {7167,7},
	{7167,7}, {7167,7}, {7167,7}, {7167,7}, {7167,7}, {7167,7},
	{7167,7}, {7167,7}, {7167,7}, {7167,7}, {7167,7}, {7167,7},
	{7167,7}, {7167,7}, {7167,7}, {7167,7}, {7167,7}, {7167,7},
	{7167,7},
};

// globals
static unsigned char zig_zag_scan[64]={
	0,1,8,16,9,2,3,10,17,24,32,25,18,11,4,5,
		12,19,26,33,40,48,41,34,27,20,13,6,7,14,21,28,
		35,42,49,56,57,50,43,36,29,22,15,23,30,37,44,51,
		58,59,52,45,38,31,39,46,53,60,61,54,47,55,62,63
};

static int convmat[8][4]={
	{117504, 138453, 13954, 34903}, /* no sequence_display_extension */
	{117504, 138453, 13954, 34903}, /* ITU-R Rec. 709 (1990) */
	{104597, 132201, 25675, 53279}, /* unspecified */
	{104597, 132201, 25675, 53279}, /* reserved */
	{104448, 132798, 24759, 53109}, /* FCC */
	{104597, 132201, 25675, 53279}, /* ITU-R Rec. 624-4 System B, G */
	{104597, 132201, 25675, 53279}, /* SMPTE 170M */
	{117579, 136230, 16907, 35559}  /* SMPTE 240M (1987) */
};

static int bscan_tab[]= {2,4,6,8};
static int bquant_tab[]= {5,6,7,8};

static int OM[5][8][8]= {
	{
		{4,5,5,5,5,5,5,4},
		{5,5,5,5,5,5,5,5},
		{5,5,6,6,6,6,5,5},
		{5,5,6,6,6,6,5,5},
		{5,5,6,6,6,6,5,5},
		{5,5,6,6,6,6,5,5},
		{5,5,5,5,5,5,5,5},
		{4,5,5,5,5,5,5,4},
	},{
		{2,2,2,2,2,2,2,2},
		{1,1,2,2,2,2,1,1},
		{1,1,1,1,1,1,1,1},
		{1,1,1,1,1,1,1,1},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
	},{
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{1,1,1,1,1,1,1,1},
		{1,1,1,1,1,1,1,1},
		{1,1,2,2,2,2,1,1},
		{2,2,2,2,2,2,2,2},
	},{
		{0,0,0,0,1,1,1,2},
		{0,0,0,0,1,1,2,2},
		{0,0,0,0,1,1,2,2},
		{0,0,0,0,1,1,2,2},
		{0,0,0,0,1,1,2,2},
		{0,0,0,0,1,1,2,2},
		{0,0,0,0,1,1,2,2},
		{0,0,0,0,1,1,1,2},
	},{
		{2,1,1,1,0,0,0,0},
		{2,2,1,1,0,0,0,0},
		{2,2,1,1,0,0,0,0},
		{2,2,1,1,0,0,0,0},
		{2,2,1,1,0,0,0,0},
		{2,2,1,1,0,0,0,0},
		{2,2,1,1,0,0,0,0},
		{2,1,1,1,0,0,0,0},
	}
};

static int roundtab[16]=  {0,0,0,1,1,1,1,1,1,1,1,1,1,1,2,2};

// indices

static int codtab[2]= {0,1};
static int mcbpctab[21] = {0,16,32,48,1,17,33,49,2,18,34,50,3,19,35,51,4,20,36,52,255};
static int mcbpc_intratab[9] = {3,19,35,51,4,20,36,52,255};
static int modb_tab[3] = {0, 1, 2};
static int ycbpb_tab[2] = {0, 1};
static int uvcbpb_tab[2] = {0, 1};
static int cbpytab[16] = {15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0};
static int cbpy_intratab[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
static int dquanttab[4] = {1,0,3,4};
static int mvdtab[64] = {32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
static int intradctab[254] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,255,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,240,241,242,243,244,245,246,247,248,249,250,251,252,253,254};
static int tcoeftab[103] = {1,2,3,4,5,6,7,8,9,10,11,12,17,18,19,20,21,22,33,34,35,36,49,50,51,65,66,67,81,82,83,97,98,99,113,114,129,130,145,146,161,162,177,193,209,225,241,257,273,289,305,321,337,353,369,385,401,417,4097,4098,4099,4113,4114,4129,4145,4161,4177,4193,4209,4225,4241,4257,4273,4289,4305,4321,4337,4353,4369,4385,4401,4417,4433,4449,4465,4481,4497,4513,4529,4545,4561,4577,4593,4609,4625,4641,4657,4673,4689,4705,4721,4737,7167};
static int signtab[2] = {0,1};
static int lasttab[2] = {0,1};
static int last_intratab[2] = {0,1};
static int runtab[64] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63};
static int leveltab[254] = {129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127};

// sactables

static int cumf_COD[3]={16383, 6849, 0};
static int cumf_MCBPC[22]={16383, 4105, 3088, 2367, 1988, 1621, 1612, 1609, 1608, 496, 353, 195, 77, 22, 17, 12, 5, 4, 3, 2, 1, 0};
static int cumf_MCBPC_intra[10]={16383, 7410, 6549, 5188, 442, 182, 181, 141, 1, 0};
static int cumf_MODB[4]={16383, 6062, 2130, 0};
static int cumf_YCBPB[3]={16383,6062,0};
static int cumf_UVCBPB[3]={16383,491,0};
static int cumf_CBPY[17]={16383, 14481, 13869, 13196, 12568, 11931, 11185, 10814, 9796, 9150, 8781, 7933, 6860, 6116, 4873, 3538, 0};
static int cumf_CBPY_intra[17]={16383, 13619, 13211, 12933, 12562, 12395, 11913, 11783, 11004, 10782, 10689, 9928, 9353, 8945, 8407, 7795, 0};
static int cumf_DQUANT[5]={16383, 12287, 8192, 4095, 0};
static int cumf_MVD[65]={16383, 16380, 16369, 16365, 16361, 16357, 16350, 16343, 16339, 16333, 16326, 16318, 16311, 16306, 16298, 16291, 16283, 16272, 16261, 16249, 16235, 16222, 16207, 16175, 16141, 16094, 16044, 15936, 15764, 15463, 14956, 13924, 11491, 4621, 2264, 1315, 854, 583, 420, 326, 273, 229, 196, 166, 148, 137, 123, 114, 101, 91, 82, 76, 66, 59, 53, 46, 36, 30, 26, 24, 18, 14, 10, 5, 0};
static int cumf_INTRADC[255]={16383, 16380, 16379, 16378, 16377, 16376, 16370, 16361, 16360, 16359, 16358, 16357, 16356, 16355, 16343, 16238, 16237, 16236, 16230, 16221, 16220, 16205, 16190, 16169, 16151, 16130, 16109, 16094, 16070, 16037, 16007, 15962, 15938, 15899, 15854, 15815, 15788, 15743, 15689, 15656, 15617, 15560, 15473, 15404, 15296, 15178, 15106, 14992, 14868, 14738, 14593, 14438, 14283, 14169, 14064, 14004, 13914, 13824, 13752, 13671, 13590, 13515, 13458, 13380, 13305, 13230, 13143, 13025, 12935, 12878, 12794, 12743, 12656, 12596, 12521, 12443, 12359, 12278, 12200, 12131, 12047, 12002, 11948, 11891, 11828, 11744, 11663, 11588, 11495, 11402, 11288, 11204, 11126, 11039, 10961, 10883, 10787, 10679, 10583, 10481, 10360, 10227, 10113, 9961, 9828, 9717, 9584, 9485, 9324, 9112, 9019, 8908, 8766, 8584, 8426, 8211, 7920, 7663, 7406, 7152, 6904, 6677, 6453, 6265, 6101, 5904, 5716, 5489, 5307, 5056, 4850, 4569, 4284, 3966, 3712, 3518, 3342, 3206, 3048, 2909, 2773, 2668, 2596, 2512, 2370, 2295, 2232, 2166, 2103, 2022, 1956, 1887, 1830, 1803, 1770, 1728, 1674, 1635, 1599, 1557, 1500, 1482, 1434, 1389, 1356, 1317, 1284, 1245, 1200, 1179, 1140, 1110, 1092, 1062, 1044, 1035, 1014, 1008, 993, 981, 954, 936, 912, 894, 876, 864, 849, 828, 816, 801, 792, 777, 756, 732, 690, 660, 642, 615, 597, 576, 555, 522, 489, 459, 435, 411, 405, 396, 387, 375, 360, 354, 345, 344, 329, 314, 293, 278, 251, 236, 230, 224, 215, 214, 208, 199, 193, 184, 178, 169, 154, 127, 100, 94, 73, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 20, 19, 18, 17, 16, 15, 9, 0};
static int cumf_TCOEF1[104]={16383, 13455, 12458, 12079, 11885, 11800, 11738, 11700, 11681, 11661, 11651, 11645, 11641, 10572, 10403, 10361, 10346, 10339, 10335, 9554, 9445, 9427, 9419, 9006, 8968, 8964, 8643, 8627, 8624, 8369, 8354, 8352, 8200, 8192, 8191, 8039, 8036, 7920, 7917, 7800, 7793, 7730, 7727, 7674, 7613, 7564, 7513, 7484, 7466, 7439, 7411, 7389, 7373, 7369, 7359, 7348, 7321, 7302, 7294, 5013, 4819, 4789, 4096, 4073, 3373, 3064, 2674, 2357, 2177, 1975, 1798, 1618, 1517, 1421, 1303, 1194, 1087, 1027, 960, 890, 819, 758, 707, 680, 656, 613, 566, 534, 505, 475, 465, 449, 430, 395, 358, 335, 324, 303, 295, 286, 272, 233, 215, 0};
static int cumf_TCOEF2[104]={16383, 13582, 12709, 12402, 12262, 12188, 12150, 12131, 12125, 12117, 12113, 12108, 12104, 10567, 10180, 10070, 10019, 9998, 9987, 9158, 9037, 9010, 9005, 8404, 8323, 8312, 7813, 7743, 7726, 7394, 7366, 7364, 7076, 7062, 7060, 6810, 6797, 6614, 6602, 6459, 6454, 6304, 6303, 6200, 6121, 6059, 6012, 5973, 5928, 5893, 5871, 5847, 5823, 5809, 5796, 5781, 5771, 5763, 5752, 4754, 4654, 4631, 3934, 3873, 3477, 3095, 2758, 2502, 2257, 2054, 1869, 1715, 1599, 1431, 1305, 1174, 1059, 983, 901, 839, 777, 733, 683, 658, 606, 565, 526, 488, 456, 434, 408, 380, 361, 327, 310, 296, 267, 259, 249, 239, 230, 221, 214, 0};
static int cumf_TCOEF3[104]={16383, 13532, 12677, 12342, 12195, 12112, 12059, 12034, 12020, 12008, 12003, 12002, 12001, 10586, 10297, 10224, 10202, 10195, 10191, 9223, 9046, 8999, 8987, 8275, 8148, 8113, 7552, 7483, 7468, 7066, 7003, 6989, 6671, 6642, 6631, 6359, 6327, 6114, 6103, 5929, 5918, 5792, 5785, 5672, 5580, 5507, 5461, 5414, 5382, 5354, 5330, 5312, 5288, 5273, 5261, 5247, 5235, 5227, 5219, 4357, 4277, 4272, 3847, 3819, 3455, 3119, 2829, 2550, 2313, 2104, 1881, 1711, 1565, 1366, 1219, 1068, 932, 866, 799, 750, 701, 662, 605, 559, 513, 471, 432, 403, 365, 336, 312, 290, 276, 266, 254, 240, 228, 223, 216, 206, 199, 192, 189, 0};
static int cumf_TCOEFr[104]={16383, 13216, 12233, 11931, 11822, 11776, 11758, 11748, 11743, 11742, 11741, 11740, 11739, 10203, 9822, 9725, 9691, 9677, 9674, 8759, 8609, 8576, 8566, 7901, 7787, 7770, 7257, 7185, 7168, 6716, 6653, 6639, 6276, 6229, 6220, 5888, 5845, 5600, 5567, 5348, 5327, 5160, 5142, 5004, 4900, 4798, 4743, 4708, 4685, 4658, 4641, 4622, 4610, 4598, 4589, 4582, 4578, 4570, 4566, 3824, 3757, 3748, 3360, 3338, 3068, 2835, 2592, 2359, 2179, 1984, 1804, 1614, 1445, 1234, 1068, 870, 739, 668, 616, 566, 532, 489, 453, 426, 385, 357, 335, 316, 297, 283, 274, 266, 259, 251, 241, 233, 226, 222, 217, 214, 211, 209, 208, 0};
static int cumf_TCOEF1_intra[104]={16383, 13383, 11498, 10201, 9207, 8528, 8099, 7768, 7546, 7368, 7167, 6994, 6869, 6005, 5474, 5220, 5084, 4964, 4862, 4672, 4591, 4570, 4543, 4397, 4337, 4326, 4272, 4240, 4239, 4212, 4196, 4185, 4158, 4157, 4156, 4140, 4139, 4138, 4137, 4136, 4125, 4124, 4123, 4112, 4111, 4110, 4109, 4108, 4107, 4106, 4105, 4104, 4103, 4102, 4101, 4100, 4099, 4098, 4097, 3043, 2897, 2843, 1974, 1790, 1677, 1552, 1416, 1379, 1331, 1288, 1251, 1250, 1249, 1248, 1247, 1236, 1225, 1224, 1223, 1212, 1201, 1200, 1199, 1198, 1197, 1196, 1195, 1194, 1193, 1192, 1191, 1190, 1189, 1188, 1187, 1186, 1185, 1184, 1183, 1182, 1181, 1180, 1179, 0};
static int cumf_TCOEF2_intra[104]={16383, 13242, 11417, 10134, 9254, 8507, 8012, 7556, 7273, 7062, 6924, 6839, 6741, 6108, 5851, 5785, 5719, 5687, 5655, 5028, 4917, 4864, 4845, 4416, 4159, 4074, 3903, 3871, 3870, 3765, 3752, 3751, 3659, 3606, 3580, 3541, 3540, 3514, 3495, 3494, 3493, 3474, 3473, 3441, 3440, 3439, 3438, 3425, 3424, 3423, 3422, 3421, 3420, 3401, 3400, 3399, 3398, 3397, 3396, 2530, 2419, 2360, 2241, 2228, 2017, 1687, 1576, 1478, 1320, 1281, 1242, 1229, 1197, 1178, 1152, 1133, 1114, 1101, 1088, 1087, 1086, 1085, 1072, 1071, 1070, 1069, 1068, 1067, 1066, 1065, 1064, 1063, 1062, 1061, 1060, 1059, 1058, 1057, 1056, 1055, 1054, 1053, 1052, 0};
static int cumf_TCOEF3_intra[104]={16383, 12741, 10950, 10071, 9493, 9008, 8685, 8516, 8385, 8239, 8209, 8179, 8141, 6628, 5980, 5634, 5503, 5396, 5327, 4857, 4642, 4550, 4481, 4235, 4166, 4151, 3967, 3922, 3907, 3676, 3500, 3324, 3247, 3246, 3245, 3183, 3168, 3084, 3069, 3031, 3030, 3029, 3014, 3013, 2990, 2975, 2974, 2973, 2958, 2943, 2928, 2927, 2926, 2925, 2924, 2923, 2922, 2921, 2920, 2397, 2298, 2283, 1891, 1799, 1591, 1445, 1338, 1145, 1068, 1006, 791, 768, 661, 631, 630, 615, 592, 577, 576, 561, 546, 523, 508, 493, 492, 491, 476, 475, 474, 473, 472, 471, 470, 469, 468, 453, 452, 451, 450, 449, 448, 447, 446, 0};
static int cumf_TCOEFr_intra[104]={16383, 12514, 10776, 9969, 9579, 9306, 9168, 9082, 9032, 9000, 8981, 8962, 8952, 7630, 7212, 7053, 6992, 6961, 6940, 6195, 5988, 5948, 5923, 5370, 5244, 5210, 4854, 4762, 4740, 4384, 4300, 4288, 4020, 3968, 3964, 3752, 3668, 3511, 3483, 3354, 3322, 3205, 3183, 3108, 3046, 2999, 2981, 2974, 2968, 2961, 2955, 2949, 2943, 2942, 2939, 2935, 2934, 2933, 2929, 2270, 2178, 2162, 1959, 1946, 1780, 1651, 1524, 1400, 1289, 1133, 1037, 942, 849, 763, 711, 591, 521, 503, 496, 474, 461, 449, 442, 436, 426, 417, 407, 394, 387, 377, 373, 370, 367, 366, 365, 364, 363, 362, 358, 355, 352, 351, 350, 0};
static int cumf_SIGN[3]={16383, 8416, 0};
static int cumf_LAST[3]={16383, 9469, 0};
static int cumf_LAST_intra[3]={16383, 2820, 0};
static int cumf_RUN[65]={16383, 15310, 14702, 13022, 11883, 11234, 10612, 10192, 9516, 9016, 8623, 8366, 7595, 7068, 6730, 6487, 6379, 6285, 6177, 6150, 6083, 5989, 5949, 5922, 5895, 5828, 5774, 5773, 5394, 5164, 5016, 4569, 4366, 4136, 4015, 3867, 3773, 3692, 3611, 3476, 3341, 3301, 2787, 2503, 2219, 1989, 1515, 1095, 934, 799, 691, 583, 435, 300, 246, 206, 125, 124, 97, 57, 30, 3, 2, 1, 0};
static int cumf_RUN_intra[65]={16383, 10884, 8242, 7124, 5173, 4745, 4246, 3984, 3034, 2749, 2607, 2298, 966, 681, 396, 349, 302, 255, 254, 253, 206, 159, 158, 157, 156, 155, 154, 153, 106, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
static int cumf_LEVEL[255]={16383, 16382, 16381, 16380, 16379, 16378, 16377, 16376, 16375, 16374, 16373, 16372, 16371, 16370, 16369, 16368, 16367, 16366, 16365, 16364, 16363, 16362, 16361, 16360, 16359, 16358, 16357, 16356, 16355, 16354, 16353, 16352, 16351, 16350, 16349, 16348, 16347, 16346, 16345, 16344, 16343, 16342, 16341, 16340, 16339, 16338, 16337, 16336, 16335, 16334, 16333, 16332, 16331, 16330, 16329, 16328, 16327, 16326, 16325, 16324, 16323, 16322, 16321, 16320, 16319, 16318, 16317, 16316, 16315, 16314, 16313, 16312, 16311, 16310, 16309, 16308, 16307, 16306, 16305, 16304, 16303, 16302, 16301, 16300, 16299, 16298, 16297, 16296, 16295, 16294, 16293, 16292, 16291, 16290, 16289, 16288, 16287, 16286, 16285, 16284, 16283, 16282, 16281, 16280, 16279, 16278, 16277, 16250, 16223, 16222, 16195, 16154, 16153, 16071, 15989, 15880, 15879, 15878, 15824, 15756, 15674, 15606, 15538, 15184, 14572, 13960, 10718, 7994, 5379, 2123, 1537, 992, 693, 611, 516, 448, 421, 380, 353, 352, 284, 257, 230, 203, 162, 161, 160, 133, 132, 105, 104, 103, 102, 101, 100, 99, 98, 97, 96, 95, 94, 93, 92, 91, 90, 89, 88, 87, 86, 85, 84, 83, 82, 81, 80, 79, 78, 77, 76, 75, 74, 73, 72, 71, 70, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
static int cumf_LEVEL_intra[255]={16383, 16379, 16378, 16377, 16376, 16375, 16374, 16373, 16372, 16371, 16370, 16369, 16368, 16367, 16366, 16365, 16364, 16363, 16362, 16361, 16360, 16359, 16358, 16357, 16356, 16355, 16354, 16353, 16352, 16351, 16350, 16349, 16348, 16347, 16346, 16345, 16344, 16343, 16342, 16341, 16340, 16339, 16338, 16337, 16336, 16335, 16334, 16333, 16332, 16331, 16330, 16329, 16328, 16327, 16326, 16325, 16324, 16323, 16322, 16321, 16320, 16319, 16318, 16317, 16316, 16315, 16314, 16313, 16312, 16311, 16268, 16267, 16224, 16223, 16180, 16179, 16136, 16135, 16134, 16133, 16132, 16131, 16130, 16129, 16128, 16127, 16126, 16061, 16018, 16017, 16016, 16015, 16014, 15971, 15970, 15969, 15968, 15925, 15837, 15794, 15751, 15750, 15749, 15661, 15618, 15508, 15376, 15288, 15045, 14913, 14781, 14384, 13965, 13502, 13083, 12509, 12289, 12135, 11892, 11738, 11429, 11010, 10812, 10371, 9664, 9113, 8117, 8116, 8028, 6855, 5883, 4710, 4401, 4203, 3740, 3453, 3343, 3189, 2946, 2881, 2661, 2352, 2132, 1867, 1558, 1382, 1250, 1162, 1097, 1032, 967, 835, 681, 549, 439, 351, 350, 307, 306, 305, 304, 303, 302, 301, 300, 299, 298, 255, 212, 211, 210, 167, 166, 165, 164, 163, 162, 161, 160, 159, 158, 115, 114, 113, 112, 111, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};

// getbits
static unsigned int msk[33] = {
	0x00000000,0x00000001,0x00000003,0x00000007,
		0x0000000f,0x0000001f,0x0000003f,0x0000007f,
		0x000000ff,0x000001ff,0x000003ff,0x000007ff,
		0x00000fff,0x00001fff,0x00003fff,0x00007fff,
		0x0000ffff,0x0001ffff,0x0003ffff,0x0007ffff,
		0x000fffff,0x001fffff,0x003fffff,0x007fffff,
		0x00ffffff,0x01ffffff,0x03ffffff,0x07ffffff,
		0x0fffffff,0x1fffffff,0x3fffffff,0x7fffffff,
		0xffffffff
};

#endif
