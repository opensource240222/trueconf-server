/**
 **************************************************************************
 * \file RANGECD.C
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief Range coder/decoder
 *
 * \b Project Standart Libraries
 * \author Dm.Vatolin dmitriy@compression.ru
 * \date 22.11.02
 *
 * $Revision: 1 $
 *
 * $History: RANGECD.C $
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/std/clib
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/std/clib
 *
 * *****************  Version 10  *****************
 * User: Smirnov      Date: 30.12.04   Time: 14:12
 * Updated in $/VS/std/clib
 * files headers
 *
 * *****************  Version 9  *****************
 * User: Stass        Date: 5.08.04    Time: 17:44
 * Updated in $/VS/std/clib
 * added return -1 to unsafe ver
 *
 * *****************  Version 8  *****************
 * User: Vatolin      Date: 5.08.04    Time: 16:25
 * Updated in $/VS/std/clib
 * Add comment
 *
 * *****************  Version 7  *****************
 * User: Stass        Date: 20.07.04   Time: 12:33
 * Updated in $/VS/std/clib
 *
 * *****************  Version 6  *****************
 * User: Stass        Date: 5.07.04    Time: 14:02
 * Updated in $/VS/std/clib
 * fixed return code
 *
 * *****************  Version 5  *****************
 * User: Dmitriy      Date: 28.06.04   Time: 18:16
 * Updated in $/VS/std/clib
 * Add Safe decoder
 *
 * *****************  Version 4  *****************
 * User: Vatolin      Date: 3.04.03    Time: 16:47
 * Updated in $/VS/std/clib
 ****************************************************************************/

/****************************************************************************
 * Includes
 ****************************************************************************/
#include "rangecd.h"

#include <string.h>
#include <stdio.h>
#include <stdint.h>

#ifndef uint
typedef unsigned int  uint;
#endif


/****************************************************************************
 * Defines
 ****************************************************************************/
#define TOP        (1<<24)
#define BOT        (1<<16)
#define R_STEP     (64)
#define R_STEP2    (64)

const int env_tbl[32]=
// 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13,
{ 32,128, 16,  1, 64, 64, 64,256, 32,  2,  1, 32,256, 32,

//14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
 256,256, 32,256,256,128,128, 32,  2, 16,256, 32, 32, 32, 32, 32, 32, 32};


/**
 **************************************************************************
 * \brief Encode data
 ****************************************************************************/
size_t RCDV_Encode(const void* pSRC, size_t Size, void* pDST)
{
    const uint8_t* pSrc = (const uint8_t*)pSRC;
    uint8_t* pDst = (uint8_t*)pDST;
    int  cnt__[256], i, i3;
    int* cnt = cnt__;
    uint low=0, range=(uint)(-1), totFreq=256;
//    uint cum[8]={32,64,96,128,160,192,224,256};
    uint cum[8]={0,0,0,0,0,0,0,0};

    memset(cnt__,0,sizeof(cnt__));

    for (size_t i2 = 0; i2 < Size; i2++) {
        int c=pSrc[i2], cumFreq=c, cumFreq2=c;

        switch(c>>5) {
        case 0:                  i=0;  goto RCDV_En0;
        case 1: cumFreq+=cum[1]; i=32; goto RCDV_En1;
        case 2: cumFreq+=cum[2]; i=64; goto RCDV_En2;
        case 3: cumFreq+=cum[3]; i=96; goto RCDV_En3;
        case 4: cumFreq+=cum[4]; i=128;goto RCDV_En4;
        case 5: cumFreq+=cum[5]; i=160;goto RCDV_En5;
        case 6: cumFreq+=cum[6]; i=192;goto RCDV_En6;
        case 7: cumFreq+=cum[7]; i=224;goto RCDV_En7;
        }

RCDV_En0:   cum[1]+=R_STEP;
RCDV_En1:   cum[2]+=R_STEP;
RCDV_En2:   cum[3]+=R_STEP;
RCDV_En3:   cum[4]+=R_STEP;
RCDV_En4:   cum[5]+=R_STEP;
RCDV_En5:   cum[6]+=R_STEP;
RCDV_En6:   cum[7]+=R_STEP;
RCDV_En7:

		for(;i<c;i++)
            cumFreq+=cnt[i]; // +1 делается выше +с оптом

/*        for(i=0;i<c;i++)
            cumFreq2+=cnt[i]; // +1 делается выше +с оптом
        assert(cumFreq==cumFreq2);  cumFreq=cumFreq2;
*/
//        if(i2<30) printf("===== %12u %12u %12u, %3d \n",low, range, totFreq, c);
//        assert(cumFreq+cnt[c]+1<=totFreq && (cnt[c]+1) && totFreq<=BOT);

        low  += cumFreq * (range/= totFreq);
        range*= (cnt[c]+1);

        while ((low ^ low+range)<TOP || range<BOT && ((range= (-(int)low) & BOT-1),1))
            *pDst++=(low>>24), range<<=8, low<<=8;

        cnt[c]+=R_STEP; totFreq+=R_STEP;

/*        if(totFreq > BOT)
            for(i=0, totFreq=256;i<256;i++)
                cnt[i]>>=1, totFreq+= cnt[i];
*/
        if(totFreq > BOT)
            for(i3=0, totFreq=0, c=0;i3<8;i3++)
                for(i=0, cum[i3]=totFreq-c, totFreq+=32;i<32;i++)
                    cnt[c]>>=1, totFreq+= cnt[c], c++;
    }

    for(i=0;i<4;i++) *pDst++=(low>>24), low<<=8;

    return pDst-(uint8_t*)pDST;
}

