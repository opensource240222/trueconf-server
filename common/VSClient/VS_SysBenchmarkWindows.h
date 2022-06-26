#pragma once

#include "Transcoder/VS_SysBenchmarkBase.h"
#include "VSClient/VSClientBase.h"

class VS_SysBenchmarkWindows: public VS_SysBenchmarkBase, public CVSThread
{
private:
	HANDLE				m_BenchEvent;
	DWORD				Loop(LPVOID lpParameter);
	virtual bool		CheckNeedDecreaseRating() override;

public:
	VS_SysBenchmarkWindows();
	~VS_SysBenchmarkWindows();
	/// Init benchmark
	bool			Run();
	/// Levels
	HANDLE			GetBenchEvent() {return m_BenchEvent;}
};
