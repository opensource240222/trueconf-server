#include "VS_H264ResolutionCalc.h"

const size_t VS_H264ResolutionCalc::RESOLUTIONS[NUM_RESOLUTIONS][2] =
{
	1920, 1080,
	1280, 720,
	1024, 576,		// W4CIF
	864, 480,
	704, 576,
	800, 448,
	768, 448,
	640, 360,
	480, 264,
	352, 288
};

const size_t VS_H264ResolutionCalc::BITRATES[NUM_RESOLUTIONS] =
{
	0 // not defined yet
};

VS_H264ResolutionCalc::VS_H264ResolutionCalc(size_t level) : level_(level)
{
	Init();
	CalcResolution();
}

void VS_H264ResolutionCalc::Init()
{
	/* if (level_ >= 51) // for future
	{
		maxMbps_ = 983040;
		maxFs_ = 36864;
		maxBr_ = 240000;
	}
	else if (level_ >= 50)
	{
		maxMbps_ = 589824;
		maxFs_ = 22080;
		maxBr_ = 135000;
	}
	else if (level_ >= 42)
	{
		maxMbps_ = 522240;
		maxFs_ = 8704;
		maxBr_ = 50000;
	}
	else if (level_ >= 41)
	{
		maxMbps_ = 245760;
		maxFs_ = 8192;
		maxBr_ = 50000;
	}
	else */
	if (level_ >= 40)
	{
		maxMbps_ = 245760;
		maxFs_ = 8192;
		maxBr_ = 20000;
	}
	else if (level_ >= 31)
	{
		maxMbps_ = 108000;
		maxFs_ = 3600;
		maxBr_ = 14000;
	}
	else if (level_ >= 30)
	{
		maxMbps_ = 40500;
		maxFs_ = 1620;
		maxBr_ = 10000;
	}
	else if (level_ >= 22)
	{
		maxMbps_ = 20250;
		maxFs_ = 1620;
		maxBr_ = 4000;
	}
	else if (level_ >= 21)
	{
		maxMbps_ = 19800;
		maxFs_ = 792;
		maxBr_ = 4000;
	}
	else if (level_ >= 20)
	{
		maxMbps_ = 11880;
		maxFs_ = 396;
		maxBr_ = 2000;
	}
	else
	{
		maxMbps_ = 11880;
		maxFs_ = 396;
		maxBr_ = 768;
	}

}

void VS_H264ResolutionCalc::CalcResolution()
{
	width_ = RESOLUTIONS[NUM_RESOLUTIONS - 1][0];
	height_ = RESOLUTIONS[NUM_RESOLUTIONS - 1][1];

	for (size_t i = 0; i < NUM_RESOLUTIONS; ++i)
	{
		size_t width = RESOLUTIONS[i][0];
		size_t height = RESOLUTIONS[i][1];

		// check by fs (assuming that macroblock size is 16*16)
		size_t mbWidth = (width + 15) / 16;
		size_t mbHeight = (height + 15) / 16;

		if (mbWidth * mbHeight > maxFs_)
			continue;

		// check by mbps
		if (mbWidth * mbHeight * FPS > maxMbps_)
			continue;

		// check by br (br set for different resolutions hasn't been defined yet, so not really used now)
		if (BITRATES[i] > maxBr_)
			continue;

		width_ = width;
		height_ = height;
		break;
	}
}

void VS_H264ResolutionCalc::SetMaxMbps(size_t maxMbps)
{
	if (maxMbps > maxMbps_)
	{
		maxMbps_ = maxMbps;
		CalcResolution();
	}
}

void VS_H264ResolutionCalc::SetMaxFs(size_t maxFs)
{
	if (maxFs > maxFs_)
	{
		maxFs_ = maxFs;
		CalcResolution();
	}
}

void VS_H264ResolutionCalc::SetMaxBr(size_t maxBr)
{
	if (maxBr > maxBr_)
	{
		maxBr_ = maxBr;
		CalcResolution();
	}
}