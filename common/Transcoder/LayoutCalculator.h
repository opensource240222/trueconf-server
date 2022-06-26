#include "CAdaptiveGrid.h"

int MakeMultiple(int a, int b);
int MakeMultipleUp(int a, int b);

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
	int * columnsInPartialRow);

bool CalculateUniformLayoutRowBased(int screenWidth, int screenHeight,
	int numWindows,
	float windowAspectRatio,
	int frameIntervalWidth, int frameIntervalHeight,
	int windowMultW, int windowMultH,
	int maxWindowHeight,
	int * windowWidth, int * windowHeight,
	int * fullRows, int * fullColumns,
	int * columnsInPartialRow);

bool CalculateUniformLayoutSmart(int screenWidth, int screenHeight,
	int numWindows,
	float windowAspectRatio,
	int frameIntervalWidth, int frameIntervalHeight,
	int windowMultW, int windowMultH,
	int maxWindowWidth, int maxWindowHeight,
	int * windowWidth, int * windowHeight,
	int * fullRows, int * fullColumns,
	int * columnsInPartialRow);

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
	int * columnsInPartialRow);

bool CalculateTopLayoutOrdinaryBased(int screenWidth, int screenHeight,
	int numWindows,
	float windowAspectRatio,
	int frameIntervalWidth, int frameIntervalHeight,
	int windowMultW, int windowMultH,
	int * topWindowWidth, int * topWindowHeight,
	int * ordinaryWindowWidth, int * ordinaryWindowHeight,
	int * fullRows, int * fullColumns,
	int * columnsInPartialRow);

bool CalculateTopLayoutSmart(int screenWidth, int screenHeight,
	int numWindows,
	float windowAspectRatio,
	int frameIntervalWidth, int frameIntervalHeight,
	int windowMultW, int windowMultH,
	int * topWindowWidth, int * topWindowHeight,
	int * ordinaryWindowWidth, int * ordinaryWindowHeight,
	int * fullRows, int * fullColumns,
	int * columnsInPartialRow);

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
	int * columnsInPartialRow);

bool CalculateLeftLayoutPriorityBased(int screenWidth, int screenHeight,
	int numWindows,
	float windowAspectRatio,
	int frameIntervalWidth, int frameIntervalHeight,
	int windowMultW, int windowMultH,
	int * leftWindowWidth, int * leftWindowHeight,
	int * ordinaryWindowWidth, int * ordinaryWindowHeight,
	int * fullRows, int * fullColumns,
	int * columnsInPartialRow);

bool CalculateLeftLayoutSmart(int screenWidth, int screenHeight,
	int numWindows,
	float windowAspectRatio,
	int frameIntervalWidth, int frameIntervalHeight,
	int windowMultW, int windowMultH,
	int * leftWindowWidth, int * leftWindowHeight,
	int * ordinaryWindowWidth, int * ordinaryWindowHeight,
	int * fullRows, int * fullColumns,
	int * rowsInPartialColumn);

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
	int * bottomRows, int * bottomColumns);

bool CalculateCornerLayoutRowBased(int screenWidth, int screenHeight,
	int numWindows,
	float windowAspectRatio,
	int ordinaryInCorner,
	int frameIntervalWidth, int frameIntervalHeight,
	int windowMultW, int windowMultH,
	int * cornerWindowWidth, int * cornerWindowHeight,
	int * ordinaryWindowWidth, int * ordinaryWindowHeight,
	int * rightRows, int * rightColumns,
	int * bottomRows, int * bottomColumns);

bool CalculateCornerLayoutSmart(int screenWidth, int screenHeight,
	int numWindows,
	float windowAspectRatio,
	int ordinaryInCorner,
	int frameIntervalWidth, int frameIntervalHeight,
	int windowMultW, int windowMultH,
	int * cornerWindowWidth, int * cornerWindowHeight,
	int * ordinaryWindowWidth, int * ordinaryWindowHeight,
	int * rightRows, int * rightColumns,
	int * bottomRows, int * bottomColumns);

/*******************************************************
/ Aspect ratio adaptive layout
/*******************************************************/

std::vector<std::vector<SFloatRect>> ComputeRectGridByAspectRatiosGrid(
	float screenWidth, float screenHeight,
	const std::vector<std::vector<float>>& aspectRatios);

std::vector<std::vector<SFloatRect>> ComputeBestGrid(
	int screenWidth, int screenHeight,
	int frameIntervalWidth, int frameIntervalHeight,
	const std::vector<float>& aspectRatios,
	std::vector<size_t>& ordering,
	bool& isResultColumnBased);
