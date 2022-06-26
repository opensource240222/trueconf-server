#include <cmath>

#include "LayoutCalculator.h"
#include "CAdaptiveGrid.h"
#include "CPartitionTree.h"

int MakeMultiple(int a, int b)
{
	if (b == 1)
		return a;

	return a - a % b;
}

int MakeMultipleUp(int a, int b)
{
	if (b == 1)
		return a;

	return (a % b) ? (a - a % b + b) : (a);
}

/*******************************************************
/ Uniform layout
/*******************************************************/

bool CalculateUniformLayoutColumnBased(int screenWidth, int screenHeight,
	int numWindows,
	float windowAspectRatio,
	int frameIntervalWidth, int frameIntervalHeight,
	int windowMultW, int windowMultH,
	int maxWindowWidth,
	int * windowWidth, int * windowHeight,
	int * fullRows, int * fullColumns,
	int * columnsInPartialRow)
{
	int numRow = 0;
	int numCol = 0;
	float winW = float(screenWidth);
	float winH = float(screenHeight);

	while (numRow * numCol < numWindows ||
		int(winW) > maxWindowWidth)
	{
		numCol++;

		winW = float(screenWidth - frameIntervalWidth * (numCol + 1)) / float(numCol);

		if (winW <= 1.0f)
			return false;

		winH = winW / windowAspectRatio;

		if (winH <= 0.0f)
			return false;

		numRow = static_cast<int>((screenHeight - frameIntervalHeight) / (winH + frameIntervalHeight));
	}

	winW = static_cast<float>(MakeMultiple((int)winW, windowMultW));
	winH = static_cast<float>(MakeMultiple((int)winH, windowMultH));

	*windowWidth = (int)winW;
	*windowHeight = (int)winH;

	if (fullColumns)
		*fullColumns = numCol;

	if (numRow * numCol == numWindows)
	{
		if (fullRows) *fullRows = numRow;
		if (columnsInPartialRow) *columnsInPartialRow = 0;
	}
	else
	{
		if (fullRows)
		{
			*fullRows = numWindows / numCol;

			if (!(*fullRows)) // if in row less than numCol windows
			{
				*fullRows = 1;

				if (fullColumns) *fullColumns = numWindows;
			}
		}

		if (columnsInPartialRow && numWindows / numCol)
			*columnsInPartialRow = numWindows - (numWindows / numCol) * numCol; // In C++ (3 / 4) * 4 == 0
	}

	return true;
}

bool CalculateUniformLayoutRowBased(int screenWidth, int screenHeight,
	int numWindows,
	float windowAspectRatio,
	int frameIntervalWidth, int frameIntervalHeight,
	int windowMultW, int windowMultH,
	int maxWindowHeight,
	int * windowWidth, int * windowHeight,
	int * fullRows, int * fullColumns,
	int * columnsInPartialRow)
{
	if (screenHeight - frameIntervalHeight * 2 <= 0 || screenWidth - frameIntervalWidth * 2 <= 0)
		return false;

	int numRow = 0;
	int numCol = 0;
	float winW = float(screenWidth);
	float winH = float(screenHeight);

	while (numRow * numCol < numWindows ||
		int(winH) > maxWindowHeight)
	{
		numRow++;

		winH = float(screenHeight - frameIntervalHeight * (numRow + 1)) / float(numRow);

		if (winH <= 1.0f)
			return false;

		winW = winH * windowAspectRatio;

		if (winW <= 0.0f)
			return false;

		numCol = static_cast<int>((screenWidth - frameIntervalWidth) / (winW + frameIntervalWidth));
	}

	winW = static_cast<float>(MakeMultiple((int)winW, windowMultW));
	winH = static_cast<float>(MakeMultiple((int)winH, windowMultH));

	*windowWidth = (int)winW;
	*windowHeight = (int)winH;

	if (fullColumns) *fullColumns = numCol;

	if (numRow * numCol == numWindows)
	{
		if (fullRows) *fullRows = numRow;
		if (columnsInPartialRow) *columnsInPartialRow = 0;
	}
	else
	{
		if (fullRows) *fullRows = numWindows / numCol;
		if (columnsInPartialRow) *columnsInPartialRow = numWindows - (numWindows / numCol) * numCol; // In C++ (3 / 4) * 4 == 0
	}

	return true;
}

