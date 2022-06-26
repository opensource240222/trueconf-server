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
//          Intel(R) Integrated Performance Primitives
//             String Manipulations Library (ippCH)
//                        Legacy Library
//
*/

#if !defined( __IPPCH_90_LEGACY_H__ )
#define __IPPCH_90_LEGACY_H__

#include "ippdefs90legacy.h"
#include "ippch90legacy_redef.h"

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
LEGACY90IPPAPI( IppStatus, legacy90ippchInit, ( void ))


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
LEGACY90IPPAPI( IppStatus, legacy90ippchSetNumThreads, ( int numThr ) )

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
LEGACY90IPPAPI( IppStatus, legacy90ippchGetNumThreads, (int* pNumThr) )

/*////////////////////////////////////////////////////////////////////////////*/


/* /////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//                   Functions declarations
////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////// */


/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippchGetLibVersion
//  Purpose:    getting of the library version
//  Returns:    the structure of information about version
//              of ippCH library
//  Parameters:
//
//  Notes:      not necessary to release the returned structure
*/
LEGACY90IPPAPI( const IppLibraryVersion*, legacy90ippchGetLibVersion, (void) )

/* /////////////////////////////////////////////////////////////////////////////
//                String Functions
///////////////////////////////////////////////////////////////////////////// */

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsFind_8u        ippsFind_16u
//              ippsFindC_8u       ippsFindC_16u
//              ippsFindRev_8u     ippsFindRev_16u
//              ippsFindRevC_8u    ippsFindRevC_16u
//
//  Purpose:    Finds the match for string of elements or single element
//              within source string in direct or reverse direction
//
//  Arguments:
//     pSrc    - pointer to the source string
//     len     - source string length
//     pFind   - pointer to the searching string
//     lenFind - searching string length
//     valFind - searching element
//     pIndex  - pointer to the result index:
//               *pIndex = index of first occurrence ;
//               *pIndex = -1 if no match;
//
//  Return:
//   ippStsNoErr       Ok
//   ippStsNullPtrErr  pSrc, pFind or pIndex are NULL
//   ippStsLengthErr   len or lenFind are negative
*/
LEGACY90IPPAPI (IppStatus, legacy90ippsFind_16u, (const Ipp16u* pSrc, int len,
                                  const Ipp16u* pFind, int lenFind,
                                  int *pIndex))
LEGACY90IPPAPI (IppStatus, legacy90ippsFindC_16u, (const Ipp16u* pSrc, int len,
                                   Ipp16u valFind, int *pIndex))

LEGACY90IPPAPI (IppStatus, legacy90ippsFindRev_16u, (const Ipp16u* pSrc, int len,
                                     const Ipp16u* pFind, int lenFind,
                                     int *pIndex))
LEGACY90IPPAPI (IppStatus, legacy90ippsFindRevC_16u, (const Ipp16u* pSrc, int len,
                                      Ipp16u valFind, int *pIndex))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsFind_Z_8u        ippsFind_Z_16u
//              ippsFindC_Z_8u       ippsFindC_Z_16u
//
//  Purpose:    Finds the match for zero-ended string of elements or single element
//              within source zero-ended string in direct or reverse direction
//
//  Arguments:
//     pSrcZ   - pointer to the source zero-ended string
//     pFindZ  - pointer to the searching zero-ended string
//     valFind - searching element
//     pIndex  - pointer to the result index:
//               *pIndex = index of first occurrence;
//               *pIndex = -1 if no match;
//
//  Return:
//   ippStsNoErr       Ok
//   ippStsNullPtrErr  pSrcZ, pFindZ or pIndex are NULL
*/
LEGACY90IPPAPI (IppStatus, legacy90ippsFind_Z_16u,  (const Ipp16u* pSrcZ,
                                     const Ipp16u* pFindZ, int *pIndex))
