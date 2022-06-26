//#define STRICT

#include <windows.h>
#include <windowsx.h>
#include "../vsclient/VS_PlayAvi.h"

#define ClassName	"DX_Window"
#define AppName		"DirectDraw Application"

//����������� ��������� Windows
BOOL DX_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);
void DX_OnDestroy(HWND hwnd);
void DX_OnActivate(HWND hwnd, UINT state, HWND hwndActDeact, BOOL fMinimized);
void DX_OnKey(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags);
void DX_OnIdle(HWND hwnd);

//������� ���������
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);


//���� ���������� ������ ����������
BOOL	bActive=FALSE;

extern 	RECT rPic;
extern 	RECT rPic2;
int frame = 0;

VS_PlayAviFile g_pavi;


extern "C" {
__declspec(dllexport) int  PlayAvi(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpszCmdLine, int nCmdShow)
{
	WNDCLASSEX wndClass;
	HWND hWnd;
	MSG msg;

	//����������� �������� ������
	wndClass.cbSize       =sizeof(wndClass);
	wndClass.style        =CS_HREDRAW|CS_VREDRAW;
	wndClass.lpfnWndProc  =WndProc;
	wndClass.cbClsExtra   =0;
	wndClass.cbWndExtra   =0;
	wndClass.hInstance    =hInst;
	wndClass.hIcon        =LoadIcon(NULL,IDI_WINLOGO);
	wndClass.hCursor      =LoadCursor(NULL,IDC_ARROW);
	wndClass.hbrBackground=NULL;//�������� ��������!!!
	wndClass.lpszMenuName =NULL;
	wndClass.lpszClassName=ClassName;
	wndClass.hIconSm      =LoadIcon(NULL,IDI_WINLOGO);

	RegisterClassEx(&wndClass);

	//�������� ���� �� ������ ������
	hWnd=CreateWindowEx(
		WS_EX_TOPMOST,	//�������������� ����� ����
		ClassName,		//����� ����
		AppName,		//����� ���������
		WS_POPUP|WS_CAPTION|WS_MINIMIZEBOX|WS_SYSMENU|WS_SIZEBOX|WS_MAXIMIZEBOX,	//����� ����
		10,10,			//���������� X � Y
		320,
		240,		//������ � ������
		NULL,			//���������� ������������� ����
		NULL,			//���������� ����
		hInst,			//��������� ����������
		NULL);			//�������������� ������

	ShowWindow(hWnd,nCmdShow);
	UpdateWindow(hWnd);

	//������������� �����������, ��������� � DirectDraw
	VS_AudioDeviceManager::Open(hWnd);
	g_pavi.Init(lpszCmdLine, hWnd, hWnd, 0);
	g_pavi.Start();

	//������� ���� ���������
	while (TRUE) {
		if (PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
			if (msg.message==WM_QUIT)
				break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
			if (bActive)//������ ���� ���������� �������!
			{
				DX_OnIdle(hWnd);
			}
	}
	return (msg.wParam);
}
}

/////////////////////////////////////////////////
//	������� ���������
////////////////////////////////////////////////

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg,
						 WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		HANDLE_MSG(hWnd,WM_CREATE,DX_OnCreate);
		HANDLE_MSG(hWnd,WM_DESTROY,DX_OnDestroy);
		HANDLE_MSG(hWnd,WM_ACTIVATE,DX_OnActivate);
		HANDLE_MSG(hWnd,WM_KEYDOWN,DX_OnKey);
	default:
		return DefWindowProc(hWnd,msg,wParam,lParam);
	}
}

/* ����������� ��������� */

//----------------------------------------------------
BOOL DX_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
	return (TRUE);
}

//----------------------------------------------------
void DX_OnDestroy(HWND hwnd)
{
	g_pavi.Release();
	VS_AudioDeviceManager::Close();
	PostQuitMessage(0);
}


//----------------------------------------------------
void DX_OnIdle(HWND hwnd)
{
	Sleep(10);
}

int timeShift = 0;
//----------------------------------------------------
void DX_OnKey(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
	//��� ������� ������� ���������� ������ ���������
	if (vk==VK_SPACE)
		DestroyWindow(hwnd);
	else if (vk==VK_UP) {
		if (timeShift < 1000)
			timeShift+=50;
		g_pavi.SetTimeShift(timeShift);
	}
	else if (vk==VK_DOWN) {
		if (timeShift > -1000)
			timeShift-=50;
		g_pavi.SetTimeShift(timeShift);
	}
}

//----------------------------------------------------
void DX_OnActivate(HWND hwnd, UINT state, HWND hwndActDeact, BOOL fMinimized)
{
	//�������� ���� ��������� ����������
	bActive =! fMinimized;
}
//----------------------------------------------------
