#include <windows.h>
#include "../VSVideoProc.h"

int funk1(int argc, char* argv[]);
int funk2(int argc, char* argv[]);
int funk3(int argc, char* argv[]);
int funk4(int argc, char* argv[]);
int funk5(int argc, char* argv[]);
int funk6(int argc, char* argv[]);
int funk7(int argc, char* argv[]);

int main(int argc, char* argv[])
{
	//FILE *f = fopen("foreman_348.yuv", "rb");
	//FILE *ff = fopen("ffroreman_348.yuv", "wb");
	//unsigned char frame[348*288*3/2];
	//fread(frame, 348*288*3/2, 1, f);
	//fwrite(frame, 348*288*3/2, 1, ff);
	//fclose(ff);
	//fclose(f);
	//funk1(argc, argv);
	//funk2(argc, argv);
	//funk3(argc, argv);
	//funk5(argc, argv);
	//funk6(argc, argv);
	funk7(argc, argv);
	return 0;
}

int funk1(int argc, char* argv[]) {
	bool res = false;
	VS_VideoProc *vp;
	if (argc<5) goto error;
	int new_x = atoi(argv[3]);
	int new_y = atoi(argv[4]);
	vp = new  VS_VideoProc(argc<=5);

	FILE* file;
	file = fopen(argv[1], "rb");
	if (file== 0) goto error;

	BITMAPFILEHEADER bmf;
	fread(&bmf, sizeof(bmf), 1, file);

	BITMAPINFOHEADER *bmi;
	bmi = (BITMAPINFOHEADER*)malloc(bmf.bfOffBits - sizeof(bmf));
	fread(bmi, bmf.bfOffBits - sizeof(bmf), 1, file);
	if (bmi->biBitCount!= 24 && bmi->biBitCount!= 32 && bmi->biBitCount!= 8 && bmi->biBitCount!= 16) goto error;

	bmi->biSizeImage = bmi->biHeight*bmi->biWidth*bmi->biBitCount/8;
	fseek(file, bmf.bfOffBits, SEEK_SET);

	unsigned char *image = (unsigned char *)malloc(bmi->biSizeImage);
	fread(image, bmi->biSizeImage, 1, file);
	fclose(file);

	bmi->biSizeImage = new_x*new_y*bmi->biBitCount/8;
	UCHAR * ImagRes = (UCHAR * )malloc(bmi->biSizeImage);

	DWORD stime = timeGetTime();
	int repeat = 100;
	int i= 0;
	for (; i<repeat; i++) {
		switch (bmi->biBitCount)
		{
		case 8: vp->ResampleRGB8(image, ImagRes, bmi->biWidth, bmi->biHeight, new_x, new_y, new_x); break;
		case 16:vp->ResampleRGB565(image, ImagRes, bmi->biWidth, bmi->biHeight, new_x, new_y, new_x*2);break;
		case 24:vp->ResampleRGB24(image, ImagRes, bmi->biWidth, bmi->biHeight, new_x, new_y, new_x*3);break;
		case 32:vp->ResampleRGB32(image, ImagRes, bmi->biWidth, bmi->biHeight, new_x, new_y, new_x*4);break;
		default: break;
		}
	}
	stime = timeGetTime() - stime;
	printf(" time = %5.2f ms\n", (float)stime/(float)repeat);

	bmi->biHeight = new_y;
	bmi->biWidth = new_x;

	file = fopen(argv[2], "wb");
	bmf.bfSize = bmf.bfOffBits + bmi->biSizeImage;
	fwrite(&bmf, sizeof(bmf), 1, file);
	fwrite(bmi, bmf.bfOffBits - sizeof(bmf), 1, file);
	if (bmi->biBitCount!=8) {
	if (bmi->biBitCount!=24)
		fwrite(ImagRes, bmi->biSizeImage, 1, file);
	else
		for(i = 0; i<bmi->biHeight; i++)
			fwrite(ImagRes + bmi->biWidth*3*i, (bmi->biWidth*3+3)&(~3), 1, file);
	} else {
		unsigned char table[1024];
		int ii = 0, jj = 0;
		for (ii = 0; ii < 256; ii++)
			for (jj = 0; jj < 4; jj++)
				table[4*ii + jj] = ii;
		fwrite(&table, 1024, 1, file);
		fwrite(ImagRes, bmi->biSizeImage, 1, file);
	}

	if (file) fclose(file);

	if (ImagRes) free(ImagRes);
	if (image) free(image);
	if (bmi) free(bmi);
	return 0;

error:
	puts("Usage: ProcessImage.exe InFileName outname x y");
	return -1;
}

