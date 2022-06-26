/*/////////////////////////////////////////////////////////////////////////////
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 2005-2011 Intel Corporation. All Rights Reserved.
//
//     Intel(R) Integrated Performance Primitives
//     USC - Unified Speech Codec interface library
//
// By downloading and installing USC codec, you hereby agree that the
// accompanying Materials are being provided to you under the terms and
// conditions of the End User License Agreement for the Intel(R) Integrated
// Performance Primitives product previously accepted by you. Please refer
// to the file ippEULA.rtf or ippEULA.txt located in the root directory of your Intel(R) IPP
// product installation for more information.
//
// A speech coding standards promoted by ITU, ETSI, 3GPP and other
// organizations. Implementations of these standards, or the standard enabled
// platforms may require licenses from various entities, including
// Intel Corporation.
//
//
// Purpose: AMRWBE speech codec: common utilities.
//
*/

#include "ownamrwbe.h"

#define MASK      0x0001


Ipp16s Bin2int(Ipp32s no_of_bits, Ipp16s *bitstream)
{
   Ipp16s value=0;
   Ipp32s i;

   for (i=0; i<no_of_bits; i++) {
      value  = ShiftL_16s(value, 1);
      value  = Add_16s(value, (Ipp16s)(*bitstream & MASK));
      bitstream ++;
   }
   return(value);
}

void Int2bin(
  Ipp16s value,
  Ipp32s no_of_bits,
  Ipp16s *bitstream
)
{
  Ipp16s *pt_bitstream;
  Ipp32s i;

  pt_bitstream = bitstream + no_of_bits;

  for (i = 0; i < no_of_bits; i++)
  {
    --pt_bitstream;
    *pt_bitstream = (Ipp16s)(value & MASK);
    value = (Ipp16s)(value>>1);
  }

}

void ownScaleHPFilterState(Ipp16s* pMemHPFilter, Ipp32s scale, Ipp32s order) {

   Ipp32s tmp32;
   Ipp16s tmp16;

   if (order == 2) {
      Ipp16s y2H,y2L,y1H,y1L,x0,x1;
      y2H=pMemHPFilter[0]; y2L=pMemHPFilter[1];
      y1H=pMemHPFilter[2]; y1L=pMemHPFilter[3];
      x0=pMemHPFilter[4];
      x1=pMemHPFilter[5];
      if (scale > 0) {
         tmp32 = x0 << 16;
         tmp32 = ShiftL_32s(tmp32, (Ipp16u)(scale));
         Unpack_32s(tmp32, &x0, &tmp16);

         tmp32 = (y1H << 16) + (y1L << 1);
         tmp32 = ShiftL_32s(tmp32, (Ipp16u)(scale));
         Unpack_32s(tmp32, &y1H, &y1L);

         y1L = (Ipp16s)(y1L - (tmp16 >> 1));

         tmp32 = x1 << 16;
         tmp32 = ShiftL_32s(tmp32, (Ipp16u)(scale));
         Unpack_32s(tmp32, &x1, &tmp16);

         tmp32 = (y2H << 16) + (y2L << 1);
         tmp32 = ShiftL_32s(tmp32, (Ipp16u)(scale));
         Unpack_32s(tmp32, &y2H, &y2L);

         y2L = (Ipp16s)(y2L - (tmp16 >> 1));
      } else {
         scale = -scale;
         tmp32 = x0 << 16;
         tmp32 >>=  scale;
         Unpack_32s(tmp32, &x0, &tmp16);

         tmp32 = (y1H << 16) + (y1L << 1);
         tmp32 >>= scale;
         Unpack_32s(tmp32, &y1H, &y1L);

         y1L = (Ipp16s)(y1L - (tmp16 >> 1));

         tmp32 = x1 << 16;
         tmp32 >>= scale;
         Unpack_32s(tmp32, &x1, &tmp16);

         tmp32 = (y2H << 16) + (y2L << 1);
         tmp32 >>= scale;
         Unpack_32s(tmp32, &y2H, &y2L);

         y2L = (Ipp16s)(y2L - (tmp16 >> 1));
      }
      pMemHPFilter[0]=y2H; pMemHPFilter[1]=y2L;
      pMemHPFilter[2]=y1H; pMemHPFilter[3]=y1L;
      pMemHPFilter[4]=x0;
      pMemHPFilter[5]=x1;
   }
}

