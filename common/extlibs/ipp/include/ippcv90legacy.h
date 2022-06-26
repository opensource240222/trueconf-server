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
//      Intel(R) Integrated Performance Primitives
//             Computer Vision (ippCV)
//                 Legacy Library
//
*/

#if !defined( __IPPCV_90_LEGACY_H__ )
#define __IPPCV_90_LEGACY_H__

#include "ippdefs90legacy.h"
#include "ippcv90legacy_redef.h"

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
LEGACY90IPPAPI( IppStatus, legacy90ippcvInit, ( void ))


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
LEGACY90IPPAPI( IppStatus, legacy90ippcvSetNumThreads, ( int numThr ) )

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
LEGACY90IPPAPI( IppStatus, legacy90ippcvGetNumThreads, (int* pNumThr) )

/*////////////////////////////////////////////////////////////////////////////*/


/* ///////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//                   Functions declarations
//////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////// */

/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippcvGetLibVersion
//
//  Purpose:    getting of the library version
//
//  Returns:    the structure of information about  version of ippcv library
//
//  Parameters:
//
//  Notes:      not necessary to release the returned structure
*/
LEGACY90IPPAPI( const IppLibraryVersion*, legacy90ippcvGetLibVersion, (void) )


/****************************************************************************************\
*                                    Accumulation                                        *
\****************************************************************************************/

/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiAdd_8u32f_C1IR,   ippiAdd_8s32f_C1IR,
//              ippiAdd_16u32f_C1IR,
//              ippiAdd_8u32f_C1IMR,  ippiAdd_8s32f_C1IMR,
//              ippiAdd_16u32f_C1IMR, ippiAdd_32f_C1IMR
//
//  Purpose:    Add image to accumulator.
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width or height of images is less or equal zero
//    ippStsStepErr            Step is too small to fit image.
//    ippStsNotEvenStepErr     Step is not multiple of element.
//
//  Arguments:
//    pSrc                     Pointer to source image
//    srcStep                  Step in the source image
//    pMask                    Pointer to mask
//    maskStep                 Step in the mask image
//    pSrcDst                  Pointer to accumulator image
//    srcDstStep               Step in the accumulator image
//    roiSize                  Image size
*/

LEGACY90IPPAPI(IppStatus, legacy90ippiAdd_8s32f_C1IR, (const Ipp8s*  pSrc, int srcStep,
                                       Ipp32f* pSrcDst, int srcDstStep,
                                       IppiSize roiSize ))

LEGACY90IPPAPI(IppStatus, legacy90ippiAdd_8s32f_C1IMR,(const Ipp8s*  pSrc, int srcStep,
                                       const Ipp8u* pMask, int maskStep,
                                       Ipp32f* pSrcDst, int srcDstStep,
                                       IppiSize roiSize ))


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:   ippiAddSquare_8u32f_C1IR,   ippiAddSquare_8s32f_C1IR,
//          ippiAddSquare_16u32f_C1IR,  ippiAddSquare_32f_C1IR,
//          ippiAddSquare_8u32f_C1IMR,  ippiAddSquare_8s32f_C1IMR,
//          ippiAddSquare_16u32f_C1IMR, ippiAddSquare_32f_C1IMR
//
//  Purpose:    Add squared image (i.e. multiplied by itself) to accumulator.
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width or height of images is less or equal zero
//    ippStsStepErr            Step is too small to fit image.
//    ippStsNotEvenStepErr     Step is not multiple of element.
//
//  Arguments:
//    pSrc                     Pointer to source image
//    srcStep                  Step in the source image
//    pMask                    Pointer to mask
//    maskStep                 Step in the mask image
//    pSrcDst                  Pointer to accumulator image
//    srcDstStep               Step in the accumulator image
//    roiSize                  Image size
*/

LEGACY90IPPAPI(IppStatus, legacy90ippiAddSquare_8s32f_C1IR, (const Ipp8s*  pSrc, int srcStep,
                                             Ipp32f* pSrcDst, int srcDstStep,
                                             IppiSize roiSize ))

LEGACY90IPPAPI(IppStatus, legacy90ippiAddSquare_8s32f_C1IMR,(const Ipp8s* pSrc, int srcStep,
                                             const Ipp8u* pMask, int maskStep,
                                             Ipp32f* pSrcDst, int srcDstStep,
                                             IppiSize roiSize ))


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name: ippiAddProduct_8u32f_C1IR,   ippiAddProduct_8s32f_C1IR,
//        ippiAddProduct_16u32f_C1IR,  ippiAddProduct_32f_C1IR,
//        ippiAddProduct_8u32f_C1IMR,  ippiAddProduct_8s32f_C1IMR,
//        ippiAddProduct_16u32f_C1IMR, ippiAddProduct_32f_C1IMR
//
//  Purpose:  Add product of two images to accumulator.
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width or height of images is less or equal zero
//    ippStsStepErr            Step is too small to fit image.
//    ippStsNotEvenStepErr     Step is not multiple of element.
//
//  Arguments:
//    pSrc1                    Pointer to first source image
//    src1Step                 Step in the first source image
//    pSrc2                    Pointer to second source image
//    src2Step                 Step in the second source image
//    pMask                    Pointer to mask
//    maskStep                 Step in the mask image
//    pSrcDst                  Pointer to accumulator image
//    srcDstStep               Step in the accumulator image
//    roiSize                  Image size
*/

LEGACY90IPPAPI(IppStatus, legacy90ippiAddProduct_8s32f_C1IR, (const Ipp8s*  pSrc1, int src1Step,
                                              const Ipp8s*  pSrc2, int src2Step,
                                              Ipp32f* pSrcDst, int srcDstStep,
                                              IppiSize roiSize ))

LEGACY90IPPAPI(IppStatus, legacy90ippiAddProduct_8s32f_C1IMR,(const Ipp8s*  pSrc1, int src1Step,
                                              const Ipp8s*  pSrc2, int src2Step,
                                              const Ipp8u* pMask, int maskStep,
                                              Ipp32f* pSrcDst, int srcDstStep,
                                              IppiSize roiSize ))



/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name: ippiAddWeighted_8u32f_C1IR,  ippiAddWeighted_8s32f_C1IR,
//        ippiAddWeighted_16u32f_C1IR, ippiAddWeighted_32f_C1IR,
//        ippiAddWeighted_8u32f_C1IMR, ippiAddWeighted_8s32f_C1IMR,
//        ippiAddWeighted_16u32f_C1IMR,ippiAddWeighted_32f_C1IMR
//        ippiAddWeighted_32f_C1R
//
//  Purpose:  Add image, multiplied by alpha, to accumulator, multiplied by (1 - alpha).
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width or height of images is less or equal zero
//    ippStsStepErr            Step is too small to fit image.
//    ippStsNotEvenStepErr     Step is not multiple of element.
//
//  Arguments:
//    pSrc1                    Pointer to first source image
//    src1Step                 Step in the first source image
//    pSrc2                    Pointer to second source image
//    src2Step                 Step in the second source image
//    pMask                    Pointer to mask
//    maskStep                 Step in the mask image
//    pSrcDst                  Pointer to accumulator image
//    srcDstStep               Step in the accumulator image
//    pDst                     Pointer to destination image
//    dstStep                  Step in the destination image
//    roiSize                  Image size
//    alpha                    Weight of source image
*/

LEGACY90IPPAPI(IppStatus, legacy90ippiAddWeighted_8s32f_C1IR, (const Ipp8s*  pSrc, int srcStep,
                                               Ipp32f* pSrcDst, int srcDstStep,
                                               IppiSize roiSize, Ipp32f alpha ))


LEGACY90IPPAPI(IppStatus, legacy90ippiAddWeighted_8s32f_C1IMR,(const Ipp8s* pSrc, int srcStep,
                                               const Ipp8u* pMask, int maskStep,
                                               Ipp32f* pSrcDst, int srcDstStep,
                                               IppiSize roiSize, Ipp32f alpha ))


/****************************************************************************************\
*                                Morphological Operations                                *
\****************************************************************************************/

