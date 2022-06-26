#pragma once

#include <cmath>
#include <cstring>
#include <cstdint>

struct tVideoMetrics
{
	double Y_YUV = 0.0;
	double U_YUV = 0.0;
	double V_YUV = 0.0;
	double L_LUV = 0.0;
	double LUV_LUV = 0.0;
	double R_RGB = 0.0;
	double G_RGB = 0.0;
	double B_RGB = 0.0;
	double D_AVG_Y = 0.0;
};

void CalculatePSNR(unsigned char *pImg0, unsigned char *pImg1, int w, int h, tVideoMetrics *vm, tVideoMetrics *vm_max, tVideoMetrics *vm_avg);
