#include <algorithm>
#include <iterator>
#include <cstring>
#include <fstream>
#include <sstream>
#include <windows.h>
#include <timeapi.h>

#include "VSEchoDelayDetector.h"
#include "common_audio\signal_processing\include\real_fft.h"
#include "common_audio\signal_processing\include\signal_processing_library.h"
#include "pattern.h"
#include "../../std/VS_ProfileTools.h"


VSEchoDelayDetector::VSEchoDelayDetector()
{
	WebRtcSpl_Init();

	FftOrder = 8;
	FftWindowLen = (1 << FftOrder);
	VerticalWindowSize = (FftWindowLen / 2) / 8;
}

VSEchoDelayDetector::~VSEchoDelayDetector()
{
}

bool VSEchoDelayDetector::Init(size_t delayBeforePlayPattern, size_t delayAfterPlayPAttern)
{
	Release();

	DelayBeforePlayPattern = delayBeforePlayPattern;
	CurrAnalyzedAudioLenMs = 0;

	AdeedPatternSamples = 0;

	Pattern.reserve(pattern_pcm_len / sizeof(int16_t));

	std::copy((int16_t*)pattern_pcm, (int16_t*)(&pattern_pcm[pattern_pcm_len]), std::back_inserter(Pattern));

	AnalyzedAudioLenMs = delayBeforePlayPattern + Pattern.size() * 1000 / 16000 + delayAfterPlayPAttern;
	PatternFft = GetFftFromSamples(Pattern);

	State = EState::S_READY_TO_START;

	return State == EState::S_READY_TO_START;
}

void VSEchoDelayDetector::Release()
{
	if (State != EState::S_NOT_INITIALIZED)
	{
		DelayBeforePlayPattern = 0;
		AnalyzedAudioLenMs = 0;

		CurrAnalyzedAudioLenMs = 0;
		AdeedPatternSamples = 0;

		Pattern.clear();
		Captured.clear();
		Rendered.clear();

		PatternFft.clear();
		CapturedFft.clear();
		RenderedFft.clear();

		CapturedDiff.clear();
		RenderedDiff.clear();

		DetectedEchoCount = 0;
		EchoDelay = 0;

		State = EState::S_NOT_INITIALIZED;
	}
}

bool VSEchoDelayDetector::Start()
{
	if (State == EState::S_READY_TO_START)
	{
		State = EState::S_STARTED;

		return true;
	}
	else
	{
		return false;
	}
}

bool VSEchoDelayDetector::Stop()
{
	if (State == EState::S_DONE || State == EState::S_STARTED)
	{
		CurrAnalyzedAudioLenMs = 0;
		AdeedPatternSamples = 0;

		Captured.clear();
		Rendered.clear();

		CapturedFft.clear();
		RenderedFft.clear();

		State = EState::S_READY_TO_START;

		return true;
	}
	else
	{
		return false;
	}
}

void VSEchoDelayDetector::AddPatternToRenderedAudio(void* data, size_t samples)
{
	if (State == EState::S_STARTED && CurrAnalyzedAudioLenMs > DelayBeforePlayPattern && AdeedPatternSamples < Pattern.size())
	{
		size_t samplesToAdd = std::min(Pattern.size() - AdeedPatternSamples, samples);

		memcpy(data, &Pattern[AdeedPatternSamples], samplesToAdd * sizeof(int16_t));

		AdeedPatternSamples += samplesToAdd;
	}
}

void VSEchoDelayDetector::ProcessAudio(void* capturedData, const void* renderedData, size_t samples)
{
	if (State == EState::S_STARTED && CurrAnalyzedAudioLenMs < AnalyzedAudioLenMs)
	{
		Captured.insert(Captured.end(), (const int16_t*)capturedData, (const int16_t*)capturedData + samples);
		Rendered.insert(Rendered.end(), (const int16_t*)renderedData, (const int16_t*)renderedData + samples);

		memset(capturedData, 0, sizeof(int16_t) * samples);

		while (Captured.size() >= FftWindowLen)
		{
			std::vector<std::vector<uint16_t>> fft = GetFftFromSamples(Captured);

			Captured.erase(Captured.begin(), Captured.begin() + FftWindowLen * fft.size());

			std::move(fft.begin(), fft.end(), std::back_inserter(CapturedFft));
		}

		while (Rendered.size() >= FftWindowLen)
		{
			std::vector<std::vector<uint16_t>> fft = GetFftFromSamples(Rendered);

			Rendered.erase(Rendered.begin(), Rendered.begin() + FftWindowLen * fft.size());

			std::move(fft.begin(), fft.end(), std::back_inserter(RenderedFft));
		}

		CurrAnalyzedAudioLenMs += samples * 1000 / 16000;
	}
}

bool VSEchoDelayDetector::IsPatternAdding()
{
	return State == EState::S_STARTED && CurrAnalyzedAudioLenMs > DelayBeforePlayPattern && AdeedPatternSamples < Pattern.size();
}

