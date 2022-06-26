#pragma once

#include <vector>
#include <iostream>
#include <iomanip>
#include <algorithm>

struct SFloatRect
{
	float W;
	float H;
	float AR;

	SFloatRect();
	SFloatRect(float w, float h);

	void CalcAR();
	void Scale(float factor);
};

class CAdaptiveGrid
{
public:
	CAdaptiveGrid(const std::vector<std::vector<SFloatRect>>& grid);

	void CalcSize();
	void Scale(float factor);

	void ExpandToHeight(float dstHeight);
	void ExpandToRect(SFloatRect dstRect);

	void PrintDebug();

	const SFloatRect& GetSize() const;
	std::vector<std::vector<SFloatRect>> GetRects() const;

private:
	class CLine
	{
	public:
		CLine(const std::vector<SFloatRect>& rects);

		void CalcSize();

		void Scale(float factor);
		void ExpandToWidth(float dstWidth);

		const SFloatRect& GetSize() const;
		const std::vector<SFloatRect>& GetRects() const;

		void PrintDebug() const;

	private:
		std::vector<SFloatRect> Rects;
		SFloatRect Size;
	};

	std::vector<CLine> Lines;
	SFloatRect Size;
};