/**
 **************************************************************************
 * \brief Decode data
 ****************************************************************************/
size_t RCDV_Decode(const void* pSRC, void* pDST, size_t Size)
{
    const uint8_t* pSrc = (const uint8_t*)pSRC;
    uint8_t* pDst = (uint8_t*)pDST;
    int  cnt__[256], i, i3;
    int* cnt = cnt__;
    uint  low=0, code, range=(uint)(-1), totFreq=256;
    uint cum[8]={0,32,64,96,128,160,192,224};

    memset(cnt__,0,sizeof(cnt__));

    for(i=0;i<4;i++) code= code<<8 | *pSrc++;

    for (size_t i2 = 0; i2 < Size; i2++) {
        int c, cumFreq;
        uint value, freq;

        value= (code-low) / (range/= totFreq);

				if (value >= totFreq)
				{
#ifdef _DEBUG
					printf("RangeCoder: Input data corrupt\n");
#endif
					return -1;
				};

        if(value<cum[4])
            if(value<cum[2])
                if(value<cum[1]) {
                    cumFreq=0;       c=0;
                    cum[1]+=R_STEP;
                    cum[2]+=R_STEP;
                    cum[3]+=R_STEP;
                    cum[4]+=R_STEP;
                    cum[5]+=R_STEP;
                    cum[6]+=R_STEP;
                    cum[7]+=R_STEP;
                }
                else {
                    cumFreq=cum[1]; c=32;
                    cum[2]+=R_STEP;
                    cum[3]+=R_STEP;
                    cum[4]+=R_STEP;
                    cum[5]+=R_STEP;
                    cum[6]+=R_STEP;
                    cum[7]+=R_STEP;
                }
            else
                if(value<cum[3]) {
                    cumFreq=cum[2]; c=64;
                    cum[3]+=R_STEP;
                    cum[4]+=R_STEP;
                    cum[5]+=R_STEP;
                    cum[6]+=R_STEP;
                    cum[7]+=R_STEP;
                }
                else {
                    cumFreq=cum[3]; c=96;
                    cum[4]+=R_STEP;
                    cum[5]+=R_STEP;
                    cum[6]+=R_STEP;
                    cum[7]+=R_STEP;
                }
        else
            if(value<cum[6])
                if(value<cum[5]) {
                    cumFreq=cum[4]; c=128;
                    cum[5]+=R_STEP;
                    cum[6]+=R_STEP;
                    cum[7]+=R_STEP;
                }
                else {
                    cumFreq=cum[5]; c=160;
                    cum[6]+=R_STEP;
                    cum[7]+=R_STEP;
                }
            else
                if(value<cum[7]) {
                    cumFreq=cum[6]; c=192;
                    cum[7]+=R_STEP;
                }
                else {
                    cumFreq=cum[7]; c=224;
                }

        for( ; c<256; c++ ) {
            if (value>=(uint)cumFreq+cnt[c]+1)
                cumFreq+=cnt[c]+1;
            else
                break;
        }

/*        for( c=0,cumFreq2=0; c<256; c++ ) {
            if (value>=cumFreq2+cnt[c]+1)
                cumFreq2+=cnt[c]+1;
            else
                break;
        }
        assert(cumFreq==cumFreq2);
        //if(i2<30) printf("ddd== %12u %12u %12u, %3d \n",low, range, totFreq, c);
*/
//        assert(c<256 && cumFreq+cnt[c]+1<=totFreq && (cnt[c]+1) && totFreq<=BOT);
        freq = cnt[c]+1;
        pDst[i2]=c;

        low  += cumFreq*range;
        range*= freq;

        while ((low ^ low+range)<TOP || range<BOT && ((range= (-(int)low) & BOT-1),1))
           code= code<<8 | *pSrc++, range<<=8, low<<=8;

        cnt[c]+=R_STEP; totFreq+=R_STEP;

        if(totFreq > BOT)
            for(i3=0, totFreq=0, c=0;i3<8;i3++)
                for(i=0, cum[i3]=totFreq, totFreq+=32;i<32;i++)
                     totFreq+= (cnt[c++]>>=1);
    }

    return pSrc-(const uint8_t*)pSRC;
}