bool CalculateUniformLayoutSmart(int screenWidth, int screenHeight,
	int numWindows,
	float windowAspectRatio,
	int frameIntervalWidth, int frameIntervalHeight,
	int windowMultW, int windowMultH,
	int maxWindowWidth, int maxWindowHeight,
	int * windowWidth, int * windowHeight,
	int * fullRows, int * fullColumns,
	int * columnsInPartialRow)
{
	int windowWidth1 = 0, windowHeight1 = 0;
	int fullRows1 = 0, fullColumns1 = 0;
	int columnsInPartialRow1 = 0;
	int windowWidth2 = 0, windowHeight2 = 0;
	int fullRows2 = 0, fullColumns2 = 0;
	int columnsInPartialRow2 = 0;

	if (!CalculateUniformLayoutColumnBased(screenWidth, screenHeight,
		numWindows,
		windowAspectRatio,
		frameIntervalWidth, frameIntervalHeight,
		windowMultW, windowMultH,
		maxWindowWidth,
		&windowWidth1, &windowHeight1,
		&fullRows1, &fullColumns1,
		&columnsInPartialRow1))
	{
		windowWidth1 = 0;
		windowHeight1 = 0;
	}

	if (!CalculateUniformLayoutRowBased(screenWidth, screenHeight,
		numWindows,
		windowAspectRatio,
		frameIntervalWidth, frameIntervalHeight,
		windowMultW, windowMultH,
		maxWindowHeight,
		&windowWidth2, &windowHeight2,
		&fullRows2, &fullColumns2,
		&columnsInPartialRow2))
	{
		windowWidth2 = 0;
		windowHeight2 = 0;
	}

	if (windowWidth1 * windowHeight1 == 0 && windowWidth2 * windowHeight2 == 0)
		return false;

	if (windowWidth1 * windowHeight1 > windowWidth2 * windowHeight2)
	{
		*windowWidth = windowWidth1;
		*windowHeight = windowHeight1;
		if (fullRows)	 *fullRows = fullRows1;
		if (fullColumns) *fullColumns = fullColumns1;
		if (columnsInPartialRow) *columnsInPartialRow = columnsInPartialRow1;
	}
	else
	{
		*windowWidth = windowWidth2;
		*windowHeight = windowHeight2;
		if (fullRows)	 *fullRows = fullRows2;
		if (fullColumns) *fullColumns = fullColumns2;
		if (columnsInPartialRow) *columnsInPartialRow = columnsInPartialRow2;
	}

	return true;
}

/*******************************************************
/ Top layout
/*******************************************************/

bool CalculateTopLayoutPriorityBased(int screenWidth, int screenHeight,
	int numWindows,
	float windowAspectRatio,
	int frameIntervalWidth, int frameIntervalHeight,
	int windowMultW, int windowMultH,
	int * topWindowWidth, int * topWindowHeight,
	int * ordinaryWindowWidth, int * ordinaryWindowHeight,
	int * fullRows, int * fullColumns,
	int * columnsInPartialRow)
{
	int	fullR, fullC, colsInPartRow;

	if (!CalculateUniformLayoutSmart(screenWidth, screenHeight / 3,
		numWindows - 1,
		windowAspectRatio,
		frameIntervalWidth, frameIntervalHeight,
		windowMultW, windowMultH,
		screenWidth, screenHeight,
		ordinaryWindowWidth, ordinaryWindowHeight,
		&fullR, &fullC,
		&colsInPartRow))
	{
		return false;
	}

	int heightToPriority = screenHeight - (fullR + (colsInPartRow ? 1 : 0)) * (*ordinaryWindowHeight + frameIntervalHeight);

	if (!CalculateUniformLayoutSmart(screenWidth, heightToPriority,
		1,
		windowAspectRatio,
		frameIntervalWidth, frameIntervalHeight,
		windowMultW, windowMultH,
		screenWidth, screenHeight,
		topWindowWidth, topWindowHeight,
		0, 0,
		0))
	{
		return false;
	}

	if (fullRows)	 *fullRows = fullR;
	if (fullColumns) *fullColumns = fullC;
	if (columnsInPartialRow) *columnsInPartialRow = colsInPartRow;

	return true;
}

