
#ifndef VS_H263PARSER_H
#define VS_H263PARSER_H

#include "types.h"

struct MBLayer
{
	int bitpos;
	int quant;
	int hmv1, vmv1;
	int hmv2, vmv2;
};

struct GobLayer
{
	int bitpos;
};


struct PictureLayer
{
	PictureLayer();
	bool Set(int numOfGOB, int numOfMB);
	void Clear();
	int NumOfGOB;
	int NumOfMB;
	GobLayer* gob;
	MBLayer* mb;
};


class VS_H263Parser
{
	// sac
	long low, high, code_value, bit, length, sacindex, cum, zerorun;
	// globals
	int quiet;
	int trace;
	char errortext[256];
	int MV[2][5][MBR_MAX+1][MBC_MAX+2];
	int modemap[MBR_MAX+1][MBC_MAX+2];
	int horizontal_size,vertical_size,mb_width,mb_height;
	int coded_picture_width, coded_picture_height;
	int blk_cnt;
	int pict_type,newgob;
	int mv_outside_frame,syntax_arith_coding,adv_pred_mode,pb_frame;
	int long_vectors;
	int fault;
	int temp_ref, quant, source_format;
	int trd, trb, bscan, bquant;
	int MBC, MBR;
	RdBuffer base,*ld;

	// functions

	void bit_out_psc_layer();
	void clearblock(int comp);
	int decode_a_symbol(int cumul_freq[ ]);
	RunCoef Decode_Escape_Char(int intra, int *last);
	void decoder_reset( );
	int DecodeTCoef(int position, int intra);
	void fillbfr();
	int find_pmv( int x,  int y,  int block, int comp);
	void flushbits(int n);
	unsigned int getbits1();
	unsigned int getbits(int n);
	void getblock(int comp, int mode);
	int getCBPY();
	int getheader();
	void getMBs();
	int getMCBPCintra();
	int getMCBPC();
	int getMODB();
	bool getpicturehdr();
	void get_sac_block(int comp, int ptype);
	int getTMNMV();
	void initbits();
	bool initdecoder();
	int motion_decode( int vec, int pmv);
	void printbits(int code, int bits, int len);
	unsigned int showbits(int n);
	void startcode();
	RunCoef vlc_word_decode(int symbol_word, int *last);

public:
	PictureLayer FrameSplit;
	VS_H263Parser(){
		ld = &base; /* select base layer context */
		zerorun=0;
		trace = 0;
		quiet = 0;
	}
	~VS_H263Parser(){}
	bool play_movie(void * buffer, unsigned int buffer_size);
};

#endif
