/**
 **************************************************************************
 * \file VSDirectShow.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief ќбъ€влени€ общих функций используемых дл€ работы с DirectShow
 *
 * \b Project Client
 * \author Melechko Ivan
 * \date 07.10.02
 *
 * $Revision: 2 $
 *
 * $History: VSDirecShow.h $
 *
 * *****************  Version 2  *****************
 * User: Sanufriev    Date: 24.07.12   Time: 15:21
 * Updated in $/VSNA/VSClient
 * - ench direct show : were add hw h.264 description mode
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/VSClient
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/VSClient
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 17.12.04   Time: 20:22
 * Updated in $/VS/VSClient
 * header to files
 ****************************************************************************/
#ifndef _VSDIRECTSHOW_
#define _VSDIRECTSHOW_

/****************************************************************************
 * Functions
 ****************************************************************************/
/**
 **************************************************************************
 * \brief get Pin of DirectShow filter
 ****************************************************************************/
IPin * GetPin( IBaseFilter * pFilter, int PinNum,PIN_DIRECTION PinDir );
HRESULT GetPinCategory(IPin *pPin, GUID *pPinCategory);
void DestroySubgraph(IFilterGraph *pGraph, IBaseFilter *pFilt);

#endif