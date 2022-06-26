#include <algorithm>
#include <cmath>

#include "VS_VideoWindow.h"
#include "LayoutCalculator.h"
#include "std/cpplib/layout_json.h"

#pragma warning (default : 4700)

VS_WindowGrid::sRect::sRect()
{
	left = 0;
	top = 0;
	right = 0;
	bottom = 0;
}

VS_WindowGrid::sRect::sRect(int l, int t, int r, int b)
{
	left = l;
	top = t;
	right = r;
	bottom = b;
}

VS_WindowGrid::sOneWindow::sOneWindow()
{
	priority = PRIORITY_UNK;
	aspectRatio = 0.0f;
}

VS_WindowGrid::sOneWindow::sOneWindow(const std::string &windowId, const sRect &windowRect)
{
	userId = windowId;
	rect = windowRect;
	priority = PRIORITY_LOW;
	aspectRatio = float(rect.right - rect.left) / float(rect.bottom - rect.top);
}

VS_WindowGrid::sOneWindow::sOneWindow(const std::string &windowId, float ar)
{
	userId = windowId;
	priority = PRIORITY_LOW;
	aspectRatio = ar;
}

VS_WindowGrid::sOneWindow::~sOneWindow()
{}

VS_WindowGrid::VS_WindowGrid()
{
	m_width = 0;
	m_height = 0;
	m_frameIntervalX = 0;
	m_frameIntervalY = 0;
	m_windowMultW = 1;
	m_windowMultH = 1;
	m_offsetMultW = 1;
	m_offsetMultH = 1;
	m_aspectRatio = 1;
	m_isInit = false;
	m_isValidGrid = false;
	m_priorityLayoutType = PRIORITY_LAYOUT_CORNER;
	m_layoutAlgorithm = LAYOUT_ALGORITHM_FIXED;
}

VS_WindowGrid::~VS_WindowGrid()
{
	Release();
}

int VS_WindowGrid::Init(int width, int height, float aspectRatio)
{
	Release();

	SetScreenSize(width, height);
	SetAspectRatio(aspectRatio);

	m_isInit = true;
	return 0;
}

int VS_WindowGrid::Release()
{
	m_videoWindows.clear();

	m_isInit = false;
	return 0;
}

int	VS_WindowGrid::AddWindow(const std::string& userId, float aspectRatio)
{
	for (size_t i = 0; i < m_videoWindows.size(); i++)
	{
		if (m_videoWindows[i].userId == userId)
			return -1;
	}

	if (aspectRatio == 0.0f)
		aspectRatio = m_aspectRatio;

	m_videoWindows.emplace_back(userId, aspectRatio);

	m_isValidGrid = false;
	return 0;
}

int VS_WindowGrid::SetUserAspectRatio(const std::string& userId, float aspectRatio)
{
	for (size_t i = 0; i < m_videoWindows.size(); i++)
	{
		if (m_videoWindows[i].userId == userId)
		{
			m_videoWindows[i].aspectRatio = aspectRatio;

			m_isValidGrid = false;
			return 0;
		}
	}

	return -1;
}

int	VS_WindowGrid::RemoveWindow(const std::string& userId)
{
	for (size_t i = 0; i < m_videoWindows.size(); i++)
	{
		if (m_videoWindows[i].userId == userId)
			m_videoWindows.erase(m_videoWindows.begin() + i);
	}

	m_isValidGrid = false;
	return 0;
}

int	VS_WindowGrid::SetP0()
{
	for (size_t i = 0; i < m_videoWindows.size(); i++)
	{
		m_videoWindows[i].priority = PRIORITY_LOW;
	}

	m_isValidGrid = false;
	return 0;
}

int	VS_WindowGrid::SetP1(const std::string& userId1)
{
	int foundRes = -1;

	for (size_t i = 0; i < m_videoWindows.size(); i++)
	{
		if (m_videoWindows[i].userId == userId1)
		{
			foundRes = i;
			break;
		}
	}

	if (foundRes == -1)
		return -1;

	SetP0(); // reset priority

	m_videoWindows[foundRes].priority = PRIORITY_HIGH;
	m_isValidGrid = false;

	return 0;
}

int	VS_WindowGrid::SetP2(const std::string& userId1, const std::string& userId2)
{
	int foundRes1 = -1;
	int foundRes2 = -1;

	for (size_t i = 0; i < m_videoWindows.size(); i++)
	{
		if (m_videoWindows[i].userId == userId1)
			foundRes1 = i;

		if (m_videoWindows[i].userId == userId2)
			foundRes2 = i;
	}

	if (foundRes1 == -1 || foundRes2 == -1 || (foundRes1 == foundRes2))
		return -1;

	SetP0(); // reset priority

	m_videoWindows[foundRes1].priority = PRIORITY_HIGH;
	m_videoWindows[foundRes2].priority = PRIORITY_HIGH;
	m_isValidGrid = false;

	return 0;
}

