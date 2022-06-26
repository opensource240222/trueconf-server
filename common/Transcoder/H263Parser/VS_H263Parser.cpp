
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "VS_H263Parser.h"
#include "tables.h"
#include "../../std/debuglog/VS_Debug.h"

#define DEBUG_CURRENT_MODULE VS_DM_STDERR

PictureLayer::PictureLayer()
{
	NumOfGOB = NumOfMB = 0;
	gob = 0; mb = 0;
}

bool PictureLayer::Set(int numOfGOB, int numOfMB)
{
	Clear();
	NumOfGOB = numOfGOB;
	NumOfMB = numOfMB;
	gob = (GobLayer*)malloc(sizeof(GobLayer)*NumOfGOB);
	memset(gob, 0, sizeof(GobLayer)*NumOfGOB);
	mb = (MBLayer*)malloc(sizeof(MBLayer)*NumOfMB);
	memset(mb, 0, sizeof(MBLayer)*NumOfMB);
	return true;
}
void PictureLayer::Clear()
{
	NumOfGOB = NumOfMB = 0;
	if (gob) free(gob); gob = 0;
	if (mb) free(mb); mb = 0;
}


//GETBITS

/* initialize buffer, call once before first getbits or showbits */
void VS_H263Parser::initbits()
{
	ld->plb = ld->large_buffer; //k
	ld->incnt = 0;
	ld->rdptr = ld->rdbfr + 2048;
	ld->bitcnt = 0;
}

void VS_H263Parser::fillbfr()
{
	int l;

	ld->inbfr[0] = ld->inbfr[8];
	ld->inbfr[1] = ld->inbfr[9];
	ld->inbfr[2] = ld->inbfr[10];
	ld->inbfr[3] = ld->inbfr[11];

	if (ld->rdptr>=ld->rdbfr+2048)
	{
		l = mmin(2048, ld->lbsize - (int)(ld->plb - ld->large_buffer));
		memcpy(ld->rdbfr, ld->plb, l);
		ld->plb+=l;

		ld->rdptr = ld->rdbfr;
		if (l<2048)
		{
			if (l<0)
				l = 0;

			while (l<2048)   /* Add recognizable sequence end code */
			{
				ld->rdbfr[l++] = 0;
				ld->rdbfr[l++] = 0;
				ld->rdbfr[l++] = (1<<7) | (SEC<<2);
			}
		}
	}

	for (l=0; l<8; l++)
		ld->inbfr[l+4] = ld->rdptr[l];

	ld->rdptr+= 8;
	ld->incnt+= 64;
}


/* return next n bits (right adjusted) without advancing */
unsigned int VS_H263Parser::showbits(int n)
{
	unsigned char *v;
	unsigned int b;
	int c;

	if (ld->incnt<n)
		fillbfr();

	v = ld->inbfr + ((96 - ld->incnt)>>3);
	b = (v[0]<<24) | (v[1]<<16) | (v[2]<<8) | v[3];
	c = ((ld->incnt-1) & 7) + 25;
	return (b>>(c-n)) & msk[n];
}


/* return next bit (could be made faster than getbits(1)) */
unsigned int VS_H263Parser::getbits1()
{
	return getbits(1);
}


/* advance by n bits */
void VS_H263Parser::flushbits(int n)
{

	ld->bitcnt+= n;
	ld->incnt-= n;
	if (ld->incnt < 0)
		fillbfr();
}


/* return next n bits (right adjusted) */
unsigned int VS_H263Parser::getbits(int n)
{
	unsigned int l;

	l = showbits(n);
	flushbits(n);

	return l;
}

//GETBLK

void VS_H263Parser::getblock(int comp, int mode)
{
	int val, i, j, sign;
	unsigned int code;
	VLCtabI *tab;
	short *bp;
	int run, last, level, QP;
	short *qval;

	bp = ld->block[comp];

	/* decode AC coefficients */
	for (i=(mode==0); ; i++) {
		code = showbits(12);
		if (code>=512)
			tab = &DCT3Dtab0[(code>>5)-16];
		else if (code>=128)
			tab = &DCT3Dtab1[(code>>2)-32];
		else if (code>=8)
			tab = &DCT3Dtab2[(code>>0)-8];
		else {
			if (!quiet)
				dprint4("invalid Huffman code in getblock()\n");
			fault = 1;
			return;
		}

		flushbits(tab->len);

		run = (tab->val >> 4) & 255;
		level = tab->val & 15;
		last = (tab->val >> 12) & 1;

		if (trace) {
			printf(" (");
			printbits(code,12,tab->len);
		}

		if (tab->val==ESCAPE) { /* escape */
			if (trace) {
				putchar(' ');
				printbits(showbits(1),1,1);
			}
			last = getbits1();
			if (trace) {
				putchar(' ');
				printbits(showbits(6),6,6);
			}
			i += run = getbits(6);
			if (trace) {
				putchar(' ');
				printbits(showbits(8),8,8);
			}
			level = getbits(8);

			if ((sign = (level>=128)))
				val = 256 - level;
			else
				val = level;
		}
		else {
			i+= run;
			val = level;
			sign = getbits(1);
			if (trace)
				printf("%d",sign);
		}

		if (i >= 64)
		{
			if (!quiet)
				dprint4("DCT coeff index (i) out of bounds\n");
			fault = 1;
			return;
		}

		if (trace)
			printf("): %d/%d\n",run,sign ? -val : val);


		j = zig_zag_scan[i];
		qval = &bp[j];
		if (comp >= 6)
			QP = mmax (1, mmin( 31, ( bquant_tab[bquant] * quant ) >> 2 ));
		else
			QP = quant;

		/* TMN3 dequantization */
		if ((QP % 2) == 1)
			*qval = ( sign ? -(QP * (2* val+1))  : QP * (2* val+1) );
		else
			*qval = ( sign ? -(QP * (2* val+1)-1): QP * (2* val+1)-1 );

		if (last) { /* That's it */
			if (trace)
				printf("last\n");
			return;
		}
	}
}