void ownIntLPC(
   const Ipp16s*  pIspOld,
   const Ipp16s*  pIspNew,
   const Ipp16s*  pIntWnd,
   Ipp16s*        pAz,
   Ipp32s         nSubfr,
   Ipp32s         m
) {
   IPP_ALIGNED_ARRAY(16, Ipp16s, pIspTmp, M);
   Ipp32s k;
   Ipp16s fac_old, fac_new;

   for(k=0; k<nSubfr; k++) {
      fac_new = pIntWnd[k];  /* modify tables and remove "Add_16s" */
      fac_old = Add_16s((Ipp16s)(32767-fac_new) ,1);
      ippsInterpolateC_NR_G729_16s_Sfs(pIspOld, fac_old, pIspNew, fac_new, pIspTmp, m, 15);
      ippsISPToLPC_AMRWB_16s(pIspTmp, pAz, m);
      pAz += (m+1);
  }

   ippsISPToLPC_AMRWB_16s(pIspNew, pAz, m);
   return;
}

void ownFindWSP(const Ipp16s* Az, const Ipp16s* speech_ns,
                Ipp16s* wsp, Ipp16s* mem_wsp, Ipp32s lg)
{
   IPP_ALIGNED_ARRAY(16, Ipp16s, Azp, (M+1));
   const Ipp16s* p_Az;
   Ipp32s i_subfr;

   p_Az = Az;
   for (i_subfr=0; i_subfr<lg; i_subfr+=L_SUBFR) {
      ippsMulPowerC_NR_16s_Sfs(p_Az, GAMMA1_FX, Azp, (M+1), 15);
      ippsResidualFilter_Low_16s_Sfs(Azp, M, &speech_ns[i_subfr], &wsp[i_subfr], L_SUBFR, 12);
      p_Az += (M+1);
   }
   ippsDeemphasize_AMRWBE_NR_16s_I(TILT_FAC_FX, 15, wsp, lg, mem_wsp);

   return;
}


Ipp16s Match_gain_6k4(Ipp16s *AqLF, Ipp16s *AqHF)
{
  Ipp32s i, Ltmp;
  Ipp16s buf[M+L_SUBFR], tmp16, max16s, scale;
  Ipp16s code[L_SUBFR], exp_g, frac;
  Ipp16s gain16;

   ippsZero_16s(buf, M);
   tmp16 = 128;
   for (i=0; i<L_SUBFR; i++) {
      buf[i+M] = tmp16;
      tmp16 = MulHR_16s(tmp16, -29490);
   }
   /*^^^^ make table ??!! ^^^^*/

   ippsResidualFilter_Low_16s_Sfs(AqLF, M, buf+M, code, L_SUBFR, 11);

   tmp16 = AqHF[0];
   AqHF[0] >>= 1;
   ippsSynthesisFilter_G729E_16s_I(AqHF, MHF, code, L_SUBFR, buf);
   AqHF[0] = tmp16;

   ippsMaxAbs_16s(code, L_SUBFR, &max16s);

   scale = (Ipp16s)(Exp_16s(max16s) - 3);
   ownScaleSignal_AMRWB_16s_ISfs(code, L_SUBFR, scale);

   ippsDotProd_16s32s_Sfs(code, code, L_SUBFR, &Ltmp, -1);
   Ltmp = Add_32s(Ltmp, 1);
   exp_g = (Ipp16s)(30 - Norm_32s_I(&Ltmp));

   _Log2_norm(Ltmp,exp_g, &tmp16, &frac);
   exp_g = Sub_16s(29, Add_16s(tmp16, ShiftL_16s((Ipp16s)(scale+7),1)));

   Ltmp    = Mpy_32_16(exp_g, frac, LG10);
   gain16 = (Ipp16s)(Ltmp>>(14-8));

   return Negate_16s(gain16);
}

static __ALIGN(16) CONST Ipp16s tabLog_16s[33] = {
   0x0000, 0x05AF, 0x0B32, 0x108C, 0x15C0, 0x1ACF, 0x1FBC, 0x2488,
   0x2935, 0x2DC4, 0x3237, 0x368F, 0x3ACE, 0x3EF5, 0x4304, 0x46FC,
   0x4ADF, 0x4EAE, 0x5269, 0x5611, 0x59A7, 0x5D2C, 0x609F, 0x6403,
   0x6757, 0x6A9B, 0x6DD1, 0x70FA, 0x7414, 0x7721, 0x7A22, 0x7D17,
   0x7FFF};