VS_WindowGrid::sOneWindow VS_WindowGrid::GetWindow(const std::string& userId)
{
	if (!m_isInit)
		return sOneWindow();

	if (!m_isValidGrid)
		ComputeGrid();

	for (size_t i = 0; i < m_videoWindows.size(); i++)
	{
		if (m_videoWindows[i].userId == userId)
			return sOneWindow(m_videoWindows[i]);
	}

	return sOneWindow();
}

VS_WindowGrid::ePriorityLayoutType VS_WindowGrid::GetPriorityLayoutType()
{
	return m_priorityLayoutType;
}

void VS_WindowGrid::SetFrameInterval(int frameIntervalX, int frameIntervalY)
{
	m_frameIntervalX = frameIntervalX;
	m_frameIntervalY = frameIntervalY;
	m_isValidGrid = false;
}

void VS_WindowGrid::SetWidowMultiplicity(int windowMultW, int windowMultH)
{
	m_windowMultW = windowMultW;
	m_windowMultH = windowMultH;
	m_isValidGrid = false;
}

void VS_WindowGrid::SetWindowOffsetMultiplicity(int offsetMultW, int offsetMultH)
{
	m_offsetMultW = offsetMultW;
	m_offsetMultH = offsetMultH;
	m_isValidGrid = false;
}

void VS_WindowGrid::SetScreenSize(int width, int height)
{
	m_width = width;
	m_height = height;
	m_isValidGrid = false;
}

void VS_WindowGrid::SetAspectRatio(float aspectRatio)
{
	m_aspectRatio = aspectRatio;
	m_isValidGrid = false;
}

void VS_WindowGrid::SetPriorityLayoutType(ePriorityLayoutType priorityLayoutType)
{
	m_priorityLayoutType = priorityLayoutType;
	m_isValidGrid = false;
}

void VS_WindowGrid::SetLayoutAlgorithm(eLayoutAlgorithm layoutAlgorithm)
{
	m_layoutAlgorithm = layoutAlgorithm;
	m_isValidGrid = false;
}

int VS_WindowGrid::ComputeGrid()
{
	if ((!m_isInit) && (!m_videoWindows.size()))
		return -1;

	// indexes of priority and ordinary windows
	std::vector<int> prioritySet;
	std::vector<int> ordinarySet;
	int countPriorityHigh;
	sRect emptyRect;

	for (size_t i = 0; i < m_videoWindows.size(); i++)
		if (m_videoWindows[i].priority == PRIORITY_HIGH)
			prioritySet.push_back(i);
		else
			ordinarySet.push_back(i);

	countPriorityHigh = prioritySet.size();

	if (countPriorityHigh == 0)
	{
		if (m_layoutAlgorithm == LAYOUT_ALGORITHM_FIXED)
		{
			std::vector<sRect> rects = GetMuxerGrid(m_width, m_height, m_videoWindows.size()); //retrieve ordinary mixer grid

			for (size_t i = 0; i < m_videoWindows.size(); i++)
				if (rects.size())
					m_videoWindows[i].rect = rects[i];
				else
					m_videoWindows[i].rect = emptyRect;
		}
		else if (m_layoutAlgorithm == LAYOUT_ALGORITHM_ADAPTIVE)
		{
			GetMuxerGridAdaptive(m_width, m_height, m_videoWindows);
		}
	}
	else
	{
		std::vector<sRect> priorWindows;
		std::vector<sRect> ordinaryWindows;

		if (countPriorityHigh == 1)
		{
			if (m_priorityLayoutType == PRIORITY_LAYOUT_CORNER)
				GetMuxerGridPriorityOne_TYPE1(m_width, m_height, m_videoWindows.size(), priorWindows, ordinaryWindows);

			if (m_priorityLayoutType == PRIORITY_LAYOUT_TOP)
				GetMuxerGridPriorityOne_TYPE2(m_width, m_height, m_videoWindows.size(), priorWindows, ordinaryWindows);

			if (m_priorityLayoutType == PRIORITY_LAYOUT_LEFT)
				GetMuxerGridPriorityOne_TYPE3(m_width, m_height, m_videoWindows.size(), priorWindows, ordinaryWindows);

			if (m_priorityLayoutType == PRIORITY_LAYOUT_OVERLAY)
				GetMuxerGridPriorityOne_Overlay(m_width, m_height, m_videoWindows.size(), priorWindows, ordinaryWindows);
		}
		else if (countPriorityHigh == 2)
		{
			GetMuxerGridPriorityTwo(m_width, m_height, m_videoWindows.size(), priorWindows, ordinaryWindows);
		}

		for (size_t i = 0; i < prioritySet.size(); i++)
			if (priorWindows.size())
				m_videoWindows[prioritySet[i]].rect = priorWindows[i];
			else
				m_videoWindows[prioritySet[i]].rect = emptyRect;

		for (size_t i = 0; i < ordinarySet.size(); i++)
			if (ordinaryWindows.size())
				m_videoWindows[ordinarySet[i]].rect = ordinaryWindows[i];
			else
				m_videoWindows[ordinarySet[i]].rect = emptyRect;
	}

	for (size_t i = 0; i < m_videoWindows.size(); i++)
	{
		m_videoWindows[i].rect.left = MakeMultipleUp(m_videoWindows[i].rect.left, m_offsetMultW);
		m_videoWindows[i].rect.top = MakeMultipleUp(m_videoWindows[i].rect.top, m_offsetMultH);
		m_videoWindows[i].rect.right = m_videoWindows[i].rect.left + MakeMultiple(m_videoWindows[i].rect.right - m_videoWindows[i].rect.left, m_windowMultW);
		m_videoWindows[i].rect.bottom = m_videoWindows[i].rect.top + MakeMultiple(m_videoWindows[i].rect.bottom - m_videoWindows[i].rect.top, m_windowMultH);
	}

	m_isValidGrid = true;

	return 0;
}