/*********************************************************************
*
* 	Name:		get_sac_block
*
*	Description:	Decodes blocks of Arithmetic Encoded DCT Coeffs.
*			and performs Run Length Decoding and Coefficient
*			Dequantisation.
*
*	Input:		Picture block type and number.
*
*	Returns:	Nothing.
*
*	Side Effects:
*
*	Author:		Wayne Ellis <ellis_w_wayne@bt-web.bt.co.uk>
*
*********************************************************************/
void VS_H263Parser::get_sac_block(int comp, int ptype)
{
	int position=0;
	int TCOEF_index, symbol_word;
	int last=0, QP, i, j;
	short *qval, *bp;
	RunCoef DCTcoef;

	bp = ld->block[comp];

	i = (ptype==0);

	while (!last) {	/* while there are DCT coefficients remaining */
		position++;	/* coefficient counter relates to Coeff. model */
		TCOEF_index = DecodeTCoef(position, !ptype);

		if (TCOEF_index == ESCAPE_INDEX) { 	/* ESCAPE code encountered */
			DCTcoef = Decode_Escape_Char(!ptype, &last);
			if (trace)
				printf("ESC: ");
		}
		else {
			symbol_word = tcoeftab[TCOEF_index];

			DCTcoef = vlc_word_decode(symbol_word,&last);
		}

		if (trace) {
			printf("val: %d, run: %d, sign: %d, last: %d\n",
				DCTcoef.val, DCTcoef.run, DCTcoef.sign, last);
		}

		i += DCTcoef.run;

		j = zig_zag_scan[i];

		qval = &bp[j];

		i++;

		if (comp >= 6)
			QP = mmax (1, mmin( 31, ( bquant_tab[bquant] * quant ) >> 2 ));
		else
			QP = quant;

		if ((QP % 2) == 1)
			*qval = ( (DCTcoef.sign) ? -(QP * (2* (DCTcoef.val)+1))  :
		QP * (2* (DCTcoef.val)+1) );
		else
			*qval = ( (DCTcoef.sign) ? -(QP * (2* (DCTcoef.val)+1)-1):
		QP * (2* (DCTcoef.val)+1)-1 );

	}
	return;
}

/*********************************************************************
*
* 	Name:		vlc_word_decode
*
*	Description:	Fills Decoder FIFO after a fixed word length
*			string has been detected.
*
*	Input:		Symbol to be decoded, last data flag.
*
*	Returns:	Decoded Symbol via the structure DCTcoeff.
*
*	Side Effects:	Updates last flag.
*
*	Author:		Wayne Ellis <ellis_w_wayne@bt-web.bt.co.uk>
*
*********************************************************************/
RunCoef VS_H263Parser::vlc_word_decode(int symbol_word, int *last)
{
	int sign_index;
	RunCoef DCTcoef;

	*last = (symbol_word >> 12) & 01;

	DCTcoef.run = (symbol_word >> 4) & 255;

	DCTcoef.val = (symbol_word) & 15;

	sign_index = decode_a_symbol(cumf_SIGN);

	DCTcoef.sign = signtab[sign_index];

	return (DCTcoef);
}

/*********************************************************************
*
* 	Name:		Decode_Escape_Char
*
*	Description:	Decodes all components for a Symbol when an
*			ESCAPE character has been detected.
*
*	Input:		Picture Type and last data flag.
*
*	Returns:	Decoded Symbol via the structure DCTcoeff.
*
*	Side Effects:	Modifies last data flag.
*
*	Author:		Wayne Ellis <ellis_w_wayne@bt-web.bt.co.uk>
*
*********************************************************************/
RunCoef VS_H263Parser::Decode_Escape_Char(int intra, int *last)
{
	int last_index, run, run_index, level, level_index;
	RunCoef DCTcoef;

	if (intra) {
		last_index = decode_a_symbol(cumf_LAST_intra);
		*last = last_intratab[last_index];
	}
	else {
		last_index = decode_a_symbol(cumf_LAST);
		*last = lasttab[last_index];
	}

	if (intra)
		run_index = decode_a_symbol(cumf_RUN_intra);
	else
		run_index = decode_a_symbol(cumf_RUN);

	run = runtab[run_index];

	/*$if (mrun) run|=64;$*/

	DCTcoef.run = run;

	if (intra)
		level_index = decode_a_symbol(cumf_LEVEL_intra);
	else
		level_index = decode_a_symbol(cumf_LEVEL);

	if (trace)
		printf("level_idx: %d ",level_index);

	level = leveltab[level_index];

	if (level >128)
		level -=256;

	if (level < 0) {
		DCTcoef.sign = 1;
		DCTcoef.val = abs(level);
	}

	else {
		DCTcoef.sign = 0;
		DCTcoef.val = level;
	}

	return (DCTcoef);

}
/*********************************************************************
*
* 	Name:		DecodeTCoef
*
*	Description:	Decodes a.c DCT Coefficients using the
*			relevant arithmetic decoding model.
*
*	Input:		DCT Coeff count and Picture Type.
*
*	Returns:	Index to LUT
*
*	Side Effects:	None
*
*	Author:		Wayne Ellis <ellis_w_wayne@bt-web.bt.co.uk>
*
*********************************************************************/
int VS_H263Parser::DecodeTCoef(int position, int intra)
{
	int index;

	switch (position) {
  case 1:
	  {
		  if (intra)
			  index = decode_a_symbol(cumf_TCOEF1_intra);
		  else
			  index = decode_a_symbol(cumf_TCOEF1);
		  break;
	  }
  case 2:
	  {
		  if (intra)
			  index = decode_a_symbol(cumf_TCOEF2_intra);
		  else
			  index = decode_a_symbol(cumf_TCOEF2);
		  break;
	  }
  case 3:
	  {
		  if (intra)
			  index = decode_a_symbol(cumf_TCOEF3_intra);
		  else
			  index = decode_a_symbol(cumf_TCOEF3);
		  break;
	  }
  default:
	  {
		  if (intra)
			  index = decode_a_symbol(cumf_TCOEFr_intra);
		  else
			  index = decode_a_symbol(cumf_TCOEFr);
		  break;
	  }
	}

	return (index);
}