/**
 **************************************************************************
 * \brief Decode data with input data range checking in case of missing data
 ****************************************************************************/
size_t RCDV_DecodeSafe(const void* pSRC, void* pDST, size_t SizeDst, size_t SizeSrc)
{
    if (SizeSrc < 4)
        return -1;

    const uint8_t* pSrc = (const uint8_t*)pSRC;
    uint8_t* pDst = (uint8_t*)pDST;
    int  cnt__[256], i, i3;
    int* cnt = cnt__;
    uint  low=0, code, range=(uint)(-1), totFreq=256;
    uint cum[8]={0,32,64,96,128,160,192,224};

    memset(cnt__,0,sizeof(cnt__));

// TODO: Decode poisones stack when input buffer is broken
    for(i=0;i<4;i++) code= code<<8 | *pSrc++;

    for (size_t i2 = 0; i2 < SizeDst; i2++) {
        int c, cumFreq;
        uint value, freq;

        value= (code-low) / (range/= totFreq);
		if (value >= totFreq)
		{
#ifdef _DEBUG
			printf("\nRangeCoderSafe: Input data corrupt\n\n");
#endif
			return -1;
		}

        if(value<cum[4])
            if(value<cum[2])
                if(value<cum[1]) {
                    cumFreq=0;       c=0;
                    cum[1]+=R_STEP;
                    cum[2]+=R_STEP;
                    cum[3]+=R_STEP;
                    cum[4]+=R_STEP;
                    cum[5]+=R_STEP;
                    cum[6]+=R_STEP;
                    cum[7]+=R_STEP;
                }
                else {
                    cumFreq=cum[1]; c=32;
                    cum[2]+=R_STEP;
                    cum[3]+=R_STEP;
                    cum[4]+=R_STEP;
                    cum[5]+=R_STEP;
                    cum[6]+=R_STEP;
                    cum[7]+=R_STEP;
                }
            else
                if(value<cum[3]) {
                    cumFreq=cum[2]; c=64;
                    cum[3]+=R_STEP;
                    cum[4]+=R_STEP;
                    cum[5]+=R_STEP;
                    cum[6]+=R_STEP;
                    cum[7]+=R_STEP;
                }
                else {
                    cumFreq=cum[3]; c=96;
                    cum[4]+=R_STEP;
                    cum[5]+=R_STEP;
                    cum[6]+=R_STEP;
                    cum[7]+=R_STEP;
                }
        else
            if(value<cum[6])
                if(value<cum[5]) {
                    cumFreq=cum[4]; c=128;
                    cum[5]+=R_STEP;
                    cum[6]+=R_STEP;
                    cum[7]+=R_STEP;
                }
                else {
                    cumFreq=cum[5]; c=160;
                    cum[6]+=R_STEP;
                    cum[7]+=R_STEP;
                }
            else
                if(value<cum[7]) {
                    cumFreq=cum[6]; c=192;
                    cum[7]+=R_STEP;
                }
                else {
                    cumFreq=cum[7]; c=224;
                }

        for( ; c<256; c++ ) {
            if (value>=(uint)cumFreq+cnt[c]+1)
                cumFreq+=cnt[c]+1;
            else
                break;
        }

//      assert(cumFreq==cumFreq2);
//		if(i2<30) printf("ddd== %12u %12u %12u, %3d \n",low, range, totFreq, c);
//      assert(c<256 && cumFreq+cnt[c]+1<=totFreq && (cnt[c]+1) && totFreq<=BOT);

        freq = cnt[c]+1;
        pDst[i2]=c;

        low  += cumFreq*range;
        range*= freq;
		while ((low ^ low+range)<TOP || range<BOT && ((range= (-(int)low) & BOT-1),1))
			if((pSrc-(const uint8_t*)pSRC)<SizeSrc)
				code= code<<8 | *pSrc++, range<<=8, low<<=8;
			else
				return -1;  //source overflow

		cnt[c]+=R_STEP; totFreq+=R_STEP;

        if(totFreq > BOT)
            for(i3=0, totFreq=0, c=0;i3<8;i3++)
                for(i=0, cum[i3]=totFreq, totFreq+=32;i<32;i++)
                     totFreq+= (cnt[c++]>>=1);
    }

    return pSrc-(const uint8_t*)pSRC;
}

