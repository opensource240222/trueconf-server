#include <cmath>
#include <vector>
#include <iostream>
#include <iomanip>
#include <algorithm>

#include "CAdaptiveGrid.h"

SFloatRect::SFloatRect()
{
	W = 0.0f;
	H = 0.0f;
	AR = 0.0f;
}

SFloatRect::SFloatRect(float w, float h)
{
	W = w;
	H = h;
	CalcAR();
}

void SFloatRect::CalcAR()
{
	AR = W / H;
}

void SFloatRect::Scale(float factor)
{
	W *= factor;
	H *= factor;
}

static float GetScaleFactor(const SFloatRect& from, const SFloatRect& to)
{
	if (from.AR < to.AR)
		return to.H / from.H;
	else
		return to.W / from.W;
}

CAdaptiveGrid::CAdaptiveGrid(const std::vector<std::vector<SFloatRect>>& grid)
{
	for (const std::vector<SFloatRect>& line : grid)
		Lines.emplace_back(line);

	CalcSize();
}

void CAdaptiveGrid::CalcSize()
{
	Size.W = 0.0f;
	Size.H = 0.0f;

	for (CLine& l : Lines)
	{
		Size.W = std::max(l.GetSize().W, Size.W);
		Size.H += l.GetSize().H;
	}

	Size.AR = Size.W / Size.H;
}

void CAdaptiveGrid::Scale(float factor)
{
	for (CLine& l : Lines)
		l.Scale(factor);

	CalcSize();
}

void CAdaptiveGrid::ExpandToHeight(float dstHeight)
{
	float eps = 0.01f;

	while (true)
	{
		SFloatRect expanded;
		SFloatRect free = SFloatRect(Size.W, dstHeight);

		if (std::abs(dstHeight - Size.H) < eps)
			break;

		//PrintDebug();

		std::vector<size_t> idxToScale;

		for (size_t i = 0; i < Lines.size(); i++)
		{
			SFloatRect lineSize = Lines[i].GetSize();

			if (std::abs(Size.W - lineSize.W) > eps)
			{
				idxToScale.push_back(i);

				expanded.W = std::max(expanded.W, lineSize.W);
				expanded.H += lineSize.H;
			}
			else
			{
				free.H -= lineSize.H;
			}
		}

		if (!idxToScale.size())
			break;

		expanded.CalcAR();
		free.CalcAR();

		float scaleFactor = GetScaleFactor(expanded, free);

		for (size_t idx : idxToScale)
			Lines[idx].Scale(scaleFactor);

		CalcSize();
	}
}

void CAdaptiveGrid::ExpandToRect(SFloatRect dstRect)
{
	Scale(GetScaleFactor(Size, dstRect));
	ExpandToHeight(dstRect.H);

	for (CLine& l : Lines)
		l.ExpandToWidth(dstRect.W);
}

void CAdaptiveGrid::PrintDebug()
{
	for (CLine& l : Lines)
	{
		l.PrintDebug();
		std::cout << std::endl;
	}
}

const SFloatRect& CAdaptiveGrid::GetSize() const
{
	return Size;
}

std::vector<std::vector<SFloatRect>> CAdaptiveGrid::GetRects() const
{
	std::vector<std::vector<SFloatRect>> rects;

	for (const CLine& l : Lines)
		rects.emplace_back(l.GetRects());

	return rects;
}

CAdaptiveGrid::CLine::CLine(const std::vector<SFloatRect>& rects) :
Rects(rects)
{
	CalcSize();
}

void CAdaptiveGrid::CLine::CalcSize()
{
	Size.W = 0.0f;
	Size.H = 0.0f;

	for (SFloatRect& r : Rects)
	{
		Size.W += r.W;
		Size.H = std::max(Size.H, r.H);
	}

	Size.AR = Size.W / Size.H;
}

void CAdaptiveGrid::CLine::Scale(float factor)
{
	for (SFloatRect& r : Rects)
		r.Scale(factor);

	CalcSize();
}

void CAdaptiveGrid::CLine::ExpandToWidth(float dstWidth)
{
	float eps = 0.01f;

	while (true)
	{
		SFloatRect expanded;
		SFloatRect free = SFloatRect(dstWidth, Size.H);

		if (std::abs(dstWidth - Size.W) < eps)
			break;

		//PrintDebug();

		std::vector<size_t> idxToScale;

		for (size_t i = 0; i < Rects.size(); i++)
		{
			if (std::abs(Size.H - Rects[i].H) > eps)
			{
				idxToScale.push_back(i);

				expanded.W += Rects[i].W;
				expanded.H = std::max(expanded.H, Rects[i].H);
			}
			else
			{
				free.W -= Rects[i].W;
			}
		}

		expanded.CalcAR();
		free.CalcAR();

		if (!idxToScale.size())
			break;

		float scaleFactor = GetScaleFactor(expanded, free);

		for (size_t idx : idxToScale)
			Rects[idx].Scale(scaleFactor);

		CalcSize();
	}
}

const SFloatRect& CAdaptiveGrid::CLine::GetSize() const
{
	return Size;
}

const std::vector<SFloatRect>& CAdaptiveGrid::CLine::GetRects() const
{
	return Rects;
}

void CAdaptiveGrid::CLine::PrintDebug() const
{
	std::cout << "[ ";

	for (const SFloatRect& r : Rects)
	{
		std::cout << std::setprecision(2) << std::fixed
			<< "<" << r.W << " x " << r.H << "> ";
	}

	std::cout << "]";
}