bool VS_WindowGrid::IsValidGrid()
{
	if (!m_isInit)
		return false;

	return m_isValidGrid;
}

int VS_WindowGrid::UpdateGrid()
{
	if (!m_isInit)
		return -1;

	ComputeGrid();

	return 0;
}

std::vector<VS_WindowGrid::sOneWindow> VS_WindowGrid::GetAllWindows()
{
	if (!m_isInit)
		return std::vector<sOneWindow>();

	if (!m_isValidGrid)
		ComputeGrid();

	return m_videoWindows;
}

int	VS_WindowGrid::GetAllWindows(json::Object &list)
{
	if (!m_isInit)
		return -1;

	if (!m_isValidGrid)
		ComputeGrid();

	int size = m_videoWindows.size();

	json::Array part_list;
	for (auto &it : m_videoWindows) {
		json::Object part;

		part[layout_json::id] = json::String(it.userId);
		part[layout_json::priority] = json::Boolean(it.priority != 0);
		json::Array coord;
		coord.Insert(json::Number(it.rect.left));
		coord.Insert(json::Number(it.rect.top));
		coord.Insert(json::Number(it.rect.right));
		coord.Insert(json::Number(it.rect.bottom));
		part[layout_json::rect] = coord;

		part_list.Insert(part);
	}
	list["list"] = part_list;

	return size;
}


std::pair<int, int> VS_WindowGrid::GetMainWindowSize() const
{
	return std::make_pair(m_width, m_height);
}

int	VS_WindowGrid::Swap(const std::string& id1, const std::string& id2)
{
	if (!m_isInit)
		return -1;

	int res1, res2;

	res1 = -1;
	res2 = -1;

	for (size_t i = 0; i < m_videoWindows.size(); i++)
	{
		if (m_videoWindows[i].userId == id1)
			res1 = i;
		else if (m_videoWindows[i].userId == id2)
			res2 = i;
	}

	if (res1 < 0 || res2 < 0 || res1 == res2)
		return -1;

	//std::swap(m_videoWindows[res1], m_videoWindows[res2]);
	m_videoWindows[res1].userId = id2;
	m_videoWindows[res2].userId = id1;
	m_isValidGrid = false;

	return 0;
}

int	VS_WindowGrid::RemoveAll()
{
	m_videoWindows.clear();

	return 0;
}