void ownLog2_AMRWBE(Ipp32s L_x, Ipp16s *logExponent, Ipp16s *logFraction)
{
  Ipp16s exp, i, a, tmp;
  Ipp32s L_y;

  if( L_x <= 0 ){
    *logExponent = 0;
    *logFraction = 0;
    return;
  }

  exp = Norm_32s_I(&L_x);

  *logExponent = (Ipp16s)(30 - exp);

  i = (Ipp16s)((L_x >> 25) - 32);
  a = (Ipp16s)((L_x >> 10) & 0x7fff);
  L_y = tabLog_16s[i] << 15;
  tmp = (Ipp16s)(tabLog_16s[i] - tabLog_16s[i+1]);
  L_y -= tmp * a;
  *logFraction =  (Ipp16s)(L_y >> 15);

  return;
}

void _Log2_norm (Ipp32s L_x, Ipp16s exp, Ipp16s *exponent, Ipp16s *fraction)
{
    Ipp16s i, a, tmp;
    Ipp32s L_y;

    if (L_x <= (Ipp32s) 0)
    {
        *exponent = 0;
        *fraction = 0;
        return;
    }

    *exponent = Sub_16s (30, exp);

    L_x = L_x >> 9;
    i = ExtractHigh(L_x);
    L_x = L_x >> 1;
    a = (Ipp16s)(L_x);
    a = (Ipp16s)(a & (Ipp16s)0x7fff);

    i = Sub_16s (i, 32);

    L_y = (Ipp32s)tabLog_16s[i] << 16;
    tmp = Sub_16s (tabLog_16s[i], tabLog_16s[i + 1]);
    L_y = Sub_32s(L_y, Mul2_Low_32s(tmp*a));

    *fraction = ExtractHigh(L_y);

    return;
}


void Int_gain(Ipp16s old_gain, Ipp16s new_gain, const Ipp16s *Int_wind,
      Ipp16s *gain, Ipp32s nb_subfr)
{
  Ipp16s fold;
  Ipp32s k;
  const Ipp16s *fnew;
  Ipp32s Ltmp;

  fnew = Int_wind;

  for (k=0; k<nb_subfr; k++) {
    fold = Sub_16s(32767, *fnew);
    Ltmp = Mul2_Low_32s(old_gain*fold);
    Ltmp = Add_32s(Ltmp, Mul2_Low_32s(new_gain*(*fnew)));
    *gain = Cnvrt_NR_32s16s(Ltmp);
    fnew ++;
    gain++;
  }
  return;
}


