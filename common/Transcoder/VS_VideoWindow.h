#ifndef VS_VIDEOVINDOW
#define VS_VIDEOVINDOW

#include <vector>
#include <string>
#include "std/cpplib/json/elements.h"


class VS_WindowGrid
{
public:
	enum eWindowPriority
	{
		PRIORITY_UNK = -1,
		PRIORITY_LOW = 0,
		PRIORITY_HIGH = 1
	};

	enum ePriorityLayoutType
	{
		PRIORITY_LAYOUT_CORNER = 0,
		PRIORITY_LAYOUT_TOP,
		PRIORITY_LAYOUT_LEFT,
		PRIORITY_LAYOUT_OVERLAY
	};

	enum eLayoutAlgorithm
	{
		LAYOUT_ALGORITHM_FIXED = 0,
		LAYOUT_ALGORITHM_ADAPTIVE
	};

	struct sRect
	{
		sRect();
		sRect(int l, int t, int r, int b);

		int left;
		int top;
		int right;
		int bottom;
	};

	struct sOneWindow
	{
	public:
		sOneWindow();
		sOneWindow(const std::string &windowId, const sRect &windowRect);
		sOneWindow(const std::string &windowId, float ar);
		~sOneWindow();

		eWindowPriority priority;
		std::string userId;
		sRect rect;
		float aspectRatio;
	};

	VS_WindowGrid();
	~VS_WindowGrid();

	int							Init(int width, int height, float aspectRatio);
	int							Release();

	int							AddWindow(const std::string& userId, float aspectRatio = 0.0f);
	int							SetUserAspectRatio(const std::string& userId, float aspectRatio);
	int							Swap(const std::string& userId1, const std::string& userId2);
	int							RemoveWindow(const std::string& userId);
	int							RemoveAll();

	std::vector<sOneWindow>		GetAllWindows();
	int							GetAllWindows(json::Object &list);
	std::pair<int, int>         GetMainWindowSize() const;
	sOneWindow					GetWindow(const std::string& userId);
	ePriorityLayoutType			GetPriorityLayoutType();

	int							SetP0();
	int							SetP1(const std::string& userId);
	int							SetP2(const std::string& userId1, const std::string& userId2);

	bool						IsValidGrid();
	void						SetFrameInterval(int frameIntervalX, int frameIntervalY);
	void						SetWidowMultiplicity(int windowMultW, int windowMultH);
	void						SetWindowOffsetMultiplicity(int offsetMultW, int offsetMultH);
	void						SetScreenSize(int width, int height);
	void						SetAspectRatio(float aspectRatio);
	void						SetPriorityLayoutType(ePriorityLayoutType priorityLayoutType);
	void						SetLayoutAlgorithm(eLayoutAlgorithm layoutAlgorithm);

private:
	int m_width;
	int m_height;
	int m_frameIntervalX;
	int m_frameIntervalY;
	int m_windowMultW;
	int m_windowMultH;
	int m_offsetMultW;
	int m_offsetMultH;
	float m_aspectRatio;
	bool m_isValidGrid;
	bool m_isInit;

	ePriorityLayoutType m_priorityLayoutType;
	eLayoutAlgorithm m_layoutAlgorithm;
	std::vector<sOneWindow> m_videoWindows;

	int ComputeGrid();
	int	UpdateGrid();

	std::vector<sRect> GetMuxerGrid(int widthMuxer, int heightMuxer, int numStreams);
	int GetMuxerGridAdaptive(int widthMuxer, int heightMuxer, std::vector<sOneWindow>& result);
	int GetMuxerGridPriorityOne_TYPE1(int widthMuxer, int heightMuxer, int numStreams, std::vector<sRect> &priorWindows, std::vector<sRect> &ordinaryWindows);
	int GetMuxerGridPriorityOne_TYPE2(int widthMuxer, int heightMuxer, int numStreams, std::vector<sRect> &priorWindows, std::vector<sRect> &ordinaryWindows);
	int GetMuxerGridPriorityOne_TYPE3(int widthMuxer, int heightMuxer, int numStreams, std::vector<sRect> &priorWindows, std::vector<sRect> &ordinaryWindows);
	int GetMuxerGridPriorityOne_Overlay(int widthMuxer, int heightMuxer, int numStreams, std::vector<sRect> &priorWindows, std::vector<sRect> &ordinaryWindows);
	int GetMuxerGridPriorityTwo(int widthMuxer, int heightMuxer, int numStreams, std::vector<sRect> &priorWindows, std::vector<sRect> &ordinaryWindows);
};

#endif // VS_VIDEOVINDOW