int funk2(int argc, char* argv[]) {
	bool res = false;
	VS_VideoProc *vp;
	if (argc<3) goto error;
	vp = new  VS_VideoProc(argc<=3);

	FILE* file;
	file = fopen(argv[1], "rb");
	if (file== 0) goto error;

	BITMAPFILEHEADER bmf;
	fread(&bmf, sizeof(bmf), 1, file);

	BITMAPINFOHEADER *bmi;
	bmi = (BITMAPINFOHEADER*)malloc(bmf.bfOffBits - sizeof(bmf));
	fread(bmi, bmf.bfOffBits - sizeof(bmf), 1, file);
	if (bmi->biBitCount!= 24 && bmi->biBitCount!= 32 && bmi->biBitCount!= 8 && bmi->biBitCount!= 16) goto error;

	bmi->biSizeImage = bmi->biHeight*bmi->biWidth*bmi->biBitCount/8;
	fseek(file, bmf.bfOffBits, SEEK_SET);

	unsigned char *image = (unsigned char *)malloc(bmi->biSizeImage);
	fread(image, bmi->biSizeImage, 1, file);
	fclose(file);

	bmi->biSizeImage = bmi->biHeight * bmi->biWidth * 3 / 2;
	UCHAR * ImagRes = (UCHAR * )malloc(bmi->biSizeImage);
	memset(ImagRes,0,  bmi->biSizeImage);

	DWORD stime = timeGetTime();
	int repeat = 1000;
	int i= 0;
	for (; i<repeat; i++) {
		switch (bmi->biBitCount)
		{
		//case 8: vp->ResampleRGB8(image, ImagRes, bmi->biWidth, bmi->biHeight, new_x, new_y, new_x); break;
		//case 16:vp->ResampleRGB565(image, ImagRes, bmi->biWidth, bmi->biHeight, new_x, new_y, new_x*2);break;
		case 24:vp->ConvertBMF24ToI420(image, ImagRes, ImagRes + bmi->biHeight * bmi->biWidth, ImagRes + 5 * bmi->biHeight * bmi->biWidth /4, bmi->biWidth*3, bmi->biHeight, bmi->biWidth) ;break;
		case 32:vp->ConvertBMF32ToI420(image, ImagRes, ImagRes + bmi->biHeight * bmi->biWidth, ImagRes + 5 * bmi->biHeight * bmi->biWidth /4, bmi->biWidth*4, bmi->biHeight, bmi->biWidth) ;break;
		default: break;
		}
	}
	stime = timeGetTime() - stime;
	printf(" time = %5.2f ms\n", (float)stime/(float)repeat);

	file = fopen(argv[2], "wb");
	fwrite(ImagRes, bmi->biSizeImage, 1, file);

	if (file) fclose(file);
	if (ImagRes) free(ImagRes);
	if (image) free(image);
	if (bmi) free(bmi);

	return 0;

error:
	puts("Usage: ProcessImage.exe InFileName outname x y");
	return -1;
}