bool CalculateTopLayoutOrdinaryBased(int screenWidth, int screenHeight,
	int numWindows,
	float windowAspectRatio,
	int frameIntervalWidth, int frameIntervalHeight,
	int windowMultW, int windowMultH,
	int * topWindowWidth, int * topWindowHeight,
	int * ordinaryWindowWidth, int * ordinaryWindowHeight,
	int * fullRows, int * fullColumns,
	int * columnsInPartialRow)
{
	if (!CalculateUniformLayoutSmart(screenWidth, screenHeight * 2 / 3,
		1,
		windowAspectRatio,
		frameIntervalWidth, frameIntervalHeight,
		windowMultW, windowMultH,
		screenWidth, screenHeight,
		topWindowWidth, topWindowHeight,
		0, 0,
		0))
	{
		return false;
	}

	int heightToOrdinary = screenHeight - (*topWindowHeight + frameIntervalHeight);

	if (!CalculateUniformLayoutSmart(screenWidth, heightToOrdinary,
		numWindows - 1,
		windowAspectRatio,
		frameIntervalWidth, frameIntervalHeight,
		windowMultW, windowMultH,
		screenWidth, screenHeight,
		ordinaryWindowWidth, ordinaryWindowHeight,
		fullRows, fullColumns,
		columnsInPartialRow))
	{
		return false;
	}

	return true;
}

bool CalculateTopLayoutSmart(int screenWidth, int screenHeight,
	int numWindows,
	float windowAspectRatio,
	int frameIntervalWidth, int frameIntervalHeight,
	int windowMultW, int windowMultH,
	int * topWindowWidth, int * topWindowHeight,
	int * ordinaryWindowWidth, int * ordinaryWindowHeight,
	int * fullRows, int * fullColumns,
	int * columnsInPartialRow)
{
	int heightToPriority = screenHeight * 8 / 10;

	do
	{
		// calc prior size
		if (!CalculateUniformLayoutSmart(screenWidth, heightToPriority,
			1,
			windowAspectRatio,
			frameIntervalWidth, frameIntervalHeight,
			windowMultW, windowMultH,
			screenWidth, screenHeight,
			topWindowWidth, topWindowHeight,
			0, 0,
			0))
		{
			return false;
		}

		int heightToOrdinary = screenHeight - (*topWindowHeight + frameIntervalHeight);

		// calc ordinary size
		if (!CalculateUniformLayoutSmart(screenWidth, heightToOrdinary,
			numWindows - 1,
			windowAspectRatio,
			frameIntervalWidth, frameIntervalHeight,
			windowMultW, windowMultH,
			*topWindowWidth / 3, *topWindowHeight / 3,
			ordinaryWindowWidth, ordinaryWindowHeight,
			fullRows, fullColumns,
			columnsInPartialRow))
		{
			return false;
		}

		int numRows = *fullRows + (*columnsInPartialRow ? 1 : 0);

		if (float(heightToOrdinary) / float(screenHeight) < 0.25f && numRows == 2)
		{
			heightToPriority = screenHeight * 75 / 100;
		}
		else if (float(heightToOrdinary) / float(screenHeight) < 0.3f && numRows > 2)
		{
			heightToPriority = screenHeight * 70 / 100;
		}
		else
		{
			heightToPriority = screenHeight -
				numRows * (*ordinaryWindowHeight) -
				numRows * frameIntervalHeight;

			CalculateUniformLayoutSmart(screenWidth, heightToPriority,
				1,
				windowAspectRatio,
				frameIntervalWidth, frameIntervalHeight,
				windowMultW, windowMultH,
				screenWidth, screenHeight,
				topWindowWidth, topWindowHeight,
				0, 0,
				0);

			break;
		}
	} while (true);

	return true;
}

