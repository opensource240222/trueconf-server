// _ArmMakeKeys.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <cstdlib>
#include <ctime>
#include <memory>
#include <fstream>
#include <windows.h>

int _tmain(int argc, _TCHAR* argv[])
{
	srand(static_cast<unsigned int>(time(0)));
	char sym[128];
	memset(sym,0,128);
	int i,j,n=0;
	for(i='a';i<='z';i++)
	{
		sym[n++]=i;
		
	}
	for(i='A';i<='Z';i++)
	{
		sym[n++]=i;
		
	}
	for(i='0';i<='9';i++)
	{
		sym[n++]=i;
		
	}

	FILE *f(0);
	fopen_s(&f, "arm_prepared_keys.txt", "w+b");
	char arr[256];
	for (int i = 0; i < (1 << 11) ; ++i) {
		if (!(rand() % 89)) { printf("crr .. "); srand(static_cast<unsigned int>(time(0))); }
		memset(arr, 0, 256);
		for (j = 0; j < 255; j++) arr[j] = sym[(rand() % n)];
		fputs(arr, f);
		fputs("\r\n", f);
		Sleep(rand() % 100);
		printf("%d .. ", i);
		Sleep(17);
	}
	fclose(f);
	return 0;
}