int funk3(int argc, char* argv[]) {
	bool res = false;
	VS_VideoProc *vp;
	if (argc<5) goto error;
	int x = atoi(argv[3]);
	int y = atoi(argv[4]);
	int size = x * y * 3 / 2;
	vp = new  VS_VideoProc(argc<=5);

	FILE* file;
	file = fopen(argv[1], "rb");
	if (file== 0) goto error;

	unsigned char *image = (unsigned char *)malloc(size);
	fread(image, size, 1, file);
	fclose(file);

	size = (x * 3 / 2) * (y * 3 / 2) * 3 / 2;
	unsigned char * ImagRes = (UCHAR * )malloc(size);
	memset(ImagRes,0,  size);

	DWORD stime = timeGetTime();
	int repeat = 1000;
	int i= 0;
	for (; i<repeat; i++) {
		vp->ResampleUp_1d5(image, ImagRes, x, y);
	}
	stime = timeGetTime() - stime;
	printf(" time = %5.2f ms\n", (float)stime/(float)repeat);

	file = fopen(argv[2], "wb");
	fwrite(ImagRes, size, 1, file);

	if (file) fclose(file);
	if (ImagRes) free(ImagRes);
	if (image) free(image);

	return 0;

error:
	puts("Usage: ProcessImage.exe InFileName outname x y");
	return -1;
}

int funk4(int argc, char* argv[]) {
	bool res = false;
	VS_VideoProc *vp;
	if (argc<5) goto error;
	int x = atoi(argv[3]);
	int y = atoi(argv[4]);
	int size = x * y * 3 / 2;
	vp = new  VS_VideoProc(argc<=5);

	FILE* file;
	file = fopen(argv[1], "rb");
	if (file== 0) goto error;

	unsigned char *image = (unsigned char *)malloc(size);
	fread(image, size, 1, file);
	fclose(file);

	size = (x * 2) * (y * 2) * 3 / 2;
	unsigned char * ImagRes = (UCHAR * )malloc(size);
	memset(ImagRes,0,  size);

	DWORD stime = timeGetTime();
	int repeat = 1000;
	int i= 0;
	for (; i<repeat; i++) {
		vp->ResampleUp_2(image, ImagRes, x, y);
	}
	stime = timeGetTime() - stime;
	printf(" time = %5.2f ms\n", (float)stime/(float)repeat);

	file = fopen(argv[2], "wb");
	fwrite(ImagRes, size, 1, file);

	if (file) fclose(file);
	if (ImagRes) free(ImagRes);
	if (image) free(image);

	return 0;

error:
	puts("Usage: ProcessImage.exe InFileName outname x y");
	return -1;
}

int funk5(int argc, char* argv[]) {
	bool res = false;
	VS_VideoProc *vp;
	if (argc<3) goto error;
	vp = new  VS_VideoProc(argc<=3);

	FILE* file;
	file = fopen(argv[1], "rb");
	if (file== 0) goto error;

	BITMAPFILEHEADER bmf;
	fread(&bmf, sizeof(bmf), 1, file);

	BITMAPINFOHEADER *bmi;
	bmi = (BITMAPINFOHEADER*)malloc(bmf.bfOffBits - sizeof(bmf));
	fread(bmi, bmf.bfOffBits - sizeof(bmf), 1, file);
	if (bmi->biBitCount!= 32) goto error;

	bmi->biSizeImage = bmi->biHeight*bmi->biWidth*bmi->biBitCount/8;
	fseek(file, bmf.bfOffBits, SEEK_SET);

	unsigned char *image0 = (unsigned char *)malloc(bmi->biSizeImage);
	unsigned char *image1 = (unsigned char *)malloc(bmi->biSizeImage);
	fread(image0, bmi->biSizeImage, 1, file);
	memset(image1, 0, bmi->biSizeImage);
	fclose(file);

	vp->ConvertBMF32ToI420(image0, image1, image1 + bmi->biHeight * bmi->biWidth, image1 + 5 * bmi->biHeight * bmi->biWidth /4, bmi->biWidth * 4, bmi->biHeight, bmi->biWidth);
	vp->ConvertI420ToYUV444(image1, image1 + bmi->biHeight * bmi->biWidth, image1 + 5 * bmi->biHeight * bmi->biWidth /4, image0, bmi->biWidth, bmi->biHeight, bmi->biWidth);

	DWORD stime = timeGetTime();
	int repeat = 1000;
	int i= 0;
	for (; i<repeat; i++) {
		vp->ConvertYUV444ToBMF32_Vflip(image0, image0 + bmi->biHeight * bmi->biWidth, image0 + 2 * bmi->biHeight * bmi->biWidth, image1, bmi->biWidth, bmi->biHeight, bmi->biWidth * 4);
	}
	stime = timeGetTime() - stime;
	printf(" time = %5.2f ms\n", (float)stime/(float)repeat);

	file = fopen(argv[2], "wb");
	fwrite(&bmf, sizeof(bmf), 1, file);
	fwrite(bmi, bmf.bfOffBits - sizeof(bmf), 1, file);
	fwrite(image1, bmi->biSizeImage, 1, file);

	if (file) fclose(file);
	if (image0) free(image0);
	if (image1) free(image1);
	if (bmi) free(bmi);

	return 0;

error:
	puts("Usage: ProcessImage.exe InFileName outname x y");
	return -1;
}