/*******************************************************
/ Left layout
/*******************************************************/

bool CalculateLeftLayoutOrdinaryBased(int screenWidth, int screenHeight,
	int numWindows,
	float windowAspectRatio,
	int frameIntervalWidth, int frameIntervalHeight,
	int windowMultW, int windowMultH,
	int * leftWindowWidth, int * leftWindowHeight,
	int * ordinaryWindowWidth, int * ordinaryWindowHeight,
	int * fullRows, int * fullColumns,
	int * columnsInPartialRow)
{
	int	fullR, fullC, colsInPartRow;

	if (!CalculateUniformLayoutSmart(screenWidth * 1 / 3, screenHeight,
		numWindows - 1,
		windowAspectRatio,
		frameIntervalWidth, frameIntervalHeight,
		windowMultW, windowMultH,
		screenWidth, screenHeight,
		ordinaryWindowWidth, ordinaryWindowHeight,
		&fullR, &fullC,
		&colsInPartRow))
	{
		return false;
	}

	int widthToPriority = screenWidth - (fullC)* (*ordinaryWindowWidth + frameIntervalWidth);

	if (!CalculateUniformLayoutSmart(widthToPriority, screenHeight,
		1,
		windowAspectRatio,
		frameIntervalWidth, frameIntervalHeight,
		windowMultW, windowMultH,
		screenWidth, screenHeight,
		leftWindowWidth, leftWindowHeight,
		0, 0,
		0))
	{
		return false;
	}

	if (fullRows)	 *fullRows = fullR;
	if (fullColumns) *fullColumns = fullC;
	if (columnsInPartialRow) *columnsInPartialRow = colsInPartRow;

	return true;
}

bool CalculateLeftLayoutPriorityBased(int screenWidth, int screenHeight,
	int numWindows,
	float windowAspectRatio,
	int frameIntervalWidth, int frameIntervalHeight,
	int windowMultW, int windowMultH,
	int * leftWindowWidth, int * leftWindowHeight,
	int * ordinaryWindowWidth, int * ordinaryWindowHeight,
	int * fullRows, int * fullColumns,
	int * columnsInPartialRow)
{
	if (!CalculateUniformLayoutSmart(screenWidth * 2 / 3, screenHeight,
		1,
		windowAspectRatio,
		frameIntervalWidth, frameIntervalHeight,
		windowMultW, windowMultH,
		screenWidth, screenHeight,
		leftWindowWidth, leftWindowHeight,
		0, 0,
		0))
	{
		return false;
	}

	int widthToOrdinary = screenWidth - (*leftWindowWidth + frameIntervalWidth);

	if (!CalculateUniformLayoutSmart(widthToOrdinary, screenHeight,
		numWindows - 1,
		windowAspectRatio,
		frameIntervalWidth, frameIntervalHeight,
		windowMultW, windowMultH,
		screenWidth, screenHeight,
		ordinaryWindowWidth, ordinaryWindowHeight,
		fullRows, fullColumns,
		columnsInPartialRow))
	{
		return false;
	}

	return true;
}