std::vector<VS_WindowGrid::sRect> VS_WindowGrid::GetMuxerGrid(int widthMuxer, int heightMuxer, int numStreams)
{
	std::vector<sRect> mixerGrid;

	if (numStreams == 0)
		return mixerGrid;

	int windowWidth, windowHeight;
	int fullRows, fullColumns;
	int columnsInPartialRow;

	if (!CalculateUniformLayoutSmart(widthMuxer, heightMuxer,
		numStreams,
		m_aspectRatio,
		m_frameIntervalX, m_frameIntervalY,
		m_windowMultW, m_windowMultH,
		widthMuxer, heightMuxer,
		&windowWidth, &windowHeight,
		&fullRows, &fullColumns,
		&columnsInPartialRow))
	{
		return mixerGrid;
	}

	sRect rect;

	int widthOffset = (widthMuxer - (fullColumns * (windowWidth + m_frameIntervalX) + m_frameIntervalX)) / 2;
	int heightOffset = (heightMuxer - ((fullRows + (columnsInPartialRow ? 1 : 0)) * (windowHeight + m_frameIntervalY) + m_frameIntervalY)) / 2;

	for (int row = 0; row < fullRows; row++)
	{
		for (int col = 0; col < fullColumns; col++)
		{
			rect.left = widthOffset + m_frameIntervalX + (windowWidth + m_frameIntervalX) * col;
			rect.top = heightOffset + m_frameIntervalY + (windowHeight + m_frameIntervalY) * row;
			rect.right = rect.left + windowWidth;
			rect.bottom = rect.top + windowHeight;

			mixerGrid.push_back(rect);
		}
	}

	if (columnsInPartialRow)
	{
		int partialWidthOffset = (widthMuxer - widthOffset * 2 - columnsInPartialRow * (windowWidth + m_frameIntervalX) - m_frameIntervalX) / 2;

		for (int col = 0; col < columnsInPartialRow; col++)
		{
			rect.left = widthOffset + partialWidthOffset + m_frameIntervalX + (windowWidth + m_frameIntervalX) * col;
			rect.top = heightOffset + m_frameIntervalY + (windowHeight + m_frameIntervalY) * fullRows;
			rect.right = rect.left + windowWidth;
			rect.bottom = rect.top + windowHeight;

			mixerGrid.push_back(rect);
		}
	}

	return mixerGrid;
}

int VS_WindowGrid::GetMuxerGridAdaptive(int widthMuxer, int heightMuxer, std::vector<sOneWindow>& windows)
{
	std::vector<float> aspectRatios;

	for (sOneWindow& w : windows)
		aspectRatios.push_back(w.aspectRatio);

	bool isResultColumnBased = false;
	std::vector<size_t> ordering;

	std::vector<std::vector<SFloatRect>> resultRects = ComputeBestGrid(
		widthMuxer, heightMuxer,
		m_frameIntervalX, m_frameIntervalY,
		aspectRatios,
		ordering, isResultColumnBased);

	size_t heightOffset = 0;
	size_t widthOffset = 0;
	size_t winNumber = 0;

	if (isResultColumnBased)
	{
		widthOffset = m_frameIntervalX;

		for (const std::vector<SFloatRect>& line : resultRects)
		{
			size_t lineWidth = 0;
			size_t lineHeight = 0;

			for (const SFloatRect& rect : line)
			{
				lineWidth = std::max(lineWidth, size_t(rect.W));
				lineHeight += size_t(rect.H);
			}

			heightOffset = (heightMuxer - (lineHeight + (line.size() - 1) * m_frameIntervalY)) / 2;

			for (const SFloatRect& rect : line)
			{
				size_t currWin = ordering[winNumber];
				size_t rectWidthOffset = (lineWidth - size_t(rect.W)) / 2;

				windows[currWin].rect.left = widthOffset + rectWidthOffset;
				windows[currWin].rect.top = heightOffset;
				windows[currWin].rect.right = windows[currWin].rect.left + size_t(rect.W);
				windows[currWin].rect.bottom = windows[currWin].rect.top + size_t(rect.H);

				heightOffset += size_t(rect.H) + m_frameIntervalY;

				winNumber++;
			}

			heightOffset = 0;
			widthOffset += lineWidth + m_frameIntervalX;
		}

		if (size_t(widthMuxer) > widthOffset)
		{
			widthOffset = (widthMuxer - widthOffset) / 2; // widthOffset == offset from screen border

			for (sOneWindow& w : windows)
			{
				w.rect.left += widthOffset;
				w.rect.right += widthOffset;
			}
		}
	}
	else
	{
		heightOffset = m_frameIntervalY;

		for (const std::vector<SFloatRect>& line : resultRects)
		{
			size_t lineWidth = 0;
			size_t lineHeight = 0;

			for (const SFloatRect& rect : line)
			{
				lineWidth += size_t(rect.W);
				lineHeight = std::max(lineHeight, size_t(rect.H));
			}

			widthOffset = (widthMuxer - (lineWidth + (line.size() - 1) * m_frameIntervalX)) / 2;

			for (const SFloatRect& rect : line)
			{
				size_t currWin = ordering[winNumber];
				size_t rectHeightOffset = (lineHeight - size_t(rect.H)) / 2;

				windows[currWin].rect.left = widthOffset;
				windows[currWin].rect.top = heightOffset + rectHeightOffset;
				windows[currWin].rect.right = windows[currWin].rect.left + size_t(rect.W);
				windows[currWin].rect.bottom = windows[currWin].rect.top + size_t(rect.H);

				widthOffset += size_t(rect.W) + m_frameIntervalX;

				winNumber++;
			}

			heightOffset += lineHeight + m_frameIntervalY;
			widthOffset = 0;
		}

		if (size_t(heightMuxer) > heightOffset)
		{
			heightOffset = (heightMuxer - heightOffset) / 2; // widthOffset == offset from screen border

			for (sOneWindow& w : windows)
			{
				w.rect.top += heightOffset;
				w.rect.bottom += heightOffset;
			}
		}
	}

	return 0;
}