static __ALIGN(16) CONST Ipp16s tblGainQuantHF_16s[] = {
    -10688, -11631, -11179,  -8967, -10742,  -9961,  -8212,  -4368,
    -10378,  -7753,  -4459,  -3413,  -1254, -11821, -11394, -10940,
    -10478,  -7282,    -13,    147,  -3589,  -3556,  -3367,  -3359,
     -2463,  -2785,  -3076,  -3002,  -3081,  -2832,  -2550,  -1746,
     -2563,  -2619,  -1622,  -2779,  -2263,  -1982,  -2380,  -2122,
     -1506,  -1700,  -2652,  -3072,  -1198,  -2544,  -2305,  -2035,
     -1986,  -2148,  -1794,  -1369,  -1555,  -1604,  -1359,  -2247,
     -2232,   -824,  -2212,  -1505,  -1109,  -1377,  -1938,  -1623,
     -1720,  -1335,  -1259,  -1200,  -2743,  -1831,  -1102,  -1234,
     -1057,  -2021,  -1210,  -1074,  -1191,  -1584,  -2263,   -507,
      -844,  -1108,  -1102,  -1508,   -987,  -1022,  -1358,   -775,
     -1567,  -1616,  -1008,   -255,  -1509,  -1052,   -478,   -787,
      -677,  -1238,   -729,   -671,  -1117,  -1752,   -123,  -1752,
      -868,   -417,   -808,   -918,  -1558,   -357,   -899,  -1746,
     -1736,   -498,  -1286,   -293,  -1020,   -829,   -641,     10,
      -752,   -707,    -41,  -1029,   -146,   -742,  -1074,  -1011,
      -431,   -561,   -572,   -418,   -193,   -556,   -700,  -1984,
      -586,   -181,  -1717,  -1279,    -85,   -216,   -461,   -999,
      -512,    -48,  -1132,   -158,   -118,  -1020,  -1384,    -98,
       112,  -1621,  -1590,  -1482,   -439,   -972,  -2012,  -2561,
       810,   -278,  -1513,  -1600,    372,   -223,   -732,   -423,
       259,  -1169,   -260,   -826,    -68,   -376,    -79,   -349,
       -37,   -777,   -473,    393,    -95,  -2528,   -582,    -44,
      -658,  -1152,     71,   -115,   -579,   -313,   -126,    119,
     -1264,   -237,   -256,   -336,   -425,    214,   -272,   -411,
        48,     58,   -358,    121,   -776,   -123,    512,   -395,
        -9,    131,    279,     -2,    -77,   -528,    469,    237,
      -689,    214,    460,    442,   -704,    124,   -573,    751,
        -9,    -40,    157,    714,  -1705,    -98,    280,    540,
      -821,   -719,    344,   1016,  -1850,  -1536,    199,    257,
      -106,    783,    177,    444,  -1189,  -1189,  -1120,   1059,
     -2851,   -538,   -224,   -431,  -1165,   1044,   -486,   -440,
      -426,    285,    229,  -1491,    360,    297,      5,   -517,
       674,   -283,     68,     75,    230,   -441,    666,   -819,
       312,    670,   -772,  -1156,   1028,     -9,   -134,  -1102,
       229,   1042,   -555,    -50,    573,    479,    202,    222,
      1212,    381,   -382,   -122,    576,    419,  -2060,   -107,
       670,    178,   -614,    886,   1053,   1005,    315,   -356,
      1452,  -1084,   -639,   -378,    936,    175,    926,    -94,
       -42,    819,    723,   -398,    522,    346,    539,    899,
        96,    357,    935,    392,    655,   1017,    760,    524,
      1308,    626,    405,    679,    831,  -1056,    602,    809,
       -48,    -87,   1088,   1151,   -428,    484,    510,   1347,
       282,    885,   1189,   1266,    863,    392,    657,   1750,
      1202,    332,   1407,    862,    405,   1283,     96,   1322,
      1136,   1220,   1106,   1228,   -843,   1077,   1185,    744,
       554,   1009,   1863,    410,   1392,   1316,   1115,    298,
       373,   2085,    753,    291,    952,   1026,   1091,  -1328,
      1523,   1551,    -79,    531,   2154,   1327,   1109,   1005,
      2319,    495,    415,    110,   2192,   1876,    855,   -439,
      1790,   1252,  -1093,   -997,    593,    429,    187,  -2730,
      1532,   2138,   1290,   1169,   1707,   1635,    720,   1995,
      1921,   1754,   2240,    675,   1524,   1505,   1874,   1685,
      1946,    576,   1562,   1925,    475,   1928,   1914,   1521,
       711,    716,   2005,   1870,    384,   1252,   1227,   2383,
      1434,   2002,   1837,   2601,   2482,   2036,   1852,   1833,
      -785,    162,   1567,   2143,   -161,   -360,   -173,   2122,
      -634,   -520,   1715,     32,    458,   1569,   2820,   3129,
      2088,   1881,   2690,   2583,   1848,   2777,   2458,   1888,
      2948,   2596,   1460,    775,   2827,   2700,   2394,   2706,
      3374,   2965,   2715,   1688,   2294,   2953,   3317,   3423,
      3580,   3506,   3378,   3200,   4255,   4367,   4433,   4275,
     -2474,  -2520,  -1527,     18,    115,   1502,   -284, -11831,
      2425,   1138,  -5138, -10391,   1329,  -2063, -11649, -10952
};

void Q_gain_hf(Ipp16s *gain, Ipp16s *gain_q, Ipp16s *indice)
{
   Ipp16s gain2[Q_GN_ORDER], tmp16;
   Ipp32s i, k, iQ, min_err, distance, idx=0;

   distance = IPP_MAX_32S;
   for (k=0; k<SIZE_BK_HF; k++) {
      for (i=0; i<Q_GN_ORDER; i++) {
         gain2[i] = Sub_16s(Sub_16s(gain[i], tblGainQuantHF_16s[(k*Q_GN_ORDER)+i]), MEAN_GAIN_HF_FX);
      }
      min_err = 0;
      for (i=0; i<Q_GN_ORDER; i++) {
         tmp16 = (Ipp16s)((gain2[i]*8192)>>15);
         min_err = Add_32s(min_err, Mul2_Low_32s(tmp16*tmp16));
      }

      if (min_err < distance) {
         distance = min_err;
         idx = k;
      }
   }
   indice[0] = (Ipp16s) idx;
   iQ = idx << 2;
   for (i=0; i<Q_GN_ORDER; i++) {
      gain_q[i] = Add_16s(tblGainQuantHF_16s[iQ+i], MEAN_GAIN_HF_FX);
   }
   return;
}