VSEchoDelayDetector::EGetDelayResult VSEchoDelayDetector::GetDelay(int32_t& delay)
{
	if (State == EState::S_NOT_INITIALIZED)
		return GDR_NOT_INITED;

	if (State == EState::S_READY_TO_START)
		return GDR_NOT_STARTED;

	if (State == EState::S_DONE)
	{
		if (!DetectedEchoCount)
		{
			return GDR_NO_ECHO;
		}
		else if (DetectedEchoCount == 1)
		{
			delay = EchoDelay;

			return GDR_OK;
		}
		else
		{
			return GDR_MULTIPLE_ECHO;
		}
	}

	if (CurrAnalyzedAudioLenMs < AnalyzedAudioLenMs)
		return GDR_NOT_ENOUGH_DATA;

	CapturedDiff = GetMatchFunction(PatternFft, CapturedFft);
	RenderedDiff = GetMatchFunction(PatternFft, RenderedFft);

	size_t captMinIdx;
	size_t rendMinIdx;

	// rendered

	rendMinIdx = GetMinIdx(RenderedDiff);

	// captured

	//Log("rendered", RenderedDiff);
	//Log("captured", CapturedDiff);

	IdxValVector localMins = GetLocalMinimums(CapturedDiff);

	std::vector<std::pair<size_t, IdxValVector>> verticalCuts;

	for (size_t i = 0; i < localMins.size(); i++)
	{
		verticalCuts.emplace_back(
			localMins[i].first,
			GetMatchFunctionVertical(localMins[i].first, PatternFft, CapturedFft)
		);
	}

	DetectedEchoCount = 0;

	for (size_t i = 0; i < verticalCuts.size(); i++)
	{
		IdxValVector outliers = GetMajorOutliers(verticalCuts[i].second);
		bool founded = false;

		for (const std::pair<size_t, double>& o : outliers)
		{
			if (o.first > VerticalWindowSize - 2 &&
				o.first < VerticalWindowSize + 2)
			{
				DetectedEchoCount++;
				captMinIdx = verticalCuts[i].first;
				founded = true;
				break;
			}
		}

		if (founded)
		{
			//Log("vert_" + std::to_string(verticalCuts[i].first) + "_ok", verticalCuts[i].second);
		}
		else
		{
			//Log("vert_" + std::to_string(verticalCuts[i].first) + "_fail", verticalCuts[i].second);
		}
	}

	State = EState::S_DONE;

	if (!DetectedEchoCount)
	{
		return GDR_NO_ECHO;
	}
	else if (DetectedEchoCount == 1)
	{
		EchoDelay = (int32_t(captMinIdx) - int32_t(rendMinIdx)) * int32_t(FftWindowLen) * 1000 / 16000;
		delay = EchoDelay;

		return GDR_OK;
	}
	else
	{
		return GDR_MULTIPLE_ECHO;
	}
}

std::vector<std::vector<uint16_t>> VSEchoDelayDetector::GetFftFromSamples(const std::vector<int16_t>& samples)
{
	RealFFT* fft = WebRtcSpl_CreateRealFFT(FftOrder);

	std::vector<int16_t> outBuff(FftWindowLen + 2);

	std::vector<std::vector<uint16_t>> histogram;

	for (size_t offset = 0; offset + FftWindowLen <= samples.size(); offset += FftWindowLen)
	{
		WebRtcSpl_RealForwardFFT(fft, samples.data() + offset, outBuff.data());

		histogram.emplace_back(FftWindowLen / 2);

		for (size_t i = 0; i < histogram.back().size(); i++)
		{
			double re = outBuff[2 * (i + 1)];
			double im = outBuff[2 * (i + 1) + 1];
			uint16_t mag = sqrt(re * re + im * im);

			histogram.back()[i] = mag;
		}
	}

	WebRtcSpl_FreeRealFFT(fft);

	return histogram;
}

VSEchoDelayDetector::IdxValVector VSEchoDelayDetector::GetMatchFunction(
	const std::vector<std::vector<uint16_t>>& pattern,
	const std::vector<std::vector<uint16_t>>& audio)
{
	IdxValVector result;

	for (size_t offset = 0; offset < audio.size() - pattern.size(); offset++)
	{
		double res = 0.0;

		for (size_t i = 0; i < pattern.size(); i++)
		{
			for (size_t j = 0; j < pattern.front().size(); j++)
			{
				double p = pattern[i][j];
				double a = audio[i + offset][j];

				res += p * (p - a);
			}
		}

		result.push_back({ result.size(), res });
	}

	return result;
}