int VS_WindowGrid::GetMuxerGridPriorityOne_TYPE1(int widthMuxer, int heightMuxer, int numStreams, std::vector<sRect> &priorWindows, std::vector<sRect> &ordinaryWindows)
{
	if (numStreams < 2)
	{
		priorWindows = GetMuxerGrid(widthMuxer, heightMuxer, numStreams);
		return 0;
	}

	int ordinaryInCorner = 0;

	switch (numStreams)
	{
		case 2: ordinaryInCorner = 3; break;

		case 3: ordinaryInCorner = 2; break;

		case 4: ordinaryInCorner = 3; break;

		default:
		{
			ordinaryInCorner = (numStreams - 1) / 2;

			if (ordinaryInCorner < 2)
				ordinaryInCorner = 2;

			if (ordinaryInCorner > 5)
				ordinaryInCorner = int(sqrt(float(numStreams)));

			break;
		}
	}

	int cornerWindowWidth = 0, cornerWindowHeight = 0;
	int ordinaryWindowWidth = 0, ordinaryWindowHeight = 0;
	int rightRows = 0, rightColumns = 0;
	int bottomRows = 0, bottomColumns = 0;

	if (!CalculateCornerLayoutSmart(widthMuxer, heightMuxer,
		numStreams,
		m_aspectRatio,
		ordinaryInCorner,
		m_frameIntervalX, m_frameIntervalY,
		m_windowMultW, m_windowMultH,
		&cornerWindowWidth, &cornerWindowHeight,
		&ordinaryWindowWidth, &ordinaryWindowHeight,
		&rightRows, &rightColumns,
		&bottomRows, &bottomColumns))
	{
		priorWindows = GetMuxerGrid(widthMuxer, heightMuxer, 1);
		return 0;
	}

	int widthOffset = (widthMuxer - (cornerWindowWidth + m_frameIntervalX * 2 + rightColumns * (ordinaryWindowWidth + m_frameIntervalX))) / 2;
	int heightOffset = 0;
	int row = 0;
	int col = 0;
	int currWin = 0;
	sRect rect;

	if (numStreams <= rightRows * rightColumns)
		heightOffset = (heightMuxer - (cornerWindowHeight + m_frameIntervalY * 2)) / 2;
	else
	{
		int rowsUnderCorner = ((numStreams - 1) - rightRows * rightColumns) / bottomColumns +
			((((numStreams - 1) - rightRows * rightColumns) % bottomColumns) ? 1 : 0);

		heightOffset = (heightMuxer - (cornerWindowHeight + m_frameIntervalY * 2 +
			rowsUnderCorner * (ordinaryWindowHeight + m_frameIntervalY))) / 2;
	}

	rect.left = widthOffset + m_frameIntervalX;
	rect.top = heightOffset + m_frameIntervalY;
	rect.right = rect.left + cornerWindowWidth;
	rect.bottom = rect.top + cornerWindowHeight;

	priorWindows.push_back(rect);

	while (currWin < numStreams - 1)
	{
		rect.left = widthOffset + cornerWindowWidth + m_frameIntervalX * 2 + col * (ordinaryWindowWidth + m_frameIntervalX);
		rect.top = heightOffset + m_frameIntervalY + row * (ordinaryWindowHeight + m_frameIntervalY);
		rect.right = rect.left + ordinaryWindowWidth;
		rect.bottom = rect.top + ordinaryWindowHeight;

		ordinaryWindows.push_back(rect);

		currWin++;
		col++;

		if (col == rightColumns)
		{
			col = 0;
			row++;

			if (row == rightRows)
				break;
		}
	}

	row = 0;
	col = 0;

	while (currWin < numStreams - 1)
	{
		rect.left = widthOffset + m_frameIntervalX + col * (ordinaryWindowWidth + m_frameIntervalX);
		rect.top = heightOffset + cornerWindowHeight + m_frameIntervalY * 2 + row * (ordinaryWindowHeight + m_frameIntervalY);
		rect.right = rect.left + ordinaryWindowWidth;
		rect.bottom = rect.top + ordinaryWindowHeight;

		ordinaryWindows.push_back(rect);

		currWin++;
		col++;

		if (col == bottomColumns)
		{
			col = 0;
			row++;

			if (row == bottomRows)
				break;
		}
	}

	return 0;
}