int funk6(int argc, char* argv[]) {
	bool res = false;
	VS_VideoProc *vp;
	if (argc<4) goto error;
	vp = new  VS_VideoProc(argc<=4);
	int type = atoi(argv[3]);

	FILE* file;
	file = fopen(argv[1], "rb");
	if (file== 0) goto error;

	BITMAPFILEHEADER bmf;
	fread(&bmf, sizeof(bmf), 1, file);

	BITMAPINFOHEADER *bmi;
	bmi = (BITMAPINFOHEADER*)malloc(bmf.bfOffBits - sizeof(bmf));
	fread(bmi, bmf.bfOffBits - sizeof(bmf), 1, file);
	if (bmi->biBitCount!= 32) goto error;

	bmi->biSizeImage = bmi->biHeight*bmi->biWidth*bmi->biBitCount/8;
	fseek(file, bmf.bfOffBits, SEEK_SET);

	unsigned char *image0 = (unsigned char *)malloc(bmi->biSizeImage);
	unsigned char *image1 = (unsigned char *)malloc(bmi->biSizeImage);
	fread(image0, bmi->biSizeImage, 1, file);
	memset(image1, 0, bmi->biSizeImage);
	fclose(file);

	vp->ConvertBMF32ToI420(image0, image1, image1 + bmi->biHeight * bmi->biWidth, image1 + 5 * bmi->biHeight * bmi->biWidth /4, bmi->biWidth * 4, bmi->biHeight, bmi->biWidth);

	DWORD stime = timeGetTime();
	int repeat = 1000;
	int i= 0;
	for (; i<repeat; i++) {
		switch (type)
		{
		case 0: vp->ConvertI420ToBMF24(image1, image1 + bmi->biHeight * bmi->biWidth, image1 + 5 * bmi->biHeight * bmi->biWidth /4, image0, bmi->biWidth, bmi->biHeight, bmi->biWidth * 3); break;
		case 1: vp->ConvertI420ToBMF32_Vflip(image1, image1 + bmi->biHeight * bmi->biWidth, image1 + 5 * bmi->biHeight * bmi->biWidth /4, image0, bmi->biWidth, bmi->biHeight, bmi->biWidth * 4); break;
		case 2: vp->ConvertI420ToYUY2(image1, image1 + bmi->biHeight * bmi->biWidth, image1 + 5 * bmi->biHeight * bmi->biWidth /4, image0, bmi->biWidth, bmi->biHeight, bmi->biWidth); break;
		case 3: vp->ConvertI420ToUYVY(image1, image1 + bmi->biHeight * bmi->biWidth, image1 + 5 * bmi->biHeight * bmi->biWidth /4, image0, bmi->biWidth, bmi->biHeight, bmi->biWidth); break;
		default: break;
		}
	}
	stime = timeGetTime() - stime;
	printf(" time = %5.2f ms\n", (float)stime/(float)repeat);

	file = fopen(argv[2], "wb");

	switch (type)
	{
	case 0:
		{
			bmi->biBitCount = 24;
			bmi->biSizeImage = bmi->biWidth * bmi->biHeight * 3;
			bmf.bfSize = bmf.bfOffBits + bmi->biSizeImage;
			fwrite(&bmf, sizeof(bmf), 1, file);
			fwrite(bmi, bmf.bfOffBits - sizeof(bmf), 1, file);
			for(i = 0; i < bmi->biHeight; i++)
				fwrite(image0 + bmi->biWidth*3*i, (bmi->biWidth*3+3)&(~3), 1, file);
			break;
		}
	case 1:
		{
			fwrite(&bmf, sizeof(bmf), 1, file);
			fwrite(bmi, bmf.bfOffBits - sizeof(bmf), 1, file);
			fwrite(image0, bmi->biSizeImage, 1, file);
			break;
		}
	case 2:
	case 3:
		{
			fwrite(image0, bmi->biHeight * bmi->biWidth * 3 / 2, 1, file);
			break;
		}
	default: break;
	}

	if (file) fclose(file);
	if (image0) free(image0);
	if (image1) free(image1);
	if (bmi) free(bmi);

	return 0;

error:
	puts("Usage: ProcessImage.exe InFileName outname x y");
	return -1;
}

