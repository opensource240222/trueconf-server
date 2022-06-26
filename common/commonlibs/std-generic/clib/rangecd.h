/**
 **************************************************************************
 * \file rangecd.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief Range coder/decoder functions daclaration
 *
 * \b Project Standart Libraries
 * \author Dm.Vatolin dmitriy@compression.ru
 * \date 22.11.02
 *
 * $Revision: 1 $
 *
 * $History: rangecd.h $
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/std/clib
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/std/clib
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 30.12.04   Time: 14:12
 * Updated in $/VS/std/clib
 * files headers
 ****************************************************************************/
#ifndef _RANGECD_H_
#define _RANGECD_H_


/****************************************************************************
 * Includes
 ****************************************************************************/
#include <stddef.h>

#if defined __cplusplus /* Begin "C" */
extern "C" {
#endif /* __cplusplus */

/////////////// Range Coders ///////////////
size_t RCDV_Encode     (const void* pSRC, size_t Size, void* pDST);
size_t RCDV_Decode     (const void* pSRC, void* pDST, size_t Size);
size_t RCDV_DecodeSafe (const void* pSRC, void* pDST, size_t SizeDst, size_t SizeSrc);

#if defined __cplusplus /* End "C" */
}
#endif /* __cplusplus */

#endif // _RANGECD_H_