LEGACY90IPPAPI (IppStatus, legacy90ippsFindC_Z_16u, (const Ipp16u* pSrcZ,
                                     Ipp16u valFind, int *pIndex))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsCompare_8u        ippsCompare_16u
//
//  Purpose:    Compares two strings element-by-element
//
//  Arguments:
//     pSrc1   - pointer to the first string
//     pSrc2   - pointer to the second string
//     len     - string length to compare
//     pResult - pointer to the result:
//               *pResult =  0 if src1 == src2;
//               *pResult = >0 if src1 >  src2;
//               *pResult = <0 if src1 <  src2;
//
//  Return:
//   ippStsNoErr       Ok
//   ippStsNullPtrErr  pSrc1, pSrc2 or pResult are NULL
//   ippStsLengthErr   len is negative
*/
LEGACY90IPPAPI (IppStatus, legacy90ippsCompare_16u, (const Ipp16u* pSrc1, const Ipp16u* pSrc2,
                                     int len, int *pResult))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsEqual_8u        ippsEqual_16u
//
//  Purpose:    Compares two strings element-by-element
//
//  Arguments:
//     pSrc1   - pointer to the first string
//     pSrc2   - pointer to the second string
//     len     - string length to compare
//     pResult - pointer to the result:
//               *pResult =  1 if src1 == src2;
//               *pResult =  0 if src1 != src2;
//
//  Return:
//   ippStsNoErr       Ok
//   ippStsNullPtrErr  pSrc1, pSrc2 or pResult are NULL
//   ippStsLengthErr   len is negative
*/
LEGACY90IPPAPI (IppStatus, legacy90ippsEqual_16u, (const Ipp16u* pSrc1, const Ipp16u* pSrc2,
                                   int len, int *pResult))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsTrimC_8u_I        ippsTrimC_16u_I