bool CalculateLeftLayoutSmart(int screenWidth, int screenHeight,
	int numWindows,
	float windowAspectRatio,
	int frameIntervalWidth, int frameIntervalHeight,
	int windowMultW, int windowMultH,
	int * leftWindowWidth, int * leftWindowHeight,
	int * ordinaryWindowWidth, int * ordinaryWindowHeight,
	int * fullRows, int * fullColumns,
	int * rowsInPartialColumn)
{
	int widthToPriority = screenWidth * 8 / 10;

	do
	{
		// calc prior size
		if (!CalculateUniformLayoutSmart(widthToPriority, screenHeight,
			1,
			windowAspectRatio,
			frameIntervalWidth, frameIntervalHeight,
			windowMultW, windowMultH,
			screenWidth, screenHeight,
			leftWindowWidth, leftWindowHeight,
			0, 0,
			0))
		{
			return false;
		}

		int widthToOrdinary = screenWidth - (*leftWindowWidth + frameIntervalWidth);

		// calc ordinary size
		if (!CalculateUniformLayoutSmart(screenHeight, widthToOrdinary,
			numWindows - 1,
			1 / windowAspectRatio,
			frameIntervalHeight, frameIntervalWidth,
			windowMultW, windowMultH,
			*leftWindowHeight / 3, *leftWindowWidth / 3,
			ordinaryWindowHeight, ordinaryWindowWidth,
			fullColumns, fullRows,
			rowsInPartialColumn))
		{
			return false;
		}

		int numColumns = *fullColumns + (*rowsInPartialColumn ? 1 : 0);

		if (float(widthToOrdinary) / float(screenWidth) < 0.25f && numColumns == 2)
		{
			widthToPriority = screenWidth * 75 / 100;
		}
		else if (float(widthToOrdinary) / float(screenWidth) < 0.3f && numColumns > 2)
		{
			widthToPriority = screenWidth * 70 / 100;
		}
		else
		{
			widthToPriority = screenWidth -
				numColumns * (*ordinaryWindowWidth) -
				numColumns * frameIntervalWidth;

			CalculateUniformLayoutSmart(widthToPriority, screenHeight,
				1,
				windowAspectRatio,
				frameIntervalWidth, frameIntervalHeight,
				windowMultW, windowMultH,
				screenWidth, screenHeight,
				leftWindowWidth, leftWindowHeight,
				0, 0,
				0);

			break;
		}
	} while (true);

	return true;
}

/*******************************************************
/ Corner layout
/*******************************************************/

bool CalculateCornerLayoutColumnBased(int screenWidth, int screenHeight,
	int numWindows,
	float windowAspectRatio,
	int ordinaryInCorner,
	int frameIntervalWidth, int frameIntervalHeight,
	int windowMultW, int windowMultH,
	int * cornerWindowWidth, int * cornerWindowHeight,
	int * ordinaryWindowWidth, int * ordinaryWindowHeight,
	int * rightRows, int * rightColumns,
	int * bottomRows, int * bottomColumns)
{
	if (screenWidth - frameIntervalWidth * 2 < 0 || screenHeight - frameIntervalHeight * 2 < 0 || !numWindows)
		return false;

	int numRow = 0;
	int numCol = ordinaryInCorner;
	float winW, winH;

	while (numRow * numCol < (numWindows - 1) + ordinaryInCorner * ordinaryInCorner ||
		numRow < ordinaryInCorner ||
		numCol < ordinaryInCorner + 1)
	{
		numCol++;

		winW = float(screenWidth - frameIntervalWidth * (numCol + 1)) / float(numCol);

		if (winW <= 1.0f)
			return false;

		winH = winW / windowAspectRatio;

		if (winH <= 0.0f)
			return false;

		numRow = (screenHeight - frameIntervalHeight) / (int(winH) + frameIntervalHeight);
	}

	winW = static_cast<float>(MakeMultiple((int)winW, windowMultW));
	winH = winW / windowAspectRatio;
	winH = static_cast<float>(MakeMultiple((int)winH, windowMultH));

	*ordinaryWindowWidth = (int)winW;
	*ordinaryWindowHeight = (int)winH;
	if (cornerWindowWidth)	*cornerWindowWidth = (int)winW * ordinaryInCorner + (ordinaryInCorner - 1) * frameIntervalWidth;
	if (cornerWindowHeight)	*cornerWindowHeight = (int)winH * ordinaryInCorner + (ordinaryInCorner - 1) * frameIntervalHeight;

	if (rightColumns)	*rightColumns = numCol - ordinaryInCorner;
	if (rightRows)		*rightRows = ordinaryInCorner;
	if (bottomColumns)	*bottomColumns = numCol;
	if (bottomRows)		*bottomRows = numRow - ordinaryInCorner;

	return true;
}