int funk7(int argc, char* argv[]) {
	bool res = false;
	VS_VideoProc *vp0, *vp1;
	if (argc<3) goto error;
	vp0 = new  VS_VideoProc(true);
	vp1 = new  VS_VideoProc(argc<=3);

	FILE* file;
	file = fopen(argv[1], "rb");
	if (file== 0) goto error;

	BITMAPFILEHEADER bmf;
	fread(&bmf, sizeof(bmf), 1, file);

	BITMAPINFOHEADER *bmi;
	bmi = (BITMAPINFOHEADER*)malloc(bmf.bfOffBits - sizeof(bmf));
	fread(bmi, bmf.bfOffBits - sizeof(bmf), 1, file);
	if (bmi->biBitCount!= 32) goto error;

	bmi->biSizeImage = bmi->biHeight*bmi->biWidth*bmi->biBitCount/8;
	fseek(file, bmf.bfOffBits, SEEK_SET);

	unsigned char *image0 = (unsigned char *)malloc(bmi->biSizeImage);
	unsigned char *image1 = (unsigned char *)malloc(bmi->biSizeImage);
	fread(image0, bmi->biSizeImage, 1, file);
	memset(image1, 0, bmi->biSizeImage);
	fclose(file);

	vp0->ConvertBMF32ToI420(image0, image1, image1 + bmi->biHeight * bmi->biWidth, image1 + 5 * bmi->biHeight * bmi->biWidth /4, bmi->biWidth * 4, bmi->biHeight, bmi->biWidth);
	vp0->ConvertI420ToUYVY(image1, image1 + bmi->biHeight * bmi->biWidth, image1 + 5 * bmi->biHeight * bmi->biWidth /4, image0, bmi->biWidth, bmi->biHeight, 2 * bmi->biWidth);

	memset(image1, 0x80, bmi->biSizeImage);

	DWORD stime = timeGetTime();
	int repeat = 1000;
	int i= 0;
	for (; i<repeat; i++) {
		vp1->ConvertUYVYToI420(image0, image1, image1 + bmi->biHeight * bmi->biWidth, image1 + 5 * bmi->biHeight * bmi->biWidth /4, 2 * bmi->biWidth, bmi->biHeight, bmi->biWidth);
	}
	stime = timeGetTime() - stime;
	printf(" time = %5.2f ms\n", (float)stime/(float)repeat);

	file = fopen(argv[2], "wb");
	fwrite(image1, bmi->biHeight * bmi->biWidth * 3 / 2, 1, file);

	if (file) fclose(file);
	if (image0) free(image0);
	if (image1) free(image1);
	if (bmi) free(bmi);

	return 0;

error:
	puts("Usage: ProcessImage.exe InFileName outname x y");
	return -1;
}
