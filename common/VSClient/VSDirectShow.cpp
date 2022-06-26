/**
 **************************************************************************
 * \file VSDirectShow.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief Общие функции используемые для работы с DirectShow
 *
 * \b Project Client
 * \author Melechko Ivan
 * \date 07.10.02
 *
 * $Revision: 3 $
 *
 * $History: VSDirectShow.cpp $
 *
 * *****************  Version 3  *****************
 * User: Sanufriev    Date: 24.07.12   Time: 15:21
 * Updated in $/VSNA/VSClient
 * - ench direct show : were add hw h.264 description mode
 *
 * *****************  Version 2  *****************
 * User: Dront78      Date: 28.10.09   Time: 13:57
 * Updated in $/VSNA/VSClient
 * - directx dshow headers changed
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/VSClient
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/VSClient
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 17.12.04   Time: 20:22
 * Updated in $/VS/VSClient
 * header to files
 ****************************************************************************/

/****************************************************************************
 * Includes
 ****************************************************************************/
#include <atlbase.h>
#include <dshow.h>

/****************************************************************************
 * Functions
 ****************************************************************************/
/**
* Функция ищет в фильтре пин данных необходимой направленности с заданным номером
* \param pFilter       [IN] Указатель на фильтр
* \param PinNum        [IN] Номер необходимого пина исходящих данных
* \param PinDir        [IN] Направление пина (PINDIR_INPUT или PINDIR_OUTPUT)
* \return Указатель на найденный пин, или NULL если такой пин отсутствует
*/
IPin * GetPin( IBaseFilter * pFilter, int PinNum ,PIN_DIRECTION PinDir)
{
	IEnumPins * pEnum = 0;
	HRESULT hr = pFilter->EnumPins( &pEnum );
	if(hr!=NOERROR){
		return NULL;
	}
	hr=pEnum->Reset( );
	if(hr!=NOERROR){
		pEnum->Release( );
		return NULL;
	}
	ULONG Fetched;
	do {
		Fetched = 0;
		IPin * pPin = 0;
		hr=pEnum->Next( 1, &pPin, &Fetched );
		if(hr!=NOERROR){
			Fetched = 0;
		}
		if( Fetched ) {
			PIN_DIRECTION pd;
			hr=pPin->QueryDirection( &pd);
			pPin->Release( );
			if( pd == PinDir ) {
				if( PinNum == 0 ) {
					pEnum->Release( );
					return pPin;
				}
				PinNum--;
			}
		}
	}
	while( Fetched );
	pEnum->Release( );
	return NULL;
}

HRESULT GetPinCategory(IPin *pPin, GUID *pPinCategory)
{
    IKsPropertySet *pKs = NULL;

    HRESULT hr = pPin->QueryInterface(IID_PPV_ARGS(&pKs));
    if (FAILED(hr))
    {
        return hr;
    }

    // Try to retrieve the pin category.
    DWORD cbReturned = 0;
    hr = pKs->Get(AMPROPSETID_Pin, AMPROPERTY_PIN_CATEGORY, NULL, 0,
        pPinCategory, sizeof(GUID), &cbReturned);

    // If this succeeded, pPinCategory now contains the category GUID.

	if (pKs) pKs->Release(); pKs = NULL;
    return hr;
}

void DestroySubgraph(IFilterGraph *pGraph, IBaseFilter *pFilt)
{
	IEnumPins *pEnum;
	if (!pFilt) return;
	if ( SUCCEEDED(pFilt->EnumPins(&pEnum)) ) {
		IPin *pPin;
		pEnum->Reset();
		for(;;) {
			HRESULT hr = pEnum->Next(1, &pPin, 0);
			if (hr == VFW_E_ENUM_OUT_OF_SYNC) {
				hr = pEnum->Reset();
				if ( SUCCEEDED(hr) ) continue;
			}
			if (hr != S_OK) break;
			PIN_DIRECTION dir;
			hr = pPin->QueryDirection(&dir);
			if ( SUCCEEDED(hr) ) {
				if (dir == PINDIR_OUTPUT) {
					IPin *pPin2;
					if ( SUCCEEDED(pPin->ConnectedTo(&pPin2)) ) {
						PIN_INFO pi;
						if ( SUCCEEDED(pPin2->QueryPinInfo(&pi)) ) {
							DestroySubgraph(pGraph, pi.pFilter);
							pGraph->RemoveFilter(pi.pFilter);
							pi.pFilter->Release();
						}
						pPin2->Release();
					}
				}
			}
			pPin->Release();
		}
		pEnum->Release();
	}
}