// GETHDR

/*
* decode headers from one input stream
* until an End of Sequence or picture start code
* is found
*/
int VS_H263Parser::getheader()
{
	unsigned int code, gob;

	/* look for startcode */
	startcode();
	code = getbits(PSC_LENGTH);
	gob = getbits(5);
	if (gob == SEC)
		return 0;
	if (gob == 0) {
		if (!getpicturehdr())
			return 0;
		if (syntax_arith_coding)		/* reset decoder after receiving */
			decoder_reset();	        /* fixed length PSC string */
	}
	return gob + 1;
}


/* align to start of next startcode */
void VS_H263Parser::startcode()
{
	/* search for new picture start code */
	while (showbits(PSC_LENGTH)!=1l)
		flushbits(1);
}

/* decode picture header */
bool VS_H263Parser::getpicturehdr()
{
	int pos, pei, tmp;
	static int prev_temp_ref;

	pos = ld->bitcnt;
	prev_temp_ref = temp_ref;
	temp_ref = getbits(8);
	trd = temp_ref - prev_temp_ref;
	if (trd < 0)
		trd += 256;
	tmp = getbits(1); /* always "1" */
	if (!tmp)
		if (!quiet)
			printf("warning: spare in picture header should be \"1\"\n");
	tmp = getbits(1); /* always "0" */
	if (tmp)
		if (!quiet)
			printf("warning: H.261 distinction bit should be \"0\"\n");
	tmp = getbits(1); /* split_screen_indicator */
	if (tmp) {
		if (!quiet)
			printf("error: split-screen not supported in this version\n");
		return false;
	}
	tmp = getbits(1); /* document_camera_indicator */
	if (tmp)
		if (!quiet)
			printf("warning: document camera indicator not supported in this version\n");

	tmp = getbits(1); /* freeze_picture_release */
	if (tmp)
		if (!quiet)
			printf("warning: frozen picture not supported in this version\n");

	source_format = getbits(3);
	pict_type = getbits(1);
	mv_outside_frame = getbits(1);
	long_vectors = (mv_outside_frame ? 1 : 0);
	syntax_arith_coding = getbits(1);
	adv_pred_mode = getbits(1);
	mv_outside_frame = (adv_pred_mode ? 1 : mv_outside_frame);
	pb_frame = getbits(1);
	quant = getbits(5);
	tmp = getbits(1);
	if (tmp) {
		if (!quiet)
			printf("error: CPM not supported in this version\n");
		return false;
	}
	if (pb_frame) {
		trb = getbits(3);
		bquant = getbits(2);
	}

	pei = getbits(1);
pspare:
	if (pei) {
		/* extra info for possible future backward compatible additions */
		getbits(8);  /* not used */
		pei = getbits(1);
		if (pei) goto pspare; /* keep on reading pspare until pei=0 */
	}
	return true;
}


// GETPIC