//
//  Purpose:    Deletes an odd symbol at the end and the beginning of a string
//              in-place
//
//  Arguments:
//     pSrcDst - pointer to the string
//     pLen    - pointer to the string length:
//               *pLen = source length on input;
//               *pLen = destination length on output;
//     odd     - odd symbol
//
//  Return:
//   ippStsNoErr       Ok
//   ippStsNullPtrErr  pSrcDst or pLen are NULL
//   ippStsLengthErr   *pLen is negative
*/
LEGACY90IPPAPI (IppStatus, legacy90ippsTrimC_16u_I, (Ipp16u* pSrcDst, int* pLen, Ipp16u odd ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsTrimC_8u        ippsTrimC_16u
//
//  Purpose:    Deletes an odd symbol at the end and the beginning of a string
//
//  Arguments:
//     pSrc    - pointer to the source string
//     srcLen  - source string length
//     odd     - odd symbol
//     pDst    - pointer to the destination string
//     pDstLen - pointer to the destination string length:
//               *pDstLen doesn't use as input value;
//               *pDstLen = destination length on output;
//
//  Return:
//   ippStsNoErr       Ok
//   ippStsNullPtrErr  pSrcDst, pDst or pDstLen are NULL
//   ippStsLengthErr   srcLen is negative
*/
LEGACY90IPPAPI (IppStatus, legacy90ippsTrimC_16u, (const Ipp16u* pSrc, int srcLen, Ipp16u odd,
                                   Ipp16u* pDst, int* pDstLen ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:          ippsUppercase_16u_I
//                 ippsLowercase_16u_I
//                 ippsUppercase_16u
//                 ippsLowercase_16u
//
//  Purpose:    Forms an uppercase or lowercase version of the Unicode string
//
//  Arguments:
//     pSrc    - pointer to the source string
//     pDst    - pointer to the destination string
//     pSrcDst - pointer to the string for in-place operation
//     len     - string length
//
//  Return:
//   ippStsNoErr       Ok
//   ippStsNullPtrErr  pSrc, pDst or pSrcDst are NULL;
//   ippStsLengthErr   len is negative;
*/

LEGACY90IPPAPI (IppStatus, legacy90ippsUppercase_16u_I,( Ipp16u* pSrcDst, int len ))
LEGACY90IPPAPI (IppStatus, legacy90ippsLowercase_16u_I,( Ipp16u* pSrcDst, int len ))
LEGACY90IPPAPI (IppStatus, legacy90ippsUppercase_16u, (const Ipp16u* pSrc, Ipp16u* pDst, int len))
LEGACY90IPPAPI (IppStatus, legacy90ippsLowercase_16u, (const Ipp16u* pSrc, Ipp16u* pDst, int len))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsUppercaseLatin_8u_I   ippsUppercaseLatin_16u_I
//              ippsLowercaseLatin_8u_I   ippsLowercaseLatin_16u_I
//              ippsLowercaseLatin_8u     ippsUppercaseLatin_16u
//              ippsUppercaseLatin_8u     ippsLowercaseLatin_16u
//
//  Purpose:    Forms an uppercase or lowercase version of the ASCII string
//
//  Arguments:
//     pSrc    - pointer to the source string
//     pDst    - pointer to the destination string
//     pSrcDst - pointer to the string for in-place operation
//     len     - string length
//
//  Return:
//   ippStsNoErr       Ok
//   ippStsNullPtrErr  pSrc, pDst or pSrcDst are NULL;
//   ippStsLengthErr   len is negative;
*/
LEGACY90IPPAPI (IppStatus, legacy90ippsUppercaseLatin_16u_I,( Ipp16u* pSrcDst, int len ))
LEGACY90IPPAPI (IppStatus, legacy90ippsLowercaseLatin_16u_I,( Ipp16u* pSrcDst, int len ))
LEGACY90IPPAPI (IppStatus, legacy90ippsUppercaseLatin_16u, (const Ipp16u* pSrc, Ipp16u* pDst, int len))
LEGACY90IPPAPI (IppStatus, legacy90ippsLowercaseLatin_16u, (const Ipp16u* pSrc, Ipp16u* pDst, int len))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsConcat_8u   ippsConcat_16u
//
//  Purpose:    Concatenates two strings together
//
//  Arguments:
//     pSrc1   - pointer to the first source string
//     len1    - first source string length
//     pSrc2   - pointer to the second source string
//     len2    - second source string length
//     pDst    - pointer to the destination string
//
//  Return:
//   ippStsNoErr       Ok
//   ippStsNullPtrErr  pSrc1, pSrc2 or pDst are NULL
//   ippStsLengthErr   len1 or len2 are negative
*/
LEGACY90IPPAPI (IppStatus, legacy90ippsConcat_16u, (const Ipp16u* pSrc1, int len1,
                                    const Ipp16u* pSrc2, int len2,
                                    Ipp16u* pDst))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsConcat_8u_D2L   ippsConcat_16u_D2L
//
//  Purpose:    Concatenates several strings together
//
//  Arguments:
//     pSrc    - pointer to the array of source strings
//     srcLen  - pointer to the array of source strings' lengths
//     numSrc  - number of source strings
//     pDst    - pointer to the destination string
//
//  Return:
//   ippStsNoErr       Ok
//   ippStsNullPtrErr  pSrc, srcLen or pDst are NULL;
//                     pSrc[i] is NULL for i < numSrc
//   ippStsLengthErr   srcLen[i] is negative for i < numSrc
//   ippStsSizeErr     numSrc is not positive
*/
LEGACY90IPPAPI (IppStatus, legacy90ippsConcat_16u_D2L, (const Ipp16u* const pSrc[], const int srcLen[], int numSrc,
                                        Ipp16u* pDst ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsConcatC_8u_D2L   ippsConcatC_16u_D2L
//
//  Purpose:    Concatenates several strings together and separates them
//              by the symbol delimiter
//
//  Arguments:
//     pSrc    - pointer to the array of source strings
//     srcLen  - pointer to the array of source strings' lengths
//     numSrc  - number of source strings
//     delim   - delimiter
//     pDst    - pointer to the destination string
//
//  Return:
//   ippStsNoErr       Ok
//   ippStsNullPtrErr  pSrc, srcLen or pDst are NULL;
//                     pSrc[i] is NULL for i < numSrc
//   ippStsLengthErr   srcLen[i] is negative for i < numSrc
//   ippStsSizeErr     numSrc is not positive
*/
LEGACY90IPPAPI (IppStatus, legacy90ippsConcatC_16u_D2L, (const Ipp16u* const pSrc[], const int srcLen[], int numSrc,
                                         Ipp16u delim, Ipp16u* pDst ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsSplitC_8u_D2L   ippsSplitC_16u_D2L
//
//  Purpose:    Splits source string to several destination strings
//              using the symbol delimiter; all delimiters are significant,
//              in the case of double delimiter empty string is inserted.
//
//  Arguments:
//     pSrc    - pointer to the source string
//     srcLen  - source string length
//     delim   - delimiter
//     pDst    - pointer to the array of destination strings
//     dstLen  - pointer to the array of destination strings' lengths
//     pNumDst - pointer to the number of destination strings:
//               *pNumDst = initial number of destination strings on input;
//               *pNumDst = number of splitted strings on output;
//
//  Return:
//     ippStsNoErr       Ok
//  ERRORS:
//     ippStsNullPtrErr  pSrc, pDst, dstLen or pNumDst are NULL;
//                       pDst[i] is NULL for i < number of splitted strings
//     ippStsLengthErr   srcLen is negative;
//                       dstLen[i] is negative for i < number of splitted strings
//     ippStsSizeErr     *pNumDst is not positive
//  WARNINGS:
//     ippStsOvermuchStrings  the initial number of destination strings is less
//                            than the number of splitted strings;
//                            number of destination strings is truncated to
//                            initial number in this case
//     ippStsOverlongString   the length of one of destination strings is less than
//                            length of corresponding splitted string;
//                            splitted string is truncated to destination length
//                            in this case
*/
LEGACY90IPPAPI (IppStatus, legacy90ippsSplitC_16u_D2L, (const Ipp16u* pSrc, int srcLen, Ipp16u delim,
                                        Ipp16u* pDst[], int dstLen[], int* pNumDst))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsFindCAny_8u
//              ippsFindCAny_16u
//              ippsFindRevCAny_8u
//              ippsFindRevCAny_16u
//
//  Purpose:    Reports the index of the first/last occurrence in
//              the vector of any value in a specified array.
//
//  Arguments:
//     pSrc    - The pointer of vector to find.
//     len     - The length of the vector.
//     pAnyOf  - A pointer of array containing one or more values to seek.
//     lenFind - The length of array.
//     pIndex  - The positive integer index of the first occurrence in
//               the vector where any value in pAnyOf was found;
//               otherwise, -1 if no value in pAnyOf was found.
//
//  Return:
//   ippStsNoErr       Ok
//   ippStsNullPtrErr  Any of pointers is NULL.
//   ippStsLengthErr   len or lenAnyOf are negative.
*/
LEGACY90IPPAPI (IppStatus, legacy90ippsFindCAny_16u, ( const Ipp16u* pSrc, int len,
        const Ipp16u* pAnyOf, int lenAnyOf, int* pIndex ))
LEGACY90IPPAPI (IppStatus, legacy90ippsFindRevCAny_16u, ( const Ipp16u* pSrc, int len,
        const Ipp16u* pAnyOf, int lenAnyOf, int* pIndex ))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsReplaceC_8u
//              ippsReplaceC_16u
//
//  Purpose:    Replaces all occurrences of a specified value in
//              the vector with another specified value.
//
//  Arguments:
//     pSrc    - The pointer of vector to replace.
//     pDst    - The ponter of replaced vector.
//     len     - The length of the vector.
//     oldVal  - A value to be replaced.
//     newVal  - A value to replace all occurrences of oldVal.
//
//  Return:
//   ippStsNoErr       Ok
//   ippStsNullPtrErr  Any of pointers is NULL.
//   ippStsLengthErr   len is negative.
*/
LEGACY90IPPAPI (IppStatus, legacy90ippsReplaceC_16u, ( const Ipp16u* pSrc, Ipp16u* pDst, int len,
        Ipp16u oldVal, Ipp16u newVal ))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsTrimCAny_8u
//              ippsTrimCAny_16u
//              ippsTrimEndCAny_8u
//              ippsTrimEndCAny_16u
//              ippsTrimStartCAny_8u
//              ippsTrimStartCAny_16u
//
//  Purpose:    Removes all occurrences of a set of specified values
//              from:
//                TrimCAny  - the beginning and end of the vector.
//                TrimEndCAny   - the end of the vector.
//                TrimStartCAny - the beginning of the vector.
//
//  Arguments:
//     pSrc    - The pointer of src vector to remove.
//     srcLen  - The length of the src vector.
//     pTrim   - An array of values to be removed.
//     trimLen - The length of the array values.
//     pDst    - The pointer of dst vector to result save.
//     pDstLen - The result length of the dst vector.
//
//  Return:
//   ippStsNoErr       Ok
//   ippStsNullPtrErr  Any of pointers is NULL.
//   ippStsLengthErr   srcLen or trimLen are negative.
//
//  Note:
//   The length of the pDst should be sufficient;
//   if values not found, *pDstLen = srcLen.
*/
LEGACY90IPPAPI (IppStatus, legacy90ippsTrimCAny_16u, ( const Ipp16u* pSrc, int srcLen,
        const Ipp16u* pTrim, int trimLen, Ipp16u* pDst, int* pDstLen ))
LEGACY90IPPAPI (IppStatus, legacy90ippsTrimEndCAny_16u, ( const Ipp16u* pSrc, int srcLen,
        const Ipp16u* pTrim, int trimLen, Ipp16u* pDst, int* pDstLen ))
LEGACY90IPPAPI (IppStatus, legacy90ippsTrimStartCAny_16u, ( const Ipp16u* pSrc, int srcLen,
        const Ipp16u* pTrim, int trimLen, Ipp16u* pDst, int* pDstLen ))


/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsCompareIgnoreCase_16u
//
//  Purpose:    Compares two Unicode strings element-by-element
//
//  Arguments:
//     pSrc1   - pointer to the first string
//     pSrc2   - pointer to the second string
//     len     - string length to compare
//     pResult - pointer to the result:
//               *pResult = 0 if src1 == src2;
//               *pResult > 0 if src1 >  src2;
//               *pResult < 0 if src1 <  src2;
//
//  Return:
//   ippStsNoErr       Ok
//   ippStsNullPtrErr  pSrc1, pSrc2 or pResult is NULL
//   ippStsLengthErr   len is negative
*/
LEGACY90IPPAPI (IppStatus, legacy90ippsCompareIgnoreCase_16u, (const Ipp16u* pSrc1, const Ipp16u* pSrc2, int len,
                                               int *pResult))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsCompareIgnoreCaseLatin_8u
//              ippsCompareIgnoreCaseLatin_16u
//
//  Purpose:    Compares two ASCII strings element-by-element
//
//  Arguments:
//     pSrc1   - pointer to the first string
//     pSrc2   - pointer to the second string
//     len     - string length to compare
//     pResult - pointer to the result:
//               *pResult = 0 if src1 == src2;
//               *pResult > 0 if src1 >  src2;
//               *pResult < 0 if src1 <  src2;
//
//  Return:
//   ippStsNoErr       Ok
//   ippStsNullPtrErr  pSrc1, pSrc2 or pResult is NULL
//   ippStsLengthErr   len is negative
*/
LEGACY90IPPAPI (IppStatus, legacy90ippsCompareIgnoreCaseLatin_16u, (const Ipp16u* pSrc1, const Ipp16u* pSrc2,
                                                    int len, int *pResult))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsInsert_8u_I       ippsInsert_16u_I
//              ippsInsert_8u         ippsInsert_16u
//
//  Purpose:    Inserts one string at a specified index position in other string
//
//  Arguments:
//     pSrc       - pointer to the source string
//     srcLen     - source string length
//     pInsert    - pointer to the string to be inserted
//     insertLen  - length of the string to be inserted
//     pDst       - pointer to the destination string
//     pSrcDst    - pointer to the string for in-place operation
//     pSrcDstLen - pointer to the string length:
//                 *pSrcDstLen = source length on input;
//                 *pSrcDstLen = destination length on output;
//     startIndex - index of start position
//
//  Return:
//   ippStsNoErr       Ok
//   ippStsNullPtrErr  pSrc, pInsert, pDst, pSrcDst or pSrcDstLen is NULL
//   ippStsLengthErr   srcLen, insertLen, *pSrcDstLen or startIndex is negative Or
//                     startIndex is greater than srcLen or *pSrcDstLen
*/
LEGACY90IPPAPI (IppStatus, legacy90ippsInsert_16u_I, (const Ipp16u* pInsert, int insertLen, Ipp16u* pSrcDst,
                                      int* pSrcDstLen, int startIndex))
LEGACY90IPPAPI (IppStatus, legacy90ippsInsert_16u,   (const Ipp16u* pSrc, int srcLen, const Ipp16u* pInsert,
                                      int insertLen, Ipp16u* pDst, int startIndex))

/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippsRemove_8u_I       ippsRemove_16u_I
//              ippsRemove_8u         ippsRemove_16u
//
//  Purpose:    Deletes a specified number of characters from the string
//              beginning at a specified position.
//
//  Arguments:
//     pSrc       - pointer to the source string
//     srcLen     - source string length
//     pDst       - pointer to the destination string
//     pSrcDst    - pointer to the string for in-place operation
//     pSrcDstLen - pointer to the string length:
//                 *pSrcDstLen = source length on input;
//                 *pSrcDstLen = destination length on output;
//     startIndex - index of start position
//     len        - number of characters to be deleted
//
//  Return:
//   ippStsNoErr       Ok
//   ippStsNullPtrErr  pSrc, pDst, pSrcDst or pSrcDstLen are NULL
//   ippStsLengthErr   srcLen, *pSrcDstLen, len or startIndex is negative Or
//                     (startIndex + len) is greater than srcLen or *pSrcDstLen
*/
LEGACY90IPPAPI (IppStatus, legacy90ippsRemove_16u_I, (Ipp16u* pSrcDst, int* pSrcDstLen, int startIndex, int len))
LEGACY90IPPAPI (IppStatus, legacy90ippsRemove_16u,   (const Ipp16u* pSrc, int srcLen, Ipp16u* pDst, int startIndex,
                                      int len))


/* /////////////////////////////////////////////////////////////////////////////
// Name:                ippsRegExpMultiGetSize
// Purpose:             Computes the size of necessary memory (in bytes) for
//                      structure containing internal form of multi patterns search engine.
//
// Parameters:
//    maxPatterns       Maximum number of pattern.
//    pSize             Pointer to the computed size of structure containing
//                      internal form of search engine
//
// Return:
//    ippStsNullPtrErr  One or several pointer(s) is NULL
//    ippStsSizeErr     When maxPatterns is less or equal 0.
//    ippStsNoErr       No errors
//
*/
LEGACY90IPPAPI(IppStatus, legacy90ippsRegExpMultiGetSize, ( Ipp32u maxPatterns, int *pSize))

/* /////////////////////////////////////////////////////////////////////////////
// Name:                ippsRegExpMultiInit
// Purpose:             Initialize internal form of multi patterns search engine.
//
// Parameters:
//    maxPatterns       Maximum number of pattern.
//    pState            Pointer to the structure containing internal form of
//                      multi patterns search engine.
//
// Return:
//    ippStsNoErr       No errors
//    ippStsNullPtrErr  One or several pointer(s) is NULL
//    ippStsSizeErr     When maxPatterns is less or equal 0.
//
*/
LEGACY90IPPAPI(IppStatus, legacy90ippsRegExpMultiInit, ( IppRegExpMultiState* pState, Ipp32u maxPatterns))

/* /////////////////////////////////////////////////////////////////////////////
// Name:                ippsRegExpMultiInitAlloc
// Purpose:             Allocates necessary memory, initialize internal form of multi patterns search engine
//
// Parameters:
//    maxPatterns       Maximum number of pattern.
//    pState            Double pointer to the structure containing internal form of
//                      multi patterns search engine.
//
// Return:
//    ippStsNoErr       No errors
//    ippStsNullPtrErr  One or several pointer(s) is NULL
//    ippStsSizeErr     When maxPatterns is less or equal 0.
*/
LEGACY90IPPAPI(IppStatus, legacy90ippsRegExpMultiInitAlloc, ( IppRegExpMultiState** ppState, Ipp32u maxPatterns))

/* /////////////////////////////////////////////////////////////////////////////
// Name:                ippsRegExpMultiFree
// Purpose:             Frees allocated memory for the structure containing
//                      internal form of multi patterns search engine
//
// Parameters:
//    pState            Pointer to the structure containing internal form of
//                      multi patterns search engine.
//
*/
LEGACY90IPPAPI(void, legacy90ippsRegExpMultiFree, (IppRegExpMultiState* pState))

/* /////////////////////////////////////////////////////////////////////////////
// Name:                ippsRegExpMulti*
// Purpose:             Controls multi patterns database. Add, remove or modify patterns.
//
// Parameters:
//    pRegExpState      Pointer to the structure containing internal form of a
//                      compiled regular expression.
//    regexpID          Pattern ID. 0 is invalid ID.
//    pState            Pointer to the structure containing internal form of
//                      multi patterns search engine.
//
// Return:
//    ippStsNoErr       No errors
//    ippStsNullPtrErr  One or several pointer(s) is NULL
//    ippStsSizeErr     When ID is equal 0.
//    ippStsMemAllocErr When number of patterns exceeded its maximum value.
//
*/
LEGACY90IPPAPI(IppStatus, legacy90ippsRegExpMultiAdd, ( const IppRegExpState* pRegExpState, Ipp32u regexpID, IppRegExpMultiState* pState))
LEGACY90IPPAPI(IppStatus, legacy90ippsRegExpMultiModify, ( const IppRegExpState* pRegExpState, Ipp32u regexpID, IppRegExpMultiState* pState))
LEGACY90IPPAPI(IppStatus, legacy90ippsRegExpMultiDelete, (Ipp32u regexpID, IppRegExpMultiState* pState))

/* /////////////////////////////////////////////////////////////////////////////
// Name:                ippsRegExpMultiFind_8u
// Purpose:             Looks for the occurrences of the substrings matching
//                      the specified patterns.
//
// Parameters:
//    pSrc              Pointer to the source string
//    srcLen            Number of elements in the source string.
//    pState            Pointer to the structure containing internal form of
//                      multi patterns search engine
//    pDstMultiFind     Array of pointers to the matching patterns
//
// Return:
//    ippStsNullPtrErr          One or several pointer(s) is NULL
//    ippStsSizeErr             Length of the source vector is less or equal zero.
//    ippStsNoErr               No errors
//
*/
LEGACY90IPPAPI(IppStatus, legacy90ippsRegExpMultiFind_8u, ( const Ipp8u* pSrc, int srcLen, IppRegExpMultiFind *pDstMultiFind, const IppRegExpMultiState* pState))


#ifdef __cplusplus
}
#endif

#endif /* __IPPCH_90_LEGACY_H__ */