int VS_WindowGrid::GetMuxerGridPriorityOne_TYPE2(int widthMuxer, int heightMuxer, int numStreams, std::vector<sRect> &priorWindows, std::vector<sRect> &ordinaryWindows)
{
	if (numStreams < 2)
	{
		priorWindows = GetMuxerGrid(widthMuxer, heightMuxer, numStreams);
		return 0;
	}

	int priorityWindowWidth = 0, priorityWindowHeight = 0;
	int ordinaryWindowWidth = 0, ordinaryWindowHeight = 0;
	int fullRows = 0, fullColumns = 0;
	int columnsInPartialRow = 0;

	if (!CalculateTopLayoutSmart(widthMuxer, heightMuxer,
		numStreams,
		m_aspectRatio,
		m_frameIntervalX, m_frameIntervalY,
		m_windowMultW, m_windowMultH,
		&priorityWindowWidth, &priorityWindowHeight,
		&ordinaryWindowWidth, &ordinaryWindowHeight,
		&fullRows, &fullColumns,
		&columnsInPartialRow))
	{
		priorWindows = GetMuxerGrid(widthMuxer, heightMuxer, 1);
		return 0;
	};

	int widthOffset = (widthMuxer - (priorityWindowWidth + m_frameIntervalX * 2)) / 2;
	int heightOffset = (heightMuxer - (priorityWindowHeight + m_frameIntervalY * 2 + (fullRows + (columnsInPartialRow ? 1 : 0)) * (ordinaryWindowHeight + m_frameIntervalY))) / 2;

	sRect rect;

	rect.left = widthOffset + m_frameIntervalX;
	rect.top = heightOffset + m_frameIntervalY;
	rect.right = rect.left + priorityWindowWidth;
	rect.bottom = rect.top + priorityWindowHeight;

	priorWindows.push_back(rect);

	widthOffset = (widthMuxer - (ordinaryWindowWidth + (fullColumns - 1) * (ordinaryWindowWidth + m_frameIntervalX))) / 2;
	heightOffset += priorityWindowHeight + m_frameIntervalY;

	if (columnsInPartialRow)
	{
		int partialWidthOffset = (widthMuxer - widthOffset * 2 - (ordinaryWindowWidth + (columnsInPartialRow - 1) * (ordinaryWindowWidth + m_frameIntervalX))) / 2;

		for (int col = 0; col < columnsInPartialRow; col++)
		{
			rect.left = widthOffset + partialWidthOffset + (ordinaryWindowWidth + m_frameIntervalX) * col;
			rect.top = heightOffset + m_frameIntervalY;
			rect.right = rect.left + ordinaryWindowWidth;
			rect.bottom = rect.top + ordinaryWindowHeight;

			ordinaryWindows.push_back(rect);
		}

		heightOffset += m_frameIntervalY + ordinaryWindowHeight;
	}

	for (int col = 0; col < fullColumns; col++)
	{
		for (int row = 0; row < fullRows; row++)
		{
			rect.left = widthOffset + (ordinaryWindowWidth + m_frameIntervalX) * col;
			rect.top = heightOffset + m_frameIntervalY + (ordinaryWindowHeight + m_frameIntervalY) * row;
			rect.right = rect.left + ordinaryWindowWidth;
			rect.bottom = rect.top + ordinaryWindowHeight;

			ordinaryWindows.push_back(rect);
		}
	}

	return 0;
}