/* decode all macroblocks of the current picture */
void VS_H263Parser::getMBs()
{
	int comp;
	int MBA, MBAmax;
	int bx, by;

	int COD=0,MCBPC, CBPY, CBP=0, CBPB=0, MODB=0, Mode=0, DQUANT;
	int COD_index, CBPY_index, MODB_index, DQUANT_index, MCBPC_index;
	int INTRADC_index, YCBPB_index, UVCBPB_index, mvdbx_index, mvdby_index;
	int mvx, mvy, mvy_index, mvx_index, pmv0, pmv1, xpos, ypos, gob, i,k;
	int mvdbx=0, mvdby=0, pmvdbx, pmvdby, gfid, YCBPB, UVCBPB, gobheader_read;
	int startmv,stopmv,offset,bsize,last_done=0,pCBP=0,pCBPB=0,pCOD=0;
	int DQ_tab[4] = {-1,-2,1,2};
	short *bp;

	/* number of macroblocks per picture */
	MBAmax = mb_width*mb_height;

	MBA = 0; /* macroblock address */
	newgob = 0;

	/* mark MV's above the picture */
	for (i = 1; i < MBC+1; i++) {
		for (k = 0; k < 5; k++) {
			MV[0][k][0][i] = NO_VEC;
			MV[1][k][0][i] = NO_VEC;
		}
		modemap[0][i] = MODE_INTRA;
	}
	/* zero MV's on the sides of the picture */
	for (i = 0; i < MBR+1; i++) {
		for (k = 0; k < 5; k++) {
			MV[0][k][i][0] = 0;
			MV[1][k][i][0] = 0;
			MV[0][k][i][MBC+1] = 0;
			MV[1][k][i][MBC+1] = 0;
		}
		modemap[i][0] = MODE_INTRA;
		modemap[i][MBC+1] = MODE_INTRA;
	}

	fault = 0;
	gobheader_read = 0;

	for (;;) {
		int startGobBit = 0;
		int startMBBit = 0;

		if (trace)
			printf(" MB %d\n",MBA);

resync:

		/* This version of the decoder does not resync on every possible
		error, and it does not do all possible error checks. It is not
		difficult to make it much more error robust, but I do not think
		it is necessary to include this in the freely available
		version. */

		if (fault) {
			printf("Warning: A Fault Condition Has Occurred - Resyncing \n");
			startcode();  /* sync on new startcode */
			fault = 0;
		}

		if (!(showbits(22)>>6)) { /* startcode */
			startGobBit = ld->bitcnt;

			startcode();
			/* in case of byte aligned start code, ie. PSTUF, GSTUF or ESTUF
			is used */

			if (showbits(22) == (32|SEC)) { /* end of sequence */
				if (!(syntax_arith_coding && MBA < MBAmax)) {
					return;
				}
			}
			else if ((showbits(22) == PSC<<5) ) { /* new picture */
				if (!(syntax_arith_coding && MBA < MBAmax)) {
					return;
				}
			}
			else {
				if (!(syntax_arith_coding && MBA%MBC)) {

					if (syntax_arith_coding) {   /* SAC hack to finish GOBs which   */
						gob = (showbits(22) & 31); /* end with MBs coded with no bits */
						if (gob * mb_width != MBA)
							goto finish_gob;
					}

					gob = getheader() - 1;
					if (gob > MBR) {
						if (!quiet)
							printf("GN out of range\n");
						return;
					}

					/* GFID is not allowed to change unless PTYPE in picture header
					changes */
					gfid = getbits(2);
					/* NB: in error-prone environments the decoder can use this
					value to determine whether a picture header where the PTYPE
					has changed, has been lost */

					quant = getbits(5);
					if (trace)
						printf("GQUANT: %d\n", quant);
					xpos = 0;
					ypos = gob;
					MBA = ypos * mb_width;

					newgob = 1;
					gobheader_read = 1;
					if (syntax_arith_coding)
						decoder_reset();	/* init. arithmetic decoder buffer after gob */
				}
				FrameSplit.gob[gob].bitpos = startGobBit;
			}
		}

finish_gob:  /* SAC specific label */

		if (!gobheader_read) {
			xpos = MBA%mb_width;
			ypos = MBA/mb_width;
			if (xpos == 0 && ypos > 0)
				newgob = 0;
		}
		else {
			gobheader_read = 0;
		}

		if (MBA>=MBAmax)
			return; /* all macroblocks decoded */

		FrameSplit.mb[MBA].bitpos = ld->bitcnt;

read_cod:
		if (syntax_arith_coding) {
			if (pict_type == PCT_INTER) {
				COD_index = decode_a_symbol(cumf_COD);
				COD = codtab[COD_index];
				if (trace) {
					printf("Arithmetic Decoding Debug \n");
					printf("COD Index: %d COD: %d \n", COD_index, COD);
				}
			}
			else
				COD = 0;  /* COD not used in I-pictures, set to zero */
		}
		else {
			if (pict_type == PCT_INTER) {
				COD = showbits(1);
			}
			else
				COD = 0; /* Intra picture -> not skipped */
		}


		if (!COD) {  /* COD == 0 --> not skipped */

			if (syntax_arith_coding)  {
				if (pict_type == PCT_INTER) {
					MCBPC_index = decode_a_symbol(cumf_MCBPC);
					MCBPC = mcbpctab[MCBPC_index];
				}
				else {
					MCBPC_index = decode_a_symbol(cumf_MCBPC_intra);
					MCBPC = mcbpc_intratab[MCBPC_index];
				}
				if (trace)
					printf("MCBPC Index: %d MCBPC: %d \n",MCBPC_index, MCBPC);
			}

			else {
				if (pict_type == PCT_INTER)
					flushbits(1); /* flush COD bit */
				if (pict_type == PCT_INTRA)
					MCBPC = getMCBPCintra();
				else
					MCBPC = getMCBPC();
			}

			if (fault) goto resync;

			if (MCBPC == -1) { /* stuffing */
				goto read_cod;   /* read next COD without advancing MB count */
			}

			else {             /* normal MB data */

				Mode = MCBPC & 7;

				/* MODB and CBPB */
				if (pb_frame) {
					CBPB = 0;
					if (syntax_arith_coding)  {
						MODB_index = decode_a_symbol(cumf_MODB);
						MODB = modb_tab[MODB_index];
					}
					else
						MODB = getMODB();
					if (trace)
						printf("MODB: %d\n", MODB);
					if (MODB == PBMODE_CBPB_MVDB) {
						if (syntax_arith_coding)  {
							for(i=0; i<4; i++) {
								YCBPB_index = decode_a_symbol(cumf_YCBPB);
								YCBPB = ycbpb_tab[YCBPB_index];
								CBPB |= (YCBPB << (6-1-i));
							}

							for(i=4; i<6; i++) {
								UVCBPB_index = decode_a_symbol(cumf_UVCBPB);
								UVCBPB = uvcbpb_tab[UVCBPB_index];
								CBPB |= (UVCBPB << (6-1-i));
							}
						}
						else
							CBPB = getbits(6);
						if (trace)
							printf("CBPB = %d\n",CBPB);
					}
				}

				if (syntax_arith_coding) {

					if (Mode == MODE_INTRA || Mode == MODE_INTRA_Q) { /* Intra */
						CBPY_index = decode_a_symbol(cumf_CBPY_intra);
						CBPY = cbpy_intratab[CBPY_index];
					}
					else {
						CBPY_index = decode_a_symbol(cumf_CBPY);
						CBPY = cbpytab[CBPY_index];

					}
					if (trace)
						printf("CBPY Index: %d CBPY %d \n",CBPY_index, CBPY);

				}
				else
					CBPY = getCBPY();

				/* Decode Mode and CBP */


				if (Mode == MODE_INTRA || Mode == MODE_INTRA_Q)
				{/* Intra */
					if (!syntax_arith_coding)
						CBPY = CBPY^15;		/* needed in huffman coding only */
				}

				CBP = (CBPY << 2) | (MCBPC >> 4);
			}

			if (Mode == MODE_INTER4V && !adv_pred_mode)
				if (!quiet)
					printf("8x8 vectors not allowed in normal prediction mode\n");
			/* Could set fault-flag and resync */


			if (Mode == MODE_INTER_Q || Mode == MODE_INTRA_Q) {
				/* Read DQUANT if necessary */

				if (syntax_arith_coding) {
					DQUANT_index = decode_a_symbol(cumf_DQUANT);
					DQUANT = dquanttab[DQUANT_index] - 2;
					quant +=DQUANT;
					if (trace)
						printf("DQUANT Index: %d DQUANT %d \n",DQUANT_index, DQUANT);
				}
				else {
					DQUANT = getbits(2);
					quant += DQ_tab[DQUANT];
					if (trace) {
						printf("DQUANT (");
						printbits(DQUANT,2,2);
						printf("): %d = %d\n",DQUANT,DQ_tab[DQUANT]);
					}
				}

				if (quant > 31 || quant < 1) {
					if (!quiet)
						printf("Quantizer out of range: clipping\n");
					quant = mmax(1,mmin(31,quant));
					/* could set fault-flag and resync here */
				}
			}

			/* motion vectors */
			if (Mode == MODE_INTER || Mode == MODE_INTER_Q ||
				Mode == MODE_INTER4V || pb_frame) {

					if (Mode == MODE_INTER4V) { startmv = 1; stopmv = 4;}
					else { startmv = 0; stopmv = 0;}

					for (k = startmv; k <= stopmv; k++) {
						if (syntax_arith_coding) {
							mvx_index = decode_a_symbol(cumf_MVD);
							mvx = mvdtab[mvx_index];
							mvy_index = decode_a_symbol(cumf_MVD);
							mvy = mvdtab[mvy_index];
							if (trace)
								printf("mvx_index: %d mvy_index: %d \n", mvy_index, mvx_index);
						}
						else {
							mvx = getTMNMV();
							mvy = getTMNMV();
						}

						pmv0 = find_pmv(xpos,ypos,k,0);
						pmv1 = find_pmv(xpos,ypos,k,1);
						mvx = motion_decode(mvx, pmv0);
						mvy = motion_decode(mvy, pmv1);
						if (trace) {
							printf("mvx: %d\n", mvx);
							printf("mvy: %d\n", mvy);
						}

						/* Check mv's to prevent seg.faults when error rate is high */
						if (!mv_outside_frame) {
							bsize = k ? 8 : 16;
							offset = k ? (((k-1)&1)<<3) : 0;
							/* checking only integer component */
							if ((xpos<<4) + (mvx/2) + offset < 0 ||
								(xpos<<4) + (mvx/2) + offset > (mb_width<<4) - bsize) {
									if (!quiet)
										printf("mvx out of range: searching for sync\n");
									fault = 1;
								}
								offset = k ? (((k-1)&2)<<2) : 0;
								if ((ypos<<4) + (mvy/2) + offset < 0 ||
									(ypos<<4) + (mvy/2) + offset > (mb_height<<4) - bsize) {
										if (!quiet)
											printf("mvy out of range: searching for sync\n");
										fault = 1;
									}
						}
						MV[0][k][ypos+1][xpos+1] = mvx;
						MV[1][k][ypos+1][xpos+1] = mvy;
					}

					/* PB frame delta vectors */

					if (pb_frame) {
						if (MODB == PBMODE_MVDB || MODB == PBMODE_CBPB_MVDB) {
							if (syntax_arith_coding) {
								mvdbx_index = decode_a_symbol(cumf_MVD);
								mvdbx = mvdtab[mvdbx_index];

								mvdby_index = decode_a_symbol(cumf_MVD);
								mvdby = mvdtab[mvdby_index];
							}
							else {
								mvdbx = getTMNMV();
								mvdby = getTMNMV();
							}


							mvdbx = motion_decode(mvdbx, 0);
							mvdby = motion_decode(mvdby, 0);
							/* This will not work if the PB deltas are so large they
							require the second colums of the motion vector VLC
							table to be used.  To fix this it is necessary to
							calculate the MV predictor for the PB delta: TRB*MV/TRD
							here, and use this as the second parameter to
							motion_decode(). The B vector itself will then be
							returned from motion_decode(). This will have to be
							changed to the PB delta again, since it is the PB delta
							which is used later in this program. I don't think PB
							deltas outside the range mentioned above is useful, but
							you never know... */

							if (trace) {
								printf("MVDB x: %d\n", mvdbx);
								printf("MVDB y: %d\n", mvdby);
							}
						}
						else {
							mvdbx = 0;
							mvdby = 0;
						}
					}
				}

				if (fault) goto resync;

		}
		else { /* COD == 1 --> skipped MB */
			if (MBA>=MBAmax)
				return; /* all macroblocks decoded */
			if (!syntax_arith_coding)
				if (pict_type == PCT_INTER)
					flushbits(1);

			Mode = MODE_INTER;

			/* Reset CBP */
			CBP = CBPB = 0;

			/* reset motion vectors */
			MV[0][0][ypos+1][xpos+1] = 0;
			MV[1][0][ypos+1][xpos+1] = 0;
			mvdbx = 0;
			mvdby = 0;
		}

		/* Store Mode*/
		modemap[ypos+1][xpos+1] = Mode;

		if (Mode == MODE_INTRA || Mode == MODE_INTRA_Q)
			if (!pb_frame)
				MV[0][0][ypos+1][xpos+1]=MV[1][0][ypos+1][xpos+1] = 0;


reconstruct_mb:
		if (!last_done) {
			FrameSplit.mb[MBA].quant = quant;
			if (MBA>=FrameSplit.NumOfMB)
				puts("aaa");
			if (modemap[ypos+1][xpos+1]== MODE_INTER4V) {
				FrameSplit.mb[MBA].hmv1 = MV[0][1][ypos+1][xpos+1];
				FrameSplit.mb[MBA].vmv1 = MV[1][1][ypos+1][xpos+1];
				FrameSplit.mb[MBA].hmv2 = MV[0][2][ypos+1][xpos+1];
				FrameSplit.mb[MBA].vmv2 = MV[1][2][ypos+1][xpos+1];
			}
			else {
				FrameSplit.mb[MBA].hmv1 = MV[0][0][ypos+1][xpos+1];
				FrameSplit.mb[MBA].vmv1 = MV[1][0][ypos+1][xpos+1];
			}
		}

		/* pixel coordinates of top left corner of current macroblock */
		/* one delayed because of OBMC */
		if (xpos > 0) {
			bx = 16*(xpos-1);
			by = 16*ypos;
		}
		else {
			bx = coded_picture_width-16;
			by = 16*(ypos-1);
		}

		if (!COD) {

			Mode = modemap[ypos+1][xpos+1];

			/* decode blocks */
			for (comp=0; comp<blk_cnt; comp++) {

				clearblock(comp);
				if (Mode == MODE_INTRA || Mode == MODE_INTRA_Q) { /* Intra */
					bp = ld->block[comp];
					if(syntax_arith_coding) {
						INTRADC_index = decode_a_symbol(cumf_INTRADC);
						bp[0] = intradctab[INTRADC_index];
						if (trace)
							printf("INTRADC Index: %d INTRADC: %d \n", INTRADC_index, bp[0]);
					}
					else {
						bp[0] = getbits(8);
						if (trace) {
							printf("DC[%d]: (",comp);
							printbits((int)bp[0],8,8);
							printf("): %d\n",(int)bp[0]);
						}
					}

					if (bp[0] == 128)
						if (!quiet)
							dprint4("Illegal DC-coeff: 1000000\n");
					if (bp[0] == 255)  /* Spec. in H.26P, not in TMN4 */
						bp[0] = 128;
					bp[0] *= 8; /* Iquant */
					if ( (CBP & (1<<(blk_cnt-1-comp))) ) {
						if (!syntax_arith_coding)
							getblock(comp,0);
						else
							get_sac_block(comp,0);
					}
				}
				else { /* Inter */
					if ( (CBP & (1<<(blk_cnt-1-comp))) ) {
						if (!syntax_arith_coding)
							getblock(comp,1);
						else
							get_sac_block(comp,1);
					}

				}
				if (fault) goto resync;
			}

			/* Decode B blocks */
			if (pb_frame) {
				for (comp=6; comp<blk_cnt+6; comp++) {
					clearblock(comp);
					if ( (CBPB & (1<<(blk_cnt-1-comp%6))) ) {
						if (!syntax_arith_coding)
							getblock(comp,1);
						else
							get_sac_block(comp,1);
					}
					if (fault) goto resync;
				}
			}

		}

		/* advance to next macroblock */
		MBA++;

		pCBP = CBP; pCBPB = CBPB; pCOD = COD;
		pmvdbx = mvdbx; pmvdby = mvdby;
		fflush(stdout);

		if (MBA >= MBAmax && !last_done) {
			COD = 1;
			xpos = 0;
			ypos++;
			last_done = 1;
			goto reconstruct_mb;
		}

	}
}

