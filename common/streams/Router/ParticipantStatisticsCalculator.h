#pragma once

#include "StatisticsInterface.h"
#include "VS_StreamsMonitor.h"
#include "../fwd.h"
#include "../streams_v2/Router/Monitor.h"

#include <memory>

namespace stream {

class ParticipantStatisticsCalculator
	: public stream::ParticipantStatisticsInterface
{
public:
	ParticipantStatisticsCalculator();
	~ParticipantStatisticsCalculator();

	void InitSndStatCalc(const unsigned char mtracks[]);
	void InitRcvStatCalc(const unsigned char mtracks[]);
	void FreeSndStatCalc(stream::ParticipantStatistics* partStat);
	void FreeRcvStatCalc(stream::ParticipantStatistics* partStat);

	void ProcessingSend(unsigned currTm, const stream::Buffer* sndBuffer);
	void ProcessingReceive(unsigned currTm);
	void SndWrite(const stream::FrameHeader& writeHead, unsigned currTm);
	void SndPutBuffer(const stream::FrameHeader& readHead, unsigned currTm, const stream::Buffer* sndBuffer);
	void RcvRead(const stream::FrameHeader& readHead, unsigned currTm);
	size_t FormSndStatistics(stream::StreamStatistics* s, size_t s_size, bool* _video = nullptr) override;
	size_t FormRcvStatistics(stream::StreamStatistics* s, size_t s_size, bool* _video = nullptr) override;
	size_t GetSndStatisticsSize() override;
	size_t GetRcvStatisticsSize() override;
	void FillMonitorStruct(VS_StreamsMonitor::SmReply::Participant& participant);
	void FillMonitorStruct(Monitor::StreamReply::Participant& participant);

private:
	struct SndStat;
	struct RcvStat;
	struct SndCalc;
	struct RcvCalc;

	stream::Track sndTracks[256];
	stream::Track rcvTracks[256];
	unsigned char sndTracksIndexes[256];
	unsigned char rcvTracksIndexes[256];
	unsigned nSndTracks, nRcvTracks;
	std::unique_ptr<SndStat> sndStat;
	std::unique_ptr<RcvStat> rcvStat;
	std::unique_ptr<SndCalc> sndCalc;
	std::unique_ptr<RcvCalc> rcvCalc;
};

bool CalculateParticipantBandwidth(ParticipantBandwidthInfo *bandwidthInfo, int32_t percentDetectBandwidth, bool forceDetectBandwidth);

}