/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:   ippiMorphologyGetSize_8u_C1R,            ippiMorphologyGetSize_32f_C1R,
//          ippiMorphologyGetSize_8u_C3R,            ippiMorphologyGetSize_32f_C3R,
//          ippiMorphologyGetSize_8u_C4R,            ippiMorphologyGetSize_32f_C4R,
//          ippiMorphologyBorderGetSize_16u_C1R,     ippiMorphologyBorderGetSize_16s_C1R,
//          ippiMorphologyBorderGetSize_1u_C1R,      ippiMorphologyBorderGetSize_8u_C1R,
//          ippiMorphologyBorderGetSize_8u_C3R,      ippiMorphologyBorderGetSize_8u_C4R,
//          ippiMorphologyBorderGetSize_32f_C1R,     ippiMorphologyBorderGetSize_32f_C3R
//          ippiMorphologyBorderGetSize_32f_C4R
//
//
//  Purpose:  Gets the size of the internal state or specification structure for morphological operations.
//
//  Return:
//    ippStsNoErr              Ok.
//    ippStsNullPtrErr         One of the pointers is NULL.
//    ippStsSizeErr            Width of the image, or width or height of the structuring
//                             element is less than,or equal to zero.
//
//  Parameters:
//    roiWidth                 Width of the image ROI in pixels.
//    pMask                    Pointer to the structuring element (mask).
//    maskSize                 Size of the structuring element.
//    pSize                    Pointer to the state structure length.
//    pSpecSize                Pointer to the specification structure size.
//    pBufferSize              Pointer to the buffer size value for the morphological initialization function.
*/
LEGACY90IPPAPI(IppStatus, legacy90ippiMorphologyGetSize_8u_C1R,(int roiWidth, const Ipp8u* pMask, IppiSize maskSize, int* pSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiMorphologyGetSize_8u_C3R,(int roiWidth, const Ipp8u* pMask, IppiSize maskSize, int* pSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiMorphologyGetSize_8u_C4R,(int roiWidth, const Ipp8u* pMask, IppiSize maskSize, int* pSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiMorphologyGetSize_32f_C1R,(int roiWidth, const Ipp8u* pMask, IppiSize maskSize, int* pSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiMorphologyGetSize_32f_C3R,(int roiWidth, const Ipp8u* pMask, IppiSize maskSize, int* pSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiMorphologyGetSize_32f_C4R,(int roiWidth, const Ipp8u* pMask, IppiSize maskSize, int* pSize))


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:   ippiMorphologyInit_8u_C1R,               ippiMorphologyInit_32f_C1R,
//          ippiMorphologyInit_8u_C3R,               ippiMorphologyInit_32f_C3R,
//          ippiMorphologyInit_8u_C4R,               ippiMorphologyInit_32f_C4R,
//          ippiMorphologyBorderInit_16u_C1R,        ippiMorphologyBorderInit_16s_C1R
//          ippiMorphologyBorderInit_1u_C1R,         ippiMorphologyBorderInit_8u_C1R
//          ippiMorphologyBorderInit_8u_C3R,         ippiMorphologyBorderInit_8u_C4R
//          ippiMorphologyBorderInit_32f_C1R,        ippiMorphologyBorderInit_32f_C3R,
//          ippiMorphologyBorderInit_32f_C4R,
//
//  Purpose:  Initialize the internal state or specification structure for morphological operation.
//
//  Return:
//    ippStsNoErr              Ok.
//    ippStsNullPtrErr         One of the pointers is NULL.
//    ippStsSizeErr            Width of the image or width or height of the structuring
//                             element is less than, or equal to zero.
//    ippStsAnchorErr          Anchor point is outside the structuring element.
//
//  Parameters:
//    roiWidth                 Width of the image ROI in pixels.
//    pMask                    Pointer to the structuring element (mask).
//    maskSize                 Size of the structuring element.
//    anchor                   Anchor of the structuring element.
//    pState                   Pointer to the morphology state structure.
//    pMorphSpec               Pointer to the morphology specification structure.
//    pBuffer                  Pointer to the external work buffer.
*/

LEGACY90IPPAPI(IppStatus, legacy90ippiMorphologyInit_8u_C1R,(int roiWidth, const Ipp8u* pMask, IppiSize maskSize,
                                              IppiPoint anchor, IppiMorphState* pState))
LEGACY90IPPAPI(IppStatus, legacy90ippiMorphologyInit_8u_C3R,(int roiWidth, const Ipp8u* pMask, IppiSize maskSize,
                                              IppiPoint anchor, IppiMorphState* pState))
LEGACY90IPPAPI(IppStatus, legacy90ippiMorphologyInit_8u_C4R,(int roiWidth, const Ipp8u* pMask, IppiSize maskSize,
                                              IppiPoint anchor, IppiMorphState* pState))
LEGACY90IPPAPI(IppStatus, legacy90ippiMorphologyInit_32f_C1R,(int roiWidth, const Ipp8u* pMask, IppiSize maskSize,
                                               IppiPoint anchor, IppiMorphState* pState))
LEGACY90IPPAPI(IppStatus, legacy90ippiMorphologyInit_32f_C3R,(int roiWidth, const Ipp8u* pMask, IppiSize maskSize,
                                               IppiPoint anchor, IppiMorphState* pState))
LEGACY90IPPAPI(IppStatus, legacy90ippiMorphologyInit_32f_C4R,(int roiWidth, const Ipp8u* pMask, IppiSize maskSize,
                                               IppiPoint anchor, IppiMorphState* pState))


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:   ippiMorphologyInitAlloc_8u_C1R,          ippiMorphologyInitAlloc_32f_C1R,
//          ippiMorphologyInitAlloc_8u_C3R,          ippiMorphologyInitAlloc_32f_C3R,
//          ippiMorphologyInitAlloc_8u_C4R,          ippiMorphologyInitAlloc_32f_C4R
//
//  Purpose:  Allocate buffers and initialize internal state of morphological operation.
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width of image or width or height of structuring
//                             element is less or equal zero.
//    ippStsAnchorErr          Anchor point is outside the structuring element
//    ippStsMemAllocErr        Memory allocation error
//
//  Arguments:
//    roiWidth                 Width of image ROI in pixels
//    pMask                    Pointer to structuring element (mask)
//    maskSize                 Size of structuring element
//    anchor                   Anchor of the structuring element
//    ppState                  Double pointer to morphological state (InitAlloc)
*/

LEGACY90IPPAPI(IppStatus, legacy90ippiMorphologyInitAlloc_8u_C1R,(int roiWidth, const Ipp8u* pMask, IppiSize maskSize,
                                                   IppiPoint anchor, IppiMorphState** ppState))
LEGACY90IPPAPI(IppStatus, legacy90ippiMorphologyInitAlloc_8u_C3R,(int roiWidth, const Ipp8u* pMask, IppiSize maskSize,
                                                   IppiPoint anchor, IppiMorphState** ppState))
LEGACY90IPPAPI(IppStatus, legacy90ippiMorphologyInitAlloc_8u_C4R,(int roiWidth, const Ipp8u* pMask, IppiSize maskSize,
                                                   IppiPoint anchor, IppiMorphState** ppState))
LEGACY90IPPAPI(IppStatus, legacy90ippiMorphologyInitAlloc_32f_C1R,(int roiWidth, const Ipp8u* pMask, IppiSize maskSize,
                                                    IppiPoint anchor, IppiMorphState** ppState))
LEGACY90IPPAPI(IppStatus, legacy90ippiMorphologyInitAlloc_32f_C3R,(int roiWidth, const Ipp8u* pMask, IppiSize maskSize,
                                                    IppiPoint anchor, IppiMorphState** ppState))
LEGACY90IPPAPI(IppStatus, legacy90ippiMorphologyInitAlloc_32f_C4R,(int roiWidth, const Ipp8u* pMask, IppiSize maskSize,
                                                    IppiPoint anchor, IppiMorphState** ppState))


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:   ippiMorphologyFree
//
//  Purpose:  Releases buffers, allocated by ippiMorphologyInitAlloc
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//
//  Arguments:
//    morphState               Pointer to morphological state.
*/
LEGACY90IPPAPI(IppStatus, legacy90ippiMorphologyFree,(IppiMorphState* pState))


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:   ippiDilateBorderReplicate_8u_C1R,    ippiDilateBorderReplicate_8u_C3R,
//          ippiDilateBorderReplicate_8u_C4R,    ippiDilateBorderReplicate_32f_C1R,
//          ippiDilateBorderReplicate_32f_C3R,   ippiDilateBorderReplicate_32f_C4R
//
//          ippiErodeBorderReplicate_8u_C1R,     ippiErodeBorderReplicate_8u_C3R,
//          ippiErodeBorderReplicate_8u_C4R,     ippiErodeBorderReplicate_32f_C1R,
//          ippiErodeBorderReplicate_32f_C3R,    ippiErodeBorderReplicate_32f_C4R,
//
//          ippiDilateBorder_16u_C1R,            ippiDilateBorder_16s_C1R,
//          ippiDilateBorder_1u_C1R
//
//  Purpose:    Perform erosion/dilation of the image arbitrary shape structuring element.
//
//  Return:
//    ippStsNoErr              Ok.
//    ippStsNullPtrErr         One of the pointers is NULL.
//    ippStsSizeErr            The ROI width or height is less than 1,
//                             or ROI width is bigger than ROI width in the state structure.
//    ippStsStepErr            Step is too small to fit the image.
//    ippStsNotEvenStepErr     Step is not multiple of the element.
//    ippStsBadArgErr          Incorrect border type.
//

//  Parameters:
//    pSrc                     Pointer to the source image.
//    srcStep                  Step in the source image.
//    pDst                     Pointer to the destination image.
//    dstStep                  Step in the destination image.
//    roiSize                  ROI size.
//    borderType               Type of border (ippBorderRepl now).
//    borderValue              Value for the constant border.
//    pState                   Pointer to the morphology state structure.
//    pMorphSpec               Pointer to the morphology specification structure.
//    pBuffer                  Pointer to the external work buffer.
*/
LEGACY90IPPAPI(IppStatus, legacy90ippiDilateBorderReplicate_8u_C1R,(const Ipp8u* pSrc, int srcStep,
                                   Ipp8u* pDst, int dstStep, IppiSize roiSize,
                                   IppiBorderType borderType, IppiMorphState* pState))
LEGACY90IPPAPI(IppStatus, legacy90ippiDilateBorderReplicate_8u_C3R,(const Ipp8u* pSrc, int srcStep,
                                   Ipp8u* pDst, int dstStep, IppiSize roiSize,
                                   IppiBorderType borderType, IppiMorphState* pState))
LEGACY90IPPAPI(IppStatus, legacy90ippiDilateBorderReplicate_8u_C4R,(const Ipp8u* pSrc, int srcStep,
                                   Ipp8u* pDst, int dstStep, IppiSize roiSize,
                                   IppiBorderType borderType, IppiMorphState* pState))
LEGACY90IPPAPI(IppStatus, legacy90ippiDilateBorderReplicate_32f_C1R,(const Ipp32f* pSrc, int srcStep,
                                   Ipp32f* pDst, int dstStep, IppiSize roiSize,
                                   IppiBorderType borderType, IppiMorphState* pState))
LEGACY90IPPAPI(IppStatus, legacy90ippiDilateBorderReplicate_32f_C3R,(const Ipp32f* pSrc, int srcStep,
                                   Ipp32f* pDst, int dstStep, IppiSize roiSize,
                                   IppiBorderType borderType, IppiMorphState* pState))
LEGACY90IPPAPI(IppStatus, legacy90ippiDilateBorderReplicate_32f_C4R,(const Ipp32f* pSrc, int srcStep,
                                   Ipp32f* pDst, int dstStep, IppiSize roiSize,
                                   IppiBorderType borderType, IppiMorphState* pState))


LEGACY90IPPAPI(IppStatus, legacy90ippiErodeBorderReplicate_8u_C1R,(const Ipp8u* pSrc, int srcStep,
                                   Ipp8u* pDst, int dstStep, IppiSize roiSize,
                                   IppiBorderType borderType, IppiMorphState* pState))
LEGACY90IPPAPI(IppStatus, legacy90ippiErodeBorderReplicate_8u_C3R,(const Ipp8u* pSrc, int srcStep,
                                   Ipp8u* pDst, int dstStep, IppiSize roiSize,
                                   IppiBorderType borderType, IppiMorphState* pState))
LEGACY90IPPAPI(IppStatus, legacy90ippiErodeBorderReplicate_8u_C4R,(const Ipp8u* pSrc, int srcStep,
                                   Ipp8u* pDst, int dstStep, IppiSize roiSize,
                                   IppiBorderType borderType, IppiMorphState* pState))
LEGACY90IPPAPI(IppStatus, legacy90ippiErodeBorderReplicate_32f_C1R,(const Ipp32f* pSrc, int srcStep,
                                   Ipp32f* pDst, int dstStep, IppiSize roiSize,
                                   IppiBorderType borderType, IppiMorphState* pState))
LEGACY90IPPAPI(IppStatus, legacy90ippiErodeBorderReplicate_32f_C3R,(const Ipp32f* pSrc, int srcStep,
                                   Ipp32f* pDst, int dstStep, IppiSize roiSize,
                                   IppiBorderType borderType, IppiMorphState* pState))
LEGACY90IPPAPI(IppStatus, legacy90ippiErodeBorderReplicate_32f_C4R,(const Ipp32f* pSrc, int srcStep,
                                   Ipp32f* pDst, int dstStep, IppiSize roiSize,
                                   IppiBorderType borderType, IppiMorphState* pState))


/****************************************************************************************\
*                       Advanced Morphological Operations                                *
\****************************************************************************************/

/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:   ippiMorphAdvInitAlloc_8u_C1R,          ippiMorphAdvInitAlloc_32f_C1R,
//          ippiMorphAdvInitAlloc_8u_C3R,          ippiMorphAdvInitAlloc_32f_C3R,
//          ippiMorphAdvInitAlloc_8u_C4R,          ippiMorphAdvInitAlloc_32f_C4R
//
//  Purpose:  Allocate buffers and initialize internal state of advanced morphological operation.
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width of image or width or height of structuring
//                             element is less or equal zero.
//    ippStsAnchorErr          Anchor point is outside the structuring element
//    ippStsMemAllocErr        Memory allocation error
//
//  Arguments:
//    ppState                  Double pointer to morphological state (InitAlloc)
//    roiSize                  Maximal image ROI in pixels
//    pMask                    Pointer to structuring element (mask)
//    maskSize                 Size of structuring element
//    anchor                   Anchor of the structuring element
*/

LEGACY90IPPAPI(IppStatus, legacy90ippiMorphAdvInitAlloc_8u_C1R,(IppiMorphAdvState** morphState, IppiSize roiSize, const Ipp8u* pMask,
                                                     IppiSize maskSize, IppiPoint anchor))
LEGACY90IPPAPI(IppStatus, legacy90ippiMorphAdvInitAlloc_8u_C3R,(IppiMorphAdvState** morphState, IppiSize roiSize, const Ipp8u* pMask,
                                                     IppiSize maskSize, IppiPoint anchor))
LEGACY90IPPAPI(IppStatus, legacy90ippiMorphAdvInitAlloc_8u_C4R,(IppiMorphAdvState** morphState, IppiSize roiSize, const Ipp8u* pMask,
                                                     IppiSize maskSize, IppiPoint anchor))
LEGACY90IPPAPI(IppStatus, legacy90ippiMorphAdvInitAlloc_32f_C1R,(IppiMorphAdvState** morphState, IppiSize roiSize, const Ipp8u* pMask,
                                                     IppiSize maskSize, IppiPoint anchor))
LEGACY90IPPAPI(IppStatus, legacy90ippiMorphAdvInitAlloc_32f_C3R,(IppiMorphAdvState** morphState, IppiSize roiSize, const Ipp8u* pMask,
                                                     IppiSize maskSize, IppiPoint anchor))
LEGACY90IPPAPI(IppStatus, legacy90ippiMorphAdvInitAlloc_32f_C4R,(IppiMorphAdvState** morphState, IppiSize roiSize, const Ipp8u* pMask,
                                                     IppiSize maskSize, IppiPoint anchor))


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:   ippiMorphAdvFree
//
//  Purpose:  Releases buffers, allocated by rippiMorphAdvInitAlloc
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         Null pointer to pointer to morphological state.
//
//  Arguments:
//    pState               double pointer to morphological state.
*/

LEGACY90IPPAPI(IppStatus, legacy90ippiMorphAdvFree,(IppiMorphAdvState* pState))


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:   ippiMorphGrayInitAlloc_8u_C1R,          ippiMorphGrayInitAlloc_32f_C1R
//
//  Purpose:  Allocate buffers and initialize internal state of gray-scale morphological operation.
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width of image or width or height of structuring
//                             element is less or equal zero.
//    ippStsAnchorErr          Anchor point is outside the structuring element
//    ippStsMemAllocErr        Memory allocation error
//
//  Arguments:
//    roiSize                  Maximal image roiSize in pixels
//    pMask                    Pointer to structuring element (mask)
//    maskSize                 Size of structuring element
//    anchor                   Anchor of the structuring element
//    ppState                  Double pointer to morphological state (InitAlloc)
*/
LEGACY90IPPAPI(IppStatus, legacy90ippiMorphGrayInitAlloc_8u_C1R,(IppiMorphGrayState_8u** ppState, IppiSize roiSize, const Ipp32s* pMask,
                                                   IppiSize maskSize, IppiPoint anchor))

LEGACY90IPPAPI(IppStatus, legacy90ippiMorphGrayInitAlloc_32f_C1R,(IppiMorphGrayState_32f** ppState, IppiSize roiSize, const Ipp32f* pMask,
                                                   IppiSize maskSize, IppiPoint anchor))


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:   ippiMorphGrayFree_8u_C1R,       ippiMorphGrayFree_32f_C1R
//
//  Purpose:  Releases buffers, allocated by rippiMorphGrayInitAlloc
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         Null pointer to pointer to morphological state.
//
//  Arguments:
//    pState                   Double pointer to morphological state.
*/

LEGACY90IPPAPI(IppStatus, legacy90ippiMorphGrayFree_8u_C1R,(IppiMorphGrayState_8u* pState))

LEGACY90IPPAPI(IppStatus, legacy90ippiMorphGrayFree_32f_C1R,(IppiMorphGrayState_32f* pState))


/*F/////////////////////////////////////////////////////////////////////////////////////
//  Name: ippiMorphReconstructGetBufferSize_8u_C1,    ippiMorphReconstructGetBufferSize_16u_C1
//        ippiMorphReconstructGetBufferSize_32f_C1,   ippiMorphReconstructGetBufferSize_64f_C1
//
//  Purpose:   returns buffer size for morphological reconstruction
//
//  Returns:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width or height of images is less or equal zero
//    ippStsStepErr            The steps in images are too small
//    ippStsNotEvenStepErr     Step is not multiple of element.
//
//  Parameters:
//    roiSize                  The maximal ROI size.
//    dataType                 The type of data
//    numChannels              The number of channels
//    p(Buf)Size               The pointer to the buffer size.
//
//  Notes:
//F*/

LEGACY90IPPAPI(IppStatus, legacy90ippiMorphReconstructGetBufferSize_8u_C1, (IppiSize roiSize, int *pSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiMorphReconstructGetBufferSize_16u_C1,(IppiSize roiSize, int *pSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiMorphReconstructGetBufferSize_32f_C1,(IppiSize roiSize, int *pSize))
LEGACY90IPPAPI(IppStatus, legacy90ippiMorphReconstructGetBufferSize_64f_C1,(IppiSize roiSize, int *pSize))


/****************************************************************************************\
*                                   Min/Max Filters                                      *
\****************************************************************************************/


/* ///////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:   ippiFilterMinGetBufferSize_8u_C1R,       ippiFilterMaxGetBufferSize_8u_C1R,
//          ippiFilterMinGetBufferSize_32f_C1R,      ippiFilterMaxGetBufferSize_32f_C1R,
//          ippiFilterMinGetBufferSize_8u_C3R,       ippiFilterMaxGetBufferSize_8u_C3R,
//          ippiFilterMinGetBufferSize_32f_C3R,      ippiFilterMaxGetBufferSize_32f_C3R,
//          ippiFilterMinGetBufferSize_8u_C4R,       ippiFilterMaxGetBufferSize_8u_C4R,
//          ippiFilterMinGetBufferSize_32f_C4R,      ippiFilterMaxGetBufferSize_32f_C4R
//
//  Purpose:    Calculate buffer size for morphology operations with rectangular kernel
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width of the image is less or equal zero
//    ippStsMaskSizeErr        Wrong mask size
//
//  Parameters:
//    roiWidth                 The image ROI width
//    maskSize                 The mask size
//    pBufferSize              The pointer to the buffer size
*/

LEGACY90IPPAPI(IppStatus, legacy90ippiFilterMinGetBufferSize_8u_C1R, (int roiWidth, IppiSize maskSize, int *pBufferSize))

LEGACY90IPPAPI(IppStatus, legacy90ippiFilterMaxGetBufferSize_8u_C1R, (int roiWidth, IppiSize maskSize, int *pBufferSize))

LEGACY90IPPAPI(IppStatus, legacy90ippiFilterMinGetBufferSize_32f_C1R, (int roiWidth, IppiSize maskSize, int *pBufferSize))

LEGACY90IPPAPI(IppStatus, legacy90ippiFilterMaxGetBufferSize_32f_C1R, (int roiWidth, IppiSize maskSize, int *pBufferSize))


LEGACY90IPPAPI(IppStatus, legacy90ippiFilterMinGetBufferSize_8u_C3R, (int roiWidth, IppiSize maskSize, int *pBufferSize))

LEGACY90IPPAPI(IppStatus, legacy90ippiFilterMaxGetBufferSize_8u_C3R, (int roiWidth, IppiSize maskSize, int *pBufferSize))

LEGACY90IPPAPI(IppStatus, legacy90ippiFilterMinGetBufferSize_32f_C3R, (int roiWidth, IppiSize maskSize, int *pBufferSize))

LEGACY90IPPAPI(IppStatus, legacy90ippiFilterMaxGetBufferSize_32f_C3R, (int roiWidth, IppiSize maskSize, int *pBufferSize))


LEGACY90IPPAPI(IppStatus, legacy90ippiFilterMinGetBufferSize_8u_C4R, (int roiWidth, IppiSize maskSize, int *pBufferSize))

LEGACY90IPPAPI(IppStatus, legacy90ippiFilterMaxGetBufferSize_8u_C4R, (int roiWidth, IppiSize maskSize, int *pBufferSize))

LEGACY90IPPAPI(IppStatus, legacy90ippiFilterMinGetBufferSize_32f_C4R, (int roiWidth, IppiSize maskSize, int *pBufferSize))

LEGACY90IPPAPI(IppStatus, legacy90ippiFilterMaxGetBufferSize_32f_C4R, (int roiWidth, IppiSize maskSize, int *pBufferSize))


/*F///////////////////////////////////////////////////////////////////////////////////////
//  Name:   ippiFilterMaxBorderReplicate_8u_C1R,       ippiFilterMinBorderReplicate_8u_C1R,
//          ippiFilterMaxBorderReplicate_32f_C1R,      ippiFilterMinBorderReplicate_32f_C1R
//          ippiFilterMaxBorderReplicate_8u_C3R,       ippiFilterMinBorderReplicate_8u_C3R,
//          ippiFilterMaxBorderReplicate_32f_C3R,      ippiFilterMinBorderReplicate_32f_C3R
//          ippiFilterMaxBorderReplicate_8u_C4R,       ippiFilterMinBorderReplicate_8u_C4R,
//          ippiFilterMaxBorderReplicate_32f_C4R,      ippiFilterMinBorderReplicate_32f_C4R
//
//  Purpose:    Perform morphology operations with rectangular kernel
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width or height of images is less or equal zero
//    ippStsStepErr            The steps in images are too small
//    ippStsNotEvenStepErr     Step is not multiple of element.
//    ippStsMaskSizeErr        Wrong mask size
//    ippStsAnchorErr          Anchor is outside the mask size.
//
//  Parameters:
//    pSrc                     The pointer to the source image
//    srcStep                  The step in the source image
//    pDst                     The pointer to the destination image
//    dstStep                  The step in the destination image
//    roiSize                  The image ROI size
//    maskSize                 The mask size
//    anchor                   The anchor position
//    pBuffer                  The pointer to the working buffer
//F*/

LEGACY90IPPAPI(IppStatus, legacy90ippiFilterMinBorderReplicate_8u_C1R, (const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
                                         IppiSize roiSize, IppiSize maskSize, IppiPoint anchor, Ipp8u *pBuffer))

LEGACY90IPPAPI(IppStatus, legacy90ippiFilterMaxBorderReplicate_8u_C1R, (const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
                                         IppiSize roiSize, IppiSize maskSize, IppiPoint anchor, Ipp8u *pBuffer))

LEGACY90IPPAPI(IppStatus, legacy90ippiFilterMinBorderReplicate_32f_C1R, (const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
                                         IppiSize roiSize, IppiSize maskSize, IppiPoint anchor, Ipp8u *pBuffer))

LEGACY90IPPAPI(IppStatus, legacy90ippiFilterMaxBorderReplicate_32f_C1R, (const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
                                         IppiSize roiSize, IppiSize maskSize, IppiPoint anchor, Ipp8u *pBuffer))


LEGACY90IPPAPI(IppStatus, legacy90ippiFilterMinBorderReplicate_8u_C3R, (const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
                                         IppiSize roiSize, IppiSize maskSize, IppiPoint anchor, Ipp8u *pBuffer))

LEGACY90IPPAPI(IppStatus, legacy90ippiFilterMaxBorderReplicate_8u_C3R, (const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
                                         IppiSize roiSize, IppiSize maskSize, IppiPoint anchor, Ipp8u *pBuffer))

LEGACY90IPPAPI(IppStatus, legacy90ippiFilterMinBorderReplicate_32f_C3R, (const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
                                         IppiSize roiSize, IppiSize maskSize, IppiPoint anchor, Ipp8u *pBuffer))

LEGACY90IPPAPI(IppStatus, legacy90ippiFilterMaxBorderReplicate_32f_C3R, (const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
                                         IppiSize roiSize, IppiSize maskSize, IppiPoint anchor, Ipp8u *pBuffer))


LEGACY90IPPAPI(IppStatus, legacy90ippiFilterMinBorderReplicate_8u_C4R, (const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
                                         IppiSize roiSize, IppiSize maskSize, IppiPoint anchor, Ipp8u *pBuffer))

LEGACY90IPPAPI(IppStatus, legacy90ippiFilterMaxBorderReplicate_8u_C4R, (const Ipp8u* pSrc, int srcStep, Ipp8u* pDst, int dstStep,
                                         IppiSize roiSize, IppiSize maskSize, IppiPoint anchor, Ipp8u *pBuffer))

LEGACY90IPPAPI(IppStatus, legacy90ippiFilterMinBorderReplicate_32f_C4R, (const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
                                         IppiSize roiSize, IppiSize maskSize, IppiPoint anchor, Ipp8u *pBuffer))

LEGACY90IPPAPI(IppStatus, legacy90ippiFilterMaxBorderReplicate_32f_C4R, (const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep,
                                         IppiSize roiSize, IppiSize maskSize, IppiPoint anchor, Ipp8u *pBuffer))


/****************************************************************************************\
*                                   Fixed Filters                                        *
\****************************************************************************************/

/* ///////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:   ippiFilterScharrHorizGetBufferSize_8u16s_C1R,       ippiFilterScharrHorizGetBufferSize_32f_C1R,
//          ippiFilterScharrVertGetBufferSize_8u16s_C1R,        ippiFilterScharrVertGetBufferSize_32f_C1R,
//          ippiFilterSobelNegVertGetBufferSize_8u16s_C1R,      ippiFilterSobelNegVertGetBufferSize_32f_C1R,
//          ippiFilterSobelHorizSecondGetBufferSize_8u16s_C1R,  ippiFilterSobelHorizSecondGetBufferSize_32f_C1R,
//          ippiFilterSobelVertSecondGetBufferSize_8u16s_C1R,   ippiFilterSobelVertSecondGetBufferSize_32f_C1R,
//          ippiFilterSobelCrossGetBufferSize_8u16s_C1R,        ippiFilterSobelCrossGetBufferSize_32f_C1R,
//          ippiFilterLaplacianGetBufferSize_8u16s_C1R,         ippiFilterLaplacianGetBufferSize_32f_C1R,
//          ippiFilterLowpassGetBufferSize_8u_C1R,              ippiFilterLowpassGetBufferSize_32f_C1R,
//          ippiFilterScharrHorizGetBufferSize_8u8s_C1R,
//          ippiFilterScharrVertGetBufferSize_8u8s_C1R,
//          ippiFilterSobelHorizGetBufferSize_8u8s_C1R,
//          ippiFilterSobelVertGetBufferSize_8u8s_C1R,
//          ippiFilterSobelNegVertGetBufferSize_8u8s_C1R,
//          ippiFilterSobelHorizSecondGetBufferSize_8u8s_C1R,
//          ippiFilterSobelVertSecondGetBufferSize_8u8s_C1R,
//          ippiFilterSobelCrossGetBufferSize_8u8s_C1R,
//          ippiFilterLaplacianGetBufferSize_8u8s_C1R,
//
//
//  Purpose:    Perform convolution operation with fixed kernels 3x3 and 5x5
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width of the image is less or equal zero
//    ippStsMaskSizeErr        Wrong mask size
//
//  Parameters:
//    roiSize                  The image ROI size
//    mask                     The mask size
//    pBufferSize              The pointer to the buffer size
*/
LEGACY90IPPAPI(IppStatus, legacy90ippiFilterScharrHorizGetBufferSize_8u16s_C1R,     (IppiSize roiSize, int* pBufferSize))

LEGACY90IPPAPI(IppStatus, legacy90ippiFilterScharrVertGetBufferSize_8u16s_C1R,      (IppiSize roiSize, int* pBufferSize))

LEGACY90IPPAPI(IppStatus, legacy90ippiFilterSobelNegVertGetBufferSize_8u16s_C1R,    (IppiSize roiSize, IppiMaskSize mask, int* pBufferSize))

LEGACY90IPPAPI(IppStatus, legacy90ippiFilterSobelHorizSecondGetBufferSize_8u16s_C1R,(IppiSize roiSize, IppiMaskSize mask, int* pBufferSize))

LEGACY90IPPAPI(IppStatus, legacy90ippiFilterSobelVertSecondGetBufferSize_8u16s_C1R, (IppiSize roiSize, IppiMaskSize mask, int* pBufferSize))


LEGACY90IPPAPI(IppStatus, legacy90ippiFilterScharrHorizGetBufferSize_8u8s_C1R,     (IppiSize roiSize, int* pBufferSize))

LEGACY90IPPAPI(IppStatus, legacy90ippiFilterScharrVertGetBufferSize_8u8s_C1R,      (IppiSize roiSize, int* pBufferSize))

LEGACY90IPPAPI(IppStatus, legacy90ippiFilterSobelHorizGetBufferSize_8u8s_C1R,      (IppiSize roiSize, IppiMaskSize mask, int* pBufferSize))

LEGACY90IPPAPI(IppStatus, legacy90ippiFilterSobelVertGetBufferSize_8u8s_C1R,       (IppiSize roiSize, IppiMaskSize mask, int* pBufferSize))

LEGACY90IPPAPI(IppStatus, legacy90ippiFilterSobelNegVertGetBufferSize_8u8s_C1R,    (IppiSize roiSize, IppiMaskSize mask, int* pBufferSize))

LEGACY90IPPAPI(IppStatus, legacy90ippiFilterSobelHorizSecondGetBufferSize_8u8s_C1R,(IppiSize roiSize, IppiMaskSize mask, int* pBufferSize))

LEGACY90IPPAPI(IppStatus, legacy90ippiFilterSobelVertSecondGetBufferSize_8u8s_C1R, (IppiSize roiSize, IppiMaskSize mask, int* pBufferSize))

LEGACY90IPPAPI(IppStatus, legacy90ippiFilterSobelCrossGetBufferSize_8u8s_C1R,      (IppiSize roiSize, IppiMaskSize mask, int* pBufferSize))

LEGACY90IPPAPI(IppStatus, legacy90ippiFilterLaplacianGetBufferSize_8u8s_C1R,       (IppiSize roiSize, IppiMaskSize mask, int* pBufferSize))


LEGACY90IPPAPI(IppStatus, legacy90ippiFilterScharrHorizGetBufferSize_32f_C1R,     (IppiSize roiSize, int* pBufferSize))

LEGACY90IPPAPI(IppStatus, legacy90ippiFilterScharrVertGetBufferSize_32f_C1R,      (IppiSize roiSize, int* pBufferSize))

LEGACY90IPPAPI(IppStatus, legacy90ippiFilterSobelNegVertGetBufferSize_32f_C1R,    (IppiSize roiSize, IppiMaskSize mask, int* pBufferSize))

LEGACY90IPPAPI(IppStatus, legacy90ippiFilterSobelHorizSecondGetBufferSize_32f_C1R,(IppiSize roiSize, IppiMaskSize mask, int* pBufferSize))

LEGACY90IPPAPI(IppStatus, legacy90ippiFilterSobelVertSecondGetBufferSize_32f_C1R, (IppiSize roiSize, IppiMaskSize mask, int* pBufferSize))


/*F///////////////////////////////////////////////////////////////////////////////////////
//  Name:   ippiFilterScharrHorizBorder_8u16s_C1R,       ippiFilterScharrHorizBorder_32f_C1R
//          ippiFilterScharrVertBorder_8u16s_C1R,        ippiFilterScharrVertBorder_32f_C1R
//          ippiFilterSobelNegVertBorder_8u16s_C1R,      ippiFilterSobelNegVertBorder_32f_C1R
//          ippiFilterSobelHorizSecondBorder_8u16s_C1R,  ippiFilterSobelHorizSecondBorder_32f_C1R
//          ippiFilterSobelVertSecondBorder_8u16s_C1R,   ippiFilterSobelVertSecondBorder_32f_C1R
//          ippiFilterSobelCrossBorder_8u16s_C1R,        ippiFilterSobelCrossBorder_32f_C1R
//          ippiFilterLaplacianBorder_8u16s_C1R,         ippiFilterLaplacianBorder_32f_C1R
//          ippiFilterLowpassBorder_8u_C1R,              ippiFilterLowpassBorder_32f_C1R,
//          ippiFilterScharrHorizBorder_8u8s_C1R,
//          ippiFilterScharrVertBorder_8u8s_C1R,
//          ippiFilterSobelHorizBorder_8u8s_C1R,
//          ippiFilterSobelVertBorder_8u8s_C1R,
//          ippiFilterSobelNegVertBorder_8u8s_C1R,
//          ippiFilterSobelHorizSecondBorder_8u8s_C1R,
//          ippiFilterSobelVertSecondBorder_8u8s_C1R,
//          ippiFilterSobelCrossBorder_8u8s_C1R,
//          ippiFilterLaplacianBorder_8u8s_C1R
//
//  Purpose:    Perform convolution operation with fixed kernels 3x3 and 5x5
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width or height of images is less or equal zero
//    ippStsStepErr            The steps in images are too small
//    ippStsNotEvenStepErr     Step is not multiple of element.
//    ippStsMaskSizeErr        Wrong mask size
//    ippStsBadArgErr          Wrong border type or zero divisor
//
//  Parameters:
//    pSrc                     The pointer to the source image
//    srcStep                  The step in the source image
//    pDst                     The pointer to the destination image
//    dstStep                  The step in the destination image
//    roiSize                  The image ROI size
//    mask                     The mask size
//    borderType               The type of the border
//    borderValue              The value for the constant border
//    pBuffer                  The pointer to the working buffer
//    divisor                  The value to divide output pixels by , (for integer functions)
//F*/

LEGACY90IPPAPI(IppStatus, legacy90ippiFilterScharrHorizBorder_8u16s_C1R, (const Ipp8u* pSrc, int srcStep,
                                      Ipp16s* pDst, int dstStep, IppiSize roiSize,
                                      IppiBorderType borderType, Ipp8u borderValue, Ipp8u* pBuffer))

LEGACY90IPPAPI(IppStatus, legacy90ippiFilterScharrVertBorder_8u16s_C1R, (const Ipp8u* pSrc, int srcStep,
                                      Ipp16s* pDst, int dstStep, IppiSize roiSize,
                                      IppiBorderType borderType, Ipp8u borderValue, Ipp8u* pBuffer))

LEGACY90IPPAPI(IppStatus, legacy90ippiFilterScharrHorizBorder_8u8s_C1R, (const Ipp8u* pSrc, int srcStep,
                                      Ipp8s* pDst, int dstStep, IppiSize roiSize,
                                      IppiBorderType borderType, Ipp8u borderValue, int divisor,
                                      Ipp8u* pBuffer))

LEGACY90IPPAPI(IppStatus, legacy90ippiFilterScharrVertBorder_8u8s_C1R, (const Ipp8u* pSrc, int srcStep,
                                      Ipp8s* pDst, int dstStep, IppiSize roiSize,
                                      IppiBorderType borderType, Ipp8u borderValue, int divisor,
                                      Ipp8u* pBuffer))

LEGACY90IPPAPI(IppStatus, legacy90ippiFilterSobelHorizBorder_8u8s_C1R, (const Ipp8u* pSrc, int srcStep,
                                      Ipp8s* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask,
                                      IppiBorderType borderType, Ipp8u borderValue, int divisor,
                                      Ipp8u* pBuffer))

LEGACY90IPPAPI(IppStatus, legacy90ippiFilterSobelVertBorder_8u8s_C1R, (const Ipp8u* pSrc, int srcStep,
                                      Ipp8s* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask,
                                      IppiBorderType borderType, Ipp8u borderValue, int divisor,
                                      Ipp8u* pBuffer))

LEGACY90IPPAPI(IppStatus, legacy90ippiFilterSobelNegVertBorder_8u8s_C1R, (const Ipp8u* pSrc, int srcStep,
                                      Ipp8s* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask,
                                      IppiBorderType borderType, Ipp8u borderValue, int divisor,
                                      Ipp8u* pBuffer))

LEGACY90IPPAPI(IppStatus, legacy90ippiFilterSobelHorizSecondBorder_8u8s_C1R, (const Ipp8u* pSrc, int srcStep,
                                      Ipp8s* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask,
                                      IppiBorderType borderType, Ipp8u borderValue, int divisor,
                                      Ipp8u* pBuffer))

LEGACY90IPPAPI(IppStatus, legacy90ippiFilterSobelVertSecondBorder_8u8s_C1R, (const Ipp8u* pSrc, int srcStep,
                                      Ipp8s* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask,
                                      IppiBorderType borderType, Ipp8u borderValue, int divisor,
                                      Ipp8u* pBuffer))

LEGACY90IPPAPI(IppStatus, legacy90ippiFilterSobelCrossBorder_8u8s_C1R, (const Ipp8u* pSrc, int srcStep,
                                      Ipp8s* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask,
                                      IppiBorderType borderType, Ipp8u borderValue, int divisor,
                                      Ipp8u* pBuffer))

LEGACY90IPPAPI(IppStatus, legacy90ippiFilterLaplacianBorder_8u8s_C1R, (const Ipp8u* pSrc, int srcStep,
                                      Ipp8s* pDst, int dstStep, IppiSize roiSize, IppiMaskSize mask,
                                      IppiBorderType borderType, Ipp8u borderValue, int divisor,
                                      Ipp8u* pBuffer))


LEGACY90IPPAPI(IppStatus, legacy90ippiFilterScharrHorizBorder_32f_C1R, (const Ipp32f* pSrc, int srcStep,
                                      Ipp32f* pDst, int dstStep, IppiSize roiSize,
                                      IppiBorderType borderType, Ipp32f borderValue, Ipp8u* pBuffer))

LEGACY90IPPAPI(IppStatus, legacy90ippiFilterScharrVertBorder_32f_C1R, (const Ipp32f* pSrc, int srcStep,
                                      Ipp32f* pDst, int dstStep, IppiSize roiSize,
                                      IppiBorderType borderType, Ipp32f borderValue, Ipp8u* pBuffer))



/****************************************************************************************\
*                                 Image Mean and Variance                                *
\****************************************************************************************/


/*F///////////////////////////////////////////////////////////////////////////////////////
//  Name: ippiMean_8u_C1MR,  ippiMean_8s_C1MR,  ippiMean_16u_C1MR,  ippiMean_32f_C1MR,
//        ippiMean_8u_C3CMR, ippiMean_8s_C3CMR, ippiMean_16u_C3CMR, ippiMean_32f_C3CMR
//
//  Purpose:  Find mean value for selected region
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width or height of images is less or equal zero
//    ippStsStepErr            The steps in images are too small
//    ippStsNotEvenStepErr     Step is not multiple of element.
//    ippStsCOIErr             COI index is illegal (coi<1 || coi>3)
//
//  Parameters:
//    pSrc                     Pointer to image
//    srcStep                  Image step
//    pMask                    Pointer to mask image
//    maskStep                 Step in the mask image
//    roiSize                  Size of image ROI
//    coi                      Index of color channel (1..3) (if color image)
//    pMean                    Returned mean value
//
//  Notes:
//F*/

LEGACY90IPPAPI( IppStatus, legacy90ippiMean_8s_C1MR, ( const Ipp8s* pSrc, int srcStep,
                                       const Ipp8u* pMask, int maskStep,
                                       IppiSize roiSize, Ipp64f* pMean ))

LEGACY90IPPAPI( IppStatus, legacy90ippiMean_8s_C3CMR, ( const Ipp8s* pSrc, int srcStep,
                                        const Ipp8u* pMask, int maskStep,
                                        IppiSize roiSize, int coi, Ipp64f* pMean ))


/*F///////////////////////////////////////////////////////////////////////////////////////
//  Name: ippiMean_StdDev_8u_C1R,   ippiMean_StdDev_8s_C1R,
//        ippiMean_StdDev_16u_C1R,  ippiMean_StdDev_32f_C1R,
//        ippiMean_StdDev_8u_C3CR,  ippiMean_StdDev_8s_C3CR,
//        ippiMean_StdDev_16u_C3CR, ippiMean_StdDev_32f_C3CR
//
//  Purpose:  Find mean and standard deviation values for selected region
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width or height of images is less or equal zero
//    ippStsStepErr            The steps in images are too small
//    ippStsNotEvenStepErr     Step is not multiple of element.
//    ippStsCOIErr             COI index is illegal (coi<1 || coi>3)
//
//  Parameters:
//    pSrc                     Pointer to image
//    srcStep                  Image step
//    roiSize                  Size of image ROI
//    coi                      Index of color channel (1..3) (if color image)
//    pMean                    Returned mean value
//    pStdDev                  Returned standard deviation
//
//  Notes:
//F*/

LEGACY90IPPAPI( IppStatus, legacy90ippiMean_StdDev_8s_C1R, ( const Ipp8s* pSrc, int srcStep,
                                             IppiSize roiSize,
                                             Ipp64f* pMean, Ipp64f* pStdDev ))

LEGACY90IPPAPI( IppStatus, legacy90ippiMean_StdDev_8s_C3CR, ( const Ipp8s* pSrc, int srcStep,
                                              IppiSize roiSize, int coi,
                                              Ipp64f* pMean, Ipp64f* pStdDev ))



/*F///////////////////////////////////////////////////////////////////////////////////////
//  Name: ippiMean_StdDev_8u_C1MR,   ippiMean_StdDev_8s_C1MR,
//        ippiMean_StdDev_16u_C1MR,  ippiMean_StdDev_32f_C1MR,
//        ippiMean_StdDev_8u_C3CMR,  ippiMean_StdDev_8s_C3CMR,
//        ippiMean_StdDev_16u_C3CMR, ippiMean_StdDev_32f_C3CMR
//
//  Purpose:  Find mean and standard deviation values for selected region
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width or height of images is less or equal zero
//    ippStsStepErr            The steps in images are too small
//    ippStsNotEvenStepErr     Step is not multiple of element.
//
//  Parameters:
//    pSrc                     Pointer to image
//    srcStep                  Image step
//    pMask                    Pointer to mask image
//    maskStep                 Step in the mask image
//    roiSize                  Size of image ROI
//    coi                      Index of color channel (1..3) (if color image)
//    pMean                    Returned mean value
//    pStdDev                  Returned standard deviation
//
//  Notes:
//F*/

LEGACY90IPPAPI( IppStatus, legacy90ippiMean_StdDev_8s_C1MR, ( const Ipp8s* pSrc, int srcStep,
                                              const Ipp8u* pMask, int maskStep,
                                              IppiSize roiSize,
                                              Ipp64f* pMean, Ipp64f* pStdDev ))

LEGACY90IPPAPI( IppStatus, legacy90ippiMean_StdDev_8s_C3CMR, ( const Ipp8s* pSrc, int srcStep,
                                               const Ipp8u* pMask, int maskStep,
                                               IppiSize roiSize, int coi,
                                               Ipp64f* pMean, Ipp64f* pStdDev ))


/****************************************************************************************\
*                                   Image Minimum and Maximum                            *
\****************************************************************************************/


/*F///////////////////////////////////////////////////////////////////////////////////////
//  Name: ippiMinMaxIndx_8u_C1R,   ippiMinMaxIndx_8s_C1R,
//        ippiMinMaxIndx_16u_C1R,  ippiMinMaxIndx_32f_C1R,
//        ippiMinMaxIndx_8u_C3CR,  ippiMinMaxIndx_8s_C3CR,
//        ippiMinMaxIndx_16u_C3CR, ippiMinMaxIndx_32f_C3CR,
//
//  Purpose:  Finds minimum and maximum values in the image and their coordinates
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width or height of images is less or equal zero
//    ippStsStepErr            The steps in images are too small
//    ippStsNotEvenStepErr     Step is not multiple of element.
//
//  Parameters:
//    pSrc                     Pointer to image
//    srcStep                  Image step
//    roiSize                  Size of image ROI
//    coi                      Index of color channel (1..3) (if color image)
//    pMinVal                  Pointer to minimum value
//    pMaxVal                  Pointer to maximum value
//    pMinIndex                Minimum's coordinates
//    pMaxIndex                Maximum's coordinates
//
//  Notes:
//    Any of output parameters is optional
//F*/

LEGACY90IPPAPI(IppStatus, legacy90ippiMinMaxIndx_8s_C1R,( const Ipp8s* pSrc, int step, IppiSize roiSize,
                                          Ipp32f* pMinVal, Ipp32f* pMaxVal,
                                          IppiPoint* pMinIndex, IppiPoint* pMaxIndex ))

LEGACY90IPPAPI(IppStatus, legacy90ippiMinMaxIndx_8s_C3CR,( const Ipp8s* pSrc, int step, IppiSize roiSize,
                                           int coi, Ipp32f* pMinVal, Ipp32f* pMaxVal,
                                           IppiPoint* pMinIndex, IppiPoint* pMaxIndex ))



/*F///////////////////////////////////////////////////////////////////////////////////////
//  Name: ippiMinMaxIndx_8u_C1MR,   ippiMinMaxIndx_8s_C1MR,
//        ippiMinMaxIndx_16u_C1MR,  ippiMinMaxIndx_32f_C1MR,
//        ippiMinMaxIndx_8u_C3CMR,  ippiMinMaxIndx_8s_C3CMR,
//        ippiMinMaxIndx_16u_C3CMR, ippiMinMaxIndx_32f_C3CMR,
//
//  Purpose:  Finds minimum and maximum values in the image and their coordinates
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width or height of images is less or equal zero
//    ippStsStepErr            The steps in images are too small
//    ippStsNotEvenStepErr     Step is not multiple of element.
//
//  Parameters:
//    pSrc                     Pointer to image
//    srcStep                  Image step
//    pMask                    Pointer to mask image
//    maskStep                 Step in the mask image
//    roiSize                  Size of image ROI
//    coi                      Index of color channel (1..3) (if color image)
//    pMinVal                  Pointer to minimum value
//    pMaxVal                  Pointer to maximum value
//    pMinIndex                Minimum's coordinates
//    pMaxIndex                Maximum's coordinates
//
//  Notes:
//    Any of output parameters is optional
//F*/

LEGACY90IPPAPI( IppStatus, legacy90ippiMinMaxIndx_8s_C1MR, ( const Ipp8s* pSrc, int srcStep,
                                             const Ipp8u* pMask, int maskStep,
                                             IppiSize roiSize,
                                             Ipp32f* pMinVal, Ipp32f* pMaxVal,
                                             IppiPoint* pMinIndex, IppiPoint* pMaxIndex ))

LEGACY90IPPAPI( IppStatus, legacy90ippiMinMaxIndx_8s_C3CMR, ( const Ipp8s* pSrc, int srcStep,
                                              const Ipp8u* pMask, int maskStep,
                                              IppiSize roiSize, int coi,
                                              Ipp32f* pMinVal, Ipp32f* pMaxVal,
                                              IppiPoint* pMinIndex, IppiPoint* pMaxIndex ))



/****************************************************************************************\
*                                     Image Norms                                        *
\****************************************************************************************/


/*F///////////////////////////////////////////////////////////////////////////////////////
//  Names: ippiNorm_Inf_8u_C1MR,       ippiNorm_Inf_8s_C1MR,
//         ippiNorm_Inf_16u_C1MR,      ippiNorm_Inf_32f_C1MR,
//         ippiNorm_Inf_8u_C3CMR,      ippiNorm_Inf_8s_C3CMR,
//         ippiNorm_Inf_16u_C3CMR,     ippiNorm_Inf_32f_C3CMR,
//         ippiNormDiff_Inf_8u_C1MR,   ippiNormDiff_Inf_8s_C1MR,
//         ippiNormDiff_Inf_16u_C1MR,  ippiNormDiff_Inf_32f_C1MR,
//         ippiNormDiff_Inf_8u_C3CMR,  ippiNormDiff_Inf_8s_C3CMR,
//         ippiNormDiff_Inf_16u_C3CMR, ippiNormDiff_Inf_32f_C3CMR,
//         ippiNormRel_Inf_8u_C1MR,    ippiNormRel_Inf_8s_C1MR,
//         ippiNormRel_Inf_16u_C1MR,   ippiNormRel_Inf_32f_C1MR,
//         ippiNormRel_Inf_8u_C3CMR,   ippiNormRel_Inf_8s_C3CMR,
//         ippiNormRel_Inf_16u_C3CMR,  ippiNormRel_Inf_32f_C3CMR,
//
//         ippiNorm_L1_8u_C1MR,        ippiNorm_L1_8s_C1MR,
//         ippiNorm_L1_16u_C1MR,       ippiNorm_L1_32f_C1MR,
//         ippiNorm_L1_8u_C3CMR,       ippiNorm_L1_8s_C3CMR,
//         ippiNorm_L1_16u_C3CMR,      ippiNorm_L1_32f_C3CMR,
//         ippiNormDiff_L1_8u_C1MR,    ippiNormDiff_L1_8s_C1MR,
//         ippiNormDiff_L1_16u_C1MR,   ippiNormDiff_L1_32f_C1MR,
//         ippiNormDiff_L1_8u_C3CMR,   ippiNormDiff_L1_8s_C3CMR,
//         ippiNormDiff_L1_16u_C3CMR,  ippiNormDiff_L1_32f_C3CMR,
//         ippiNormRel_L1_8u_C1MR,     ippiNormRel_L1_8s_C1MR,
//         ippiNormRel_L1_16u_C1MR,    ippiNormRel_L1_32f_C1MR,
//         ippiNormRel_L1_8u_C3CMR,    ippiNormRel_L1_8s_C3CMR,
//         ippiNormRel_L1_16u_C3CMR,   ippiNormRel_L1_32f_C3CMR,
//
//         ippiNorm_L2_8u_C1MR,        ippiNorm_L2_8s_C1MR,
//         ippiNorm_L2_16u_C1MR,       ippiNorm_L2_32f_C1MR,
//         ippiNorm_L2_8u_C3CMR,       ippiNorm_L2_8s_C3CMR,
//         ippiNorm_L2_16u_C3CMR,      ippiNorm_L2_32f_C3CMR,
//         ippiNormDiff_L2_8u_C1MR,    ippiNormDiff_L2_8s_C1MR,
//         ippiNormDiff_L2_16u_C1MR,   ippiNormDiff_L2_32f_C1MR,
//         ippiNormDiff_L2_8u_C3CMR,   ippiNormDiff_L2_8s_C3CMR,
//         ippiNormDiff_L2_16u_C3CMR,  ippiNormDiff_L2_32f_C3CMR,
//         ippiNormRel_L2_8u_C1MR,     ippiNormRel_L2_8s_C1MR,
//         ippiNormRel_L2_16u_C1MR,    ippiNormRel_L2_32f_C1MR,
//         ippiNormRel_L2_8u_C3CMR,    ippiNormRel_L2_8s_C3CMR,
//         ippiNormRel_L2_16u_C3CMR,   ippiNormRel_L2_32f_C3CMR
//
//  Purpose: Calculates ordinary, differential or relative norms of one or two images
//           in an arbitrary image region.
//
//  Returns:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width or height of images is less or equal zero
//    ippStsStepErr            The steps in images are too small
//    ippStsNotEvenStepErr     Step is not multiple of element.
//
//  Parameters:
//    pSrc, pSrc1              Pointers to source and mask images
//    pSrc2, pMask
//    srcStep, src1Step        Their steps
//    src2Step, maskStep
//    roiSize                  Their size or ROI size
//    coi                      COI index (1..3) (if 3-channel images)
//    pNorm                    The pointer to calculated norm
//
//  Notes:
//F*/

/* ///////////////////////////////// 8uC1 flavor ////////////////////////////////////// */

LEGACY90IPPAPI( IppStatus, legacy90ippiNorm_Inf_8s_C1MR, ( const Ipp8s* pSrc, int srcStep,
                                           const Ipp8u* pMask,int maskStep,
                                           IppiSize roiSize, Ipp64f* pNorm ) )

LEGACY90IPPAPI( IppStatus, legacy90ippiNorm_Inf_8s_C3CMR, ( const Ipp8s* pSrc, int srcStep,
                                            const Ipp8u* pMask,int maskStep,
                                            IppiSize roiSize, int coi, Ipp64f* pNorm ) )

LEGACY90IPPAPI( IppStatus, legacy90ippiNormDiff_Inf_8s_C1MR, ( const Ipp8s* pSrc1, int src1Step,
                                               const Ipp8s* pSrc2, int src2Step,
                                               const Ipp8u* pMask, int maskStep,
                                               IppiSize roiSize,   Ipp64f* pNorm ) )

LEGACY90IPPAPI( IppStatus, legacy90ippiNormDiff_Inf_8s_C3CMR, (const Ipp8s* pSrc1, int src1Step,
                                               const Ipp8s* pSrc2, int src2Step,
                                               const Ipp8u* pMask, int maskStep,
                                               IppiSize roiSize, int coi, Ipp64f* pNorm ))

LEGACY90IPPAPI( IppStatus, legacy90ippiNormRel_Inf_8s_C1MR, ( const Ipp8s* pSrc1, int src1Step,
                                              const Ipp8s* pSrc2, int src2Step,
                                              const Ipp8u* pMask, int maskStep,
                                              IppiSize roiSize,   Ipp64f* pNorm ) )

LEGACY90IPPAPI( IppStatus, legacy90ippiNormRel_Inf_8s_C3CMR, ( const Ipp8s* pSrc1, int src1Step,
                                               const Ipp8s* pSrc2, int src2Step,
                                               const Ipp8u* pMask, int maskStep,
                                               IppiSize roiSize, int coi, Ipp64f* pNorm ))

LEGACY90IPPAPI( IppStatus, legacy90ippiNorm_L1_8s_C1MR, ( const Ipp8s* pSrc, int srcStep,
                                          const Ipp8u* pMask,int maskStep,
                                          IppiSize roiSize, Ipp64f* pNorm ) )

LEGACY90IPPAPI( IppStatus, legacy90ippiNorm_L1_8s_C3CMR, ( const Ipp8s* pSrc, int srcStep,
                                           const Ipp8u* pMask,int maskStep,
                                           IppiSize roiSize, int coi, Ipp64f* pNorm ) )


LEGACY90IPPAPI( IppStatus, legacy90ippiNormDiff_L1_8s_C1MR, ( const Ipp8s* pSrc1, int src1Step,
                                              const Ipp8s* pSrc2, int src2Step,
                                              const Ipp8u* pMask, int maskStep,
                                              IppiSize roiSize,   Ipp64f* pNorm ) )


LEGACY90IPPAPI( IppStatus, legacy90ippiNormDiff_L1_8s_C3CMR, ( const Ipp8s* pSrc1, int src1Step,
                                               const Ipp8s* pSrc2, int src2Step,
                                               const Ipp8u* pMask, int maskStep,
                                               IppiSize roiSize, int coi, Ipp64f* pNorm ))

LEGACY90IPPAPI( IppStatus, legacy90ippiNormRel_L1_8s_C1MR, ( const Ipp8s* pSrc1, int src1Step,
                                             const Ipp8s* pSrc2, int src2Step,
                                             const Ipp8u* pMask, int maskStep,
                                             IppiSize roiSize,   Ipp64f* pNorm ) )


LEGACY90IPPAPI( IppStatus, legacy90ippiNormRel_L1_8s_C3CMR, ( const Ipp8s* pSrc1, int src1Step,
                                              const Ipp8s* pSrc2, int src2Step,
                                              const Ipp8u* pMask, int maskStep,
                                              IppiSize roiSize, int coi, Ipp64f* pNorm ))


LEGACY90IPPAPI( IppStatus, legacy90ippiNorm_L2_8s_C1MR, ( const Ipp8s* pSrc, int srcStep,
                                          const Ipp8u* pMask,int maskStep,
                                          IppiSize roiSize, Ipp64f* pNorm ) )

LEGACY90IPPAPI( IppStatus, legacy90ippiNorm_L2_8s_C3CMR, ( const Ipp8s* pSrc, int srcStep,
                                           const Ipp8u* pMask,int maskStep,
                                           IppiSize roiSize, int coi, Ipp64f* pNorm ) )


LEGACY90IPPAPI( IppStatus, legacy90ippiNormDiff_L2_8s_C1MR, ( const Ipp8s* pSrc1, int src1Step,
                                              const Ipp8s* pSrc2, int src2Step,
                                              const Ipp8u* pMask, int maskStep,
                                              IppiSize roiSize,   Ipp64f* pNorm ) )

LEGACY90IPPAPI( IppStatus, legacy90ippiNormDiff_L2_8s_C3CMR, ( const Ipp8s* pSrc1, int src1Step,
                                               const Ipp8s* pSrc2, int src2Step,
                                               const Ipp8u* pMask, int maskStep,
                                               IppiSize roiSize, int coi, Ipp64f* pNorm ))

LEGACY90IPPAPI( IppStatus, legacy90ippiNormRel_L2_8s_C1MR, ( const Ipp8s* pSrc1, int src1Step,
                                             const Ipp8s* pSrc2, int src2Step,
                                             const Ipp8u* pMask, int maskStep,
                                             IppiSize roiSize,   Ipp64f* pNorm ) )

LEGACY90IPPAPI( IppStatus, legacy90ippiNormRel_L2_8s_C3CMR, ( const Ipp8s* pSrc1, int src1Step,
                                              const Ipp8s* pSrc2, int src2Step,
                                              const Ipp8u* pMask, int maskStep,
                                              IppiSize roiSize, int coi, Ipp64f* pNorm ))



/****************************************************************************************\
*                                    Gaussian Pyramids                                   *
\****************************************************************************************/


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiPyrUpGetBufSize_Gauss5x5, ippiPyrDownGetBufSize_Gauss5x5
//
//  Purpose:    Calculates cyclic buffer size for pyramids.
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         The pbufSize pointer is NULL
//    ippStsSizeErr            The value of roiWidth is zerro or negative
//    ippStsDataTypeErr        The dataType is not Ipp8u, Ipp8s or Ipp32f
//    ippStsNumChannensErr     The channels is not 1 or 3
//
//  Arguments:
//    roiWidth                 Width of image ROI in pixels
//    dataType                 Data type of the source image
//    channels                 Number of image channels
//    pbufSize                 Pointer to the variable that return the size of the temporary buffer.
*/

LEGACY90IPPAPI(IppStatus, legacy90ippiPyrUpGetBufSize_Gauss5x5, (int roiWidth, IppDataType dataType,
                                                 int channels, int* bufSize))

LEGACY90IPPAPI(IppStatus, legacy90ippiPyrDownGetBufSize_Gauss5x5, (int roiWidth, IppDataType dataType,
                                                   int channels, int* bufSize))


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiPyrDown_Gauss5x5_8u_C1R, ippiPyrDown_Gauss5x5_8u_C3R,
//              ippiPyrDown_Gauss5x5_8s_C1R, ippiPyrDown_Gauss5x5_8s_C3R,
//              ippiPyrDown_Gauss5x5_32f_C1R, ippiPyrDown_Gauss5x5_32f_C3R,
//
//              ippiPyrUp_Gauss5x5_8u_C1R, ippiPyrUp_Gauss5x5_8u_C3R,
//              ippiPyrUp_Gauss5x5_8s_C1R, ippiPyrUp_Gauss5x5_8s_C3R,
//              ippiPyrUp_Gauss5x5_32f_C1R, ippiPyrUp_Gauss5x5_32f_C3R,
//
//  Purpose:    Perform downsampling/upsampling of the image with 5x5 gaussian.
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width or height of images is less or equal zero.
//    ippStsStepErr            Step is too small to fit image.
//    ippStsNotEvenStepErr     Step is not multiple of element.
//
//  Arguments:
//    pSrc                     Pointer to source image
//    srcStep                  Step in bytes through the source image
//    pDst                     Pointer to destination image
//    dstStep                  Step in bytes through the destination image
//    roiSize                  Size of the source image ROI in pixel. Destination image width and
//                             height will be twice large (PyrUp)
//                             or twice smaller (PyrDown)
//    pBuffer                  Pointer to the the temporary buffer of the size calculated by
//                             ippPyrUpGetSize_Gauss_5x5 or ippPyrDownGetSize_Gauss_5x5
*/

LEGACY90IPPAPI(IppStatus, legacy90ippiPyrUp_Gauss5x5_8u_C1R,  (const Ipp8u* pSrc, int srcStep,
                                               Ipp8u* pDst, int dstStep,
                                               IppiSize roiSize, Ipp8u* pBuffer ))

LEGACY90IPPAPI(IppStatus, legacy90ippiPyrUp_Gauss5x5_8u_C3R,  (const Ipp8u* pSrc, int srcStep,
                                               Ipp8u* pDst, int dstStep,
                                               IppiSize roiSize, Ipp8u* pBuffer ))

LEGACY90IPPAPI(IppStatus, legacy90ippiPyrUp_Gauss5x5_8s_C1R,  (const Ipp8s* pSrc, int srcStep,
                                               Ipp8s* pDst, int dstStep,
                                               IppiSize roiSize, Ipp8u* pBuffer ))

LEGACY90IPPAPI(IppStatus, legacy90ippiPyrUp_Gauss5x5_8s_C3R,  (const Ipp8s* pSrc, int srcStep,
                                               Ipp8s* pDst, int dstStep,
                                               IppiSize roiSize, Ipp8u* pBuffer ))

LEGACY90IPPAPI(IppStatus, legacy90ippiPyrUp_Gauss5x5_32f_C1R, (const Ipp32f* pSrc, int srcStep,
                                               Ipp32f* pDst, int dstStep,
                                               IppiSize roiSize, Ipp8u* pBuffer ))

LEGACY90IPPAPI(IppStatus, legacy90ippiPyrUp_Gauss5x5_32f_C3R, (const Ipp32f* pSrc, int srcStep,
                                               Ipp32f* pDst, int dstStep,
                                               IppiSize roiSize, Ipp8u* pBuffer ))

LEGACY90IPPAPI(IppStatus, legacy90ippiPyrDown_Gauss5x5_8u_C1R,  (const Ipp8u* pSrc, int srcStep,
                                                 Ipp8u* pDst, int dstStep,
                                                 IppiSize roiSize, Ipp8u* pBuffer ))

LEGACY90IPPAPI(IppStatus, legacy90ippiPyrDown_Gauss5x5_8u_C3R,  (const Ipp8u* pSrc, int srcStep,
                                                 Ipp8u* pDst, int dstStep,
                                                 IppiSize roiSize, Ipp8u* pBuffer ))

LEGACY90IPPAPI(IppStatus, legacy90ippiPyrDown_Gauss5x5_8s_C1R,  (const Ipp8s* pSrc, int srcStep,
                                                 Ipp8s* pDst, int dstStep,
                                                 IppiSize roiSize, Ipp8u* pBuffer ))

LEGACY90IPPAPI(IppStatus, legacy90ippiPyrDown_Gauss5x5_8s_C3R,  (const Ipp8s* pSrc, int srcStep,
                                                 Ipp8s* pDst, int dstStep,
                                                 IppiSize roiSize, Ipp8u* pBuffer ))

LEGACY90IPPAPI(IppStatus, legacy90ippiPyrDown_Gauss5x5_32f_C1R, (const Ipp32f* pSrc, int srcStep,
                                                 Ipp32f* pDst, int dstStep,
                                                 IppiSize roiSize, Ipp8u* pBuffer ))

LEGACY90IPPAPI(IppStatus, legacy90ippiPyrDown_Gauss5x5_32f_C3R, (const Ipp32f* pSrc, int srcStep,
                                                 Ipp32f* pDst, int dstStep,
                                                 IppiSize roiSize, Ipp8u* pBuffer ))


/****************************************************************************************\
*                              Correction of Camera Distortions                          *
\****************************************************************************************/


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippibFastArctan
//
//  Purpose:    Given "X" and "Y" images, calculates "angle" image
//              (i.e. atan(y/x)). Resultant angles are in degrees.
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The length is less or equal zero
//
//  Arguments:
//    pSrcY                    Pointer to source "Y" image
//    pSrcX                    Pointer to source "X" image
//    pDst                     Pointer to "angle" image
//    length                   Vector length
//
//  Note:
//    For current version angle precision is ~0.1 degree
*/

LEGACY90IPPAPI(IppStatus, legacy90ippibFastArctan_32f, ( const Ipp32f*  pSrcY, const Ipp32f*  pSrcX,
                                         Ipp32f* pDst, int length ))


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiForegroundHistogramInitAlloc
//
//  Purpose:    Allocates and initializes a state structure for
//              foreground/background segmentation using histograms
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width of the image is less or equal zero
//    ippStsStepErr            The steps in images are too small
//    ippStsBadArgErr          Wrong value of pModel field
//
//  Arguments:
//    pSrc                     Source image
//    srcStep                  Step in source image
//    roiSize                  Source image ROI size.
//    pModel                   Pointer to the structure of the histogram statistical model.
//    pState                   Pointer to the pointer to the segmentation state structure.
*/

LEGACY90IPPAPI(IppStatus, legacy90ippiForegroundHistogramInitAlloc_8u_C1R, ( const Ipp8u* pSrc, int srcStep,
                 IppiSize roiSize, IppFGHistogramModel* pModel, IppFGHistogramState_8u_C1R** pState))

LEGACY90IPPAPI(IppStatus, legacy90ippiForegroundHistogramInitAlloc_8u_C3R, ( const Ipp8u* pSrc, int srcStep,
                 IppiSize roiSize, IppFGHistogramModel* pModel, IppFGHistogramState_8u_C3R** pState))


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiForegroundHistogramFree
//
//  Purpose:    Frees memory allocated for the foreground/background segmentation structure
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//
//  Arguments:
//    pState                   Pointer to the structure to be freed
*/

LEGACY90IPPAPI(IppStatus, legacy90ippiForegroundHistogramFree_8u_C1R, (IppFGHistogramState_8u_C1R* pState))

LEGACY90IPPAPI(IppStatus, legacy90ippiForegroundHistogramFree_8u_C3R, (IppFGHistogramState_8u_C3R* pState))


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiForegroundHistogram
//
//  Purpose:    Calculates foreground mask using histograms
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width of the image is less or equal zero
//    ippStsStepErr            The steps in images are too small
//
//  Arguments:
//    pSrc                     Source image
//    srcStep                  Step in source image
//    pMask                    Pointer to mask
//    maskStep                 Step in the mask image
//    roiSize                  Source image ROI size.
//    pState                   Pointer to the pointer to the segmentation state structure.
*/

LEGACY90IPPAPI(IppStatus, legacy90ippiForegroundHistogram_8u_C1R, (const Ipp8u* pSrc, int srcStep,
       Ipp8u* pMask, int maskStep, IppiSize roiSize, IppFGHistogramState_8u_C1R* pState))

LEGACY90IPPAPI(IppStatus, legacy90ippiForegroundHistogram_8u_C3R, (const Ipp8u* pSrc, int srcStep,
       Ipp8u* pMask, int maskStep, IppiSize roiSize, IppFGHistogramState_8u_C3R* pState))


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiForegroundHistogramUpdate
//
//  Purpose:    Updates histogram statistical model for foreground segmentation
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width of the image is less or equal zero
//    ippStsStepErr            The steps in images are too small
//
//  Arguments:
//    pSrc                     Source image
//    srcStep                  Step in source image
//    pMask                    Pointer to mask
//    maskStep                 Step in the mask image
//    pRef                     Pointer to reference image
//    refStep                  Step in the reference image
//    roiSize                  Source image ROI size.
//    pState                   Pointer to the pointer to the segmentation state structure.
*/

LEGACY90IPPAPI(IppStatus, legacy90ippiForegroundHistogramUpdate_8u_C1R, (const Ipp8u* pSrc, int srcStep,
                  Ipp8u* pMask, int maskStep, Ipp8u* pRef, int refStep, IppiSize roiSize,
                  IppFGHistogramState_8u_C1R* pState))

LEGACY90IPPAPI(IppStatus, legacy90ippiForegroundHistogramUpdate_8u_C3R, (const Ipp8u* pSrc, int srcStep,
                  Ipp8u* pMask, int maskStep, Ipp8u* pRef, int refStep, IppiSize roiSize,
                  IppFGHistogramState_8u_C3R* pState))


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiForegroundGaussianInitAlloc
//
//  Purpose:    Allocates and initializes a state structure for
//              foreground/background segmentation using Gaussian mixtures
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width of the image is less or equal zero
//    ippStsStepErr            The steps in images are too small
//
//  Arguments:
//    pSrc                     Source image
//    srcStep                  Step in source image
//    roiSize                  Source image ROI size.
//    pModel                   Pointer to the structure of the gaussian model.
//    pState                   Pointer to the pointer to the segmentation state structure.
*/

LEGACY90IPPAPI(IppStatus, legacy90ippiForegroundGaussianInitAlloc_8u_C1R, ( const Ipp8u* pSrc, int srcStep,
                 IppiSize roiSize, IppFGGaussianModel* pModel, IppFGGaussianState_8u_C1R** pState))

LEGACY90IPPAPI(IppStatus, legacy90ippiForegroundGaussianInitAlloc_8u_C3R, ( const Ipp8u* pSrc, int srcStep,
                 IppiSize roiSize, IppFGGaussianModel* pModel, IppFGGaussianState_8u_C3R** pState))


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiForegroundGaussianFree
//
//  Purpose:    Frees memory allocated for the foreground/background segmentation structure
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//
//  Arguments:
//    pState                   Pointer to the structure to be freed
*/

LEGACY90IPPAPI(IppStatus, legacy90ippiForegroundGaussianFree_8u_C1R, (IppFGGaussianState_8u_C1R* pState))

LEGACY90IPPAPI(IppStatus, legacy90ippiForegroundGaussianFree_8u_C3R, (IppFGGaussianState_8u_C3R* pState))


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiForegroundGaussian
//
//  Purpose:    Calculates foreground mask using Gaussians
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         One of pointers is NULL
//    ippStsSizeErr            The width of the image is less or equal zero
//    ippStsStepErr            The steps in images are too small
//
//  Arguments:
//    pSrc                     Source image
//    srcStep                  Step in source image
//    pRef                     Pointer to reference image
//    refStep                  Step in the reference image
//    pDst                     Pointer to destination image
//    dstStep                  Step in the destination image
//    roiSize                  Source image ROI size.
//    pState                   Pointer to the pointer to the segmentation state structure.
*/

LEGACY90IPPAPI(IppStatus, legacy90ippiForegroundGaussian_8u_C1R, (const Ipp8u* pSrc, int srcStep,
                  Ipp8u* pRef, int refStep, Ipp8u* pDst, int dstStep,
                  IppiSize roiSize, IppFGGaussianState_8u_C1R* pState))

LEGACY90IPPAPI(IppStatus, legacy90ippiForegroundGaussian_8u_C3R, (const Ipp8u* pSrc, int srcStep,
                  Ipp8u* pRef, int refStep, Ipp8u* pDst, int dstStep,
                  IppiSize roiSize, IppFGGaussianState_8u_C3R* pState))


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiFilterGaussianGetBufferSize
//
//  Purpose:    Computes the size of the working buffer for the Gaussian filter
//
//  Return:
//     ippStsNoErr          Ok. Any other value indicates an error or a warning.
//     ippStsNullPtrErr     One of the pointers is NULL.
//     ippStsSizeErr        maxRoiSize  has a field with zero or negative value.
//     ippStsDataTypeErr    Indicates an error when dataType has an illegal value.
//     ippStsBadArgErr      Indicates an error if kernelSize is even or is less than 3.
//     ippStsChannelErr     Indicates an error when numChannels has an illegal value.
//
//  Arguments:
//     maxRoiSize           Maximal size of the image ROI in pixels.
//     kernelSize           Size of the Gaussian kernel (odd, greater or equal to 3).
//     dataType             Data type of the source and destination images.
//     numChannels          Number of channels in the images. Possible values are 1 and 3.
//     pSpecSize            Pointer to the computed size (in bytes) of the Gaussian
//                            specification structure.
//     pBufferSize          Pointer to the computed size (in bytes) of the external buffer.
*/
LEGACY90IPPAPI(IppStatus, legacy90ippiFilterGaussGetBufferSize_32f_C1R,(IppiSize roiSize, int KernelSize,
                                                 int* pBufferSize))


/* ///////////////////////////////////////////////////////////////////////////////////////
//  Name:       ippiFilterGaussianBorder
//
//  Purpose:    Applies Gaussian filter with borders
//
//  Return:
//     ippStsNoErr      Ok. Any other value indicates an error or a warning.
//     ippStsNullPtrErr One of the specified pointers is NULL.
//     ippStsSizeErr    roiSize has a field with zero or negative value.
//     ippStsStepErr    Indicates an error condition if srcStep or dstStep is less
//                        than  roiSize.width * <pixelSize>.
//     ippStsNotEvenStepErr One of the step values is not divisible by 4 for floating-point images.
//     ippStsBadArgErr  kernelSize is less than 3 or sigma is less or equal than 0.
//
//  Arguments:
//     pSrc             Pointer to the source image ROI.
//     srcStep          Distance in bytes between starts of consecutive lines in the source image.
//     pDst             Pointer to the destination image ROI.
//     dstStep          Distance in bytes between starts of consecutive lines in the destination image.
//     roiSize          Size of the source and destination image ROI.
//     pSpec            Pointer to the Gaussian specification structure.
//     pBuffer          Pointer to the working buffer.
*/

LEGACY90IPPAPI(IppStatus, legacy90ippiFilterGaussBorder_32f_C1R, (const Ipp32f* pSrc, int srcStep,Ipp32f* pDst, int dstStep,
                                           IppiSize roiSize,int KernelSize,Ipp32f sigma,
                                           IppiBorderType borderType, Ipp32f borderValue,
                                           Ipp8u* pBuffer))


/* ///////////////////////////////////////////////////////////////////////////
//  Name:    ippiSRHNInitAlloc_PSF3x3,
//           ippiSRHNInitAlloc_PSF2x2
//
//  Purpose: Allocate and initialize the internal PSF structures.
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         Some of the pointers are NULL
//    ippStsSizeErr            The table size is incorrect
//  Arguments:
//    ppPSF                    Double pointer to the created PSF structure
//    pcTab                    Input table with coefficients
//    tabSize                  The number of elements in cTab
*/

LEGACY90IPPAPI(IppStatus, legacy90ippiSRHNInitAlloc_PSF3x3, (IppiSRHNSpec_PSF3x3** ppPSF,
   const Ipp32f pcTab[][9], int tabSize))

LEGACY90IPPAPI(IppStatus, legacy90ippiSRHNInitAlloc_PSF2x2, (IppiSRHNSpec_PSF2x2** ppPSF,
   const Ipp32f pcTab[][4], int tabSize))


/* ///////////////////////////////////////////////////////////////////////////
//  Name:    ippiSRHNFree_PSF3x3,
//           ippiSRHNFree_PSF2x2
//
//  Purpose: Deallocate PSF structures
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         The PSF pointer is NULL
//  Arguments:
//    pPSF                     Pointer to the PSF structure
*/

LEGACY90IPPAPI(IppStatus, legacy90ippiSRHNFree_PSF3x3, (IppiSRHNSpec_PSF3x3* pPSF))
LEGACY90IPPAPI(IppStatus, legacy90ippiSRHNFree_PSF2x2, (IppiSRHNSpec_PSF2x2* pPSF))


/* ///////////////////////////////////////////////////////////////////////////
//  Name:    ippiSRHNCalcResidual_PSF3x3_8u32f_C1R,
//           ippiSRHNCalcResidual_PSF2x2_8u32f_C1R,
//           ippiSRHNCalcResidual_PSF3x3_16u32f_C1R,
//           ippiSRHNCalcResidual_PSF2x2_16u32f_C1R,
//
//  Purpose: Compute residuals for likelihood part of target function
//           or its gradient.
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         Some of the pointers are NULL's
//    ippStsSizeErr            The length is negative
//    ippStsStepErr            hiResStep is too small or
//                             is not divisible by sizeof(pHiRes[0])
//
//  Arguments:
//    pHiRes                   Pointer to high-resolution image
//    pHiResStep               High-resolution image step
//    pLowRes                  Array of pixel values from low-resolution images
//    pOfs                     Array of offsets of 3x3 areas
//                             inside high-resolution image
//    pCoeff                   Array of indices in cTab
//    pResidual                The output array
//    len                      Length of lowRes, ofs, coeff and residual arrays
//    pPSF                     Previously created PSF table for 3x3 or 2x2 case
*/

LEGACY90IPPAPI(IppStatus, legacy90ippiSRHNCalcResidual_PSF3x3_8u32f_C1R, (
    const Ipp32f* pHiRes, int hiResStep, const Ipp8u* pLowRes,
    const Ipp32s* pOfs, const Ipp16u* pCoeff, Ipp32f* pResidual,
    int len, const IppiSRHNSpec_PSF3x3* pPSF))

LEGACY90IPPAPI(IppStatus, legacy90ippiSRHNCalcResidual_PSF2x2_8u32f_C1R, (
    const Ipp32f* pHiRes, int hiResStep, const Ipp8u* pLowRes,
    const Ipp32s* pOfs, const Ipp16u* pCoeff, Ipp32f* pResidual,
    int len, const IppiSRHNSpec_PSF2x2* pPSF))

LEGACY90IPPAPI(IppStatus, legacy90ippiSRHNCalcResidual_PSF3x3_16u32f_C1R, (
    const Ipp32f* pHiRes, int hiResStep, const Ipp16u* pLowRes,
    const Ipp32s* pOfs, const Ipp16u* pCoeff, Ipp32f* pResidual,
    int len, const IppiSRHNSpec_PSF3x3* pPSF))

LEGACY90IPPAPI(IppStatus, legacy90ippiSRHNCalcResidual_PSF2x2_16u32f_C1R, (
    const Ipp32f* pHiRes, int hiResStep, const Ipp16u* pLowRes,
    const Ipp32s* pOfs, const Ipp16u* pCoeff, Ipp32f* pResidual,
    int len, const IppiSRHNSpec_PSF2x2* pPSF))


/* ///////////////////////////////////////////////////////////////////////////
//  Name:    ippiSRHNUpdateGradient_PSF3x3_32f_C1R,
//           ippiSRHNUpdateGradient_PSF2x2_32f_C1R
//
//  Purpose: Update likelihood part of the gradient
//
//  Return:
//    ippStsNoErr              Ok
//    ippStsNullPtrErr         Some of the pointers are NULL's
//    ippStsSizeErr            Length is negative
//    ippStsStepErr            gradStep is too small or is not divisible by
//                             sizeof(pGrad[0])
//
//  Arguments:
//    pGrad                    Pointer to gradient
//    gradStep                 The gradient step
//    pOfs                     Array of offsets of 3x3 or 2x2 areas
//                             inside high-resolution image
//    pCoeff                   Array of indices in cTab
//    pRob                     Array of computed robust function derivatives
//    pWeight                  Array of indices in wTab
//    len                      Length of ofs, coeff, errVal and weight arrays
//    pwTab                    A table of confidence weights
//    pPSF                     PSF structure
*/

LEGACY90IPPAPI(IppStatus, legacy90ippiSRHNUpdateGradient_PSF3x3_32f_C1R, (
    Ipp32f* pGrad, int gradStep, const Ipp32s* pOfs, const Ipp16u* pCoeff,
    const Ipp32f* pRob, const Ipp8u* pWeight, int len, const Ipp32f* pwTab,
    const IppiSRHNSpec_PSF3x3* pPSF))

LEGACY90IPPAPI(IppStatus, legacy90ippiSRHNUpdateGradient_PSF2x2_32f_C1R, (
    Ipp32f* pGrad, int gradStep, const Ipp32s* pOfs, const Ipp16u* pCoeff,
    const Ipp32f* pRob, const Ipp8u* pWeight, int len, const Ipp32f* pwTab,
    const IppiSRHNSpec_PSF2x2* pPSF))


#if defined __cplusplus
}
#endif

#endif /* __IPPCV_90_LEGACY_H__ */