int VS_WindowGrid::GetMuxerGridPriorityOne_TYPE3(int widthMuxer, int heightMuxer, int numStreams, std::vector<sRect> &priorWindows, std::vector<sRect> &ordinaryWindows)
{
	if (numStreams < 2)
	{
		priorWindows = GetMuxerGrid(widthMuxer, heightMuxer, numStreams);
		return 0;
	}

	int priorityWindowWidth = 0, priorityWindowHeight = 0;
	int ordinaryWindowWidth = 0, ordinaryWindowHeight = 0;
	int fullRows = 0, fullColumns = 0;
	int rowsInPartialColumn = 0;

	if (!CalculateLeftLayoutSmart(widthMuxer, heightMuxer,
		numStreams,
		m_aspectRatio,
		m_frameIntervalX, m_frameIntervalY,
		m_windowMultW, m_windowMultH,
		&priorityWindowWidth, &priorityWindowHeight,
		&ordinaryWindowWidth, &ordinaryWindowHeight,
		&fullRows, &fullColumns,
		&rowsInPartialColumn))
	{
		priorWindows = GetMuxerGrid(widthMuxer, heightMuxer, 1);
		return 0;
	}

	int widthOffset = (widthMuxer - (priorityWindowWidth + m_frameIntervalX * 2
		+ fullColumns * (ordinaryWindowWidth + m_frameIntervalX)
		+ (rowsInPartialColumn ? 1 : 0) * (ordinaryWindowWidth + m_frameIntervalX))) / 2;
	int partialWidthOffset = (widthMuxer - widthOffset * 2 - priorityWindowWidth - m_frameIntervalX
		- (rowsInPartialColumn * (ordinaryWindowWidth + m_frameIntervalX) + m_frameIntervalX)) / 2;
	int heightOffset = (heightMuxer - (priorityWindowHeight + m_frameIntervalY * 2)) / 2;

	sRect rect;

	rect.left = widthOffset + m_frameIntervalX;
	rect.top = heightOffset + m_frameIntervalY;
	rect.right = rect.left + priorityWindowWidth;
	rect.bottom = rect.top + priorityWindowHeight;

	priorWindows.push_back(rect);

	widthOffset += priorityWindowWidth + m_frameIntervalX;
	heightOffset = (heightMuxer - (rowsInPartialColumn * (ordinaryWindowHeight + m_frameIntervalY) + m_frameIntervalY)) / 2;

	if (rowsInPartialColumn)
	{
		for (int row = 0; row < rowsInPartialColumn; row++)
		{
			rect.left = widthOffset + m_frameIntervalX;
			rect.top = heightOffset + m_frameIntervalY + (ordinaryWindowHeight + m_frameIntervalY) * row;
			rect.right = rect.left + ordinaryWindowWidth;
			rect.bottom = rect.top + ordinaryWindowHeight;

			ordinaryWindows.push_back(rect);
		}

		widthOffset += ordinaryWindowWidth + m_frameIntervalX;
	}

	heightOffset = (heightMuxer - (fullRows * (ordinaryWindowHeight + m_frameIntervalY) + m_frameIntervalY)) / 2;

	for (int col = 0; col < fullColumns; col++)
	{
		for (int row = 0; row < fullRows; row++)
		{
			rect.left = widthOffset + m_frameIntervalX + (ordinaryWindowWidth + m_frameIntervalX) * col;
			rect.top = heightOffset + m_frameIntervalY + (ordinaryWindowHeight + m_frameIntervalY) * row;
			rect.right = rect.left + ordinaryWindowWidth;
			rect.bottom = rect.top + ordinaryWindowHeight;

			ordinaryWindows.push_back(rect);
		}
	}

	return 0;
}

