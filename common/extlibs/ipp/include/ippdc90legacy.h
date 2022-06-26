/* 
// Copyright 2015 Intel Corporation All Rights Reserved.
// 
// The source code, information and material ("Material") contained herein is
// owned by Intel Corporation or its suppliers or licensors, and title
// to such Material remains with Intel Corporation or its suppliers or
// licensors. The Material contains proprietary information of Intel
// or its suppliers and licensors. The Material is protected by worldwide
// copyright laws and treaty provisions. No part of the Material may be used,
// copied, reproduced, modified, published, uploaded, posted, transmitted,
// distributed or disclosed in any way without Intel's prior express written
// permission. No license under any patent, copyright or other intellectual
// property rights in the Material is granted to or conferred upon you,
// either expressly, by implication, inducement, estoppel or otherwise.
// Any license under such intellectual property rights must be express and
// approved by Intel in writing.
// 
// Unless otherwise agreed by Intel in writing,
// you may not remove or alter this notice or any other notice embedded in
// Materials by Intel or Intel's suppliers or licensors in any way.
// 
*/

/* /////////////////////////////////////////////////////////////////////////////
//
//         Intel(R) Integrated Performance Primitives
//             Data Compression Library (ippDC)
/                     Legacy Library
//
*/

#if !defined( __IPPDC_90_LEGACY_H__ )
#define __IPPDC_90_LEGACY_H__

#include "ippdefs90legacy.h"
#include "ippdc90legacy_redef.h"