#define ALPHA_FX    29491

void D_gain_hf(Ipp16s indice, Ipp16s *gain_q, Ipp16s *past_q, Ipp32s bfi)
{
   Ipp32s i, iQ, Ltmp;

   iQ = indice << 2;
   if(bfi == 0) {
      for (i=0; i<Q_GN_ORDER; i++) {
         gain_q[i] = Add_16s(tblGainQuantHF_16s[iQ+i], MEAN_GAIN_HF_FX);
      }
   } else {
      *past_q = Sub_16s(MulHR_16s(ALPHA_FX, Add_16s(*past_q ,5120)), 5120);

      for (i=0; i<Q_GN_ORDER; i++) {
         gain_q[i] = Add_16s( *past_q ,  MEAN_GAIN_HF_FX);
      }
   }

   Ltmp = 0;
   for (i=0; i<Q_GN_ORDER; i++) {
      Ltmp = Add_32s(Ltmp, gain_q[i]);
   }
   *past_q = (Ipp16s)((Sub_32s(Ltmp, 2875)>>2));
   return;
}

void Cos_window(Ipp16s *fh, Ipp32s n1, Ipp32s n2, Ipp32s inc2)
{
   Ipp32s i, j;
   Ipp32s inc, offset,offset2;
   Ipp32s Ltmp;
   const Ipp16s *pt_cosup, *pt_cosdwn;

   offset2 = 1;

   if(n1 == L_OVLP) {
      inc = 1;
      offset2 = 0;
   } else if (n1 == (L_OVLP/2)) {
      inc = 2;
   } else if (n1 == (L_OVLP/4)) {
      inc = 4;
   } else if (n1 == (L_OVLP/8)) {
      inc = 8;
   } else {
      inc = 16;
   }

   offset = (inc>>1) - 1;

   if(offset < 0) {
      offset = 0;
   }

   pt_cosup = tblCosHF_16s;
   j = offset;
   for(i=0; i<n1; i++,j+=inc) {
      Ltmp = Mul2_Low_32s(pt_cosup[j]*16384);
      Ltmp = Add_32s(Ltmp, Mul2_Low_32s(pt_cosup[j+offset2]*16384));
      fh[i] = Cnvrt_NR_32s16s(Ltmp);
   }

   offset = (inc2>>1) - 1;

   if(offset < 0) {
      offset = 0;
   }

   if(inc2 == 1) {
      offset2 = 0;
   } else {
      offset2 = 1;
   }

   pt_cosdwn = tblCosHF_16s;
   j = L_OVLP - 1 - offset;
   for(i=0; i<n2; i++,j-=inc2) {
      Ltmp = Mul2_Low_32s(pt_cosdwn[j]*16384);
      Ltmp = Add_32s(Ltmp, Mul2_Low_32s(pt_cosdwn[j-offset2]*16384));
      fh[i+n1] = Cnvrt_NR_32s16s(Ltmp);
   }
   return;
}


Ipp32s SpPeak1k6(Ipp16s* xri, Ipp16s *exp_m, Ipp32s len)
{
   Ipp32s Lmax, Ltmp;
   Ipp32s i, j, i8;

   Lmax = 1;
   for(i=0; i<len; i+=8) {
      Ltmp = 1;
      i8 = i + 8;
      for(j=i; j<i8; j++) {
         Ltmp = Add_32s(Ltmp, Mul2_Low_32s(xri[j]*xri[j]));
      }
      if (Ltmp > Lmax) {
         Lmax = Ltmp;
      }
   }

   *exp_m = Norm_32s_I(&Lmax);
   *exp_m = (Ipp16s)(30 - (*exp_m));
   InvSqrt_32s16s_I(&Lmax, exp_m);
   return Lmax;
}

