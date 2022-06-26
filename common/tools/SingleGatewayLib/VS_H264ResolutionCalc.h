#pragma once

#include <cstddef>

class VS_H264ResolutionCalc
{
public:

	VS_H264ResolutionCalc(size_t level);

	size_t GetWidth() const { return width_; }
	size_t GetHeight() const { return height_; }

	void SetMaxMbps(size_t maxMbps);
	void SetMaxFs(size_t maxFs);
	void SetMaxBr(size_t maxBr);

private:

	static const size_t NUM_RESOLUTIONS = 10;
	static const size_t RESOLUTIONS[NUM_RESOLUTIONS][2];
	static const size_t BITRATES[NUM_RESOLUTIONS];
	static const size_t FPS = 30;

	size_t level_;

	size_t width_;
	size_t height_;

	size_t maxMbps_;
	size_t maxFs_;
	size_t maxBr_;

	void Init();
	void CalcResolution();
};
