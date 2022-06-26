// PowerPointLib.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"


#ifdef _MANAGED
#pragma managed(push, off)
#endif
#ifdef __cplusplus
extern "C" {
#endif
	BOOL APIENTRY DllMain( HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
	{
		return TRUE;
	}
#define BUFSIZE 4096
//-----------
	unsigned MaxImageDimension = 1920;
	__declspec(dllexport)void SetMaxImageDimension(unsigned newMaxDimension)
	{
		MaxImageDimension = newMaxDimension;
	}
//-----------
__declspec(dllexport)void DeletePresentationW(wchar_t*szPresentationName)
	{
		DWORD dwBufSize=BUFSIZE;
		wchar_t lpPathBuffer[BUFSIZE];
		wchar_t *lpName;
		if(GetFullPathNameW(szPresentationName,dwBufSize,lpPathBuffer,&lpName)){
			(lpName)[0]=0;
			wcscat(lpPathBuffer,L"*.*");
			(lpName)[4]=0;
			(lpName)[5]=0;
			SHFILEOPSTRUCTW os;
			memset(&os,0,sizeof(SHFILEOPSTRUCTW));
			os.wFunc=FO_DELETE;
			os.pFrom=lpPathBuffer;
			os.fFlags=FOF_NOCONFIRMATION|FOF_NOERRORUI|FOF_SILENT;
			SHFileOperationW(&os);
			(lpName-1)[0]=0;
			RemoveDirectoryW(lpPathBuffer);		
		}
	};

//-----------
	__declspec(dllexport)void DeletePresentation(char*szPresentationName)
	{
		int buf_size = MultiByteToWideChar(CP_ACP,0,szPresentationName,-1,0,0);
		wchar_t* wPresentationName = (wchar_t*)malloc(buf_size * sizeof(wchar_t));
		MultiByteToWideChar(CP_ACP,0,szPresentationName,-1,wPresentationName,buf_size);
		DeletePresentationW(wPresentationName);
		free(wPresentationName);
	};
	__declspec(dllexport)BOOL IsPPPresent()
	{
		HKEY key;
		if(RegOpenKeyEx(HKEY_CLASSES_ROOT,L"TypeLib\\{91493440-5A91-11CF-8700-00AA0060263B}",0,KEY_READ,&key)==ERROR_SUCCESS){
			RegCloseKey(key);
			return TRUE;
		}
		return FALSE;
	};
//----------
__declspec(dllexport)int GetPresentationW(wchar_t *szPPTname,wchar_t*szPresentationName)
	{

		DWORD dwBufSize=BUFSIZE;
		wchar_t lpPathBuffer[BUFSIZE];
		wchar_t szTempName[BUFSIZE];

		// Get the temp path

		GetTempPathW(dwBufSize,   // length of the buffer
			lpPathBuffer);      // buffer for path 

		// Create a temporary file. 

		GetTempFileNameW(lpPathBuffer, // directory for temp files 
			L"SS_EXPORT",                    // temp file name prefix 
			0,                        // create unique name 
			szTempName);              // buffer for name 
		DeleteFileW(szTempName);
		if (CreateDirectoryW((LPCWSTR) szTempName,NULL)==0) 
		{ 				
			return -1;
		} 
		OleInitialize(NULL);
		PowerPoint::_ApplicationPtr objPPTApp(L"PowerPoint.Application");
		if(objPPTApp==NULL){
			return -2;
		}
		PowerPoint::_PresentationPtr ppt;
		try {
			ppt=objPPTApp->Presentations->Open(szPPTname,Office::msoTrue, Office::msoTrue, Office::msoFalse );
		}
		catch (...) {
			return -3;
		}
		if(ppt){
			float wp = ppt->PageSetup->GetSlideWidth();
			float hp = ppt->PageSetup->GetSlideHeight();
			float koef = hp > wp ? (float)MaxImageDimension / hp : (float)MaxImageDimension / wp;
			int iw = (int)(wp*koef+0.5);
			int ih = (int)(hp*koef+0.5);
			int n=wcslen(szTempName);
			szTempName[n]=L'\\';
			szTempName[n+1]=0;
			for(long i=1;i<=ppt->Slides->Count;i++){
				_variant_t in=i;
				wchar_t szFileName[BUFSIZE];
				swprintf(szFileName, BUFSIZE, L"%sSlide %03d.jpg", szTempName, i);
				//Важно! использовать только long (VT_I4)		 
				PowerPoint::_SlidePtr sp= ppt->Slides->Item(in);
				sp->Export(szFileName, L"jpg", iw, ih);
			}
			ppt->Close();	  
			//  objPPTApp->Release();
			_bstr_t tmp_path=szTempName;	        
			_bstr_t bt=tmp_path;
			bt+=L"slideshow.vpl";
			FILE *f=_wfopen((wchar_t*)bt,L"wb");
			wcscpy(szPresentationName,(wchar_t*)bt);
			if(f){
				WIN32_FIND_DATAW fd;
				memset(&fd,0, sizeof(WIN32_FIND_DATAW));
				wcscat(szTempName,L"*.jpg");	  
				fputws(L"VSPL00\n",f);
				fputws(L"\n",f);
				HANDLE h=FindFirstFileW(szTempName,&fd);
				if(h!=INVALID_HANDLE_VALUE){

					do{
						_bstr_t bt=tmp_path;//fd.			  
						bt+=fd.cFileName;
						bt+=L"\n";
						fputws(bt,f);
						//fd.
					}while(FindNextFileW(h,&fd));

					FindClose(h);
					fclose(f);
				}

			}
			return 0;
		}
		//objPPTApp->Release();
		return -3;
	};
//----------
	__declspec(dllexport)int GetPresentation(char *szPPTname,char*szPresentationName)
	{
		wchar_t* wPresentationName = (wchar_t*)malloc(BUFSIZE * sizeof(wchar_t));

		int buf_size = MultiByteToWideChar(CP_ACP,0,szPPTname,-1,0,0);
		wchar_t* wPPTname = (wchar_t*)malloc(buf_size * sizeof(wchar_t));
		MultiByteToWideChar(CP_ACP,0,szPPTname,-1,wPPTname,buf_size);

		int RetValue = GetPresentationW(wPPTname,wPresentationName);

		buf_size = WideCharToMultiByte(CP_ACP,0,wPresentationName,-1,0,0,0,0);
		WideCharToMultiByte(CP_ACP,0,wPresentationName,-1,szPresentationName,buf_size,0,0);

		free(wPPTname);
		free(wPresentationName);
		return RetValue;
	};
#ifdef __cplusplus
}
#endif

#ifdef _MANAGED
#pragma managed(pop)
#endif