bool CalculateCornerLayoutRowBased(int screenWidth, int screenHeight,
	int numWindows,
	float windowAspectRatio,
	int ordinaryInCorner,
	int frameIntervalWidth, int frameIntervalHeight,
	int windowMultW, int windowMultH,
	int * cornerWindowWidth, int * cornerWindowHeight,
	int * ordinaryWindowWidth, int * ordinaryWindowHeight,
	int * rightRows, int * rightColumns,
	int * bottomRows, int * bottomColumns)
{
	if (screenWidth - frameIntervalWidth * 2 < 0 || screenHeight - frameIntervalHeight * 2 < 0 || !numWindows)
		return false;

	int numRow = ordinaryInCorner;
	int numCol = 0;
	float winW, winH;

	while (numRow * numCol < (numWindows - 1) + ordinaryInCorner * ordinaryInCorner ||
		numRow < ordinaryInCorner ||
		numCol < ordinaryInCorner + 1)
	{
		numRow++;

		winH = float(screenHeight - frameIntervalHeight * (numRow + 1)) / float(numRow);

		if (winH <= 1.0f)
			return false;

		winW = winH * windowAspectRatio;

		if (winW <= 0.0f)
			return false;

		numCol = (screenWidth - frameIntervalWidth) / (int(winW) + frameIntervalWidth);
	}

	winW = static_cast<float>(MakeMultiple((int)winW, windowMultW));
	winH = winW / windowAspectRatio;
	winH = static_cast<float>(MakeMultiple((int)winH, windowMultH));

	*ordinaryWindowWidth = (int)winW;
	*ordinaryWindowHeight = (int)winH;
	if (cornerWindowWidth)	*cornerWindowWidth = (int)winW * ordinaryInCorner + (ordinaryInCorner - 1) * frameIntervalWidth;
	if (cornerWindowHeight)	*cornerWindowHeight = (int)winH * ordinaryInCorner + (ordinaryInCorner - 1) * frameIntervalHeight;

	if (rightColumns)	*rightColumns = numCol - ordinaryInCorner;
	if (rightRows)		*rightRows = ordinaryInCorner;
	if (bottomColumns)	*bottomColumns = numCol;
	if (bottomRows)		*bottomRows = numRow - ordinaryInCorner;

	return true;
}

