#include "ParticipantStatisticsCalculator.h"
#include "Buffer.h"
#include "Types.h"
#include "../Protocol.h"
#include "../Statistics.h"
#include "../VS_StreamsDefinitions.h"
#include "std/cpplib/numerical.h"
#include "std/debuglog/VS_Debug.h"

#include "std-generic/compat/memory.h"
#include <cinttypes>
#include <cmath>
#include <cstdint>
#include <cstdlib>

#define DEBUG_CURRENT_MODULE VS_DM_STREAMS

namespace stream {

#pragma pack(push, 1)
struct SndStatTrack
{
	uint64_t writeBytes, writeFrames;
	uint32_t nWrite, nFramesBufferSum;
};

struct RcvStatTrack
{
	uint64_t readBytes, readFrames;
	uint32_t nRead;
};

struct SndStatDirection
{
	uint32_t tickTm, nWrite, allFramesBufferSum;
	uint64_t allWriteBytes, allWriteFrames;
	SndStatTrack track[1];
};

struct RcvStatDirection
{
	uint32_t tickTm, nRead;
	uint64_t allReadBytes, allReadFrames;
	RcvStatTrack track[1];
};

struct ParticipantStatisticsCalculator::SndStat
{
	explicit SndStat(const unsigned ntracks) :
		size_direction(sizeof(SndStatDirection) + ( sizeof(SndStatTrack) * ntracks )),
		end(0), head(0), all(0)
	{
		const size_t size = size_direction * VS_SR_STAT_N_TICKS;
		const auto mem = malloc(size);
		if (!mem)
		{
			*direction = nullptr;
			return;
		}
		memset(mem, 0, size);
		for (unsigned i = 0; i < VS_SR_STAT_N_TICKS; ++i)
			direction[i] = reinterpret_cast<SndStatDirection*>(static_cast<char*>(mem) + size_direction * i);
	}
	SndStat(const SndStat&) = delete;
	SndStat& operator=(const SndStat&) = delete;
	~SndStat()
	{
		if (*direction)
			free(*direction);
	}
	const size_t size_direction;
	SndStatDirection   *direction[VS_SR_STAT_N_TICKS];
	unsigned   end, head, all;
};

struct ParticipantStatisticsCalculator::RcvStat
{
	explicit RcvStat(const unsigned ntracks) :
		size_direction(sizeof(RcvStatDirection) + ( sizeof(RcvStatTrack) * ntracks )),
		end(0), head(0), all(0)
	{
		const size_t size = size_direction * VS_SR_STAT_N_TICKS;
		const auto mem = malloc(size);
		if (!mem)
		{
			*direction = nullptr;
			return;
		}
		memset(mem, 0, size);
		for (unsigned i = 0; i < VS_SR_STAT_N_TICKS; ++i)
			direction[i] = reinterpret_cast<RcvStatDirection*>(static_cast<char*>(mem) + size_direction * i);
	}
	RcvStat(const RcvStat&) = delete;
	RcvStat& operator=(const RcvStat&) = delete;
	~RcvStat()
	{
		if (*direction)
			free(*direction);
	}
	const size_t size_direction;
	RcvStatDirection   *direction[VS_SR_STAT_N_TICKS];
	unsigned   end, head, all;
};

struct SndCalcTrack
{
	uint32_t writeBytesBand, writeFramesBand, averageWriteFramesSize;
	uint16_t nFramesBufferAve;
};

struct RcvCalcTrack
{
	uint32_t readBytesBand, readFramesBand, averageReadFramesSize;
};

struct SndCalcDirection
{
	uint32_t tickTm, endTickTm, headTickTm;
	uint32_t allWriteBytesBand, allWriteFramesBand, maxAllWriteBytesBand, maxAllWriteFramesBand;
	uint16_t allFramesBufferAve;
	SndCalcTrack track[1];
};

struct RcvCalcDirection
{
	uint32_t tickTm, endTickTm, headTickTm, diffTickTm;
	uint32_t allReadBytesBand, allReadFramesBand, maxAllReadBytesBand, maxAllReadFramesBand;
	RcvCalcTrack track[1];
};

struct ParticipantStatisticsCalculator::SndCalc
{
	explicit SndCalc(const unsigned ntracks) :
		size_direction(sizeof(SndCalcDirection) + ( sizeof(SndCalcTrack) * ntracks ))
	{
		direction = static_cast<SndCalcDirection*>(malloc(size_direction));
		if (!direction)
			return;
		memset(direction, 0, size_direction);
	}
	SndCalc(const SndCalc&) = delete;
	SndCalc& operator=(const SndCalc&) = delete;
	~SndCalc()
	{
		if (direction)
			free(direction);
	}
	const size_t size_direction;
	SndCalcDirection   *direction;
};

struct ParticipantStatisticsCalculator::RcvCalc
{
	explicit RcvCalc(const unsigned ntracks) :
		size_direction(sizeof(RcvCalcDirection) + ( sizeof(RcvCalcTrack) * ntracks ))
	{
		direction = static_cast<RcvCalcDirection*>(malloc(size_direction));
		if (!direction)
			return;
		memset(direction, 0, size_direction);
	}
	RcvCalc(const RcvCalc&) = delete;
	RcvCalc& operator=(const RcvCalc&) = delete;
	~RcvCalc()
	{
		if (direction)
			free(direction);
	}
	const size_t size_direction;
	RcvCalcDirection   *direction;
};
#pragma pack(pop)

ParticipantStatisticsCalculator::ParticipantStatisticsCalculator()
	: nSndTracks(0)
	, nRcvTracks(0)
{
	memset(sndTracks, 0, sizeof(sndTracks));
	memset(rcvTracks, 0, sizeof(rcvTracks));
	memset(sndTracksIndexes, 0, sizeof(sndTracksIndexes));
	memset(rcvTracksIndexes, 0, sizeof(rcvTracksIndexes));
}

ParticipantStatisticsCalculator::~ParticipantStatisticsCalculator() = default;

static inline unsigned GetTracks(stream::Track tracks[], unsigned char tracksIndexes[], const unsigned char mtracks[])
{
	memset(tracks, 0, 256);
	memset(tracksIndexes, 0, 256);
	unsigned   ret = 0;
	for (unsigned i = 0; i < 32; ++i)
		if (mtracks[i])
		{
			const unsigned   k = i << 3;
			for (unsigned j = i ? 0 : 1; j < 8; ++j)
				if (mtracks[i] & (1 << j))
				{
					const auto track = static_cast<stream::Track>(k + j);
					tracks[++ret] = track;
					tracksIndexes[id(track)] = ret;
				}
		}
	return ret;
}

void ParticipantStatisticsCalculator::InitSndStatCalc(const unsigned char mtracks[])
{
	nSndTracks = GetTracks(sndTracks, sndTracksIndexes, mtracks);
	sndStat = vs::make_unique<SndStat>(nSndTracks);
	sndCalc = vs::make_unique<SndCalc>(nSndTracks);
	if (!sndStat || !*sndStat->direction || !sndCalc || !sndCalc->direction)
		FreeSndStatCalc(nullptr);
}

void ParticipantStatisticsCalculator::InitRcvStatCalc(const unsigned char mtracks[])
{
	nRcvTracks = GetTracks(rcvTracks, rcvTracksIndexes, mtracks);
	rcvStat = vs::make_unique<RcvStat>(nRcvTracks);
	rcvCalc = vs::make_unique<RcvCalc>(nRcvTracks);
	if (!rcvStat || !*rcvStat->direction || !rcvCalc || !rcvCalc->direction)
		FreeRcvStatCalc(nullptr);
}

void ParticipantStatisticsCalculator::FreeSndStatCalc(stream::ParticipantStatistics* partStat)
{
	if (sndStat && partStat)
	{
		SndStatDirection& endSndStatDirection = *sndStat->direction[sndStat->end];
		partStat->allReceiverBytes += endSndStatDirection.allWriteBytes;
		partStat->allReceiverFrames += endSndStatDirection.allWriteFrames;
	}
	sndStat = nullptr;
	if (sndCalc && partStat)
	{
		SndCalcDirection& sndCalcDirection = *sndCalc->direction;
		if (partStat->receiverBytesBand < sndCalcDirection.maxAllWriteBytesBand)
		{
			partStat->receiverBytesBand += sndCalcDirection.maxAllWriteBytesBand;
			partStat->receiverBytesBand /= 2;
		}
		if (partStat->receiverFramesBand < sndCalcDirection.maxAllWriteFramesBand)
		{
			partStat->receiverFramesBand += sndCalcDirection.maxAllWriteFramesBand;
			partStat->receiverFramesBand /= 2;
		}
	}
	sndCalc = nullptr;
}

void ParticipantStatisticsCalculator::FreeRcvStatCalc(stream::ParticipantStatistics* partStat)
{
	if (rcvStat && partStat)
	{
		RcvStatDirection& endRcvStatDirection = *rcvStat->direction[rcvStat->end];
		partStat->allSenderBytes += endRcvStatDirection.allReadBytes;
		partStat->allSenderFrames += endRcvStatDirection.allReadFrames;
	}
	rcvStat = nullptr;
	if (rcvCalc)
	{
		RcvCalcDirection& rcvCalcDirection = *rcvCalc->direction;
		if (partStat->senderBytesBand < rcvCalcDirection.maxAllReadBytesBand)
		{
			partStat->senderBytesBand += rcvCalcDirection.maxAllReadBytesBand;
			partStat->senderBytesBand /= 2;
		}
		if (partStat->senderFramesBand < rcvCalcDirection.allReadFramesBand)
		{
			partStat->senderFramesBand += rcvCalcDirection.allReadFramesBand;
			partStat->senderFramesBand /= 2;
		}
	}
	rcvCalc = nullptr;
}

void ParticipantStatisticsCalculator::ProcessingSend(unsigned currTm, const stream::Buffer* sndBuffer)
{
	if (!sndStat || !sndCalc)	return;
	SndStat   &stat = *sndStat;		SndCalc   &calc = *sndCalc;
	unsigned   all = stat.all;
	all = (all >= VS_SR_STAT_N_TICKS) ? VS_SR_STAT_N_TICKS : all + 1;
	stat.all = all;
	unsigned   ntracks = nSndTracks, end = stat.end, head = stat.head;
	SndStatDirection   &headSndStatDirection = *stat.direction[head];
	headSndStatDirection.tickTm = currTm;
	if (sndBuffer->Ready())
	{
		SndStatDirection   &endSndStatDirection = *stat.direction[end];
		SndCalcDirection   &sndCalcDirection = *calc.direction;
		sndCalcDirection.tickTm = currTm;
		sndCalcDirection.endTickTm = endSndStatDirection.tickTm;
		sndCalcDirection.headTickTm = headSndStatDirection.tickTm;
		const unsigned diffTickTm = sndCalcDirection.headTickTm - sndCalcDirection.endTickTm,
			  addDiffTickTm = diffTickTm / 2;
		if (!diffTickTm)
			sndCalcDirection.allWriteBytesBand = sndCalcDirection.allWriteFramesBand = 0;
		else
		{
			sndCalcDirection.allWriteBytesBand = (((unsigned)( headSndStatDirection.allWriteBytes - endSndStatDirection.allWriteBytes ) * 1000 ) + addDiffTickTm) / diffTickTm;
			if (sndCalcDirection.maxAllWriteBytesBand < sndCalcDirection.allWriteBytesBand)
			{
				sndCalcDirection.maxAllWriteBytesBand += sndCalcDirection.allWriteBytesBand;
				sndCalcDirection.maxAllWriteBytesBand /= 2;
			}
			sndCalcDirection.allWriteFramesBand = (((unsigned)( headSndStatDirection.allWriteFrames - endSndStatDirection.allWriteFrames ) * 1000 ) + addDiffTickTm) / diffTickTm;
			if (sndCalcDirection.maxAllWriteFramesBand < sndCalcDirection.allWriteFramesBand)
			{
				sndCalcDirection.maxAllWriteFramesBand += sndCalcDirection.allWriteFramesBand;
				sndCalcDirection.maxAllWriteFramesBand /= 2;
			}
		}
		for (unsigned t = 0; t <= ntracks; ++t)
		{
			const stream::Track track = sndTracks[t];
			SndStatTrack   &endSndStatTrack = endSndStatDirection.track[t],
			&headSndStatTrack = headSndStatDirection.track[t];
			SndCalcTrack   &sndCalcTrack = sndCalcDirection.track[t];
			unsigned diffWriteBytes = (headSndStatTrack.writeBytes - endSndStatTrack.writeBytes) * 1000;
			unsigned diffWriteFrames = (headSndStatTrack.writeFrames - endSndStatTrack.writeFrames) * 1000;
			if (!diffTickTm)
				sndCalcTrack.writeBytesBand = sndCalcTrack.writeFramesBand = 0;
			else
			{
				sndCalcTrack.writeBytesBand = ( diffWriteBytes + addDiffTickTm ) / diffTickTm;
				sndCalcTrack.writeFramesBand = ( diffWriteFrames + addDiffTickTm ) / diffTickTm;
			}
			sndCalcTrack.averageWriteFramesSize = !diffWriteFrames ? 0 : ( diffWriteBytes + ( diffWriteFrames / 2 )) / diffWriteFrames;
			unsigned nFramesBuffer = 0, nWrite = 0;
			for (unsigned p = end; p != head; ++p, p &= VS_SR_STAT_N_MASK)
			{
				SndStatDirection   &sndStatDirection = *stat.direction[p];
				SndStatTrack   &sndStatTrack = sndStatDirection.track[t];
				nWrite += sndStatTrack.nWrite;
				nFramesBuffer += sndStatTrack.nFramesBufferSum;
			}
			nFramesBuffer += sndBuffer->GetFrameCount(track);
			++nWrite;
			sndCalcTrack.nFramesBufferAve = (unsigned short)((unsigned)(( nFramesBuffer * 100 ) + ( nWrite / 2 )) / nWrite );
		}
		{
			unsigned allFramesBuffer = 0, nWrite = 0;
			for (unsigned p = end; p != head; ++p, p &= VS_SR_STAT_N_MASK)
			{
				SndStatDirection   &sndStatDirection = *stat.direction[p];
				nWrite += sndStatDirection.nWrite;
				allFramesBuffer += sndStatDirection.allFramesBufferSum;
			}
			allFramesBuffer += sndBuffer->GetFrameCount();
			++nWrite;
			sndCalcDirection.allFramesBufferAve = (unsigned short)((unsigned)(( allFramesBuffer * 100 ) + ( nWrite / 2 )) / nWrite );
		}
	}
	++head;		head &= VS_SR_STAT_N_MASK;		stat.head = head;
	if (all >= VS_SR_STAT_N_TICKS) {	++end;	end &= VS_SR_STAT_N_MASK;	stat.end = end;		}
	SndStatDirection   &newHeadSndStatDirection = *stat.direction[head];
	memcpy(&newHeadSndStatDirection, &headSndStatDirection, stat.size_direction);
	newHeadSndStatDirection.nWrite = newHeadSndStatDirection.allFramesBufferSum = 0;
	for (unsigned t = 0; t <= ntracks; ++t)
	{
		SndStatTrack   &sndStatTrack = newHeadSndStatDirection.track[t];
		sndStatTrack.nWrite = sndStatTrack.nFramesBufferSum = 0;
	}
}

void ParticipantStatisticsCalculator::ProcessingReceive(unsigned currTm)
{
	if (!rcvStat || !rcvCalc)	return;
	RcvStat   &stat = *rcvStat;		RcvCalc   &calc = *rcvCalc;
	unsigned   all = stat.all;
	all = (all >= VS_SR_STAT_N_TICKS) ? VS_SR_STAT_N_TICKS : all + 1;
	stat.all = all;
	unsigned   ntracks = nRcvTracks, end = stat.end, head = stat.head;
	RcvStatDirection   &headRcvStatDirection = *stat.direction[head];
	headRcvStatDirection.tickTm = currTm;
	if (all >= VS_SR_STAT_N_TICKS)
	{
		RcvStatDirection   &endRcvStatDirection = *stat.direction[end];
		RcvCalcDirection   &rcvCalcDirection = *calc.direction;
		rcvCalcDirection.tickTm = currTm;
		rcvCalcDirection.endTickTm = endRcvStatDirection.tickTm;
		rcvCalcDirection.headTickTm = headRcvStatDirection.tickTm;
		const unsigned diffTickTm = rcvCalcDirection.headTickTm - rcvCalcDirection.endTickTm,
			  addDiffTickTm = diffTickTm / 2;
		if (!diffTickTm)
			rcvCalcDirection.allReadBytesBand = rcvCalcDirection.allReadFramesBand = 0;
		else
		{
			rcvCalcDirection.allReadBytesBand = (((unsigned)( headRcvStatDirection.allReadBytes - endRcvStatDirection.allReadBytes ) * 1000 ) + addDiffTickTm ) / diffTickTm;
			if (rcvCalcDirection.maxAllReadBytesBand < rcvCalcDirection.allReadBytesBand)
			{
				rcvCalcDirection.maxAllReadBytesBand += rcvCalcDirection.allReadBytesBand;
				rcvCalcDirection.maxAllReadBytesBand /= 2;
			}
			rcvCalcDirection.allReadFramesBand = (((unsigned)( headRcvStatDirection.allReadFrames - endRcvStatDirection.allReadFrames ) * 1000 ) + addDiffTickTm ) / diffTickTm;
			if (rcvCalcDirection.maxAllReadFramesBand < rcvCalcDirection.allReadFramesBand)
			{
				rcvCalcDirection.maxAllReadFramesBand += rcvCalcDirection.allReadFramesBand;
				rcvCalcDirection.maxAllReadFramesBand /= 2;
			}
		}
		for (unsigned i = 0; i <= ntracks; ++i)
		{
			RcvStatTrack   &endRcvStatTrack = endRcvStatDirection.track[i],
			&headRcvStatTrack = headRcvStatDirection.track[i];
			RcvCalcTrack   &rcvCalcTrack = rcvCalcDirection.track[i];
			unsigned diffReadBytes = (headRcvStatTrack.readBytes - endRcvStatTrack.readBytes) * 1000;
			unsigned diffReadFrames = (headRcvStatTrack.readFrames - endRcvStatTrack.readFrames) * 1000;
			if (!diffTickTm)
				rcvCalcTrack.readBytesBand = rcvCalcTrack.readFramesBand = 0;
			else
			{
				rcvCalcTrack.readBytesBand = ( diffReadBytes + addDiffTickTm ) / diffTickTm;
				rcvCalcTrack.readFramesBand = ( diffReadFrames  + addDiffTickTm ) / diffTickTm;
			}
			rcvCalcTrack.averageReadFramesSize = !diffReadFrames ? 0 : ( diffReadBytes + ( diffReadFrames / 2 )) / diffReadFrames;
		}
	}
	++head;		head &= VS_SR_STAT_N_MASK;		stat.head = head;
	if (all >= VS_SR_STAT_N_TICKS) {	++end;	end &= VS_SR_STAT_N_MASK;	stat.end = end;		}
	memcpy(stat.direction[head], &headRcvStatDirection, stat.size_direction);
}

void ParticipantStatisticsCalculator::SndWrite(const stream::FrameHeader& writeHead, unsigned currTm)
{
	if (!sndStat)	return;
	SndStatDirection   &sndStatDirection = *sndStat->direction[sndStat->head];
	sndStatDirection.tickTm = currTm;
	++sndStatDirection.allWriteFrames;
	sndStatDirection.allWriteBytes += writeHead.length;
	const auto index = sndTracksIndexes[id(writeHead.track)];
	SndStatTrack   &sndStatTrack = sndStatDirection.track[index];
	sndStatTrack.writeBytes += writeHead.length;
}

void ParticipantStatisticsCalculator::SndPutBuffer(const stream::FrameHeader& readHead, unsigned currTm, const stream::Buffer* sndBuffer)
{
	if (!sndStat)	return;
	SndStatDirection   &sndStatDirection = *sndStat->direction[sndStat->head];
	sndStatDirection.tickTm = currTm;
	++sndStatDirection.nWrite;
	sndStatDirection.allFramesBufferSum += sndBuffer->GetFrameCount();
	const auto index = sndTracksIndexes[id(readHead.track)];
	SndStatTrack   &sndStatTrack = sndStatDirection.track[index];
	++sndStatTrack.nWrite;
	sndStatTrack.nFramesBufferSum += sndBuffer->GetFrameCount(readHead.track);
}

void ParticipantStatisticsCalculator::RcvRead(const stream::FrameHeader& readHead, unsigned currTm)
{
	if (!rcvStat)	return;
	RcvStatDirection   &rcvStatDirection = *rcvStat->direction[rcvStat->head];
	++rcvStatDirection.nRead;	++rcvStatDirection.allReadFrames;
	rcvStatDirection.allReadBytes += readHead.length;
	const auto index = rcvTracksIndexes[id(readHead.track)];
	rcvStatDirection.tickTm = currTm;
	RcvStatTrack   &rcvStatTrack = rcvStatDirection.track[index];
	rcvStatTrack.readBytes += readHead.length;
	++rcvStatTrack.readFrames;		++rcvStatTrack.nRead;
}

size_t ParticipantStatisticsCalculator::FormSndStatistics(stream::StreamStatistics* s, size_t s_size, bool* _video)
{
	const auto size = GetSndStatisticsSize();
	if (size == 0 || size > s_size)
		return 0;
	const unsigned   nTracks = nSndTracks - 1;
	SndCalcDirection   &sndCalcDirection = *sndCalc->direction;
	s->ntracks = (unsigned char)nTracks;
	s->allFramesBuffer = sndCalcDirection.allFramesBufferAve;
	s->allWriteBytesBand = sndCalcDirection.allWriteBytesBand;
	s->allWriteFramesBand = sndCalcDirection.allWriteFramesBand;
	for (unsigned i = 1; i <= nTracks; ++i)
	{
		auto& trackStatistics = s->tracks[i - 1];
		SndCalcTrack   &sndCalcTrack = sndCalcDirection.track[i];
		trackStatistics.track = sndTracks[i];
		trackStatistics.nFramesBuffer = sndCalcTrack.nFramesBufferAve;
		trackStatistics.writeBytesBand = sndCalcTrack.writeBytesBand;
		trackStatistics.writeFramesBand = sndCalcTrack.writeFramesBand;
		if ((trackStatistics.track == stream::Track::video) && (_video)) {
			*_video = (trackStatistics.writeBytesBand > 0);
		}
	}
	return size;
}

size_t ParticipantStatisticsCalculator::FormRcvStatistics(stream::StreamStatistics* s, size_t s_size, bool* _video)
{
	const auto size = GetRcvStatisticsSize();
	if (size == 0 || size > s_size)
		return 0;
	const unsigned   nTracks = nRcvTracks - 1;
	RcvCalcDirection   &rcvCalcDirection = *rcvCalc->direction;
	s->ntracks = (unsigned char)nTracks;
	s->allFramesBuffer = (unsigned short)rcvCalcDirection.allReadFramesBand;
	s->allWriteBytesBand = rcvCalcDirection.allReadBytesBand;
	s->allWriteFramesBand = rcvCalcDirection.allReadFramesBand;
	for (unsigned i = 1; i <= nTracks; ++i)
	{
		auto& trackStatistics = s->tracks[i - 1];
		RcvCalcTrack   &rcvCalcTrack = rcvCalcDirection.track[i];
		trackStatistics.track = rcvTracks[i];
		trackStatistics.nFramesBuffer = (unsigned short)rcvCalcTrack.readFramesBand;
		trackStatistics.writeBytesBand = rcvCalcTrack.readBytesBand;
		trackStatistics.writeFramesBand = rcvCalcTrack.readFramesBand;
		if ((trackStatistics.track == stream::Track::video) && (_video)) {
			*_video = (trackStatistics.writeBytesBand > 0);
		}
	}
	return size;
}

size_t ParticipantStatisticsCalculator::GetSndStatisticsSize()
{
	if (!sndCalc)
		return 0;
	return sizeof(stream::StreamStatistics) + sizeof(stream::TrackStatistics) * (nSndTracks - 2);
}

size_t ParticipantStatisticsCalculator::GetRcvStatisticsSize()
{
	if (!rcvCalc)
		return 0;
	return sizeof(stream::StreamStatistics) + sizeof(stream::TrackStatistics) * (nRcvTracks - 2);
}

void ParticipantStatisticsCalculator::FillMonitorStruct(VS_StreamsMonitor::SmReply::Participant& participant)
{
	if (rcvStat)
	{
		RcvStatDirection& rcvStatDirection = *rcvStat->direction[rcvStat->head];
		participant.rcvBytes = rcvStatDirection.allReadBytes;
		participant.rcvFrames = rcvStatDirection.allReadFrames;
	}
	if (rcvCalc)
	{
		RcvCalcDirection& rcvCalcDirection = *rcvCalc->direction;
		participant.rcvBytesBandwidth = rcvCalcDirection.allReadBytesBand;
		participant.rcvFramesBandwidth = rcvCalcDirection.allReadFramesBand;
	}
	if (sndStat)
	{
		SndStatDirection& sndStatDirection = *sndStat->direction[sndStat->head];
		participant.sndBytes = sndStatDirection.allWriteBytes;
		participant.sndFrames = sndStatDirection.allWriteFrames;
	}
	if (sndCalc)
	{
		SndCalcDirection& sndCalcDirection = *sndCalc->direction;
		participant.sndBytesBandwidth = sndCalcDirection.allWriteBytesBand;
		participant.sndFramesBandwidth = sndCalcDirection.allWriteFramesBand;
	}
}

void ParticipantStatisticsCalculator::FillMonitorStruct(Monitor::StreamReply::Participant& participant)
{
	if (rcvStat)
	{
		RcvStatDirection& rcvStatDirection = *rcvStat->direction[rcvStat->head];
		participant.r_bytes = rcvStatDirection.allReadBytes;
		participant.r_frames = rcvStatDirection.allReadFrames;
	}
	if (rcvCalc)
	{
		RcvCalcDirection& rcvCalcDirection = *rcvCalc->direction;
		participant.r_bytes_bandwith = rcvCalcDirection.allReadBytesBand;
		participant.r_frames_bandwith = rcvCalcDirection.allReadFramesBand;
	}
	if (sndStat)
	{
		SndStatDirection& sndStatDirection = *sndStat->direction[sndStat->head];
		participant.s_bytes = sndStatDirection.allWriteBytes;
		participant.s_frames = sndStatDirection.allWriteFrames;
	}
	if (sndCalc)
	{
		SndCalcDirection& sndCalcDirection = *sndCalc->direction;
		participant.s_bytes_bandwith = sndCalcDirection.allWriteBytesBand;
		participant.s_frames_bandwith = sndCalcDirection.allWriteFramesBand;
	}
}

bool CalculateParticipantBandwidth(ParticipantBandwidthInfo *bandwidthInfo, int32_t percentDetectBandwidth, bool forceDetectBandwidth)
{
	static const uint32_t _undefBandwidth = (uint32_t) (-1);
	static const uint32_t _minBandwidth = 10;
	static const uint32_t _restrictBandwidthMax = 65535;
	static const uint32_t _minQueueBytesLoadDetect = 2048;
	static const uint32_t _minQueueBytesBandwidthDetect = 4096;
	static const int32_t _percentFreezeBandwidth = 10;
	static const int32_t _percentVarianceBandwidth = 70;
	static const uint64_t _bitrateDetectTm = 2000;
	static const uint64_t _bitrateFreezeTm = 6000;
	static const uint32_t _detectCoefLimit = 5;

	auto calculateReduceBandwidth = [bandwidthInfo] (int32_t loadBandwidth, double k) -> void
	{
		double bandwidthReduce = std::max(1.0 / (1.0 + loadBandwidth / k), 0.5);
		uint32_t lowerCalculateBandwidth = static_cast<uint32_t>(bandwidthInfo->physicalBandwidth * bandwidthReduce + 0.5);
		if (bandwidthInfo->calculateBandwidth > lowerCalculateBandwidth) {
			bandwidthInfo->calculateBandwidth = lowerCalculateBandwidth;
			bandwidthInfo->lowerPhysicalBandwidth = lowerCalculateBandwidth;
			bandwidthInfo->upperPhysicalBandwidth = bandwidthInfo->physicalBandwidth;
		}
		return;
	};
	auto calculateUpperStep = [] (uint64_t tm, uint64_t tick, double k) -> double
	{
		/// increase bitrate : in "k" times for "tm" msec with delta "tick" msec
		return std::pow(k, (double) tick / (double) tm);
	};

	uint64_t ct = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
	uint32_t oldBitrate(bandwidthInfo->restrictBitrate);
	uint32_t instantBand(bandwidthInfo->physicalBandwidth);
	uint64_t band(bandwidthInfo->physicalBandwidth);
	int32_t loadBandwidth = 0;
	uint64_t bandLimitBytes = (bandwidthInfo->physicalBandwidth == _undefBandwidth) ? bandwidthInfo->receivedBytes : bandwidthInfo->physicalBandwidth * 128;

	bandwidthInfo->numPeriodicTicks++;

	/// detect physical bandwidth
	if ((bandwidthInfo->queueBytes >= _minQueueBytesLoadDetect && bandwidthInfo->receivedBytes != 0) ||
		bandwidthInfo->receivedBytes >= _minQueueBytesLoadDetect || /// 16 kbps - limit
		forceDetectBandwidth)
	{
		loadBandwidth = static_cast<int>(bandwidthInfo->queueBytes * 100 / (bandLimitBytes + 1));
		if (loadBandwidth >= percentDetectBandwidth) {
			if (bandwidthInfo->queueBytes >= _minQueueBytesBandwidthDetect ||
				bandwidthInfo->receivedBytes >= _minQueueBytesBandwidthDetect) /// 32 kbps - limit
			{
				/// detect new physical bandwidth
				band = bandwidthInfo->receivedBytes / 128;
				instantBand = static_cast<uint32_t>(band);
			}
			bandwidthInfo->overflowTime = ct;
		}
		else if (loadBandwidth >= _percentFreezeBandwidth) {
			bandwidthInfo->overflowTime = ct;
		}
	}
	auto overflowTm = ct - bandwidthInfo->overflowTime;
	/// update physical bandwidth
	if (bandwidthInfo->physicalBandwidth != _undefBandwidth) {
		if ((overflowTm >= _bitrateDetectTm) && (bandwidthInfo->receivedBytes > bandLimitBytes)) {
			int ko = 3, kn = 1;
			if (overflowTm >= 2 * _bitrateFreezeTm) {
				ko = 9;
				kn = 1;
			}
			band = (kn * bandwidthInfo->receivedBytes + ko * bandLimitBytes) / (ko + kn) / 128 + 1;
		}
		else if (band != bandwidthInfo->physicalBandwidth) {
			band = (band + bandwidthInfo->physicalBandwidth) / 2 + 1;
			if (band < bandwidthInfo->physicalBandwidth) {
				uint64_t min_band = (bandwidthInfo->physicalBandwidth * _percentVarianceBandwidth) / 100;
				band = std::max(min_band, band);
			}
		}
	}
	bandwidthInfo->physicalBandwidth = std::max<uint32_t>(_minBandwidth, band);
	/// recalc bitrates
	uint64_t freezeBandwidthTm = ct - bandwidthInfo->changeBandwidthTime;
	if (loadBandwidth >= percentDetectBandwidth) {
		if (bandwidthInfo->state & (stream::eBandwidthState::undef | stream::eBandwidthState::idle | stream::eBandwidthState::freeze)) {
			bandwidthInfo->upperPhysicalBandwidth = bandwidthInfo->physicalBandwidth;
		}
		bandwidthInfo->state = stream::eBandwidthState::limit;
		calculateReduceBandwidth(loadBandwidth, 200.0);
		if (bandwidthInfo->numPeriodicTicks < 20) {
			bandwidthInfo->upperPhysicalBandwidth = _restrictBandwidthMax;
		}
		bandwidthInfo->changeBandwidthTime = ct;
		bandwidthInfo->numDetectTicks = 0;
	}
	else if (loadBandwidth > 0) {
		if (bandwidthInfo->state & (stream::eBandwidthState::idle | stream::eBandwidthState::freeze)) {
			bandwidthInfo->state = stream::eBandwidthState::freeze;
			bandwidthInfo->changeBandwidthTime = ct;
		}
		if (loadBandwidth >= _percentFreezeBandwidth) {
			calculateReduceBandwidth(loadBandwidth, 400.0);
			if (bandwidthInfo->numPeriodicTicks < 20) {
				bandwidthInfo->upperPhysicalBandwidth = _restrictBandwidthMax;
			}
			bandwidthInfo->numDetectTicks = 0;
		}
	}
	else if (bandwidthInfo->receivedBytes == 0) {
		if (bandwidthInfo->state & (stream::eBandwidthState::idle | stream::eBandwidthState::freeze)) {
			bandwidthInfo->state = stream::eBandwidthState::freeze;
			bandwidthInfo->changeBandwidthTime = ct;
		}
		bandwidthInfo->numDetectTicks = 0;
	}
	else if (bandwidthInfo->physicalBandwidth != _undefBandwidth) {
		if (bandwidthInfo->state & stream::eBandwidthState::limit) {
			bandwidthInfo->state = stream::eBandwidthState::detect;
			if (bandwidthInfo->numPeriodicTicks < 20) {
				bandwidthInfo->upperStep = calculateUpperStep(30000, _bitrateDetectTm, 5.0);
			}
			else {
				bandwidthInfo->upperStep = calculateUpperStep(30000, _bitrateDetectTm, 2.0);
			}
			bandwidthInfo->changeBandwidthTime = ct;
		}
		else if (bandwidthInfo->state & stream::eBandwidthState::detect && bandwidthInfo->calculateBandwidth >= bandwidthInfo->upperPhysicalBandwidth) {
			bandwidthInfo->state = stream::eBandwidthState::freeze;
			bandwidthInfo->changeBandwidthTime = ct;
		}
		else if ((bandwidthInfo->state & stream::eBandwidthState::freeze) && (freezeBandwidthTm >= 3 * _bitrateFreezeTm)) {
			bandwidthInfo->state = stream::eBandwidthState::idle;
			bandwidthInfo->lowerPhysicalBandwidth = bandwidthInfo->calculateBandwidth;
			bandwidthInfo->upperPhysicalBandwidth = std::max(bandwidthInfo->upperPhysicalBandwidth, bandwidthInfo->calculateBandwidth);
			bandwidthInfo->upperStep = calculateUpperStep(30000, _bitrateDetectTm, 1.5);
			bandwidthInfo->changeBandwidthTime = ct;
		}
		else if ((bandwidthInfo->state & stream::eBandwidthState::idle) && freezeBandwidthTm >= 2 * _bitrateFreezeTm) {
			bandwidthInfo->lowerPhysicalBandwidth = bandwidthInfo->upperPhysicalBandwidth;
			if (freezeBandwidthTm >= 10 * _bitrateFreezeTm) {
				bandwidthInfo->upperStep = calculateUpperStep(30000, _bitrateDetectTm, 3.0);
			}
		}
	}
	else {
		bandwidthInfo->state = stream::eBandwidthState::undef;
	}
	/// try increase bitrate
	bandwidthInfo->numDetectTicks++;
	if ((bandwidthInfo->numDetectTicks % 4) == 0 &&
		(bandwidthInfo->state & (stream::eBandwidthState::detect | stream::eBandwidthState::idle))) {
		uint32_t highBandwidth(bandwidthInfo->upperPhysicalBandwidth);
		uint32_t doubleBandwidth = clamp_cast<uint32_t>(static_cast<uint64_t>(_detectCoefLimit) * bandwidthInfo->lowerPhysicalBandwidth);

		if (bandwidthInfo->lowerPhysicalBandwidth == bandwidthInfo->upperPhysicalBandwidth) {
			highBandwidth = clamp_cast<uint32_t>(static_cast<uint64_t>(_detectCoefLimit) * bandwidthInfo->physicalBandwidth);
		}
		else {
			doubleBandwidth = std::min(doubleBandwidth, bandwidthInfo->upperPhysicalBandwidth);
		}
		double k((bandwidthInfo->calculateBandwidth <= 100) ? std::max(bandwidthInfo->upperStep, 1.05) : bandwidthInfo->upperStep);
		uint64_t newBitrate = static_cast<uint64_t>(bandwidthInfo->calculateBandwidth * k + 3.0);
		if (newBitrate >= doubleBandwidth && (bandwidthInfo->state & stream::eBandwidthState::detect)) {
			bandwidthInfo->upperStep = calculateUpperStep(30000, _bitrateDetectTm, 5.0);
		}
		bandwidthInfo->calculateBandwidth = std::min(clamp_cast<uint32_t>(newBitrate), highBandwidth);
	}
	bandwidthInfo->restrictBitrate = std::min(bandwidthInfo->calculateBandwidth, _restrictBandwidthMax);
	bandwidthInfo->loadBandwidth = loadBandwidth;

	if (!bandwidthInfo->loggedParticipant.empty() && bandwidthInfo->state != stream::eBandwidthState::undef) {
		uint64_t dt1 = ct - bandwidthInfo->overflowTime;
		uint64_t dt2 = ct - bandwidthInfo->changeBandwidthTime;
		dprint3("CalcBitrate [sr] %30s: bw = %10u [%10u], btr = %10u, bwl = %10d, bwh = %10u, load = %5d, state = %16s, [%4u], [%7u, %7u], dt_ovf = %10" PRIu64 ", dt_pb = %10" PRIu64 "\n",
				bandwidthInfo->loggedParticipant.c_str(), bandwidthInfo->physicalBandwidth, instantBand, bandwidthInfo->calculateBandwidth,
				bandwidthInfo->lowerPhysicalBandwidth, bandwidthInfo->upperPhysicalBandwidth, loadBandwidth,
				stream::bandwidth_state_to_string((stream::eBandwidthState)(bandwidthInfo->state)),
				bandwidthInfo->queueLenght, bandwidthInfo->queueBytes, bandwidthInfo->receivedBytes, dt1, dt2);
	}

	return (oldBitrate != bandwidthInfo->restrictBitrate);
}

}