void Adap_low_freq_emph(Ipp16s* xri, Ipp32s lg)
{
   Ipp32s i, j,lg4, i8;
   Ipp16s m_max, fac, tmp16, e_max, e_tmp, e_maxtmp, max ;
   Ipp32s Ltmp;

   lg4  = lg >>2;

   m_max = (Ipp16s)(SpPeak1k6(xri, &e_max, lg4)>>16);

   fac = 20480;
   for(i=0; i<lg4; i+=8) {
      max = m_max;
      e_maxtmp = e_max;

      Ltmp = 1;
      i8 = i + 8;
      for(j=i; j<i8; j++) {
         Ltmp = Add_32s(Ltmp, Mul2_Low_32s(xri[j]*xri[j]));
      }

      e_tmp = (Ipp16s)(30 - Norm_32s_I(&Ltmp));
      InvSqrt_32s16s_I(&Ltmp, &e_tmp);
      tmp16 = (Ipp16s)(Ltmp>>16);
      if(max > tmp16) {
         max = (Ipp16s)(max >> 1);
         e_maxtmp = (Ipp16s)(e_maxtmp + 1);
      }

      tmp16 = Div_16s(max, tmp16);
      e_tmp = (Ipp16s)(e_maxtmp - e_tmp);

      Ltmp = ((Ipp32s)tmp16)<<16;
      InvSqrt_32s16s_I(&Ltmp,&e_tmp);

      if (e_tmp > 4) {
         tmp16 = (Ipp16s)(ShiftL_32s(Ltmp, (Ipp16u)(e_tmp-4))>>16);
      } else {
         tmp16 = (Ipp16s)( (Ltmp>>(4-e_tmp)) >> 16);
      }

      if (tmp16 < fac) {
         fac = tmp16;
      }
      i8 = i + 8;
      for(j=i; j<i8; j++) {
         Ltmp = Mul2_Low_32s(xri[j]*fac);
         xri[j] = Cnvrt_NR_32s16s(ShiftL_32s(Ltmp, 4));
      }
   }

  return;
}

static void Deemph1k6(Ipp16s *xri, Ipp16s e_max, Ipp16s m_max, Ipp32s len)
{
  Ipp32s Ltmp;
  Ipp32s i, j, i8;
  Ipp16s exp_tmp, tmp16,fac;

   fac = 3277;
   for(i=0; i<len; i+=8) {
      Ltmp = 0;
      i8 = i + 8;
      for(j=i; j<i8; j++) {
         Ltmp = Add_32s(Ltmp, Mul2_Low_32s(xri[j]*xri[j]));
      }

      exp_tmp = (Ipp16s)(30 - Norm_32s_I(&Ltmp));
      InvSqrt_32s16s_I(&Ltmp, &exp_tmp);

      tmp16 = (Ipp16s)(Ltmp>>16);

      if(m_max > tmp16) {
         m_max = (Ipp16s)(m_max>>1);
         e_max = (Ipp16s)(e_max + 1);
      }
      tmp16 = Div_16s(m_max, tmp16);
      exp_tmp = (Ipp16s)(e_max - exp_tmp);

      if (exp_tmp > 0) {
         tmp16 = ShiftL_16s(tmp16, exp_tmp);
      } else {
         tmp16 = (Ipp16s)(tmp16 >> (-exp_tmp));
      }

      if (tmp16 > fac) {
         fac = tmp16;
      }

      for(j=i; j<i8; j++) {
         xri[j] = (Ipp16s)((xri[j]*fac)>>15);
      }
   }
   return;
}

void Adap_low_freq_deemph(Ipp16s* xri, Ipp32s len)
{
   Ipp16s exp_m, max;
   max = (Ipp16s)(SpPeak1k6(xri, &exp_m, (len>>2))>>16);
   Deemph1k6(xri, exp_m, max, (len>>2));
   return;
}


void Scale_tcx_ifft(Ipp16s* exc, Ipp32s lg, Ipp16s *Q_exc)
{
   Ipp16s tmp, rem_bit;
   Ipp32s i, Ltmp;

   Ltmp = 0;
   for(i=0; i<lg; i++) {
      Ltmp = Add_32s(Ltmp, Mul2_Low_32s(exc[i]*exc[i]));
   }
   tmp = 0;

   rem_bit = 4;
   if(lg == 1152) {
      rem_bit = 5;
   }

   if (Ltmp != 0) {
      tmp = Exp_32s(Ltmp);
      tmp = (Ipp16s)((tmp - rem_bit) >> 1);
      if (tmp < 0) {
         tmp = 0;
      }
   }

   if(tmp > 10) {
      tmp = 10;
   }

   *Q_exc = tmp;

   if (tmp!=0) {
      ownScaleSignal_AMRWB_16s_ISfs(exc, lg, tmp);
   }
   return;
}