/* set block to zero */
void VS_H263Parser::clearblock(int comp)
{
	int *bp;
	int i;

	bp = (int *)ld->block[comp];

	for (i=0; i<8; i++)
	{
		bp[0] = bp[1] = bp[2] = bp[3] = 0;
		bp += 4;
	}
}



int VS_H263Parser::motion_decode( int vec, int pmv)
{
	if (vec > 31) vec -= 64;
	vec += pmv;
	if (!long_vectors) {
		if (vec > 31)
			vec -= 64;
		if (vec < -32)
			vec += 64;
	}
	else {
		if (pmv < -31 && vec < -63)
			vec += 64;
		if (pmv > 32 && vec > 63)
			vec -= 64;
	}
	return vec;
}


int VS_H263Parser::find_pmv( int x,  int y,  int block, int comp)
{
	int p1,p2,p3;
	int xin1,xin2,xin3;
	int yin1,yin2,yin3;
	int vec1,vec2,vec3;
	int l8,o8,or8;

	x++;y++;

	l8 = (modemap[y][x-1] == MODE_INTER4V ? 1 : 0);
	o8 =  (modemap[y-1][x] == MODE_INTER4V ? 1 : 0);
	or8 = (modemap[y-1][x+1] == MODE_INTER4V ? 1 : 0);

	switch (block) {
  case 0:
	  vec1 = (l8 ? 2 : 0) ; yin1 = y  ; xin1 = x-1;
	  vec2 = (o8 ? 3 : 0) ; yin2 = y-1; xin2 = x;
	  vec3 = (or8? 3 : 0) ; yin3 = y-1; xin3 = x+1;
	  break;
  case 1:
	  vec1 = (l8 ? 2 : 0) ; yin1 = y  ; xin1 = x-1;
	  vec2 = (o8 ? 3 : 0) ; yin2 = y-1; xin2 = x;
	  vec3 = (or8? 3 : 0) ; yin3 = y-1; xin3 = x+1;
	  break;
  case 2:
	  vec1 = 1            ; yin1 = y  ; xin1 = x;
	  vec2 = (o8 ? 4 : 0) ; yin2 = y-1; xin2 = x;
	  vec3 = (or8? 3 : 0) ; yin3 = y-1; xin3 = x+1;
	  break;
  case 3:
	  vec1 = (l8 ? 4 : 0) ; yin1 = y  ; xin1 = x-1;
	  vec2 = 1            ; yin2 = y  ; xin2 = x;
	  vec3 = 2            ; yin3 = y  ; xin3 = x;
	  break;
  case 4:
	  vec1 = 3            ; yin1 = y  ; xin1 = x;
	  vec2 = 1            ; yin2 = y  ; xin2 = x;
	  vec3 = 2            ; yin3 = y  ; xin3 = x;
	  break;
  default:
	  dprint4("Illegal block number in find_pmv (getpic.c)\n");
	  exit(1);
	  break;
	}
	p1 = MV[comp][vec1][yin1][xin1];
	p2 = MV[comp][vec2][yin2][xin2];
	p3 = MV[comp][vec3][yin3][xin3];

	if (newgob && (block == 0 || block == 1 || block == 2))
		p2 = NO_VEC;

	if (p2 == NO_VEC) { p2 = p3 = p1; }

	return p1+p2+p3 - mmax(p1,mmax(p2,p3)) - mmin(p1,mmin(p2,p3));
}