bool CalculateCornerLayoutSmart(int screenWidth, int screenHeight,
	int numWindows,
	float windowAspectRatio,
	int ordinaryInCorner,
	int frameIntervalWidth, int frameIntervalHeight,
	int windowMultW, int windowMultH,
	int * cornerWindowWidth, int * cornerWindowHeight,
	int * ordinaryWindowWidth, int * ordinaryWindowHeight,
	int * rightRows, int * rightColumns,
	int * bottomRows, int * bottomColumns)
{
	int cornerWindowWidth1 = 0, cornerWindowHeight1 = 0;
	int ordinaryWindowWidth1 = 0, ordinaryWindowHeight1 = 0;
	int rightRows1 = 0, rightColumns1 = 0;
	int bottomRows1 = 0, bottomColumns1 = 0;

	int cornerWindowWidth2 = 0, cornerWindowHeight2 = 0;
	int ordinaryWindowWidth2 = 0, ordinaryWindowHeight2 = 0;
	int rightRows2 = 0, rightColumns2 = 0;
	int bottomRows2 = 0, bottomColumns2 = 0;

	if (!CalculateCornerLayoutRowBased(screenWidth, screenHeight,
		numWindows,
		windowAspectRatio,
		ordinaryInCorner,
		frameIntervalWidth, frameIntervalHeight,
		windowMultW, windowMultH,
		&cornerWindowWidth1, &cornerWindowHeight1,
		&ordinaryWindowWidth1, &ordinaryWindowHeight1,
		&rightRows1, &rightColumns1,
		&bottomRows1, &bottomColumns1))
	{
		ordinaryWindowWidth1 = 0;
		ordinaryWindowHeight1 = 0;
	}

	if (!CalculateCornerLayoutColumnBased(screenWidth, screenHeight,
		numWindows,
		windowAspectRatio,
		ordinaryInCorner,
		frameIntervalWidth, frameIntervalHeight,
		windowMultW, windowMultH,
		&cornerWindowWidth2, &cornerWindowHeight2,
		&ordinaryWindowWidth2, &ordinaryWindowHeight2,
		&rightRows2, &rightColumns2,
		&bottomRows2, &bottomColumns2))
	{
		ordinaryWindowWidth2 = 0;
		ordinaryWindowHeight2 = 0;
	}

	if (ordinaryWindowWidth1 * ordinaryWindowHeight1 == 0 &&
		ordinaryWindowWidth2 * ordinaryWindowHeight2 == 0)
		return false;

	if (ordinaryWindowWidth1 * ordinaryWindowHeight1 >
		ordinaryWindowWidth2 * ordinaryWindowHeight2)
	{
		*cornerWindowWidth = cornerWindowWidth1;
		*cornerWindowHeight = cornerWindowHeight1;
		*ordinaryWindowWidth = ordinaryWindowWidth1;
		*ordinaryWindowHeight = ordinaryWindowHeight1;
		if (rightRows)		*rightRows = rightRows1;
		if (rightColumns)	*rightColumns = rightColumns1;
		if (bottomRows)		*bottomRows = bottomRows1;
		if (bottomColumns)	*bottomColumns = bottomColumns1;
	}
	else
	{
		*cornerWindowWidth = cornerWindowWidth2;
		*cornerWindowHeight = cornerWindowHeight2;
		*ordinaryWindowWidth = ordinaryWindowWidth2;
		*ordinaryWindowHeight = ordinaryWindowHeight2;
		if (rightRows)		*rightRows = rightRows2;
		if (rightColumns)	*rightColumns = rightColumns2;
		if (bottomRows)		*bottomRows = bottomRows2;
		if (bottomColumns)	*bottomColumns = bottomColumns2;
	}

	return true;
}

/*******************************************************
/ Aspect ratio adaptive layout
/*******************************************************/

std::vector<std::vector<SFloatRect>> ComputeRectGridByAspectRatiosGrid(
	float screenWidth, float screenHeight,
	const std::vector<std::vector<float>>& aspectRatios)
{
	std::vector<std::vector<SFloatRect>> gridRects;

	for (const std::vector<float>& line : aspectRatios)
	{
		gridRects.emplace_back();

		for (const float& ar : line)
		{
			gridRects.back().emplace_back(ar / sqrt(ar), 1 / sqrt(ar));
		}
	}

	SFloatRect muxerSize(screenWidth, screenHeight);
	CAdaptiveGrid grid(gridRects);

	//grid.PrintDebug();

	grid.ExpandToRect(muxerSize);

	//grid.PrintDebug();
	//std::cout << std::endl;

	return grid.GetRects();
}