#ifdef __cplusplus
extern "C" {
#endif


/*//////////////////////////////////////////////////////////////////////////////
//  Core functionality for legacy libraries
//////////////////////////////////////////////////////////////////////////////*/

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippInit
//  Purpose:    Automatic switching to best for current cpu library code using.
//  Returns:
//   ippStsNoErr
//
//  Parameter:  nothing
//
//  Notes:      At the moment of this function execution no any other IPP function
//              has to be working
*/
LEGACY90IPPAPI( IppStatus, legacy90ippdcInit, ( void ))


/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippSetNumThreads
//
//  Purpose:
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNoOperation        For static library internal threading is not supported
//    ippStsSizeErr            Desired number of threads less or equal zero
//
//  Arguments:
//    numThr                   Desired number of threads
*/
LEGACY90IPPAPI( IppStatus, legacy90ippdcSetNumThreads, ( int numThr ) )

/* ////////////////////////////////////////////////////////////////////////////
//  Name:       ippGetNumThreads
//
//  Purpose:
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         Pointer to numThr is Null
//    ippStsNoOperation        For static library internal threading is not supported
//                             and return value is always == 1
//
//  Arguments:
//    pNumThr                  Pointer to memory location where to store current numThr
*/
LEGACY90IPPAPI( IppStatus, legacy90ippdcGetNumThreads, (int* pNumThr) )

/*////////////////////////////////////////////////////////////////////////////*/


/* ///////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//                   Functions declarations
//////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////// */

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippdcGetLibVersion
//  Purpose:    getting of the library version
//  Returns:    the structure of information about version
//              of ippDC library
//  Parameters:
//
//  Notes:      not necessary to release the returned structure
*/
LEGACY90IPPAPI( const IppLibraryVersion*, legacy90ippdcGetLibVersion, (void) )

/* Run Length Encoding */
/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeRLE_8u
//  Purpose:            Performs the RLE encoding
//
//  Parameters:
//    pSrc              Pointer to the source vector
//    pSrcLen           Pointer to the length of source vector on input,
//                      pointer to the size of remainder on output
//    pDst              Pointer to the destination vector
//    pDstLen           Pointer to the size of destination buffer on input,
//                      pointer to the resulting length of the destination vector
//                      on output.
//  Return:
//    ippStsNullPtrErr          One or several pointer(s) is NULL
//    ippStsSizeErr             Lengths of the source/destination vector are less
//                              or equal zero
//    ippStsDstSizeLessExpected The size of destination vector less expected
//    ippStsNoErr               No errors
//
*/
LEGACY90IPPAPI(IppStatus, legacy90ippsEncodeRLE_8u, ( Ipp8u** ppSrc, int* pSrcLen,
                                      Ipp8u* pDst, int* pDstLen ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeRLE_8u
//  Purpose:            Performs the RLE decoding
//
//  Parameters:
//    pSrc              Pointer to the source vector
//    pSrcLen           Pointer to the length of source vector on input,
//                      pointer to the size of remainder on output
//    pDst              Pointer to the destination vector
//    pDstLen           Pointer to the size of destination buffer on input,
//                      pointer to the resulting length of the destination vector
//                      on output.
//  Return:
//    ippStsNullPtrErr          One or several pointer(s) is NULL
//    ippStsSizeErr             Lengths of the source/destination vector are less
//                              or equal zero
//    ippStsDstSizeLessExpected The size of destination vector less expected
//    ippStsSrcDataErr          The source vector contains unsupported data
//    ippStsNoErr               No errors
//
*/
LEGACY90IPPAPI(IppStatus, legacy90ippsDecodeRLE_8u, ( Ipp8u** ppSrc, int* pSrcLen,
                                      Ipp8u* pDst, int* pDstLen ))


/* /////////////////////////////////////////////////////////////////////////////
// Name:                ippsMTFInitAlloc_8u
// Purpose:             Allocates necessary memory and initializes structure for
//                      the MTF transform
//
// Parameters:
//    pMTFState         Pointer to the structure containing parameters for
//                       the MTF transform
//
// Return:
//    ippStsNullPtrErr  Pointer to structure is NULL
//    ippMemAllocErr    Can't allocate memory for pMTFState
//    ippStsNoErr       No errors
//
*/
LEGACY90IPPAPI(IppStatus, legacy90ippsMTFInitAlloc_8u, ( IppMTFState_8u** ppMTFState ))

/* /////////////////////////////////////////////////////////////////////////////
// Name:                ippsMTFFree_8u
// Purpose:             Frees allocated memory for MTF transform structure
//
// Parameters:
//    pMTFState         Pointer to the structure containing parameters for
//                      the MTF transform
//
*/
LEGACY90IPPAPI(void, legacy90ippsMTFFree_8u, ( IppMTFState_8u* pMTFState ))

/* /////////////////////////////////////////////////////////////////////////////
// Name:                ippsBWTGetSize_SmallBlock_8u
// Purpose:             Computes the size of necessary memory (in bytes) for
//                      additional buffer for the forward/inverse BWT transform
//
// Parameters:
//    wndSize           Window size for the BWT transform
//    pBWTBuffSize      Pointer to the computed size of buffer
//
// Return:
//    ippStsNullPtrErr  Pointer is NULL
//    ippStsSizeErr     wndSize less or equal 0 or more than 32768
//    ippStsNoErr       No errors
//
*/
LEGACY90IPPAPI(IppStatus, legacy90ippsBWTGetSize_SmallBlock_8u, ( int wndSize, int* pBuffSize ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsBWTFwd_SmallBlock_8u
//  Purpose:            Performs the forward BWT transform. This function is
//                      destined for processing of small blocks <= 32768
//
//  Parameters:
//    pSrc              Pointer to the source vector
//    pDst              Pointer to the destination vector
//    len               Length of source/destination vectors
//    index             Pointer to the index of first position for
//                      the inverse BWT transform
//    pBWTBuff          Pointer to the additional buffer
//
//  Return:
//    ippStsNullPtrErr  One or several pointer(s) is NULL
//    ippStsSizeErr     Length of vectors is less or equal 0 or more than 32768
//    ippStsNoErr       No errors
//
*/
LEGACY90IPPAPI(IppStatus, legacy90ippsBWTFwd_SmallBlock_8u, ( const Ipp8u* pSrc, Ipp8u* pDst,
                                              int len, int* index,
                                              Ipp8u* pBWTBuff ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsBWTInv_SmallBlock_8u
//  Purpose:            Performs the inverse BWT transform. This function is
//                      destined for processing of small blocks <= 32768
//
//  Parameters:
//    pSrc              Pointer to the source vector
//    pDst              Pointer to the destination vector
//    len               Length of source/destination vectors
//    index             Index of first position for the inverse BWT transform
//    pBWTBuff          Pointer to the additional buffer
//
//  Return:
//    ippStsNullPtrErr  One or several pointer(s) is NULL
//    ippStsSizeErr     Length of source/destination vectors is less or
//                      equal 0 or more than 32768 or index greater or equal srcLen
//    ippStsNoErr       No errors
//
*/
LEGACY90IPPAPI(IppStatus, legacy90ippsBWTInv_SmallBlock_8u, ( const Ipp8u* pSrc, Ipp8u* pDst,
                                              int len, int index,
                                              Ipp8u* pBWTBuff ))

/* /////////////////////////////////////////////////////////////////////////////
// Name:                ippsEncodeHuffInit_8u
// Purpose:             Initializes structure for Huffman encoding
//
// Parameters:
//    freqTable         Table of symbols' frequencies
//    pHuffState        Pointer to the structure containing parameters for
//                      Huffman coding
//
// Return:
//    ippStsNullPtrErr        One or several pointer(s) is NULL
//    ippStsFreqTableErr      Invalid freqTable
//    ippStsMaxLenHuffCodeErr Max length of Huffman code more expected
//    ippStsNoErr             No errors
//
*/
LEGACY90IPPAPI(IppStatus, legacy90ippsEncodeHuffInit_8u, ( const int freqTable[256],
                                           IppHuffState_8u* pHuffState ))

/* /////////////////////////////////////////////////////////////////////////////
// Name:                ippsHuffGetSize_8u
// Purpose:             Computes the size of necessary memory (in bytes) for
//                      structure of Huffman coding
//
// Parameters:
//    pHuffStateSize    Pointer to the computed size of structure
//
// Return:
//    ippStsNullPtrErr  Pointer is NULL
//    ippStsNoErr       No errors
//
*/
LEGACY90IPPAPI(IppStatus, legacy90ippsHuffGetSize_8u, ( int* pHuffStateSize ))

/* /////////////////////////////////////////////////////////////////////////////
// Name:                ippsEncodeHuffInitAlloc_8u
// Purpose:             Allocates necessary memory and initializes structure for
//                      Huffman encoding
//
// Parameters:
//    freqTable         Table of symbols' frequencies
//    pHuffState        Pointer to the structure containing parameters for
//                      Huffman coding
//
// Return:
//    ippStsNullPtrErr        One or several pointer(s) is NULL
//    ippMemAllocErr          Can't allocate memory for pHuffState
//    ippStsFreqTableErr      Invalid freqTable
//    ippStsMaxLenHuffCodeErr Max length of Huffman code more expected
//    ippStsNoErr             No errors
//
*/
LEGACY90IPPAPI(IppStatus, legacy90ippsEncodeHuffInitAlloc_8u, ( const int freqTable[256],
                                                IppHuffState_8u** ppHuffState ))

/* /////////////////////////////////////////////////////////////////////////////
// Name:                ippsHuffFree_8u
// Purpose:             Frees allocated memory for Huffman coding structure
//
// Parameters:
//    pHuffState        Pointer to the structure containing parameters for
//                      Huffman coding
//
*/
LEGACY90IPPAPI(void, legacy90ippsHuffFree_8u, ( IppHuffState_8u* pHuffState ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeHuffOne_8u
//  Purpose:            Performs Huffman encoding of the one source element
//
//  Parameters:
//    src               Source element
//    pDst              Pointer to the destination vector
//    dstOffsetBits     Offset in the destination vector, starting with high bit
//    pHuffState        Pointer to the structure containing parameters for
//                      Huffman coding
//  Return:
//    ippStsNullPtrErr      One or several pointer(s) is NULL
//    ippStsCodeLenTableErr Invalid codeLenTable
//    ippStsSizeErr         dstOffsetBits less than 0 or more than 7
//    ippStsNoErr           No errors
//
*/
LEGACY90IPPAPI(IppStatus, legacy90ippsEncodeHuffOne_8u, ( Ipp8u src, Ipp8u* pDst, int dstOffsetBits,
                                          IppHuffState_8u* pHuffState ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeHuff_8u
//  Purpose:            Performs Huffman encoding
//
//  Parameters:
//    pSrc              Pointer to the source vector
//    srcLen            Length of source vector
//    pDst              Pointer to the destination vector
//    pDstLen           Pointer to the resulting length of the destination vector
//                      on output.
//    pHuffState        Pointer to the structure containing parameters for
//                      Huffman coding
//  Return:
//    ippStsNullPtrErr  One or several pointer(s) is NULL
//    ippStsSizeErr     Length of the source vector is less or equal zero
//    ippStsNoErr       No errors
//
*/
LEGACY90IPPAPI(IppStatus, legacy90ippsEncodeHuff_8u, ( const Ipp8u* pSrc, int srcLen,
                                       Ipp8u* pDst, int* pDstLen,
                                       IppHuffState_8u* pHuffState ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeHuffFinal_8u
//  Purpose:            Flushes remainder after Huffman encoding
//
//  Parameters:
//    pDst              Pointer to the destination vector
//    pDstLen           Pointer to the resulting length of the destination vector
//                      on output.
//    pHuffState        Pointer to the structure containing parameters for
//                      Huffman coding
//  Return:
//    ippStsNullPtrErr  One or several pointer(s) is NULL
//    ippStsNoErr       No errors
//
*/
LEGACY90IPPAPI(IppStatus, legacy90ippsEncodeHuffFinal_8u, ( Ipp8u* pDst, int* pDstLen,
                                            IppHuffState_8u* pHuffState ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsHuffGetLenCodeTable_8u
//  Purpose:            Gives back the table with lengths of Huffman codes from
//                      pHuffState
//
//  Parameters:
//    codeLenTable      Destination table with lengths of Huffman codes
//    pHuffState        Pointer to the structure containing parameters for
//                      Huffman coding
//  Return:
//    ippStsNullPtrErr  One or several pointer(s) is NULL
//    ippStsNoErr       No errors
//
*/
LEGACY90IPPAPI(IppStatus, legacy90ippsHuffGetLenCodeTable_8u, ( int codeLenTable[256],
                                                IppHuffState_8u* pHuffState ))

/* /////////////////////////////////////////////////////////////////////////////
// Name:                ippsDecodeHuffInit_8u
// Purpose:             Initializes structure for Huffman decoding
//
// Parameters:
//    codeLenTable      Table with lengths of Huffman codes
//    pHuffState        Pointer to the structure containing parameters for
//                      Huffman coding
//
// Return:
//    ippStsNullPtrErr      One or several pointer(s) is NULL
//    ippStsCodeLenTableErr Invalid codeLenTable
//    ippStsNoErr           No errors
//
*/
LEGACY90IPPAPI(IppStatus, legacy90ippsDecodeHuffInit_8u, ( const int codeLenTable[256],
                                           IppHuffState_8u* pHuffState ))

/* /////////////////////////////////////////////////////////////////////////////
// Name:                ippsDecodeHuffInitAlloc_8u
// Purpose:             Allocates necessary memory and initializes structure for
//                      Huffman decoding
//
// Parameters:
//    codeLenTable      Table with lengths of Huffman codes
//    pHuffState        Pointer to the structure containing parameters for
//                      Huffman coding
//
// Return:
//    ippStsNullPtrErr      One or several pointer(s) is NULL
//    ippMemAllocErr        Can't allocate memory for pHuffState
//    ippStsCodeLenTableErr Invalid codeLenTable
//    ippStsNoErr           No errors
//
*/
LEGACY90IPPAPI(IppStatus, legacy90ippsDecodeHuffInitAlloc_8u, ( const int codeLenTable[256],
                                                IppHuffState_8u** ppHuffState ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeHuffOne_8u
//  Purpose:            Performs Huffman decoding of the one destination element
//
//  Parameters:
//    pSrc              Pointer to the source vector
//    srcOffsetBits     Offset in the source vector, starting with high bit
//    pDst              Pointer to the destination vector
//    pHuffState        Pointer to the structure containing parameters for
//                      Huffman coding
//  Return:
//    ippStsNullPtrErr  One or several pointer(s) is NULL
//    ippStsSizeErr     srcOffsetBits less than 0 or more than 7
//    ippStsNoErr       No errors
//
*/
LEGACY90IPPAPI(IppStatus, legacy90ippsDecodeHuffOne_8u, ( const Ipp8u* pSrc, int srcOffsetBits,
                                          Ipp8u* pDst, IppHuffState_8u* pHuffState ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeHuff_8u
//  Purpose:            Performs Huffman decoding
//
//  Parameters:
//    pSrc              Pointer to the source vector
//    srcLen            Length of source vector
//    pDst              Pointer to the destination vector
//    pDstLen           Pointer to the expected size of destination vector on input,
//                      pointer to the resulting length of the destination vector
//                      on output.
//    pHuffState        Pointer to the structure containing parameters for
//                      Huffman coding
//  Return:
//    ippStsNullPtrErr  One or several pointer(s) is NULL
//    ippStsSizeErr     Length of the source vector is less or equal zero
//    ippStsNoErr       No errors
//
*/
LEGACY90IPPAPI(IppStatus, legacy90ippsDecodeHuff_8u, ( const Ipp8u* pSrc, int srcLen,
                                       Ipp8u* pDst, int* pDstLen,
                                       IppHuffState_8u* pHuffState ))

/* /////////////////////////////////////////////////////////////////////////////
// Name:                ippsHuffGetDstBuffSize_8u
// Purpose:             Computes the size of necessary memory (in bytes) for
//                      the destination buffer (for Huffman encoding/decoding)
//
// Parameters:
//    codeLenTable      Table with lengths of Huffman codes
//    srcLen            Length of source vector
//    pEncDstBuffSize   Pointer to the computed size of the destination buffer
//                      for Huffman encoding (value returns if pointer isn't NULL)
//    pDecDstBuffSize   Pointer to the computed size of the destination buffer
//                      for Huffman decoding (value returns if pointer isn't NULL)
//
// Return:
//    ippStsNullPtrErr      Pointer to codeLenTable is NULL
//    ippStsCodeLenTableErr Invalid codeLenTable
//    ippStsNoErr           No errors
//
*/
LEGACY90IPPAPI(IppStatus, legacy90ippsHuffGetDstBuffSize_8u, ( const int codeLenTable[256], int srcLen,
                                               int* pEncDstBuffSize, int* pDecDstBuffSize ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsHuffLenCodeTablePack_8u
//  Purpose:            Packs the table with lengths of Huffman codes
//
//  Parameters:
//    codeLenTable      Table with lengths of Huffman codes
//    pDst              Pointer to the destination vector
//    pDstLen           Pointer to the size of destination buffer on input,
//                      pointer to the resulting length of the destination vector
//                      on output.
//
//  Return:
//    ippStsNullPtrErr      One or several pointer(s) is NULL
//    ippStsSizeErr         Length of the destination vector is less, equal zero or
//                          less expected
//    ippStsCodeLenTableErr Invalid codeLenTable
//    ippStsNoErr           No errors
//
*/
LEGACY90IPPAPI(IppStatus, legacy90ippsHuffLenCodeTablePack_8u,   ( const int codeLenTable[256],
                                                   Ipp8u* pDst, int* pDstLen ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsHuffLenCodeTableUnpack_8u
//  Purpose:            Unpacks the table with lengths of Huffman codes
//
//  Parameters:
//    pSrc              Pointer to the source vector
//    pSrcLen           Pointer to the length of source vector on input,
//                      pointer to the resulting length of the source vector
//    codeLenTable      Table with lengths of Huffman codes
//
//  Return:
//    ippStsNullPtrErr  One or several pointer(s) is NULL
//    ippStsSizeErr     Length of the source vector is less, equal zero or
//                      less expected
//    ippStsNoErr       No errors
//
*/
LEGACY90IPPAPI(IppStatus, legacy90ippsHuffLenCodeTableUnpack_8u, ( const Ipp8u* pSrc, int* pSrcLen,
                                                   int codeLenTable[256] ))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeGITGetSize_8u
//  Purpose:            Finds out size of GIT internal encoding state structure
//                      in bytes
//
//  Parameters:
//    maxSrcLen         Max length of source vector
//    maxDstLen         Max length of destination vector
//    pGITStateSize     Pointer to the size of GIT internal encoding state
//  Return:
//    ippStsNullPtrErr  Pointer to GITStateSize is NULL
//    ippStsSizeErr     Bad length arguments
//    ippStsNoErr       No errors
//
*/
LEGACY90IPPAPI(IppStatus, legacy90ippsEncodeGITGetSize_8u, ( int maxSrcLen, int maxDstLen,
                                             int* pGITStateSize))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeGITInit_8u
//  Purpose:            Initializes the GIT internal encoding state
//
//  Parameters:
//    maxSrcLen         Max length of source vector
//    maxDstLen         Max length of destination vector
//    pGITState         Pointer to memory allocated for GIT internal encoding
//                      state
//  Return:
//    ippStsNullPtrErr  One or several pointer(s) is NULL
//    ippStsSizeErr     Bad size arguments
//    ippStsNoErr       No errors
//
*/
LEGACY90IPPAPI(IppStatus, legacy90ippsEncodeGITInit_8u, (int maxSrcLen, int maxDstLen,
                                         IppGITState_8u* ppGITState))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeGITInitAlloc_8u
//  Purpose:            Allocates and Initializes the GIT internal encoding state
//
//  Parameters:
//    maxSrcLen         Max length of source vector
//    maxDstLen         Max length of destination vector
//    ppGITState        Pointer to pointer to GIT internal encoding state
//  Return:
//    ippStsSizeErr     Bad length arguments
//    ippStsNoErr       No errors
//
*/
LEGACY90IPPAPI(IppStatus, legacy90ippsEncodeGITInitAlloc_8u, (int maxSrcLen, int maxDstLen,
                                              IppGITState_8u** ppGITState))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeGIT_8u
//  Purpose:            Performs GIT encoding
//
//  Parameters:
//    pSrc              Pointer to the source vector
//    srcLen            Length of source vector
//    pDst              Pointer to the destination vector
//    pDstLen           Pointer to the length of destination vector
//    strategyHint      Strategy hint for lexicorgaphical reordering
//    pGITState         Pointer to GIT internal encoding state
//  Return:
//    ippStsNullPtrErr  One or several pointer(s) is NULL
//    ippStsSizeErr     Source vector is too long, more than the value of
//                      maxSrcLen parameter passed to ippsGITEncodeGetSize_8u
//                      or ippsGITEncodeInitAlloc_8u
//    ippStsNoErr       No errors
//
*/
LEGACY90IPPAPI(IppStatus, legacy90ippsEncodeGIT_8u, (const Ipp8u* pSrc, int srcLen, Ipp8u* pDst,
                                     int* pDstLen,
                                     IppGITStrategyHint strategyHint,
                                     IppGITState_8u* pGITState))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeGITGetSize_8u
//  Purpose:            Finds out size of GIT internal decoding state structure
//                      in bytes
//
//  Parameters:
//    maxSrcLen         Max length of source vector
//    pGITStateSize     Pointer to the size of GIT internal decoding state
//  Return:
//    ippStsNullPtrErr  Pointer to GITStateSize is NULL
//    ippStsSizeErr     Bad length arguments
//    ippStsNoErr       No errors
//
*/
LEGACY90IPPAPI(IppStatus, legacy90ippsDecodeGITGetSize_8u, (int maxSrcLen, int* pGITStateSize))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeGITInit_8u
//  Purpose:            Initializes the GIT internal decoding state
//
//  Parameters:
//    maxSrcLen         Max length of source vector
//    maxDstLen         Max length of destination vector
//    pGITState         Pointer to memory allocated for GIT internal decoding
//                      state
//  Return:
//    ippStsNullPtrErr  One or several pointer(s) is NULL
//    ippStsSizeErr     Bad size arguments
//    ippStsNoErr       No errors
//
*/
LEGACY90IPPAPI(IppStatus, legacy90ippsDecodeGITInit_8u, (int maxDstLen, IppGITState_8u* pGITState))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeGITInitAlloc_8u
//  Purpose:            Allocates and Initializes the GIT internal decoding state
//
//  Parameters:
//    maxSrcLen         Max length of source vector
//    maxDstLen         Max length of destination vector
//    ppGITState        Pointer to pointer to GIT internal decoding state
//  Return:
//    ippStsNullPtrErr  One or several pointer(s) is NULL
//    ippStsSizeErr     Bad length arguments
//    ippStsNoErr       No errors
//
*/
LEGACY90IPPAPI(IppStatus, legacy90ippsDecodeGITInitAlloc_8u, (int maxSrcLen, int maxDstLen,
                                              IppGITState_8u** ppGITState))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeGIT_8u
//  Purpose:            Performs GIT decoding
//
//  Parameters:
//    pSrc              Pointer to the source vector
//    srcLen            Length of source vector
//    pDst              Pointer to the destination vector
//    pDstLen           Pointer to the length of destination vector
//    strategyHint      Strategy hint for lexicorgaphical reordering
//    pGITState         Pointer to GIT internal decoding state
//  Return:
//    ippStsNullPtrErr  One or several pointer(s) is NULL
//    ippStsSizeErr     Not enough memory allocated for destination buffer
//    ippStsNoErr       No errors
//
*/
LEGACY90IPPAPI(IppStatus, legacy90ippsDecodeGIT_8u, (const Ipp8u* pSrc, int srcLen, Ipp8u* pDst, int* pDstLen,
                                     IppGITStrategyHint strategyHint,
                                     IppGITState_8u* pGITState))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsGITFree_8u
//  Purpose:            Frees the GIT internal decoding state
//
//  Parameters:
//    pGITState         Pointer to the GIT internal state
//
*/
LEGACY90IPPAPI(void, legacy90ippsGITFree_8u, (IppGITState_8u* pGITState))



/* rfc1950, 1951, 1952 - compatible functions */

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeLZ77GetSize
//  Purpose:            Computes the size of the internal encoding structure.
//
//  Parameters:
//   pLZ77VLCStateSize  Pointer to the size of the internal encoding structure.
//  Return:
//   ippStsNoErr        Indicates no error.
//   ippStsNullPtrErr   Indicates an error when the pointer pLZ77VLCStateSize is NULL.
//
*/
LEGACY90IPPAPI( IppStatus, legacy90ippsEncodeLZ77GetSize_8u, (int* pLZ77StateSize) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeLZ77Init
//  Purpose:            Initializes the internal encoding structure.
//
//  Parameters:
//   comprLevel         Compression level.
//   checksum           Algorithm to compute the checksum for input data.
//   pLZ77State         Pointer to memory allocated for the internal encoding structure.
//  Return:
//   ippStsNoErr        Indicates no error.
//   ippStsNullPtrErr   Indicates an error when the pLZ77State pointer is NULL.
//   ippStsBadArgErr    Indicates an error when the checksum or comprLevel parameter
//                      has an illegal value.
//
*/
LEGACY90IPPAPI( IppStatus, legacy90ippsEncodeLZ77Init_8u, (IppLZ77ComprLevel comprLevel,
                                         IppLZ77Chcksm checksum, IppLZ77State_8u* pLZ77State) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeLZ77InitAlloc
//  Purpose:            Allocates memory and initializes the internal encoding structure.
//
//  Parameters:
//   comprLevel         Compression level.
//   checksum           Algorithm to compute the checksum for input data.
//   ppLZ77State        Double pointer to the internal encoding structure.
//  Return:
//   ippStsNoErr        Indicates no error.
//   ippStsNullPtrErr   Indicates an error when the ppLZ77State pointer is NULL.
//   ippStsBadArgErr    Indicates an error when the checksum or comprLevel parameter
//                      has an illegal value.
//   ippStsMemAlloc     Indicates an error when memory allocation fails.
//
*/
LEGACY90IPPAPI( IppStatus, legacy90ippsEncodeLZ77InitAlloc_8u, (IppLZ77ComprLevel comprLevel,
                                              IppLZ77Chcksm checksum, IppLZ77State_8u** ppLZ77State) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeLZ77
//  Purpose:            Performs LZ77 encoding.
//
//  Parameters:
//   ppSrc              Double pointer to the source vector.
//   pSrcLen            Pointer to the length of the source vector.
//   ppDst              Double pointer to the destination vector.
//   pDstLen            Pointer to the length of the destination vector.
//   flush              Data-block encoding mode.
//   pLZ77State         Pointer to the internal encoding structure.
//  Return:
//   ippStsNoErr                Indicates no error.
//   ippStsNullPtrErr           Indicates an error when one of the specified pointers is NULL.
//   ippStsSizeErr              Indicates an error when DstLen is less than or equal to zero.
//   ippStsDstSizeLessExpected  Indicates a warning when the destination buffer is full.
//
//  Note: This function searches for substring matches using the LZ77 algorithm.
//        The technique of sliding window support is compatible with rfc1951.
//
*/
LEGACY90IPPAPI( IppStatus, legacy90ippsEncodeLZ77_8u, (Ipp8u** ppSrc, int* pSrcLen, IppLZ77Pair** ppDst,
                                     int* pDstLen, IppLZ77Flush flush, IppLZ77State_8u* pLZ77State) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeLZ77SelectHuffMode
//  Purpose:            Takes the best decision about the optimal coding strategy
//                      (use fixed Huffman coding or dynamic Huffman coding).
//
//  Parameters:
//   pSrc               Pointer to the source vector.
//   srcLen             Length of the source vector.
//   pHuffMode          Pointer to the value of coding strategy.
//   pLZ77State         Pointer to memory allocated for the internal encoding structure.
//  Return:
//   ippStsNoErr        Indicates no error.
//   ippStsNullPtrErr   Indicates an error when the pLZ77State pointer is NULL.
//
*/
LEGACY90IPPAPI( IppStatus, legacy90ippsEncodeLZ77SelectHuffMode_8u, (IppLZ77Pair* pSrc, int srcLen,
                                                    IppLZ77HuffMode* pHuffMode,
                                                    IppLZ77State_8u* pLZ77State) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeLZ77FixedHuff
//  Purpose:            Performs fixed Huffman coding of the LZ77 output.
//
//  Parameters:
//   ppSrc              Double pointer to the source vector.
//   pSrcLen            Pointer to the length of the source vector.
//   ppDst              Double pointer to the destination vector.
//   pDstLen            Pointer to the length of the destination vector.
//   flush              Data-block encoding mode.
//   pLZ77State         Pointer to the internal encoding structure.
//  Return:
//   ippStsNoErr                Indicates no error.
//   ippStsNullPtrErr           Indicates an error when one of the specified pointers is NULL.
//   ippStsSizeErr              Indicates an error when DstLen is less than or equal to zero.
//   ippStsDstSizeLessExpected  Indicates a warning when the destination buffer is full.
//   ippStsStreamEnd            Indicates a warning when the stream ends. This warning can
//                              be returned only when the flush value is FINISH.
//
//  Note: This function produces the rfc1951 compatible code for the LZ77 output.
//
*/
LEGACY90IPPAPI( IppStatus, legacy90ippsEncodeLZ77FixedHuff_8u, (IppLZ77Pair** ppSrc, int* pSrcLen, Ipp8u** ppDst,
                                     int* pDstLen, IppLZ77Flush flush, IppLZ77State_8u* pLZ77State) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeLZ77DynamicHuff
//  Purpose:            Performs dynamic Huffman coding of the LZ77 output.
//
//  Parameters:
//   ppSrc              Double pointer to the source vector.
//   pSrcLen            Pointer to the length of the source vector.
//   ppDst              Double pointer to the destination vector.
//   pDstLen            Pointer to the length of the destination vector.
//   flush              Data-block encoding mode.
//   pLZ77State         Pointer to the internal encoding structure.
//  Return:
//   ippStsNoErr                Indicates no error.
//   ippStsNullPtrErr           Indicates an error when one of the specified pointers is NULL.
//   ippStsSizeErr              Indicates an error when DstLen is less than or equal to zero.
//   ippStsDstSizeLessExpected  Indicates a warning when the destination buffer is full.
//   ippStsStreamEnd            Indicates a warning when the stream ends. This warning can
//                              be returned only when the flush value is FINISH.
//
//  Note: This function produces the rfc1951 compatible code for the LZ77 output.
//
*/
LEGACY90IPPAPI( IppStatus, legacy90ippsEncodeLZ77DynamicHuff_8u, (IppLZ77Pair** ppSrc, int* pSrcLen, Ipp8u** ppDst,
                                     int* pDstLen, IppLZ77Flush flush, IppLZ77State_8u* pLZ77State) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeLZ77StoredBlock
//  Purpose:            Transmits the block without compression.
//
//  Parameters:
//   ppSrc              Double pointer to the source vector.
//   pSrcLen            Pointer to the length of the source vector.
//   ppDst              Double pointer to the destination vector.
//   pDstLen            Pointer to the length of the destination vector.
//   flush              Data-block encoding mode.
//   pLZ77State         Pointer to the internal decoding structure.
//  Return:
//   ippStsNoErr                Indicates no error.
//   ippStsNullPtrErr           Indicates an error when one of the specified pointers is NULL.
//   ippStsSizeErr              Indicates an error when DstLen is less than or equal to zero.
//   ippStsDstSizeLessExpected  Indicates a warning when the destination buffer is full.
//
*/
LEGACY90IPPAPI( IppStatus, legacy90ippsEncodeLZ77StoredBlock_8u, (Ipp8u** ppSrc, int* pSrcLen, Ipp8u** ppDst,
                                     int* pDstLen, IppLZ77Flush flush, IppLZ77State_8u* pLZ77State) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeLZ77Flush
//  Purpose:            Performs writing the service information (accumulated
//                      checksum and total length of input data stream) in order
//                      to achieve the ZLIB/GZIP data format compatibility.
//
//  Parameters:
//   ppDst              Double pointer to the destination vector.
//   pDstLen            Pointer to the length of the destination vector.
//   pLZ77State         Pointer to the internal encoding structure.
//  Return:
//   ippStsNoErr                Indicates no error.
//   ippStsNullPtrErr           Indicates an error when one of the specified pointers is NULL.
//   ippStsSizeErr              Indicates an error when DstLen is less than or equal to zero.
//   ippStsDstSizeLessExpected  Indicates a warning when the destination buffer is full.
//
//  Note: This is a service function which is necessary for achieving compatibility with
//        the rfc1950, rfc1951, rfc1952 describing ZLIB/GZIP data format.
//
*/
LEGACY90IPPAPI( IppStatus, legacy90ippsEncodeLZ77Flush_8u, (Ipp8u** ppDst, int* pDstLen,
                                                 IppLZ77State_8u* pLZ77State) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeLZ77GetPairs
//  Purpose:            Reads the pointer to the pair buffer, it's length and current index
//                      from the internal state structure for encoding.
//
//  Parameters:
//   ppPairs            Double pointer to a variable of ippLZ77Pair type.
//   pPairsInd          Pointer to the current index in the pair buffer
//   pPairsLen          Pointer to the length of pair buffer
//   pLZ77State         Pointer to the internal encoding structure.
//  Return:
//   ippStsNoErr        Indicates no error.
//   ippStsNullPtrErr   Indicates an error when the pLZ77State or ppPairs pointer is NULL.
//
*/
LEGACY90IPPAPI( IppStatus, legacy90ippsEncodeLZ77GetPairs_8u, (IppLZ77Pair** ppPairs, int* pPairsInd,
                                              int* pPairsLen, IppLZ77State_8u* pLZ77State) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeLZ77SetPairs
//  Purpose:            Writes the pointer to the pair buffer, it's length and current index
//                      to the internal state structure for encoding.
//
//  Parameters:
//   pPairs             Pointer to a variable of ippLZ77Pair type.
//   pairsInd           Current index in the pair buffer
//   pairsLen           Length of pair buffer
//   pLZ77State         Pointer to the internal encoding structure.
//  Return:
//   ippStsNoErr        Indicates no error.
//   ippStsNullPtrErr   Indicates an error when the pLZ77State or pPairs pointer is NULL.
//
*/
LEGACY90IPPAPI( IppStatus, legacy90ippsEncodeLZ77SetPairs_8u, (IppLZ77Pair* pPairs, int pairsInd,
                                             int pairsLen, IppLZ77State_8u* pLZ77State) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeLZ77GetStatus
//  Purpose:            Reads the encoding status value from the internal state
//                      structure for encoding.
//
//  Parameters:
//   pDeflateStatus     Pointer to a variable of ippLZ77DeflateStatus type.
//   pLZ77State         Pointer to the internal encoding structure.
//  Return:
//   ippStsNoErr        Indicates no error.
//   ippStsNullPtrErr   Indicates an error when the pLZ77State or pDeflateStatus pointer is NULL.
//
*/
LEGACY90IPPAPI( IppStatus, legacy90ippsEncodeLZ77GetStatus_8u, (IppLZ77DeflateStatus* pDeflateStatus,
                                              IppLZ77State_8u* pLZ77State) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeLZ77SetStatus
//  Purpose:            Writes the encoding status value to the internal state
//                      structure for encoding.
//
//  Parameters:
//   deflateStatus      Variable of ippLZ77DeflateStatus type.
//   pLZ77State         Pointer to the internal encoding structure.
//  Return:
//   ippStsNoErr        Indicates no error.
//   ippStsNullPtrErr   Indicates an error when the pLZ77State pointer is NULL.
//   ippStsBadArgErr    Indicates an error when the deflateStatus parameter has an illegal value.
//
*/
LEGACY90IPPAPI( IppStatus, legacy90ippsEncodeLZ77SetStatus_8u, (IppLZ77DeflateStatus deflateStatus,
                                              IppLZ77State_8u* pLZ77State) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeLZ77Reset
//  Purpose:            Resets the internal state structure for encoding.
//
//  Parameters:
//   pLZ77State         Pointer to the internal encoding structure.
//  Return:
//   ippStsNoErr        Indicates no error.
//   ippStsNullPtrErr   Indicates an error when the pLZ77State pointer is NULL.
//
*/
LEGACY90IPPAPI( IppStatus, legacy90ippsEncodeLZ77Reset_8u, (IppLZ77State_8u* pLZ77State) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeLZ77GetSize
//  Purpose:            Computes the size of the internal decoding structure.
//
//  Parameters:
//   pLZ77StateSize     Pointer to the size of the internal decoding structure.
//  Return:
//   ippStsNoErr        Indicates no error.
//   ippStsNullPtrErr   Indicates an error when the pointer pLZ77StateSize is NULL.
//
*/
LEGACY90IPPAPI( IppStatus, legacy90ippsDecodeLZ77GetSize_8u, (int* pLZ77StateSize) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeLZ77Init
//  Purpose:            Initializes the internal decoding structure.
//
//  Parameters:
//   checksum           Algorithm to compute the checksum for output data.
//   pLZ77State         Pointer to memory allocated for the internal decoding structure.
//  Return:
//   ippStsNoErr        Indicates no error.
//   ippStsNullPtrErr   Indicates an error when the pLZ77State pointer is NULL.
//   ippStsBadArgErr    Indicates an error when the checksum parameter
//                      has an illegal value.
//
*/
LEGACY90IPPAPI( IppStatus, legacy90ippsDecodeLZ77Init_8u, (IppLZ77Chcksm checksum, IppLZ77State_8u* pLZ77State) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeLZ77InitAlloc
//  Purpose:            Allocates memory and initializes the internal encoding structure.
//
//  Parameters:
//   checksum           Algorithm to compute the checksum for output data.
//   ppLZ77State        Double pointer to the internal decoding structure.
//  Return:
//   ippStsNoErr        Indicates no error.
//   ippStsNullPtrErr   Indicates an error when the ppLZ77State pointer is NULL.
//   ippStsBadArgErr    Indicates an error when the checksum parameter has an illegal value.
//   ippStsMemAlloc     Indicates an error when memory allocation fails.
//
*/
LEGACY90IPPAPI( IppStatus, legacy90ippsDecodeLZ77InitAlloc_8u, (IppLZ77Chcksm checksum, IppLZ77State_8u** ppLZ77State) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeLZ77
//  Purpose:            Performs LZ77 decoding.
//
//  Parameters:
//   ppSrc              Double pointer to the source vector.
//   pSrcLen            Pointer to the length of the source vector.
//   ppDst              Double pointer to the destination vector.
//   pDstLen            Pointer to the length of the destination vector.
//   flush              Data-block encoding mode.
//   pLZ77State         Pointer to the internal decoding structure.
//  Return:
//   ippStsNoErr                Indicates no error.
//   ippStsNullPtrErr           Indicates an error when one of the specified pointers is NULL.
//   ippStsSizeErr              Indicates an error when DstLen is less than or equal to zero.
//   ippStsDstSizeLessExpected  Indicates a warning when the destination buffer is full.
//   ippStsStreamEnd            Indicates a warning when the stream ends.
//
//  Note: The technique of LZ77 sliding window support is compatible with rfc1951.
//
*/
LEGACY90IPPAPI( IppStatus, legacy90ippsDecodeLZ77_8u, (IppLZ77Pair** ppSrc, int* pSrcLen, Ipp8u** ppDst,
                                     int* pDstLen, IppLZ77Flush flush, IppLZ77State_8u* pLZ77State) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeLZ77GetBlockType
//  Purpose:            Decodes the type of the block from the DEFLATE format.
//
//  Parameters:
//   ppSrc              Double pointer to the source vector.
//   pSrcLen            Pointer to the length of the source vector.
//   pHuffMode          Pointer to the value of coding mode.
//   pLZ77State         Pointer to the internal decoding structure.
//  Return:
//   ippStsNoErr                Indicates no error.
//   ippStsNullPtrErr           Indicates an error when one of the specified pointers is NULL.
//   ippStsSizeErr              Indicates an error when SrcLen is less than zero.
//   ippStsSrcSizeLessExpected  Indicates a warning when the source buffer is less than expected.
//                              (Internal bit stream and source vector do not contain enough bits to decode
//                              the type of the block)
//
*/
LEGACY90IPPAPI( IppStatus, legacy90ippsDecodeLZ77GetBlockType_8u, (Ipp8u** ppSrc, int* pSrcLen,
                                                  IppLZ77HuffMode* pHuffMode,
                                                  IppLZ77State_8u* pLZ77State) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeLZ77FixedHuff
//  Purpose:            Performs fixed Huffman decoding of the rfc1951 compatible code.
//
//  Parameters:
//   ppSrc              Double pointer to the source vector.
//   pSrcLen            Pointer to the length of the source vector.
//   ppDst              Double pointer to the destination vector.
//   pDstLen            Pointer to the length of the destination vector.
//   flush              Data-block encoding mode.
//   pLZ77State         Pointer to the internal decoding structure.
//  Return:
//   ippStsNoErr                Indicates no error.
//   ippStsNullPtrErr           Indicates an error when one of the specified pointers is NULL.
//   ippStsSizeErr              Indicates an error when DstLen is less than or equal to zero.
//   ippStsSrcSizeLessExpected  Indicates a warning when the source buffer is less than expected
//                              (end of block marker is not decoded).
//   ippStsDstSizeLessExpected  Indicates a warning when the destination buffer is full.
//   ippStsStreamEnd            Indicates a warning when the stream ends.
//
//  Note: This function decodes the rfc1951 compatible code.
//
*/
LEGACY90IPPAPI( IppStatus, legacy90ippsDecodeLZ77FixedHuff_8u, (Ipp8u** ppSrc, int* pSrcLen, IppLZ77Pair** ppDst,
                                     int* pDstLen, IppLZ77Flush flush, IppLZ77State_8u* pLZ77State) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeLZ77DynamicHuff
//  Purpose:            Performs dynamic Huffman decoding of the rfc1951 compatible code.
//
//  Parameters:
//   ppSrc              Double pointer to the source vector.
//   pSrcLen            Pointer to the length of the source vector.
//   ppDst              Double pointer to the destination vector.
//   pDstLen            Pointer to the length of the destination vector.
//   flush              Data-block encoding mode.
//   pLZ77State         Pointer to the internal decoding structure.
//  Return:
//   ippStsNoErr                Indicates no error.
//   ippStsNullPtrErr           Indicates an error when one of the specified pointers is NULL.
//   ippStsWrongBlockType       Indicates a warning when the type of the block is not dynamic Huffman type.
//   ippStsSizeErr              Indicates an error when DstLen is less than or equal to zero.
//   ippStsSrcSizeLessExpected  Indicates a warning when the source buffer is less than expected
//                              (end of block marker is not decoded).
//   ippStsDstSizeLessExpected  Indicates a warning when the destination buffer is full.
//   ippStsStreamEnd            Indicates a warning when the stream ends.
//
//  Note: This function decodes the rfc1951 compatible code.
//
*/
LEGACY90IPPAPI( IppStatus, legacy90ippsDecodeLZ77DynamicHuff_8u, (Ipp8u** ppSrc, int* pSrcLen, IppLZ77Pair** ppDst,
                                     int* pDstLen, IppLZ77Flush flush, IppLZ77State_8u* pLZ77State) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeLZ77StoredBlock
//  Purpose:            Performs decoding of the block transmitted without compression.
//
//  Parameters:
//   ppSrc              Double pointer to the source vector.
//   pSrcLen            Pointer to the length of the source vector.
//   ppDst              Double pointer to the destination vector.
//   pDstLen            Pointer to the length of the destination vector.
//   pLZ77State         Pointer to the internal decoding structure.
//  Return:
//   ippStsNoErr                Indicates no error.
//   ippStsNullPtrErr           Indicates an error when one of the specified pointers is NULL.
//   ippStsWrongBlockType       Indicates a warning when the type of the block is not of
//                              the "stored without compression type" type.
//   ippStsSizeErr              Indicates an error when DstLen is less than or equal to zero.
//   ippStsSrcSizeLessExpected  Indicates a warning when the source buffer is less than expected
//                              (end of block marker is not decoded).
//   ippStsDstSizeLessExpected  Indicates a warning when the destination buffer is full.
//
//
*/
LEGACY90IPPAPI( IppStatus, legacy90ippsDecodeLZ77StoredBlock_8u, (Ipp8u** ppSrc, int* pSrcLen, Ipp8u** ppDst,
                                     int* pDstLen, IppLZ77State_8u* pLZ77State) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeLZ77StoredHuff_8u
//  Purpose:            Performs copying the data to the output buffer of pairs
//
//  Parameters:
//    ppSrc             Double pointer to the source vector
//    pSrcLen           Pointer to the length of source vector
//    ppDst             Double pointer to the destination vector of pairs
//    pDstLen           Pointer to the length of destination vector of pairs
//    pLZ77State        Pointer to the internal state
//
//  Return:
//   ippStsNoErr                Indicates no error
//   ippStsNullPtrErr           Indicates an error when one of the specified pointers is NULL
//   ippStsWrongBlockType       Indicates a warning when the type of the block is not of
//                              the "stored without compression type" type
//   ippStsSizeErr              Indicates an error when DstLen is less than or equal to zero
//   ippStsSrcSizeLessExpected  Indicates a warning when the source buffer is less than expected
//   ippStsDstSizeLessExpected  Indicates a warning when the destination buffer is full
//
*/

LEGACY90IPPAPI( IppStatus, legacy90ippsDecodeLZ77StoredHuff_8u, (Ipp8u** ppSrc, int* pSrcLen, IppLZ77Pair** ppDst,
                                     int* pDstLen, IppLZ77State_8u* pLZ77State) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeLZ77GetPairs
//  Purpose:            Reads the pointer to the pair buffer, it's length and current index
//                      from the internal state structure for decoding.
//
//  Parameters:
//   ppPairs            Double pointer to a variable of ippLZ77Pair type.
//   pPairsInd          Pointer to the current index in the pair buffer
//   pPairsLen          Pointer to the length of pair buffer
//   pLZ77State         Pointer to the internal decoding structure.
//  Return:
//   ippStsNoErr        Indicates no error.
//   ippStsNullPtrErr   Indicates an error when the pLZ77State or ppPairs pointer is NULL.
//
*/
LEGACY90IPPAPI( IppStatus, legacy90ippsDecodeLZ77GetPairs_8u, (IppLZ77Pair** ppPairs, int* pPairsInd,
                                              int* pPairsLen, IppLZ77State_8u* pLZ77State) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeLZ77SetPairs
//  Purpose:            Writes the pointer to the pair buffer, it's length and current index
//                      to the internal state structure for decoding.
//
//  Parameters:
//   pPairs             Pointer to a variable of ippLZ77Pair type.
//   pairsInd           Current index in the pair buffer
//   pairsLen           Length of pair buffer
//   pLZ77State         Pointer to the internal decoding structure.
//  Return:
//   ippStsNoErr        Indicates no error.
//   ippStsNullPtrErr   Indicates an error when the pLZ77State or pPairs pointer is NULL.
//
*/
LEGACY90IPPAPI( IppStatus, legacy90ippsDecodeLZ77SetPairs_8u, (IppLZ77Pair* pPairs, int pairsInd,
                                             int pairsLen, IppLZ77State_8u* pLZ77State) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeLZ77GetStatus
//  Purpose:            Reads the decoding status value from the internal state
//                      structure for decoding.
//
//  Parameters:
//   pInflateStatus     Pointer to a variable of ippLZ77InflateStatus type.
//   pLZ77State         Pointer to the internal structure.
//  Return:
//   ippStsNoErr        Indicates no error.
//   ippStsNullPtrErr   Indicates an error when the pLZ77State or pInflateStatus pointer is NULL.
//
*/
LEGACY90IPPAPI( IppStatus, legacy90ippsDecodeLZ77GetStatus_8u, ( IppLZ77InflateStatus* pInflateStatus,
                                               IppLZ77State_8u* pLZ77State ) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeLZ77SetStatus
//  Purpose:            Writes the decoding status value to the internal state
//                      structure for decoding.
//
//  Parameters:
//   inflateStatus      Variable of ippLZ77InflateStatus type.
//   pLZ77State         Pointer to the internal structure.
//  Return:
//   ippStsNoErr        Indicates no error.
//   ippStsNullPtrErr   Indicates an error when the pLZ77State pointer is NULL.
//   ippStsBadArgErr    Indicates an error when the inflateStatus parameter has an illegal value.
//
*/
LEGACY90IPPAPI( IppStatus, legacy90ippsDecodeLZ77SetStatus_8u, ( IppLZ77InflateStatus inflateStatus,
                                               IppLZ77State_8u* pLZ77State ) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsEncodeLZ77SetDictionary_8u, ippsDecodeLZ77SetDictionary_8u
//  Purpose:            Presets the dictionary for encoding/decoding.
//
//  Parameters:
//    pDictionary       Pointer to the dictionary vector
//    dictLen           Length of dictionary vector
//    pLZ77State        Pointer to the internal state
//
//  Return:
//    ippStsNullPtrErr          One or several pointer(s) is NULL
//    ippStsSizeErr             Lengths of the source/destination vector are less
//                              or equal zero
//    ippStsNoErr               No errors
//
*/

LEGACY90IPPAPI(IppStatus, legacy90ippsEncodeLZ77SetDictionary_8u, ( Ipp8u* pDictionary, int dictLen, IppLZ77State_8u* pLZ77State ))
LEGACY90IPPAPI(IppStatus, legacy90ippsDecodeLZ77SetDictionary_8u, ( Ipp8u* pDictionary, int dictLen, IppLZ77State_8u* pLZ77State ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeLZ77Reset
//  Purpose:            Resets the internal state structure for decoding.
//
//  Parameters:
//   pLZ77State         Pointer to the internal decoding structure.
//  Return:
//   ippStsNoErr        Indicates no error.
//   ippStsNullPtrErr   Indicates an error when the pLZ77State pointer is NULL.
//
*/
LEGACY90IPPAPI( IppStatus, legacy90ippsDecodeLZ77Reset_8u, (IppLZ77State_8u* pLZ77State) )

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeLZ77FixedHuffFull_8u
//  Purpose:            Performs the decoding of fixed huffman rfc1951 compatible format
//
//  Parameters:
//    ppSrc             Double pointer to the source vector
//    pSrcLen           Pointer to the length of source vector
//    ppDst             Double pointer to the destination vector
//    pDstLen           Pointer to the size of destination buffer on input,
//                      pointer to the resulting length of the destination vector
//                      on output
//    flush             Flush mode
//    pLZ77State        Pointer to internal decoding state
//
//  Return:
//    ippStsNullPtrErr          One or several pointer(s) is NULL
//    ippStsSizeErr             Lengths of the source/destination vector are less
//                              or equal zero
//    ippStsSrcSizeLessExpected The end of block symbol not decoded, so size of source vector less expected
//    ippStsDstSizeLessExpected The size of destination vector less expected
//    ippStsStreamEnd           The end of stream symbol decoded
//    ippStsNoErr               No errors
//
*/
LEGACY90IPPAPI(IppStatus, legacy90ippsDecodeLZ77FixedHuffFull_8u, (Ipp8u** ppSrc, int* pSrcLen, Ipp8u** ppDst, int* pDstLen, IppLZ77Flush flush, IppLZ77State_8u* pLZ77State))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeLZ77DynamicHuffFull_8u
//  Purpose:            Performs the decoding of dynamic huffman rfc1951 compatible format
//
//  Parameters:
//    ppSrc             Double pointer to the source vector
//    pSrcLen           Pointer to the length of source vector
//    ppDst             Double pointer to the destination vector
//    pDstLen           Pointer to the size of destination buffer on input,
//                      pointer to the resulting length of the destination vector
//                      on output
//    flush             Flush mode
//    pLZ77State        Pointer to internal decoding state
//
//  Return:
//    ippStsNullPtrErr          One or several pointer(s) is NULL
//    ippStsSizeErr             Lengths of the source/destination vector are less
//                              or equal zero
//    ippStsSrcSizeLessExpected The end of block symbol not decoded, so size of source vector less expected
//    ippStsDstSizeLessExpected The size of destination vector less expected
//    ippStsStreamEnd           The end of stream symbol decoded
//    ippStsNoErr               No errors
//
*/
LEGACY90IPPAPI(IppStatus, legacy90ippsDecodeLZ77DynamicHuffFull_8u, (Ipp8u** ppSrc, int* pSrcLen, Ipp8u** ppDst, int* pDstLen, IppLZ77Flush flush, IppLZ77State_8u* pLZ77State))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeLZ77CopyState_8u
//  Purpose:            Performs copying the fields of internal state structure
//
//  Parameters:
//    pLZ77StateSrc        Pointer to the internal state for copying from
//    pLZ77StateDst        Pointer to the internal state for copying to
//
//  Return:
//    ippStsNullPtrErr          One or several pointer(s) is NULL
//    ippStsNoErr               No errors
//
*/

LEGACY90IPPAPI(IppStatus, legacy90ippsDecodeLZ77CopyState_8u,   ( IppLZ77State_8u* pLZ77StateSrc, IppLZ77State_8u* pLZ77StateDst ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsLZ77Free
//  Purpose:            Frees the internal state structure for encoding or decoding.
//
//  Parameters:
//   pLZ77State         Pointer to the internal decoding structure.
//
*/
LEGACY90IPPAPI( void, legacy90ippsLZ77Free_8u, (IppLZ77State_8u* pLZ77State) )


/* /////////////////////////////////////////////////////////////////////////////
//  Name:               ippsDecodeRLE_BZ2_8u
//  Purpose:            Performs the RLE decoding with thresholding = 4.
//                      Specific function for bzip2 compatibility.
//
//  Parameters:
//    ppSrc             Double pointer to the source vector
//    pSrcLen           Pointer to the length of source vector on input,
//                      pointer to the size of remainder on output
//    pDst              Pointer to the destination vector
//    pDstLen           Pointer to the size of destination buffer on input,
//                      pointer to the resulting length of the destination vector
//                      on output.
//  Return:
//    ippStsNullPtrErr          One or several pointer(s) is NULL
//    ippStsSizeErr             Lengths of the source/destination vector are less
//                              or equal zero
//    ippStsDstSizeLessExpected The size of destination vector less expected
//    ippStsNoErr               No errors
//
*/

LEGACY90IPPAPI(IppStatus, legacy90ippsDecodeRLE_BZ2_8u,    (Ipp8u** ppSrc, int* pSrcLen, Ipp8u* pDst, int* pDstLen ))

/*******************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* __IPPDC_90_LEGACY_H__ */