// GETVLC
int VS_H263Parser::getTMNMV()
{
	int code;

	if (trace)
		printf("motion_code (");

	if (getbits1())
	{
		if (trace)
			printf("1): 0\n");
		return 0;
	}

	if ((code = showbits(12))>=512)
	{
		code = (code>>8) - 2;
		flushbits(TMNMVtab0[code].len);

		if (trace)
		{
			printf("0");
			printbits(code+2,4,TMNMVtab0[code].len);
			printf("): %d\n", TMNMVtab0[code].val);
		}

		return TMNMVtab0[code].val;
	}

	if (code>=128)
	{
		code = (code>>2) -32;
		flushbits(TMNMVtab1[code].len);

		if (trace)
		{
			printf("0");
			printbits(code+32,10,TMNMVtab1[code].len);
			printf("): %d\n",TMNMVtab1[code].val);
		}

		return TMNMVtab1[code].val;
	}

	if ((code-=5)<0)
	{
		if (!quiet)
			dprint4("Invalid motion_vector code\n");
		fault=1;
		return 0;
	}

	flushbits(TMNMVtab2[code].len);

	if (trace)
	{
		printf("0");
		printbits(code+5,12,TMNMVtab2[code].len);
		printf("): %d\n",TMNMVtab2[code].val);
	}

	return TMNMVtab2[code].val;
}