std::vector<std::vector<SFloatRect>> ComputeBestGrid(
	int screenWidth, int screenHeight,
	int frameIntervalWidth, int frameIntervalHeight,
	const std::vector<float>& aspectRatios,
	std::vector<size_t>& ordering,
	bool& isResultColumnBased)
{
	// compute partitions of N windows to lines
	CPartitionTree partitionTree(aspectRatios.size());

	std::vector<std::vector<uint16_t>> partitions = partitionTree.GetPartitions();

	// create vector for permutations and compute number of them
	std::vector<size_t> tmpPermutation(aspectRatios.size());
	size_t permutationNumber = 1;

	for (size_t i = 0; i < tmpPermutation.size(); i++)
		tmpPermutation[i] = i;

	for (size_t i = 1; i <= tmpPermutation.size(); i++)
		permutationNumber *= i;

	// main loop
	std::vector<std::vector<SFloatRect>> bestGrid;
	float bestGridSquare = 0.0f;
	float minGridSquareRatio = 0.50f; // maximal ratio between min and max squares of two rects in best layout

	for (const std::vector<uint16_t>& partition : partitions)
	{
		for (size_t p = 0; p < permutationNumber; p++)
		{
			std::vector<std::vector<float>> aspectRatiosGrid;
			size_t idx = 0;

			for (size_t i = 0; i < partition.size(); i++)
			{
				aspectRatiosGrid.emplace_back();

				for (size_t j = 0; j < partition[i]; j++)
				{
					aspectRatiosGrid.back().push_back(aspectRatios[tmpPermutation[idx]]);
					idx++;
				}
			}

			float aviableWidth = float(screenWidth - (partition[0] + 1) * frameIntervalWidth);
			float aviableHeight = float(screenHeight - (partition.size() + 1) * frameIntervalHeight);
			float tmpGridSquare = 0.0f;
			float minRectSquare = float(screenWidth * screenHeight);
			float maxRectSquare = 0.0f;

			/* compute row-based grid */
			std::vector<std::vector<SFloatRect>> tmpGrid = ComputeRectGridByAspectRatiosGrid(
				aviableWidth, aviableHeight, aspectRatiosGrid);

			for (const std::vector<SFloatRect>& line : tmpGrid)
			{
				for (const SFloatRect& r : line)
				{
					float square = r.W * r.H;

					tmpGridSquare += square;
					minRectSquare = std::min(minRectSquare, square);
					maxRectSquare = std::max(maxRectSquare, square);
				}
			}

			if (tmpGridSquare > bestGridSquare  && minRectSquare / maxRectSquare > minGridSquareRatio)
			{
				bestGridSquare = tmpGridSquare;
				bestGrid = std::move(tmpGrid);
				ordering = tmpPermutation;

				isResultColumnBased = false;
			}

			/* compute column-based grid ("mirroring" grid by diagonal)*/

			// invert aspect ratios
			for (std::vector<float>& line : aspectRatiosGrid)
			{
				for (float& ar : line)
				{
					ar = 1 / ar;
				}
			}

			// aviableWidth <-> screenHeight
			aviableWidth = float(screenWidth - (partition.size() + 1) * frameIntervalWidth);
			aviableHeight = float(screenHeight - (partition[0] + 1) * frameIntervalHeight);
			tmpGridSquare = 0.0f;
			minRectSquare = float(screenWidth * screenHeight);
			maxRectSquare = 0.0f;

			tmpGrid = ComputeRectGridByAspectRatiosGrid(
				aviableHeight, aviableWidth, aspectRatiosGrid);

			for (const std::vector<SFloatRect>& line : tmpGrid)
			{
				for (const SFloatRect& r : line)
				{
					float square = r.W * r.H;

					tmpGridSquare += square;
					minRectSquare = std::min(minRectSquare, square);
					maxRectSquare = std::max(maxRectSquare, square);
				}
			}

			if (tmpGridSquare > bestGridSquare && minRectSquare / maxRectSquare > minGridSquareRatio)
			{
				bestGridSquare = tmpGridSquare;
				bestGrid = std::move(tmpGrid);
				ordering = tmpPermutation;

				isResultColumnBased = true;
			}

			std::next_permutation(tmpPermutation.begin(), tmpPermutation.end());
		}
	}

	// if result is column-based, then "unmirroring" grid
	if (isResultColumnBased)
	{
		for (std::vector<SFloatRect>& line : bestGrid)
		{
			for (SFloatRect& r : line)
			{
				std::swap(r.W, r.H);
				r.AR = 1 / r.AR;
			}
		}
	}

	return bestGrid;
}