VSEchoDelayDetector::IdxValVector VSEchoDelayDetector::GetMatchFunctionVertical(
	size_t cutIdx,
	const std::vector<std::vector<uint16_t>>& pattern,
	const std::vector<std::vector<uint16_t>>& audio)
{
	IdxValVector result;

	for (ptrdiff_t offset = -VerticalWindowSize; offset < VerticalWindowSize; offset++)
	{
		double res = 0.0;

		for (size_t i = 0; i < pattern.size(); i++)
		{
			for (size_t j = 0; j < audio[cutIdx].size(); j++)
			{
				double a = audio[cutIdx + i][j];
				double p = 0.0;

				ptrdiff_t pIdx = ptrdiff_t(j) - offset;

				if (pIdx < 0)
					pIdx += pattern.front().size();

				if (pIdx >= pattern.front().size())
					pIdx -= pattern.front().size();

				if (0 <= pIdx && pIdx < pattern.front().size())
					p = pattern[i][pIdx];

				res += p * (p - a);
			}
		}

		result.push_back({ offset + VerticalWindowSize, res });
	}

	return result;
}

VSEchoDelayDetector::IdxValVector VSEchoDelayDetector::ToIndexValuePairs(const std::vector<double>& function)
{
	IdxValVector result;

	for (double f : function)
		result.push_back({ result.size(), f });

	return result;
}

double VSEchoDelayDetector::GetMinorOutlier(const IdxValVector& function)
{
	IdxValVector q(function);

	std::sort(q.begin(), q.end(),
		[](std::pair<size_t, double> a, std::pair<size_t, double> b) { return a.second < b.second; });

	double Q1, Q3;

	size_t offs = q.size() / 4;

	Q1 = q[offs].second;
	Q3 = q[q.size() - offs].second;

	double diff = Q3 - Q1;

	return Q1 - diff * 1.5;
}

double VSEchoDelayDetector::GetMajorOutlier(const IdxValVector& function)
{
	IdxValVector q(function);

	std::sort(q.begin(), q.end(),
		[](std::pair<size_t, double> a, std::pair<size_t, double> b) { return a.second < b.second; });

	double Q1, Q3;

	size_t offs = q.size() / 4;

	Q1 = q[offs].second;
	Q3 = q[q.size() - offs].second;

	double diff = Q3 - Q1;

	return Q1 - diff * 3;
}

VSEchoDelayDetector::IdxValVector VSEchoDelayDetector::GetLocalMinimums(const IdxValVector& function)
{
	IdxValVector smoothed = GetSmoothed(function);
	IdxValVector localMins(1, smoothed.front());

	for (size_t i = 1; i < function.size() - 1; i++)
	{
		if (smoothed[i].second - smoothed[i - 1].second < 0.0 &&
			smoothed[i + 1].second - smoothed[i].second > 0.0)
		{
			localMins.push_back( smoothed[i] );
		}
	}

	localMins.push_back( smoothed.back() );

	return localMins;
}

VSEchoDelayDetector::IdxValVector VSEchoDelayDetector::GetSmoothed(const IdxValVector& function)
{
	IdxValVector smoothed(function);

	for (size_t i = 1; i < function.size() - 1; i++)
		smoothed[i].second = (function[i - 1].second + function[i].second + function[i + 1].second) / 3.0;

	return smoothed;
}

VSEchoDelayDetector::IdxValVector VSEchoDelayDetector::GetMinorOutliers(const IdxValVector& function)
{
	IdxValVector q(function);

	std::sort(q.begin(), q.end(),
		[](std::pair<size_t, double> a, std::pair<size_t, double> b) { return a.second < b.second; });

	double Q1, Q3, Qmin;

	size_t offs = q.size() / 4;

	Q1 = q[offs].second;
	Q3 = q[q.size() - offs].second;

	double diff = Q3 - Q1;

	Qmin = Q1 - diff * 1.5;

	IdxValVector result;

	for (size_t i = 0; q[i].second < Qmin; i++)
	{
		result.push_back(q[i]);
	}

	return result;
}

VSEchoDelayDetector::IdxValVector VSEchoDelayDetector::GetMajorOutliers(const IdxValVector& function)
{
	IdxValVector q(function);

	std::sort(q.begin(), q.end(),
		[](std::pair<size_t, double> a, std::pair<size_t, double> b) { return a.second < b.second; });

	double Q1, Q3, Qmaj;

	size_t offs = q.size() / 4;

	Q1 = q[offs].second;
	Q3 = q[q.size() - offs].second;

	double diff = Q3 - Q1;

	Qmaj = Q1 - diff * 3;

	IdxValVector result;

	for (size_t i = 0; q[i].second < Qmaj; i++)
	{
		result.push_back(q[i]);
	}

	return result;
}

size_t VSEchoDelayDetector::GetMinIdx(const IdxValVector& matchFunction)
{
	IdxValVector tmp(matchFunction);

	IdxValVector::iterator min = std::min_element(tmp.begin(), tmp.end(),
		[](std::pair<size_t, double> a, std::pair<size_t, double> b) { return a.second < b.second; });

	return (*min).first;
}