int VS_H263Parser::getMCBPC()
{
	int code;

	if (trace)
		printf("MCBPC (");

	code = showbits(9);

	if (code == 1) {
		/* macroblock stuffing */
		if (trace)
			printf("000000001): stuffing\n");
		flushbits(9);
		return -1;
	}

	if (code == 0) {
		if (!quiet)
			dprint4("Invalid MCBPC code\n");
		fault = 1;
		return 0;
	}

	if (code>=256)
	{
		flushbits(1);
		if (trace)
			printf("1): %d\n",0);
		return 0;
	}

	flushbits(MCBPCtab[code].len);

	if (trace)
	{
		printbits(code,9,MCBPCtab[code].len);
		printf("): %d\n",MCBPCtab[code].val);
	}

	return MCBPCtab[code].val;
}

int VS_H263Parser::getMODB()
{
	int code;
	int MODB;

	if (trace)
		printf("MODB (");

	code = showbits(2);

	if (code < 2) {
		if (trace)
			printf("0): MODB = 0\n");
		MODB = 0;
		flushbits(1);
	}
	else if (code == 2) {
		if (trace)
			printf("10): MODB = 1\n");
		MODB = 1;
		flushbits(2);
	}
	else { /* code == 3 */
		if (trace)
			printf("11): MODB = 2\n");
		MODB = 2;
		flushbits(2);
	}
	return MODB;
}


int VS_H263Parser::getMCBPCintra()
{
	int code;

	if (trace)
		printf("MCBPCintra (");

	code = showbits(9);

	if (code == 1) {
		/* macroblock stuffing */
		if (trace)
			printf("000000001): stuffing\n");
		flushbits(9);
		return -1;
	}

	if (code < 8) {
		if (!quiet)
			dprint4("Invalid MCBPCintra code\n");
		fault = 1;
		return 0;
	}

	code >>= 3;

	if (code>=32)
	{
		flushbits(1);
		if (trace)
			printf("1): %d\n",3);
		return 3;
	}

	flushbits(MCBPCtabintra[code].len);

	if (trace)
	{
		printbits(code,6,MCBPCtabintra[code].len);
		printf("): %d\n",MCBPCtabintra[code].val);
	}

	return MCBPCtabintra[code].val;
}

int VS_H263Parser::getCBPY()
{
	int code;

	if (trace)
		printf("CBPY (");

	code = showbits(6);
	if (code < 2) {
		if (!quiet)
			dprint4("Invalid CBPY code\n");
		fault = 1;
		return -1;
	}

	if (code>=48)
	{
		flushbits(2);
		if (trace)
			printf("11): %d\n",0);
		return 0;
	}

	flushbits(CBPYtab[code].len);

	if (trace)
	{
		printbits(code,6,CBPYtab[code].len);
		printf("): %d\n",CBPYtab[code].val);
	}

	return CBPYtab[code].val;
}

// H263PLAY