static Ipp16s table_isqrt[49] =
{
    32767, 31790, 30894, 30070, 29309, 28602, 27945, 27330, 26755, 26214,
    25705, 25225, 24770, 24339, 23930, 23541, 23170, 22817, 22479, 22155,
    21845, 21548, 21263, 20988, 20724, 20470, 20225, 19988, 19760, 19539,
    19326, 19119, 18919, 18725, 18536, 18354, 18176, 18004, 17837, 17674,
    17515, 17361, 17211, 17064, 16921, 16782, 16646, 16514, 16384
};
void _Isqrt_n(
     Ipp32s* frac,
     Ipp16s* exp
)
{
    Ipp16s i, a, tmp;

    if (*frac <= (Ipp32s) 0)
    {
        *exp = 0;
        *frac = 0x7fffffffL;
        return;
    }

   if ((*exp & 1) == 1) {
      *frac = (*frac) >> 1;
   }

   *exp = Negate_16s((Ipp16s)((*exp - 1) >> 1));

   *frac = (*frac) >> 9;
   i = ExtractHigh(*frac);
   *frac = (*frac) >> 1;
   a = (Ipp16s)(*frac);
   a = (Ipp16s) (a & (Ipp16s) 0x7fff);

   i = (Ipp16s)(i - 16);
   *frac = (Ipp32s)table_isqrt[i] << 16;
   tmp = Sub_16s(table_isqrt[i], table_isqrt[i + 1]);
   *frac = Sub_32s(*frac, Mul2_Low_32s(tmp*a));

   return;
}
Ipp32s _Isqrt(Ipp32s L_x)
{
   Ipp16s exp;
   Ipp32s L_y;

   exp = (Ipp16s)(31 - Norm_32s_I(&L_x));
   _Isqrt_n(&L_x, &exp);

   if (exp > 0) {
      L_y = ShiftL_32s(L_x, exp);
   } else {
      L_y = L_x >> (-exp);
   }

    return (L_y);
}

/*Ipp16s Div_16s(Ipp16s var1, Ipp16s var2)
{
   if ((var1<var2) && (var1>0) && (var2>0)) {
      return (Ipp16s)( (((Ipp32s)var1)<<15) / var2);
   } else if ((var2!=0) && (var1==var2)) {
      return IPP_MAX_16S;
   }
   //else any other ((var1>var2), (var1<=0), (var2<=0))
   return 0;
}*/


void ownWindowing(const Ipp16s *pWindow, Ipp16s *pSrcDstVec, Ipp32s length)
{
   ippsMul_NR_16s_ISfs(pWindow, pSrcDstVec, length, 15);
}

void Fir_filt(Ipp16s* a, Ipp32s m, Ipp16s* x, Ipp16s* y, Ipp32s len)
{
   Ipp32s i, j, L_s;

   for (i=0; i<len; i++) {
      L_s = Mul2_Low_32s(x[i]*a[0]);
      for (j=1; j<m; j++) {
         L_s = Add_32s(L_s, Mul2_Low_32s(a[j]*x[i-j]));
      }
      y[i] = Cnvrt_NR_32s16s(ShiftL_32s(L_s,2));
   }
}

void Apply_tcx_overlap(Ipp16s* xnq_fx, Ipp16s* wovlp_fx, Ipp32s lext, Ipp32s L_frame)
{
   Ipp32s i, Ltmp;
   for (i=0; i<L_OVLP_2k; i++) {
      Ltmp = Mul2_Low_32s(xnq_fx[i]*32767);
      Ltmp = Add_32s(Ltmp, Mul2_Low_32s(wovlp_fx[i]*32767));
      xnq_fx[i] = Cnvrt_NR_32s16s(Ltmp);
   }
   for (i=0; i<lext; i++) {
      wovlp_fx[i] = xnq_fx[i+L_frame];
   }
   for (i=lext; i<L_OVLP_2k; i++) {
      wovlp_fx[i] = 0;
   }
   return;
}

