#pragma once

#include <vector>
#include <limits>
#include <fstream>
#include <cinttypes>

class VSEchoDelayDetector
{
public:
	VSEchoDelayDetector();
	~VSEchoDelayDetector();

	bool Init(size_t delayBeforePlayPattern, size_t delayAfterPlayPAttern);
	void Release();

	bool Start();
	bool Stop();

	void AddPatternToRenderedAudio(void* data, size_t samples);
	void ProcessAudio(void* capturedData, const void* renderedData, size_t samples);

	bool IsPatternAdding();

	enum EGetDelayResult : uint16_t
	{
		GDR_OK,
		GDR_NO_ECHO,
		GDR_MULTIPLE_ECHO,
		GDR_NOT_INITED,
		GDR_NOT_STARTED,
		GDR_NOT_ENOUGH_DATA
	};
	EGetDelayResult GetDelay(int32_t& delay);

private:
	enum class EState
	{
		S_NOT_INITIALIZED,
		S_READY_TO_START,
		S_STARTED,
		S_DONE
	} State = EState::S_NOT_INITIALIZED;

	using IdxValVector = std::vector<std::pair<size_t, double>>;

	std::vector<std::vector<uint16_t>> GetFftFromSamples(const std::vector<int16_t>& samples);

	IdxValVector GetMatchFunction(
		const std::vector<std::vector<uint16_t>>& pattern,
		const std::vector<std::vector<uint16_t>>& audio);

	IdxValVector GetMatchFunctionVertical(
		size_t cutIdx,
		const std::vector<std::vector<uint16_t>>& pattern,
		const std::vector<std::vector<uint16_t>>& audio);

	IdxValVector ToIndexValuePairs(const std::vector<double>& function);

	double GetMinorOutlier(const IdxValVector& function);
	double GetMajorOutlier(const IdxValVector& function);

	IdxValVector GetLocalMinimums(const IdxValVector& function);
	IdxValVector GetSmoothed(const IdxValVector& function);
	IdxValVector GetMinorOutliers(const IdxValVector& function);
	IdxValVector GetMajorOutliers(const IdxValVector& function);
	size_t GetMinIdx(const IdxValVector& matchFunction);

	size_t DelayBeforePlayPattern = 0;
	size_t AnalyzedAudioLenMs = 0;

	size_t CurrAnalyzedAudioLenMs = 0;
	size_t AdeedPatternSamples = 0;

	std::vector<int16_t> Pattern;
	std::vector<int16_t> Captured;
	std::vector<int16_t> Rendered;

	std::vector<std::vector<uint16_t>> PatternFft;
	std::vector<std::vector<uint16_t>> CapturedFft;
	std::vector<std::vector<uint16_t>> RenderedFft;

	size_t FftOrder;
	size_t FftWindowLen;

	IdxValVector CapturedDiff;
	IdxValVector RenderedDiff;

	ptrdiff_t VerticalWindowSize;

	uint32_t DetectedEchoCount = false;
	int32_t EchoDelay = 0;
};