//
// decode a file and store and/or display it
//
bool VS_H263Parser::play_movie(void * buffer, unsigned int buffer_size)
{
	ld->large_buffer = (unsigned char*)buffer;
	ld->lbsize = buffer_size;
	initbits();

	if (getheader()==0)
		return false;
	if (!initdecoder())
		return false;
	getMBs();
	return true;
}


/////////////////////////////////////////////////////////////////////
//
// MPEG related functions
//
// (portable)
//
bool VS_H263Parser::initdecoder()
{
	FrameSplit.Clear();
	if (source_format == SF_CIF) {  // Input stream is CIF
		MBC = 22;
		MBR = 18;
	}
	else if (source_format == SF_QCIF) {   // Input stream is QCIF
		MBC = 11;
		MBR = 9;
	}
	else if (source_format == SF_SQCIF) {  // Input stream is SQCIF
		MBC = 8;
		MBR = 6;
	}
	else
		return false;

	FrameSplit.Set(MBR, MBC*MBR);

	horizontal_size = MBC*16;
	vertical_size = MBR*16;

	mb_width = horizontal_size/16;
	mb_height = vertical_size/16;
	coded_picture_width = 16*mb_width;
	coded_picture_height = 16*mb_height;
	blk_cnt = 6;
	return true;
}


void VS_H263Parser::printbits(int code, int bits, int len)
{
	int i;
	for (i=0; i<len; i++)
		printf("%d",(code>>(bits-1-i))&1);
}


// SAC
#define   q1    16384
#define   q2    32768
#define   q3    49152
#define   top   65535

/*********************************************************************
* 	SAC Decoder Algorithm as Specified in H26P Annex -E
*
* 	Name:		decode_a_symbol
*
*	Description:	Decodes an Aritmetically Encoded Symbol
*
*	Input:		array holding cumulative freq. data
*			also uses static data for decoding endpoints
*			and code_value variable
*
*	Returns:	Index to relevant symbol model
*
*	Side Effects:	Modifies low, high, length, cum and code_value
*
*	Author:		Wayne Ellis <ellis_w_wayne@bt-web.bt.co.uk>
*
*********************************************************************/
int VS_H263Parser::decode_a_symbol(int cumul_freq[ ])
{

	length = high - low + 1;
	cum = (-1 + (code_value - low + 1) * cumul_freq[0]) / length;
	for (sacindex = 1; cumul_freq[sacindex] > cum; sacindex++);
	high = low - 1 + (length * cumul_freq[sacindex-1]) / cumul_freq[0];
	low += (length * cumul_freq[sacindex]) / cumul_freq[0];

	for ( ; ; ) {
		if (high < q2) ;
		else if (low >= q2) {
			code_value -= q2;
			low -= q2;
			high -= q2;
		}
		else if (low >= q1 && high < q3) {
			code_value -= q1;
			low -= q1;
			high -= q1;
		}
		else {
			break;
		}

		low *= 2;
		high = 2*high + 1;
		bit_out_psc_layer();
		code_value = 2*code_value + bit;
	}

	return (sacindex-1);
}

/*********************************************************************
*
* 	Name:		decoder_reset
*
*	Description:	Fills Decoder FIFO after a fixed word length
*			string has been detected.
*
*	Input:		None
*
*	Returns:	Nothing
*
*	Side Effects:	Fills Arithmetic Decoder FIFO
*
*	Author:		Wayne Ellis <ellis_w_wayne@bt-web.bt.co.uk>
*
*********************************************************************/
void VS_H263Parser::decoder_reset( )
{
	int i;
	zerorun = 0;			/* clear consecutive zero's counter */
	code_value = 0;
	low = 0;
	high = top;
	for (i = 1;   i <= 16;   i++) {
		bit_out_psc_layer();
		code_value = 2 * code_value + bit;
	}
	if (trace)
		printf("Arithmetic Decoder Reset \n");
}

/*********************************************************************
*
* 	Name:		bit_out_psc_layer
*
*	Description:	Gets a bit from the Encoded Stream, Checks for
*			and removes any PSC emulation prevention bits
*			inserted at the decoder, provides 'zeros' to the
*			Arithmetic Decoder FIFO to allow it to finish
*			data prior to the next PSC. (Garbage bits)
*
*	Input:		None
*
*	Returns:	Nothing
*
*	Side Effects:	Gets a bit from the Input Data Stream
*
*	Author:		Wayne Ellis <ellis_w_wayne@bt-web.bt.co.uk>
*
*********************************************************************/
void VS_H263Parser::bit_out_psc_layer()
{
	if (showbits(17)!=1) { /* check for startcode in Arithmetic Decoder FIFO */

		bit = getbits(1);

		if(zerorun > 13) {	/* if number of consecutive zeros = 14 */
			if (!bit) {
				if (trace)
					printf("PSC/GBSC, Header Data, or Encoded Stream Error \n");
				zerorun = 1;
			}
			else { /* if there is a 'stuffing bit present */
				if (trace)
					printf("Removing Startcode Emulation Prevention bit \n");
				bit = getbits(1); 	/* overwrite the last bit */
				zerorun = !bit;	  	/* zerorun=1 if bit is a '0' */
			}
		}

		else { /* if consecutive zero's not exceeded 14 */

			if (!bit)
				zerorun++;
			else
				zerorun = 0;
		}

	} /* end of if !(showbits(17)) */

	else {
		bit = 0;
		if (trace)
			printf("Startcode Found:Finishing Arithmetic Decoding using 'Garbage bits'\n");
	}

	/*
	printf("lastbit = %ld bit = %ld zerorun = %ld \n", lastbit, bit, zerorun);
	lastbit = bit;
	*/
	/* latent diagnostics */

}