int VS_WindowGrid::GetMuxerGridPriorityOne_Overlay(int widthMuxer, int heightMuxer, int numStreams, std::vector<sRect> &priorWindows, std::vector<sRect> &ordinaryWindows)
{
	if (numStreams < 2)
	{
		priorWindows = GetMuxerGrid(widthMuxer, heightMuxer, numStreams);
		return 0;
	}

	int priorityWindowWidth = 0, priorityWindowHeight = 0;
	int ordinaryWindowWidth = 0, ordinaryWindowHeight = 0;
	int fullRows = 0, fullColumns = 0;
	int columnsInPartialRow = 0;

	if (!CalculateUniformLayoutSmart(widthMuxer, heightMuxer,
		1,
		m_aspectRatio,
		m_frameIntervalX, m_frameIntervalY,
		m_windowMultW, m_windowMultH,
		widthMuxer, heightMuxer,
		&priorityWindowWidth, &priorityWindowHeight,
		NULL, NULL,
		NULL))
	{
		return -1;
	};

	int heightToOrdinary = heightMuxer - (priorityWindowHeight + m_frameIntervalY);
	int maxOrdinaryWidth = priorityWindowWidth / 3;
	int maxOrdinaryHeight = priorityWindowHeight / 3;

	if (float(heightToOrdinary) / float(heightMuxer) < 0.1f)
	{
		priorityWindowHeight = priorityWindowHeight * 9 / 10;
		priorityWindowWidth = priorityWindowWidth * 9 / 10;

		heightToOrdinary = heightMuxer * 2 / 10;
	}
	else if (float(heightToOrdinary) / float(heightMuxer) <= 0.2f)
	{
		heightToOrdinary = heightMuxer * 2 / 10;
	}
	else
	{
		heightToOrdinary = heightMuxer;
	}

	if (!CalculateUniformLayoutSmart(widthMuxer, heightToOrdinary,
		numStreams - 1,
		m_aspectRatio,
		m_frameIntervalX, m_frameIntervalY,
		m_windowMultW, m_windowMultH,
		maxOrdinaryWidth, maxOrdinaryHeight,
		&ordinaryWindowWidth, &ordinaryWindowHeight,
		&fullRows, &fullColumns,
		&columnsInPartialRow))
	{
		priorWindows = GetMuxerGrid(widthMuxer, heightMuxer, 1);
		return 0;
	};

	if (fullRows + (columnsInPartialRow ? 1 : 0) > 1 ||
		priorityWindowHeight + ordinaryWindowHeight + m_frameIntervalX * 3 < heightMuxer)
	{
		return GetMuxerGridPriorityOne_TYPE2(widthMuxer, heightMuxer, numStreams, priorWindows, ordinaryWindows);
	}

	int widthOffset = (widthMuxer - (priorityWindowWidth)) / 2;
	int heightOffset = 0;

	sRect rect;

	rect.left = widthOffset;
	rect.top = heightOffset + m_frameIntervalY;
	rect.right = rect.left + priorityWindowWidth;
	rect.bottom = rect.top + priorityWindowHeight;

	priorWindows.push_back(rect);

	widthOffset = (widthMuxer - (fullColumns * (ordinaryWindowWidth + m_frameIntervalX) - m_frameIntervalX)) / 2;
	heightOffset = heightMuxer -
		((fullRows + (columnsInPartialRow ? 1 : 0)) * (ordinaryWindowHeight + m_frameIntervalY) + m_frameIntervalY);

	if (columnsInPartialRow)
	{
		int partialWidthOffset = (widthMuxer - widthOffset * 2 - (columnsInPartialRow * (ordinaryWindowWidth + m_frameIntervalX) - m_frameIntervalX)) / 2;

		for (int col = 0; col < columnsInPartialRow; col++)
		{
			rect.left = widthOffset + partialWidthOffset + (ordinaryWindowWidth + m_frameIntervalX) * col;
			rect.top = heightOffset + m_frameIntervalY;
			rect.right = rect.left + ordinaryWindowWidth;
			rect.bottom = rect.top + ordinaryWindowHeight;

			ordinaryWindows.push_back(rect);
		}

		heightOffset += m_frameIntervalY + ordinaryWindowHeight;
	}

	for (int col = 0; col < fullColumns; col++)
	{
		for (int row = 0; row < fullRows; row++)
		{
			rect.left = widthOffset + (ordinaryWindowWidth + m_frameIntervalX) * col;
			rect.top = heightOffset + m_frameIntervalY + (ordinaryWindowHeight + m_frameIntervalY) * row;
			rect.right = rect.left + ordinaryWindowWidth;
			rect.bottom = rect.top + ordinaryWindowHeight;

			ordinaryWindows.push_back(rect);
		}
	}

	return 0;
}

int VS_WindowGrid::GetMuxerGridPriorityTwo(int widthMuxer, int heightMuxer, int numStreams, std::vector<sRect>& priorWindows, std::vector<sRect>& ordinaryWindows)
{
	if (numStreams < 3)
	{
		priorWindows = GetMuxerGrid(widthMuxer, heightMuxer, numStreams);
		return 0;
	}

	int numPriorityWindows = 2;
	int numOrdinaryWindows = numStreams - numPriorityWindows;

	int priorityWindowWidth = 0, priorityWindowHeight = 0;
	int fullRows = 0, fullColumns = 0;
	int columnsInPartialRow = 0;

	if (!CalculateUniformLayoutSmart(widthMuxer, heightMuxer * 2 / 3,
		numPriorityWindows,
		m_aspectRatio,
		m_frameIntervalX, m_frameIntervalY,
		m_windowMultW, m_windowMultH,
		widthMuxer, heightMuxer,
		&priorityWindowWidth, &priorityWindowHeight,
		&fullRows, &fullColumns,
		&columnsInPartialRow))
	{
		return -1;
	};

	int ordinaryHeightOffset = (fullRows + (columnsInPartialRow ? 1 : 0)) * (priorityWindowHeight + m_frameIntervalY) + m_frameIntervalY;

	priorWindows = GetMuxerGrid(widthMuxer, ordinaryHeightOffset,
		numPriorityWindows);

	ordinaryWindows = GetMuxerGrid(widthMuxer, heightMuxer - ordinaryHeightOffset,
		numOrdinaryWindows);

	for (size_t i = 0; i < ordinaryWindows.size(); i++)
	{
		ordinaryWindows[i].top += ordinaryHeightOffset;
		ordinaryWindows[i].bottom += ordinaryHeightOffset;
	}

	return 0;
}
