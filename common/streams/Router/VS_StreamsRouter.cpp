#if defined(_WIN32) // Not ported yet

/////////////////////////////////////////////////////////////////////////////////////////
//
//  (c) 2002 Visicron Inc.  http://www.visicron.net/
//
//  Project:
//
//  Created: 12.11.02     by  A.Slavetsky
//
/////////////////////////////////////////////////////////////////////////////////////////
///
/// \file VS_StreamsRouter.cpp
/// \brief
/// \note
///

#include <stdio.h>
#include <conio.h>
#include <direct.h>
#include <process.h>
#include <errno.h>
#include <Windows.h>

#include "VS_StreamsRouter.h"
#include "ConferencesConditionsSplitter.h"
#include "CSVLogger.h"
#include "DefaultBuffer.h"
#include "Types.h"
#include "SVCBuffer.h"
#include "VS_StreamsHandler.h"
#include "VS_StreamsMonitor.h"
#include "StatisticsInterface.h"
#include "ParticipantStatisticsCalculator.h"
#include "streams/VS_StreamsDefinitions.h"
#include "streams/Statistics.h"
#include "streams/Handshake.h"
#include "acs/Lib/VS_AcsLib.h"
#include "acs/AccessConnectionSystem/VS_AccessConnectionSystem.h"
#include "acs/AccessConnectionSystem/VS_TlsHandler.h"
#include "acs/connection/VS_ConnectionSock.h"
#include "acs/connection/VS_ConnectionMsg.h"
#include "acs/connection/VS_ConnectionByte.h"
#include "acs/connection/VS_ConnectionOv.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/VS_MediaFormat.h"
#include "std/cpplib/VS_Utils.h"
#include "std/cpplib/ThreadUtils.h"
#include "std/cpplib/fast_mutex.h"
#include "std/cpplib/VS_Protocol.h"
#include "std/debuglog/VS_Debug.h"
#include "std/Globals.h"

#include "SecureLib/VS_StreamCrypter.h"

#include <boost/filesystem/operations.hpp>

#include "std-generic/compat/memory.h"
#include <cassert>
#include <map>
#include <string>
#include <vector>

extern std::string g_tr_endpoint_name;

#define   VS_SR_MAX_CONFERENCES    400
#define   VS_SR_MAX_PARTICIPANTS   2500	//50*50
#define   VS_SR_MIN_SILENCE_MS     10000
#define   VS_SR_TIMEOUT_THREAD_SHUTDOWN    30000
#define   VS_SR_REPEAT_ERROR_NUMBER   10
#define   VS_SR_TICK_MS   500
///Connecttion death time in seconds
#define   VS_SR_CONN_SUMP_DEPTH_TIME   180

#define   VS_SR_TIMEOUT_SEND_STATISTICS   2000

#define   VS_SR_SND_CONNECTION_WRITE    1
#define   VS_SR_SND_CONNECTION_READ     2
#define   VS_SR_RCV_CONNECTION_WRITE    3
#define   VS_SR_RCV_CONNECTION_READ     4
#define   VS_SR_CONNECTION_DEAD_WRITE   5
#define   VS_SR_CONNECTION_DEAD_READ    6
#define   VS_SR_CONTROL_WRITE           7
#define   VS_SR_CONTROL_READ            8
#define   VS_SR_MONITOR_WRITE           9
#define   VS_SR_MONITOR_READ            10
#define   VS_SR_MONITOR_CONNECT         11

#define DEBUG_CURRENT_MODULE VS_DM_STREAMS

const char   VS_SrPrefixMonPipe[] = VS_SR_PREFIX_MON_PIPE;

namespace {
	/* regestry managed parameters */
	signed long __max_bitrate = 384;// maximum bitrate for conference (kbps)
	signed long __min_bitrate = 40;	// minimum bitrate for conference (kbps)
	signed long __min_bitstep = 5;	// minimal bitrate change downstep (kbps)
	signed long __log_scr = 0;		// write bitrate debug info on screen
	signed long __bitrate_false = 0;// disable bitrate changing
	signed long __broker_load = 6;	// default broker load threshold (percents)
	float __bit_variance = 0.701f;	// used as barriaer in bitrate change

	union VS_ULONGLONG
	{
		unsigned long long _rax;
		struct {
			unsigned long _raxL;
			unsigned long _raxH;
		};
		explicit VS_ULONGLONG(unsigned long init32) : _rax(init32) {};
	};

	// this function hould be called at least once in 47.5 days or it can lost GetTickCount() overflow
	inline unsigned long long VS_GetTickCount64()
	{
		static VS_ULONGLONG result(GetTickCount());
		unsigned long newValue = GetTickCount();
		if (newValue < result._raxL) ++result._raxH;
		result._raxL = newValue;
		return result._rax;
	};

	/* global conference timetick variables */
	signed long long __confStartTime = VS_GetTickCount64();	// init time value
	signed long long __confDeltaTime = 0;					// offset time value

	struct ConnectToSinkParams
	{
		ConnectToSinkParams(const stream::FrameReceivedSignalType::slot_type& st)
			:slot(st)
		{}
		const stream::FrameReceivedSignalType::slot_type& slot;
		boost::signals2::connection	res_conn;
	};
}

extern signed long __enable_QoS;// if val > 0, enable QoS IP Precedence bits

static void strcpy(char* dst, string_view src)
{
	dst[src.copy(dst, src.size())] = '\0';
}

//////////////////////////////////////////////////////////////////////////////////////////

struct VS_StreamsRouter_ImplementationBase
{
	virtual inline void	Cond_CreateConference( const char *conferenceName ) = 0;
	virtual inline void	Cond_AddParticipant( const char *conferenceName, const char *participantName ) = 0;
	virtual inline void	Cond_RemoveParticipant(const char *conferenceName, const char *participantName, const stream::ParticipantStatistics& ps) = 0;
	virtual inline void	Cond_RemoveConference(const char *conferenceName, const stream::ConferenceStatistics& cs) = 0;
	virtual inline void Cond_SetParticipantCaps(const char *conferenceName, const char *partName, const void *caps_buf,const unsigned long buf_sz) = 0;

	VS_StreamsRouter_ImplementationBase( void )
		: indexSump(0), currDiffTm(0), currTm(0), sumCurrDiffTm(0), tickDiffTm(0)
		, tickTm(0), sendBytes(0), lastSendBytes(0), receiveBytes(0), lastReceiveBytes(0)
		, out_bytes(0.), in_bytes(0.), lastBitrateTm(0), allDeleted(0), added2Port(0), deletedFromPort(0), m_counter(0)
	{}
	// end VS_StreamsRouter_ImplementationBase::VS_StreamsRouter_ImplementationBase

	unsigned long   indexSump, currDiffTm, currTm, sumCurrDiffTm, tickDiffTm, tickTm;

	std::string logDir;
	unsigned __int64   sendBytes, lastSendBytes, receiveBytes, lastReceiveBytes;
	double   out_bytes, in_bytes;		unsigned long   lastBitrateTm;
	struct VS_RemoveConn
	{
		VS_RemoveConn():conn(0),
			time(0),flag(0)
		{}
		VS_Connection* conn;
		unsigned long time;
		char flag;
	};
	unsigned long allDeleted,added2Port,deletedFromPort;
	unsigned long m_counter;
protected:
	typedef std::map< unsigned long , VS_RemoveConn> VS_TYPE_RemovedConns;
	typedef VS_TYPE_RemovedConns::iterator VS_TYPE_RemovedConnsIt;
	VS_TYPE_RemovedConns m_removed;
public:
static const unsigned long maxCouterInterval = VS_SR_CONN_SUMP_DEPTH_TIME;///xSeconds - life of deleted conns
static const unsigned long counterBarrier = 10;

	inline void PeriodicDeleteConn()
	{
		if (m_counter>=counterBarrier)
		{
			m_counter = 0;
			if (!m_removed.empty())
			{
				MakeCloseDeletedConn();
				MakeDeleteDeletedConn();
				AutoEraseDeletedConn();
			}
		}else
		{
			++m_counter;
		}
	}
private:
	inline void FastRemoveDeleted(VS_Connection *sock,bool isFromRouteDelete = false)
	{
		if (m_removed.empty())
			return;

		unsigned long key = 0;
		memcpy(&key,&sock,4);

		VS_TYPE_RemovedConnsIt it(m_removed.find( key ));
		if (it!=m_removed.end())
		{
			m_removed.erase( key );
		}
	}
	inline void AutoEraseDeletedConn()
	{
		if (m_removed.empty()) return;

		VS_TYPE_RemovedConnsIt it(m_removed.begin());
        while (it != m_removed.end())
		{
            if (it->second.flag == 2) {
                it = m_removed.erase( it );
            } else ++it;
		}
	}

	inline void MakeDeleteDeletedConn()
	{
		if (m_removed.empty())
			return;

		unsigned long a = GetTickCount();
		VS_TYPE_RemovedConnsIt it(m_removed.begin());
		for (;it!=m_removed.end();++it)
		{
			if ((( a - it->second.time ) > maxCouterInterval*1000)
				&& (it->second.flag == 1))
			{
				delete it->second.conn;
				it->second.conn = 0;
				it->second.flag = 2;
				it->second.time = a;
				++deletedFromPort;
			}
		}
	}

	inline void MakeCloseDeletedConn()
	{
		if (m_removed.empty())
			return;

		unsigned long a = GetTickCount();
		VS_TYPE_RemovedConnsIt it(m_removed.begin());
		for (;it!=m_removed.end();++it)
		{
			if ((( a - it->second.time ) > maxCouterInterval*1000)
				&& (it->second.flag==0))
			{
				if (it->second.conn)
					it->second.conn->Close();
				it->second.flag = 1;
				it->second.time = a;
			}
		}
	}
protected:
	inline void RemoveAllDeleteConns()
	{
		if (m_removed.empty())
			return;


		VS_TYPE_RemovedConnsIt it(m_removed.begin());
		unsigned long a = GetTickCount();
		for (;it!=m_removed.end();++it)
		{
			if ( it->second.conn )
			{
				it->second.conn->Close();
					delete it->second.conn;
				it->second.conn = 0;
				it->second.flag = 2;
				it->second.time = a;
			}
		}
		m_removed.clear();
	}
public:
	inline void AddRouteDeleteConn(VS_Connection *sock)
	{
		VS_RemoveConn rconn;
		unsigned long key = 0;
		memcpy(&key,&sock,4);

		rconn.conn = sock;
		rconn.time = GetTickCount();
		m_removed[ key ] =  rconn ;
		++added2Port;
	}
	inline void DeleteConn( VS_Connection *sock,
							bool isFromRouteDelete = false)
	{
		if (!sock)	return;
		if (sock->IsRW()) {
			return;
		}

		if (isFromRouteDelete)
		{
			++deletedFromPort;
		}
		FastRemoveDeleted( sock, isFromRouteDelete );

		sock->SetOvFields();		sock->Close();
		delete sock;
	}
	// end VS_StreamsRouter_Participant::DeleteConn
};
// end VS_StreamsRouter_ImplementationBase struct

struct VS_StreamsRouter_ConferenceBase
{
	VS_StreamsRouter_ConferenceBase( VS_StreamsRouter_ImplementationBase *sr,
									const unsigned long maxParts, const unsigned long index, const int type,
									const char *conferenceName )
		: sr(sr), maxParts(maxParts), index(index), type(type)
	{	strncpy( VS_StreamsRouter_ConferenceBase::conferenceName, conferenceName, sizeof(VS_StreamsRouter_ConferenceBase::conferenceName) );
		memset( (void *)&confStat, 0, sizeof(confStat) );	}
	// end VS_StreamsRouter_ConferenceBase::VS_StreamsRouter_ConferenceBase

	virtual inline void ProcessingWriteFrame( const unsigned long index ) = 0;
	virtual inline void ProcessingReadFrame( const unsigned long index,
								const stream::FrameHeader *readHead, std::unique_ptr<char[]> readBuffer) = 0;
	virtual inline void ProcessingStatistics( const unsigned long index ) = 0;
	virtual inline VS_StreamCrypter* GetCrypter() = 0;

	VS_StreamsRouter_ImplementationBase   *sr;
	const unsigned long   maxParts, index;
	const int type;
	char   conferenceName[VS_STREAMS_MAX_SIZE_CONFERENCE_NAME + 1];
	stream::ConferenceStatistics confStat;
	stream::CSVLog logFile;
};
// end VS_StreamsRouter_ConferenceBase struct

//////////////////////////////////////////////////////////////////////////////////////////

struct VS_StreamsRouter_Participant
{
	VS_StreamsRouter_Participant( VS_StreamsRouter_ImplementationBase *streamsRouter,
									VS_StreamsRouter_ConferenceBase *streamsConference,
									const unsigned long index, const char *participantName,
									stream::Buffer* sndBuffer, const bool flagBufferDelete,
									const bool formLogs, const unsigned long maxSilence )
		: isValid(false), sr(streamsRouter), sc(streamsConference), sndBuffer(sndBuffer)
		, index(index), senderConnectedFlag(0), receiverFTracks(0), receiverConnectedIndex(0)
		, nSenderConnected(0), nReceiverConnected(0), timeoutSendStatistics(VS_SR_TIMEOUT_SEND_STATISTICS)
		, readSize(0), writeSize(0), lastTickTmSendStatistics(streamsRouter->currTm), writeFlag(false)
		, flagSendStatistics(false), sndConn(0), rcvConn(0), flagBufferDelete(flagBufferDelete)
		, stateRcv(0), flagRcvHS(false), createdDate(time(0))
		, rcvLastConDt(0), rcvLastDisDt(0), minRcvFramesSz(0), aveRcvFramesSz(0), maxRcvFramesSz(0)
		, sndLastConDt(0), sndLastDisDt(0), minSndFramesSz(0)
		, aveSndFramesSz(0), maxSndFramesSz(0), maxSilence(maxSilence), activeTm(sr->currTm),
		sconf_maxParts(streamsConference->maxParts), m_hasVideoChannel(true)
	{
		*sender = *receiver = 0;
		memset( (void *)&partStat, 0, sizeof(partStat) );
		strncpy( VS_StreamsRouter_Participant::participantName, participantName, sizeof(VS_StreamsRouter_Participant::participantName) );
		VS_StreamsRouter_Participant::participantName[sizeof(VS_StreamsRouter_Participant::participantName) - 1] = 0;
		size_t   sz = streamsConference->maxParts * sizeof(bool);
		senderConnectedFlag = (bool *)malloc( sz );		if (!senderConnectedFlag)	return;
		for (unsigned long i = 0; i < streamsConference->maxParts; ++i)		senderConnectedFlag[i] = false;
		sz = streamsConference->maxParts * sizeof(unsigned long);
		receiverConnectedIndex = (unsigned long *)malloc( sz );		if (!receiverConnectedIndex)	return;
		for (unsigned long i = 0; i < streamsConference->maxParts; ++i)		receiverConnectedIndex[i] = ~0;
		sz = streamsConference->maxParts * sizeof(unsigned char *);
		receiverFTracks = (unsigned char **)malloc( sz );	if (!receiverFTracks)	return;
		memset( (void *)receiverFTracks, 0, sz );
		memset( (void *)&writeHead, 0, sizeof(writeHead) );
		memset( (void *)&readHead, 0, sizeof(readHead) );
		memset(sndMTracks, 0, sizeof(sndMTracks));
		memset(rcvMTracks, 0, sizeof(rcvMTracks));
		isValid = sndBuffer->Init(streamsConference->conferenceName, participantName);
		if (isValid) {
			VS_StreamCrypter *pCrypter = streamsConference->GetCrypter();
			if (pCrypter && pCrypter->IsValid()) sndBuffer->SetStreamCrypter(pCrypter);
			sndBuffer->SetParticipantStatisticsInterface(&statCalc);
			// Call was moved one level up
			////streamsRouter->Cond_AddParticipant( streamsConference->conferenceName, VS_StreamsRouter_Participant::participantName );
			{
				VS_RegistryKey(false, CONFIGURATION_KEY).GetString(bandwidthInfo.loggedParticipant, "p2p bitrate logged participant");
				if (bandwidthInfo.loggedParticipant != sndBuffer->ParticipantName()) {
					bandwidthInfo.loggedParticipant.clear();
				}
			}
		}
		if (formLogs)
			logFile.Open(sr->logDir, streamsConference->conferenceName, participantName);
	}
	// end VS_StreamsRouter_Participant::VS_StreamsRouter_Participant

	~VS_StreamsRouter_Participant( void )
	{
		sndBuffer->Destroy(sndBuffer->ConferenceName(), sndBuffer->ParticipantName());
		if (senderConnectedFlag)		free( (void *)senderConnectedFlag );
		if (receiverConnectedIndex)		free( (void *)receiverConnectedIndex );
		DeleteSndConn();	DeleteRcvConn();
		if (sndBuffer/* && flagBufferDelete*/)		delete sndBuffer;
		if (isValid)	sr->Cond_RemoveParticipant( sc->conferenceName, participantName, partStat );
		if (receiverFTracks)
		{
			for (unsigned long i = 0; i<sconf_maxParts; i++) {
				if (receiverFTracks[i])
					free(receiverFTracks[i]);
				receiverFTracks[i] = 0;
			}
			free(receiverFTracks);
			receiverFTracks = 0;
		}
		receivedStatisticBuffer.clear();
	}
	// end VS_StreamsRouter_Participant::VS_StreamsRouter_Participant

	bool   isValid;
	VS_StreamsRouter_ImplementationBase   *sr;
	VS_StreamsRouter_ConferenceBase   *sc;
	stream::Buffer* sndBuffer;
	char   participantName[VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME + 1];
	const unsigned long   index, timeoutSendStatistics;		bool   *senderConnectedFlag;
	unsigned long   *receiverConnectedIndex;	unsigned char   **receiverFTracks;
	unsigned long   nSenderConnected, nReceiverConnected, readSize, writeSize, lastTickTmSendStatistics;
	bool   writeFlag, flagSendStatistics;
	VS_ConnectionSock   *sndConn, *rcvConn;
	stream::FrameHeader writeHead;
	stream::FrameHeader readHead;
	const bool   flagBufferDelete;
	std::unique_ptr<char[]> readBuffer;
	unsigned   stateRcv;
	bool   flagRcvHS;
	uint8_t sndMTracks[32];
	uint8_t rcvMTracks[32];
	const time_t   createdDate;
	char   receiver[10];
	time_t   rcvLastConDt, rcvLastDisDt;
	unsigned __int64   rcvBytes, rcvFrames;
	unsigned short   minRcvFramesSz, aveRcvFramesSz, maxRcvFramesSz;
	char   sender[10];
	time_t   sndLastConDt, sndLastDisDt;
	unsigned __int64   sndBytes, sndFrames;
	unsigned short   minSndFramesSz, aveSndFramesSz, maxSndFramesSz;
	stream::ParticipantStatistics partStat;
	stream::ParticipantStatisticsCalculator statCalc;
	stream::ParticipantBandwidthInfo bandwidthInfo;
	stream::CSVLog logFile;
	stream::CSVLogger logger;
	const unsigned long   maxSilence;		unsigned long   activeTm;
	unsigned long sconf_maxParts;
	bool m_hasVideoChannel;
	std::vector<uint8_t> receivedStatisticBuffer;

	stream::StreamStatistics* RetrieveInstantSendStatistic()
	{
		auto rcvSize(statCalc.GetSndStatisticsSize());
		if (receivedStatisticBuffer.size() < rcvSize) {
			receivedStatisticBuffer.resize(rcvSize);
		}
		bool video(true);
		auto rcvStat = reinterpret_cast<stream::StreamStatistics*>(receivedStatisticBuffer.data());
		if (statCalc.FormSndStatistics(rcvStat, rcvSize, &video) <= 0) {
			return nullptr;
		}
		return rcvStat;
	}
	// end VS_StreamsRouter_Participant::RetrieveInstantSendStatistic

	inline void CalculateInstantSendStatistic()
	{
		auto rcvStat = RetrieveInstantSendStatistic();
		if (!rcvStat) {
			return;
		}
		bandwidthInfo.queueBytes = sndBuffer->GetQueueBytes();
		bandwidthInfo.queueLenght = sndBuffer->GetFrameCount();
		bandwidthInfo.receivedBytes = rcvStat->allWriteBytesBand;
	}
	// end VS_StreamsRouter_Participant::CalculateInstantSendStatistic

	inline bool CalculateBandwidth()
	{
		if (!sndBuffer->Ready()) {
			return false;
		}
		CalculateInstantSendStatistic();
		return CalculateParticipantBandwidth(&bandwidthInfo, 20, false);
	}
	// end VS_StreamsRouter_Participant::CalculateBandwidth

	inline bool ProcessingPeriodic( void )
	{
		statCalc.ProcessingSend(sr->currTm, sndBuffer);
		statCalc.ProcessingReceive(sr->currTm);
		if ((sr->currTm - lastTickTmSendStatistics) > timeoutSendStatistics)
		{
			lastTickTmSendStatistics = sr->currTm;
			flagSendStatistics = (sc->type != VS_Conference_Type::CT_PRIVATE);
			Write();
		}
		if (sndConn || rcvConn) {	activeTm = sr->currTm;	return true;	}
		return (sr->currTm - activeTm) < maxSilence;
	}
	// end VS_StreamsRouter_Participant::ProcessingPeriodic

	inline void DeleteSndConn( void )
	{
		if (sndConn)
		{
			const char* pCh = sndConn->GetPeerIp();
			if (pCh) {	strncpy( partStat.receiverIp, pCh, sizeof(partStat.receiverIp) );
						partStat.receiverIp[sizeof(partStat.receiverIp) - 1] = 0;	}
			if (!sndConn->IsRW())
			{
				sr->DeleteConn( sndConn );
			}
			else
			{
				sndConn->SetOvReadFields( VS_SR_CONNECTION_DEAD_READ, (VS_ACS_Field)sndConn );
				sndConn->SetOvWriteFields( VS_SR_CONNECTION_DEAD_WRITE, (VS_ACS_Field)sndConn );
				sndConn->Disconnect();
				sr->AddRouteDeleteConn( sndConn );
			}
			sndConn = 0;	*sender = 0;	sndLastDisDt = time(0);
		}
		writeFlag = false;		writeSize = 0;
		statCalc.FreeSndStatCalc(&partStat);
		flagSendStatistics = false;
		lastTickTmSendStatistics = sr->currTm;
	}
	// end VS_StreamsRouter_Participant::DeleteSndConn

	inline void DeleteRcvConn( void )
	{
		if (rcvConn)
		{
			const char* pCh = rcvConn->GetPeerIp();
			if (pCh) {	strncpy( partStat.senderIp, pCh, sizeof(partStat.senderIp) );
						partStat.senderIp[sizeof(partStat.senderIp) - 1] = 0;	}
			if (!rcvConn->IsRW())
			{
				sr->DeleteConn( rcvConn );
			}
			else
			{
				rcvConn->SetOvReadFields( VS_SR_CONNECTION_DEAD_READ, (VS_ACS_Field)rcvConn );
				rcvConn->SetOvWriteFields( VS_SR_CONNECTION_DEAD_WRITE, (VS_ACS_Field)rcvConn );
				rcvConn->Disconnect();
				sr->AddRouteDeleteConn( rcvConn );
			}
			rcvConn = 0;	*receiver = 0;	rcvLastDisDt = time(0);
		}
		readBuffer.reset();
		readSize = sizeof(stream::FrameHeader);		stateRcv = 0;
		statCalc.FreeRcvStatCalc(&partStat);
	}
	// end VS_StreamsRouter_Participant::DeleteRcvConn

	inline void Write( const void *buffer, const unsigned long s_buffer )
	{
		writeFlag = true;
		VS_Buffer buffers[] = {{ sizeof(stream::FrameHeader), (void *)&writeHead },
									{ (unsigned long)s_buffer, (void *)buffer }};
		if (!sndConn->RWrite( buffers, 2 ))		DeleteSndConn();
		else writeSize = s_buffer + sizeof(stream::FrameHeader);
	}
	// end VS_StreamsRouter_Participant::Write

	inline void Write( void )
	{
		if (!sndConn || sndConn->IsWrite())		return;
		if (flagSendStatistics)
		{
			flagSendStatistics = false;
			if (stream::IsInMTracks(sndMTracks, stream::Track::old_command))
			{
				sc->ProcessingStatistics( index );
				if (!sndConn || sndConn->IsWrite())		return;
		}	}
		sc->ProcessingWriteFrame( index );
	}
	// end VS_StreamsRouter_Participant::Write

	inline void SndWrite( unsigned long bTrans, VS_Overlapped &ov )
	{
		if (!sndConn)
		{	/* Here it will be necessary to throw off in TRACE */
			DeleteSndConn();	return;
		}
		int   ret = sndConn->SetWriteResult( bTrans, &ov );
		if (ret == -2)		return;
		if (ret != (int)writeSize)
		{	/* Here it will be necessary to throw off in TRACE */
			DeleteSndConn();	return;
		}
		sr->sendBytes += writeSize;
		if (writeFlag)
		{
			logger.LogFrameWrite(writeHead, participantName, &logFile, &sc->logFile);
			statCalc.SndWrite(writeHead, sr->currTm);
			writeFlag = false;
		}
		Write();
	}
	// end VS_StreamsRouter_Participant::SndWrite

	inline void SndRead( unsigned long bTrans, VS_Overlapped &ov )
	{
		if (!sndConn)
		{	/* Here it will be necessary to throw off in TRACE */
			DeleteSndConn();	return;
		}
		int   ret = sndConn->SetReadResult( bTrans, &ov );
		if (ret == -2)		return;
		DeleteSndConn();
	}
	// end VS_StreamsRouter_Participant::SndRead

	inline bool Read( void )
	{
		switch (stateRcv)
		{
		case 0 :
			readSize = readHead.length;
			if (readSize)
			{
				if (readSize > VS_STREAM_MAX_SIZE_FRAME)
				{	/* Here it will be necessary to throw off in TRACE */	return false;	}
				readBuffer = std::make_unique<char[]>(readSize);
				stateRcv = 1;
				return rcvConn->Read(readBuffer.get(), readSize);
			}
		case 1 :
			if (readHead.cksum != stream::GetFrameBodyChecksum(readBuffer.get(), readSize)
				|| readHead.track != stream::Track{} && !stream::IsInMTracks(rcvMTracks, readHead.track))
			{	/* Here it will be necessary to throw off in TRACE */	return false;	}
			logger.LogFrameRead(readHead, participantName, &logFile, &sc->logFile);
			statCalc.RcvRead(readHead, sr->currTm);
			sc->ProcessingReadFrame(index, &readHead, std::move(readBuffer));
			readSize = sizeof(readHead);
			stateRcv = 0;
			if (!rcvConn->Read( (void *)&readHead, sizeof(readHead) ))	return false;
			return true;
		default :	/* Here it will be necessary to throw off in TRACE */	return false;
	}	}
	// end VS_StreamsRouter_Participant::Read

	inline void RcvRead( unsigned long bTrans, VS_Overlapped &ov )
	{
		if (!rcvConn)
		{	/* Here it will be necessary to throw off in TRACE */
			DeleteRcvConn();	return;
		}
		int   res = rcvConn->SetReadResult( bTrans, &ov );
		if (res == -2)
			return;
		if (res != (int)readSize) {		DeleteRcvConn();	return;		}

		sr->receiveBytes += readSize;
		if (!Read())	DeleteRcvConn();
	}
	// end VS_StreamsRouter_Participant::RcvRead

	inline void RcvWrite( unsigned long bTrans, VS_Overlapped &ov )
	{
		if (!rcvConn)
		{	/* Here it will be necessary to throw off in TRACE */
			DeleteRcvConn();	return;
		}
		int   res = rcvConn->SetWriteResult( bTrans, &ov );
		if (res == -2)		return;
		if (res > 0 && flagRcvHS)
		{	sr->sendBytes += res;	flagRcvHS = false;		return;		}
		DeleteRcvConn();
	}
	// end VS_StreamsRouter_Participant::RcvWrite

	inline void SetConnectSender( const unsigned long index )
	{
		if (!senderConnectedFlag[index]) {		senderConnectedFlag[index] = true;
												++nSenderConnected;			}
	}
	// end VS_StreamsRouter_Participant::SetConnectSender

	inline void SetConnectReceiver( const unsigned long index )
	{
		unsigned long   i = 0;
		for (; i < nReceiverConnected && receiverConnectedIndex[i] != index; ++i);
		if (i >= nReceiverConnected) {	receiverConnectedIndex[nReceiverConnected] = index;
										++nReceiverConnected;	}
	}
	// end VS_StreamsRouter_Participant::SetConnectSender

	inline void SetParticipantTracks( const unsigned long index, const unsigned char *mtracks )
	{
		unsigned char   *&ftracks = receiverFTracks[index], ftrs[256];
		stream::MTracksToFTracks(ftrs, mtracks);
		const unsigned nTracks = stream::CountFTracks(ftrs);
		if (nTracks >= 255) {	if (ftracks) {		free( (void *)ftracks );	ftracks = 0;	}
								return;		}
		if (!ftracks) {		ftracks = (unsigned char *)malloc( 256 );
							if (!ftracks)	return;		}
		memcpy( (void *)ftracks, (void *)ftrs, 256 );
		m_hasVideoChannel = ftrs[id(stream::Track::video)] == 1;
	}
	// end VS_StreamsRouter_Participant::SetParticipantTracks

	inline bool IsConnectSender( const unsigned long index )
	{
		return senderConnectedFlag[index];
	}
	// end VS_StreamsRouter_Participant::IsConnectSender

	inline bool IsConnectReceiver( const unsigned long index )
	{
		for (unsigned long i = 0; i < nReceiverConnected; ++i)
			if (receiverConnectedIndex[i] == index)		return true;
		return false;
	}
	// end VS_StreamsRouter_Participant::IsConnectReceiver

	inline void SetDisconnectSender( const unsigned long index )
	{
		if (senderConnectedFlag[index]) {		senderConnectedFlag[index] = false;
												--nSenderConnected;			}
	}
	// end VS_StreamsRouter_Participant::SetDisconnectSender

	inline void SetDisconnectSender( void )
	{
		nSenderConnected = 0;
		for (unsigned long i = 0; i < sc->maxParts; ++i)
			senderConnectedFlag[i] = false;
	}
	// end VS_StreamsRouter_Participant::SetDisconnectSender

	inline void RedressReceiver( unsigned long i )
	{
		--nReceiverConnected;
		for (; i < nReceiverConnected; ++i)
			receiverConnectedIndex[i] = receiverConnectedIndex[i + 1];
	}
	// end VS_StreamsRouter_Participant::RedressReceiver

	inline void SetDisconnectReceiver( const unsigned long index )
	{
		unsigned long   i = 0;
		for (; i < nReceiverConnected && receiverConnectedIndex[i] != index; ++i);
		if (i < nReceiverConnected)		RedressReceiver( i );
	}
	// end VS_StreamsRouter_Participant::SetDisconnectReceiver

	inline void SetDisconnectReceiver( void )
	{
		nReceiverConnected = 0;
		for (unsigned long i = 0; i < sc->maxParts; ++i)
			receiverConnectedIndex[i] = ~0;
	}
	// end VS_StreamsRouter_Participant::SetDisconnectReceiver

	inline void SetConnection( const unsigned type, VS_ConnectionSock *conn,
										const unsigned char *mtracks )
	{
		conn->SetFastSocket(true);
		if (type == VS_ACS_LIB_SENDER)
		{	DeleteSndConn();	sndConn = conn;
			++partStat.receiverNConns;
			sndLastConDt = time(0);
		}
		else
		{	DeleteRcvConn();	rcvConn = conn;
			++partStat.senderNConns;
			rcvLastConDt = time(0);
		}
		auto hs = stream::CreateHandshakeResponse(nullptr);
		if (!hs)
		{
go_delete_conn:
			if (type == VS_ACS_LIB_SENDER)
			{	DeleteSndConn();	sndLastDisDt = time(0);		}
			else
			{	DeleteRcvConn();	rcvLastDisDt = time(0);		}
			return;
		}
		const size_t sz = sizeof(net::HandshakeHeader) + hs->body_length + 1;
		VS_Buffer b = { sz, hs.get() };
		if (!conn->RWrite(&b, 1))
			goto go_delete_conn;
		if (type == VS_ACS_LIB_SENDER)
		{
			writeSize = sz;
			conn->SetOvWriteFields( VS_SR_SND_CONNECTION_WRITE, (VS_ACS_Field)sc->index, (VS_ACS_Field)index );
			conn->SetOvReadFields( VS_SR_SND_CONNECTION_READ, (VS_ACS_Field)sc->index, (VS_ACS_Field)index );
			if (!conn->Read( 0, 0 )) {		DeleteSndConn();	return;		}
			statCalc.InitSndStatCalc(mtracks);
			flagSendStatistics = false;		lastTickTmSendStatistics = sr->currTm;
			memcpy(sndMTracks, mtracks, sizeof(sndMTracks));
			strcpy( sender, "TCP" );
		}
		else
		{
			flagRcvHS = true;
			conn->SetOvWriteFields( VS_SR_RCV_CONNECTION_WRITE, (VS_ACS_Field)sc->index, (VS_ACS_Field)index );
			conn->SetOvReadFields( VS_SR_RCV_CONNECTION_READ, (VS_ACS_Field)sc->index, (VS_ACS_Field)index );
			if (!rcvConn->Read( (void *)&readHead, sizeof(readHead) ))
			{	DeleteRcvConn();	return;		}
			statCalc.InitRcvStatCalc(mtracks);
			memcpy(rcvMTracks, mtracks, sizeof(rcvMTracks));
			strcpy( receiver, "TCP" );
	}	}
	// end VS_StreamsRouter_Participant::SetConnection

	inline void FillMonitorStruct( VS_StreamsMonitor::SmReply::Participant &participant )
	{
		const char* pCh;
		memset((void *)&participant, 0, sizeof(participant));
		participant.type = SM_TYPE_PERIODIC_PARTICIPANT;
		if (sc && sc->conferenceName && *sc->conferenceName) {
			strncpy( participant.conferenceName, sc->conferenceName, sizeof(participant.conferenceName) );
		}
		participant.conferenceName[sizeof(participant.conferenceName) - 1] = 0;
		strncpy( participant.participantName, participantName, sizeof(participant.participantName) );
		participant.participantName[sizeof(participant.participantName) - 1] = 0;
		participant.createdDate = createdDate;
		strncpy( participant.receiver, receiver, sizeof(participant.receiver) );
		participant.receiver[sizeof(participant.receiver) - 1] = 0;
		participant.rcvRecTms = partStat.senderNConns;
		participant.rcvLastConDt = rcvLastConDt;
		participant.rcvLastDisDt = rcvLastDisDt;
		if (rcvConn)
		{	pCh = rcvConn->GetBindIp();
			if (pCh) {	strncpy( participant.rcvLocalHost, pCh, sizeof(participant.rcvLocalHost) );
						participant.rcvLocalHost[sizeof(participant.rcvLocalHost) - 1] = 0;		}
			pCh = rcvConn->GetBindPort();
			if (pCh) {	strncpy( participant.rcvLocalPort, pCh, sizeof(participant.rcvLocalPort) );
						participant.rcvLocalPort[sizeof(participant.rcvLocalPort) - 1] = 0;		}
			pCh = rcvConn->GetPeerIp();
			if (pCh) {	strncpy( participant.rcvTargetHost, pCh, sizeof(participant.rcvTargetHost) );
						participant.rcvTargetHost[sizeof(participant.rcvTargetHost) - 1] = 0;	}
			pCh = rcvConn->GetPeerPort();
			if (pCh) {	strncpy( participant.rcvTargetPort, pCh, sizeof(participant.rcvTargetPort) );
						participant.rcvTargetPort[sizeof(participant.rcvTargetPort) - 1] = 0;	}
		}
		participant.minRcvFramesSz = minRcvFramesSz;
		participant.aveRcvFramesSz = aveRcvFramesSz;
		participant.maxRcvFramesSz = maxRcvFramesSz;
		strncpy( participant.sender, sender, sizeof(participant.sender) );
		participant.sender[sizeof(participant.sender) - 1] = 0;
		participant.sndRecTms = partStat.receiverNConns;
		participant.sndLastConDt = sndLastConDt;
		participant.sndLastDisDt = sndLastDisDt;
		if (sndConn)
		{	pCh = sndConn->GetBindIp();
			if (pCh) {	strncpy( participant.sndLocalHost, pCh, sizeof(participant.sndLocalHost) );
						participant.sndLocalHost[sizeof(participant.sndLocalHost) - 1] = 0;		}
			pCh = sndConn->GetBindPort();
			if (pCh) {	strncpy( participant.sndLocalPort, pCh, sizeof(participant.sndLocalPort) );
						participant.sndLocalPort[sizeof(participant.sndLocalPort) - 1] = 0;		}
			pCh = sndConn->GetPeerIp();
			if (pCh) {	strncpy( participant.sndTargetHost, pCh, sizeof(participant.sndTargetHost) );
						participant.sndTargetHost[sizeof(participant.sndTargetHost) - 1] = 0;	}
			pCh = sndConn->GetPeerPort();
			if (pCh) {	strncpy( participant.sndTargetPort, pCh, sizeof(participant.sndTargetPort) );
						participant.sndTargetPort[sizeof(participant.sndTargetPort) - 1] = 0;	}
		}
		participant.minSndFramesSz = minSndFramesSz;
		participant.aveSndFramesSz = aveSndFramesSz;
		participant.maxSndFramesSz = maxSndFramesSz;
		statCalc.FillMonitorStruct(participant);
	}
	// end VS_StreamsRouter_Participant::FillMonitorStruct
};
// end VS_StreamsRouter_Participant struct

//////////////////////////////////////////////////////////////////////////////////////////

struct VS_StreamsRouter_Conference : public VS_StreamsRouter_ConferenceBase
{
	VS_StreamsRouter_Conference( VS_StreamsRouter_ImplementationBase *sr,
						const unsigned long index, const char *conferenceName, const char *sslKey,
						const unsigned long maxParts, const unsigned long maxSilence, const int type,
						const bool formLogs )
		: VS_StreamsRouter_ConferenceBase( sr, maxParts + 20, index, type, conferenceName )
		, isValid(false), part(0), limitParts(maxParts), maxSilence(maxSilence)
		, activeTm(sr->currTm), nParts(0), maxIndParts(0), createdDate(time( 0 ))
	{
		size_t   sz = VS_StreamsRouter_ConferenceBase::maxParts * sizeof(VS_StreamsRouter_Participant *);
		part = (VS_StreamsRouter_Participant **)malloc( sz );
		if (!part)	return;
		memset( (void *)part, 0, sz );
		isValid = true;
		//Call was moved one level up
		///sr->Cond_CreateConference( conferenceName );
		scrypter.Init(sslKey);
		if (formLogs)
			logFile.Open(sr->logDir, conferenceName);
	}
	// end VS_StreamsRouter_Conference::VS_StreamsRouter_Conference

	~VS_StreamsRouter_Conference( void )
	{
		if (part) {
			for (unsigned long i = 0; i < maxParts; ++i)
				if (part[i])	DeleteParticipant( i );
			free( (void *)part );
		}
		if (isValid)	sr->Cond_RemoveConference( conferenceName, confStat );
		scrypter.Free();
	}
	// end VS_StreamsRouter_Conference::~VS_StreamsRouter_Conference

	bool   isValid;
	VS_StreamsRouter_Participant   **part;
	VS_StreamCrypter scrypter;
	const unsigned long   limitParts, maxSilence;
	unsigned long   activeTm, nParts, maxIndParts;
	const time_t   createdDate;
	stream::FrameReceivedSignalType	m_fireFrameReceived;

	inline VS_StreamCrypter* GetCrypter()
	{
		return &scrypter;
	}

	inline unsigned long GetFreeParticipantIndex( void )
	{
		if (nParts < limitParts)
			for (unsigned long i = 0; i < maxParts; ++i)	if (!part[i])	return i;
		return ~0;
	}
	// end VS_StreamsRouter_Conference::GetFreeParticipantIndex

	inline unsigned long GetParticipantIndex( const char *participantName )
	{
		for (unsigned long i = 0; i <= maxIndParts; ++i)
			if (part[i] && !_stricmp( part[i]->participantName, participantName ))
				return i;
		return ~0;
	}
	// end VS_StreamsRouter_Conference::GetParticipantIndex

	inline void DeleteParticipant( const unsigned long index )
	{
		DisconnectParticipant( index );
		delete part[index];		part[index] = 0;	--nParts;
		if (index >= maxIndParts)
		{	maxIndParts = index;	while (maxIndParts && !part[--maxIndParts]);	}
	}
	// end VS_StreamsRouter_Conference::DeleteParticipant

	inline void DeleteParticipant( const char *participantName )
	{
		const unsigned long   index = GetParticipantIndex( participantName );
		if (index < maxParts)	DeleteParticipant( index );
	}
	// end VS_StreamsRouter_Conference::DeleteParticipant

	inline void ConnectParticipantSender( const unsigned long index, const unsigned long connectedIndex )
	{
		VS_StreamsRouter_Participant   *pr = part[index], *connectedPr = part[connectedIndex];
		pr->SetConnectSender( connectedIndex );		connectedPr->SetConnectReceiver( index );
	}
	// end VS_StreamsRouter_Conference::ConnectParticipantSender

	inline void ConnectParticipantReceiver( const unsigned long index, const unsigned long connectedIndex )
	{
		VS_StreamsRouter_Participant   *pr = part[index], *connectedPr = part[connectedIndex];
		pr->SetConnectReceiver( connectedIndex );	connectedPr->SetConnectSender( index );
	}
	// end VS_StreamsRouter_Conference::ConnectParticipantReceiver

	inline void ConnectParticipant( const unsigned long index, const unsigned long connectedIndex )
	{	ConnectParticipantSender( index, connectedIndex );	ConnectParticipantReceiver( index, connectedIndex );	}
	// end VS_StreamsRouter_Conference::ConnectParticipant

	inline void SetParticipantSenderTracks( const unsigned long index, const unsigned long connectedIndex, const unsigned char *mtracks )
	{	part[index]->SetParticipantTracks( connectedIndex, mtracks );	}
	// end VS_StreamsRouter_Conference::SetParticipantSenderTracks

	inline void SetParticipantReceiverTracks( const unsigned long index, const unsigned long connectedIndex, const unsigned char *mtracks )
	{	part[connectedIndex]->SetParticipantTracks( index, mtracks );	}
	// end VS_StreamsRouter_Conference::SetParticipantReceiverTracks

	inline void SetParticipantTracks( const unsigned long index, const unsigned long connectedIndex, const unsigned char *mtracks )
	{	SetParticipantSenderTracks( index, connectedIndex, mtracks );	SetParticipantReceiverTracks( index, connectedIndex, mtracks );		}
	// end VS_StreamsRouter_Conference::SetParticipantTracks

	inline bool IsConnectParticipantSender( const unsigned long index, const unsigned long connectedIndex )
	{
		VS_StreamsRouter_Participant   *pr = part[index], *connectedPr = part[connectedIndex];
		if (!pr->IsConnectSender( connectedIndex ))
		{	connectedPr->SetDisconnectReceiver( index );	return false;	}
		if (!connectedPr->IsConnectReceiver( index ))
		{	pr->SetDisconnectSender( connectedIndex );		return false;	}
		return true;
	}
	// end VS_StreamsRouter_Conference::IsConnectParticipantSender

	inline bool IsConnectParticipantReceiver( const unsigned long index, const unsigned long connectedIndex )
	{
		VS_StreamsRouter_Participant   *pr = part[index], *connectedPr = part[connectedIndex];
		if (!pr->IsConnectReceiver( connectedIndex ))
		{	connectedPr->SetDisconnectSender( index );		return false;	}
		if (!connectedPr->IsConnectSender( index ))
		{	pr->SetDisconnectReceiver( connectedIndex );	return false;	}
		return true;
	}
	// end VS_StreamsRouter_Conference::IsConnectParticipantReceiver

	inline bool IsConnectParticipant( const unsigned long index, const unsigned long connectedIndex )
	{	return IsConnectParticipantSender( index, connectedIndex ) && IsConnectParticipantReceiver( index, connectedIndex );	}
	// end VS_StreamsRouter_Conference::IsConnectParticipant

	inline void DisconnectParticipantSender( const unsigned long index, const unsigned long connectedIndex )
	{
		VS_StreamsRouter_Participant   *pr = part[index], *connectedPr = part[connectedIndex];
		connectedPr->SetDisconnectReceiver( index );	pr->SetDisconnectSender( connectedIndex );
	}
	// end VS_StreamsRouter_Conference::DisconnectParticipantSender

	inline void DisconnectParticipantSender( const unsigned long index )
	{
		VS_StreamsRouter_Participant   *pr = part[index];
		for (unsigned long i = 0; i < maxParts; ++i)
		{	if (pr->senderConnectedFlag[i])
			{	VS_StreamsRouter_Participant   *connectedPr = part[i];
				if (connectedPr)	connectedPr->SetDisconnectReceiver( index );
		}	}
		pr->SetDisconnectSender();
	}
	// end VS_StreamsRouter_Conference::DisconnectParticipantSender

	inline void DisconnectParticipantReceiver( const unsigned long index, const unsigned long connectedIndex )
	{
		VS_StreamsRouter_Participant   *pr = part[index], *connectedPr = part[connectedIndex];
		connectedPr->SetDisconnectSender( index );	pr->SetDisconnectReceiver( connectedIndex );
	}
	// end VS_StreamsRouter_Conference::DisconnectParticipantReceiver

	inline void DisconnectParticipantReceiver( const unsigned long index )
	{
		VS_StreamsRouter_Participant   *pr = part[index];
		for (unsigned long i = 0; i < maxParts; ++i)
		{	const unsigned long   connectedIndex = pr->receiverConnectedIndex[i];
			if (connectedIndex < maxParts)
			{	VS_StreamsRouter_Participant   *connectedPr = part[connectedIndex];
				if (connectedPr)	connectedPr->SetDisconnectSender( index );
		}	}
		pr->SetDisconnectReceiver();
	}
	// end VS_StreamsRouter_Conference::DisconnectParticipantSender

	inline void DisconnectParticipant( const unsigned long index, const unsigned long connectedIndex )
	{	DisconnectParticipantSender( index, connectedIndex );	DisconnectParticipantReceiver( index, connectedIndex );		}
	// end VS_StreamsRouter_Conference::DisconnectParticipant

	inline void DisconnectParticipant( const unsigned long index )
	{	DisconnectParticipantSender( index );	DisconnectParticipantReceiver( index );		}
	// end VS_StreamsRouter_Conference::DisconnectParticipant

	inline bool AddParticipant(const char *participantName, stream::Buffer* sndBuffer,
									const bool flagBufferDelete, const bool formLogs,
									const unsigned long maxSilenceMs )
	{
		if (GetParticipantIndex( participantName ) < maxParts)		return false;
		const unsigned long   index = GetFreeParticipantIndex();
		if (index >= maxParts)		return false;
		VS_StreamsRouter_Participant   *pr = new VS_StreamsRouter_Participant( sr, this, index, participantName, sndBuffer, flagBufferDelete, formLogs, maxSilenceMs );
		if (!pr)	return false;
		if (!pr->isValid) {		delete pr;	return false;	}
		part[index] = pr;	++nParts;
		if (index > maxIndParts)	maxIndParts = index;
		sr->Cond_AddParticipant( conferenceName, pr->participantName );
		return true;
	}
	// end VS_StreamsRouter_Conference::AddParticipant

	inline bool ConnectParticipant( const char *participantName, const char *connectedParticipantName )
	{
		const unsigned long   index = GetParticipantIndex( participantName ),
						connectedIndex = GetParticipantIndex( connectedParticipantName );
		if (index >= maxParts || connectedIndex >= maxParts)	return false;
		ConnectParticipant( index, connectedIndex );			return true;
	}
	// end VS_StreamsRouter_Conference::ConnectParticipant

	inline bool ConnectParticipantSender( const char *participantName,
												const char *connectedParticipantName )
	{
		const unsigned long   index = GetParticipantIndex( participantName ),
						connectedIndex = GetParticipantIndex( connectedParticipantName );
		if (index >= maxParts || connectedIndex >= maxParts)	return false;
		ConnectParticipantSender( index, connectedIndex );		return true;
	}
	// end VS_StreamsRouter_Conference::ConnectParticipantSender

	inline bool ConnectParticipantReceiver( const char *participantName,
												const char *connectedParticipantName )
	{
		const unsigned long   index = GetParticipantIndex( participantName ),
						connectedIndex = GetParticipantIndex( connectedParticipantName );
		if (index >= maxParts || connectedIndex >= maxParts)	return false;
		ConnectParticipantReceiver( index, connectedIndex );	return true;
	}
	// end VS_StreamsRouter_Conference::ConnectParticipantReceiver

	inline bool SetParticipantTracks( const char *participantName, const char *connectedParticipantName, const unsigned char *mtracks )
	{
		const unsigned long   index = GetParticipantIndex( participantName ),
						connectedIndex = GetParticipantIndex( connectedParticipantName );
		if (index >= maxParts || connectedIndex >= maxParts)	return false;
		SetParticipantTracks( index, connectedIndex, mtracks );		return true;
	}
	// end VS_StreamsRouter_Conference::SetParticipantTracks

	inline bool SetParticipantSenderTracks( const char *participantName, const char *connectedParticipantName, const unsigned char *mtracks )
	{
		const unsigned long   index = GetParticipantIndex( participantName ),
						connectedIndex = GetParticipantIndex( connectedParticipantName );
		if (index >= maxParts || connectedIndex >= maxParts)	return false;
		SetParticipantSenderTracks( index, connectedIndex, mtracks );	return true;
	}
	// end VS_StreamsRouter_Conference::SetParticipantSenderTracks

	inline bool SetParticipantReceiverTracks( const char *participantName, const char *connectedParticipantName, const unsigned char *mtracks )
	{
		const unsigned long   index = GetParticipantIndex( participantName ),
						connectedIndex = GetParticipantIndex( connectedParticipantName );
		if (index >= maxParts || connectedIndex >= maxParts)	return false;
		SetParticipantReceiverTracks( index, connectedIndex, mtracks );		return true;
	}
	// end VS_StreamsRouter_Conference::SetParticipantReceiverTracks

	inline bool IsConnectParticipant( const char *participantName, const char *connectedParticipantName )
	{
		const unsigned long   index = GetParticipantIndex( participantName ),
						connectedIndex = GetParticipantIndex( connectedParticipantName );
		if (index >= maxParts || connectedIndex >= maxParts)	return false;
		return IsConnectParticipant( index, connectedIndex );
	}
	// end VS_StreamsRouter_Conference::IsConnectParticipant

	inline bool IsConnectParticipantSender( const char *participantName, const char *connectedParticipantName )
	{
		const unsigned long   index = GetParticipantIndex( participantName ),
						connectedIndex = GetParticipantIndex( connectedParticipantName );
		if (index >= maxParts || connectedIndex >= maxParts)	return false;
		return IsConnectParticipantSender( index, connectedIndex );
	}
	// end VS_StreamsRouter_Conference::IsConnectParticipantSender

	inline bool IsConnectParticipantReceiver( const char *participantName, const char *connectedParticipantName )
	{
		const unsigned long   index = GetParticipantIndex( participantName ),
						connectedIndex = GetParticipantIndex( connectedParticipantName );
		if (index >= maxParts || connectedIndex >= maxParts)	return false;
		return IsConnectParticipantReceiver( index, connectedIndex );
	}
	// end VS_StreamsRouter_Conference::IsConnectParticipantReceiver

	inline void DisconnectParticipant( const char *participantName )
	{
		const unsigned long   index = GetParticipantIndex( participantName );
		if (index < maxParts)	DisconnectParticipant( index );
	}
	// end VS_StreamsRouter_Conference::DisconnectParticipant

	inline void DisconnectParticipant( const char *participantName, const char *connectedParticipantName )
	{
		const unsigned long   index = GetParticipantIndex( participantName ),
							connectedIndex = GetParticipantIndex( connectedParticipantName );
		if (index < maxParts && connectedIndex < maxParts)
			DisconnectParticipant( index, connectedIndex );
	}
	// end VS_StreamsRouter_Conference::DisconnectParticipant

	inline void DisconnectParticipantSender( const char *participantName )
	{
		const unsigned long   index = GetParticipantIndex( participantName );
		if (index < maxParts)	DisconnectParticipantSender( index );
	}
	// end VS_StreamsRouter_Conference::DisconnectParticipantSender

	inline void DisconnectParticipantSender( const char *participantName, const char *connectedParticipantName )
	{
		const unsigned long   index = GetParticipantIndex( participantName ),
							connectedIndex = GetParticipantIndex( connectedParticipantName );
		if (index < maxParts && connectedIndex < maxParts)
			DisconnectParticipantSender( index, connectedIndex );
	}
	// end VS_StreamsRouter_Conference::DisconnectParticipantSender

	inline void DisconnectParticipantReceiver( const char *participantName )
	{
		const unsigned long   index = GetParticipantIndex( participantName );
		if (index < maxParts)	DisconnectParticipantReceiver( index );
	}
	// end VS_StreamsRouter_Conference::DisconnectParticipantReceiver

	inline void DisconnectParticipantReceiver( const char *participantName, const char *connectedParticipantName )
	{
		const unsigned long   index = GetParticipantIndex( participantName ),
							connectedIndex = GetParticipantIndex( connectedParticipantName );
		if (index < maxParts && connectedIndex < maxParts)
			DisconnectParticipantReceiver( index, connectedIndex );
	}
	// end VS_StreamsRouter_Conference::DisconnectParticipantReceiver

	inline void SetConnection( const char *participantName, const unsigned type,
									VS_ConnectionSock *conn, const unsigned char *mtracks )
	{
		const unsigned long   index = GetParticipantIndex( participantName );
		if (index >= maxParts) {	sr->DeleteConn( conn );		return;		}
		part[index]->SetConnection( type, conn, mtracks );
	}
	// end VS_StreamsRouter_Conference::SetConnection

	inline void SndRead( unsigned long bTrans, VS_Overlapped &ov )
	{
		const unsigned long   index = (const unsigned long)ov.field3;
		if (index >= maxParts)
		{	/* Here it will be necessary to throw off in TRACE */	return;		}
		VS_StreamsRouter_Participant   *pr = part[index];
		if (!pr)
		{	/* Here it will be necessary to throw off in TRACE */	return;		}
		pr->SndRead( bTrans, ov );
	}
	// end VS_StreamsRouter_Conference::SndRead

	inline void SndWrite( unsigned long bTrans, VS_Overlapped &ov )
	{
		const unsigned long   index = (const unsigned long)ov.field3;
		if (index >= maxParts)
		{	/* Here it will be necessary to throw off in TRACE */	return;		}
		VS_StreamsRouter_Participant   *pr = part[index];
		if (!pr)
		{	/* Here it will be necessary to throw off in TRACE */	return;		}
		pr->SndWrite( bTrans, ov );
	}
	// end VS_StreamsRouter_Conference::SndWrite

	inline void RcvRead( unsigned long bTrans, VS_Overlapped &ov )
	{
		const unsigned long   index = (const unsigned long)ov.field3;
		if (index >= maxParts)
		{	/* Here it will be necessary to throw off in TRACE */	return;		}
		VS_StreamsRouter_Participant   *pr = part[index];
		if (!pr)
		{	/* Here it will be necessary to throw off in TRACE */	return;		}
		pr->RcvRead( bTrans, ov );
	}
	// end VS_StreamsRouter_Conference::RcvRead

	inline void RcvWrite( unsigned long bTrans, VS_Overlapped &ov )
	{
		const unsigned long   index = (const unsigned long)ov.field3;
		if (index >= maxParts)
		{	/* Here it will be necessary to throw off in TRACE */	return;		}
		VS_StreamsRouter_Participant   *pr = part[index];
		if (!pr)
		{	/* Here it will be necessary to throw off in TRACE */	return;		}
		pr->RcvWrite( bTrans, ov );
	}
	// end VS_StreamsRouter_Conference::RcvWrite

	inline bool ProcessingPeriodic( void )
	{
		if (nParts) {	activeTm = sr->currTm;	return true;	}
		return (sr->currTm - activeTm) < maxSilence;
	}
	// end VS_StreamsRouter_Conference::ProcessingPeriodic

	inline void ProcessingWriteFrame( const unsigned long index )
	{
		if (index >= maxParts)
		{	/* Here it will be necessary to throw off in TRACE */	return;		}
		VS_StreamsRouter_Participant   *pr = part[index];
		if (!pr)
		{	/* Here it will be necessary to throw off in TRACE */	return;		}
		uint32_t tick_count;
		stream::Track track;
		std::unique_ptr<char[]> buffer;
		size_t size;
		switch (pr->sndBuffer->GetFrame(tick_count, track, buffer, size))
		{
		case stream::Buffer::Status::success:
		{
			stream::FrameHeader& writeHead = pr->writeHead;
			writeHead.length = size;
			writeHead.tick_count = tick_count;
			writeHead.track = track;
			writeHead.cksum = stream::GetFrameBodyChecksum(buffer.get(), size);
			pr->Write(buffer.get(), size);
			return;
		}
		case stream::Buffer::Status::non_fatal:
			return;
		default :	DeleteParticipant( index );		return;
	}	}
	// end VS_StreamsRouter_Conference::ProcessingWriteFrame

	inline void ProcessingReadFrame( const unsigned long index,
										const stream::FrameHeader *readHead, std::unique_ptr<char[]> readBuffer)
	{
		if (index >= maxParts)
		{	/* Here it will be necessary to throw off in TRACE */	return;		}
		VS_StreamsRouter_Participant   *pr = part[index];
		if (!pr)
		{	/* Here it will be necessary to throw off in TRACE */	return;		}
		if (!readHead)
		{	/* Here it will be necessary to throw off in TRACE */	return;		}

		m_fireFrameReceived(conferenceName, pr->participantName, readHead, readBuffer.get());

		const int   s_buffer = (const int)readHead->length;
		const stream::Track track = readHead->track;
		VS_StreamsRouter_Participant   *connPr = 0, *connPrNext = 0;
		unsigned long   i = 0, connectedIndex = ~0, connectedIndexNext = ~0;
		for (; i < pr->nReceiverConnected; ++i)
		{	connectedIndex = pr->receiverConnectedIndex[i];
			if (connectedIndex < maxParts)
			{	connPr = part[connectedIndex];
				if (connPr) {	++i;	break;	}
				else {	/* Here it will be necessary to throw off in TRACE */	}
			} else {	/* Here it will be necessary to throw off in TRACE */	}
			pr->RedressReceiver( i-- );
		}
		std::unique_ptr<char[]> buffer;
		for (; i < pr->nReceiverConnected; ++i)
		{	connectedIndexNext = pr->receiverConnectedIndex[i];
			if (connectedIndexNext < maxParts)
			{	connPrNext = part[connectedIndexNext];
				if (connPrNext)
				{	const unsigned char   *ftracks = connPr->receiverFTracks[index];
					if (!ftracks || ftracks[id(track)])
					{	if (!buffer && s_buffer)
						{
							buffer = vs::make_unique_default_init<char[]>(s_buffer);
							memcpy(buffer.get(), readBuffer.get(), s_buffer);
						}
						switch (connPr->sndBuffer->PutFrame(sr->currTm, readHead->track, std::move(buffer), s_buffer))
						{
						case stream::Buffer::Status::success:
							connPr->Write();
							connPr->statCalc.SndPutBuffer(*readHead, sr->currTm, connPr->sndBuffer);
							break;
						case stream::Buffer::Status::non_fatal:
							connPr->Write();
							break;
						default :	DeleteParticipant( connectedIndex );	--i;
					}	}
					connectedIndex = connectedIndexNext;	connPr = connPrNext;
					connectedIndexNext = ~0;	connPrNext = 0;		continue;
				} else {	/* Here it will be necessary to throw off in TRACE */	}
			} else {	/* Here it will be necessary to throw off in TRACE */	}
			pr->RedressReceiver( i-- );		connectedIndexNext = ~0;	connPrNext = 0;
		}
		if (connPr)
		{	const unsigned char   *ftracks = connPr->receiverFTracks[index];
			if (!ftracks || ftracks[id(track)])
			{	if (!buffer && s_buffer)
					buffer = std::move(readBuffer);
				switch (connPr->sndBuffer->PutFrame(sr->currTm, readHead->track, std::move(buffer), s_buffer))
				{
				case stream::Buffer::Status::success:
					connPr->Write();
					connPr->statCalc.SndPutBuffer(*readHead, sr->currTm, connPr->sndBuffer);
					break;
				case stream::Buffer::Status::non_fatal:
					connPr->Write();
					break;
				default :	DeleteParticipant( connectedIndex );
		}	}	}
	}
	// end VS_StreamsRouter_Conference::ProcessingReadFrame
	inline bool SetParticipantCaps(const char *partName, const void *caps_buf, const unsigned long buf_sz)
	{
		unsigned long part_index = GetParticipantIndex(partName);
		if(part_index >= maxParts)
			return false;
		sr->Cond_SetParticipantCaps(conferenceName,partName,caps_buf,buf_sz);
		return true;
	}
	inline void ProcessingStatistics( const unsigned long index )
	{
		if (index >= maxParts)
			return;
		VS_StreamsRouter_Participant   *pr = part[index];
		if (!pr)
			return;
		unsigned long   statSize = 0;
		for (unsigned long i = 0; i < pr->nReceiverConnected; ++i ) {
			const unsigned long   connectedIndex = pr->receiverConnectedIndex[i];
			if (connectedIndex >= maxParts)
				continue;
			VS_StreamsRouter_Participant   *connectedPr = part[connectedIndex];
			if (!connectedPr)
				continue;
			if (pr->senderConnectedFlag[connectedIndex])
				statSize += connectedPr->statCalc.GetSndStatisticsSize();
		}
		uint32_t   statIndex = 0;
		unsigned char   *statBuffer = 0;
		if (statSize) {
			statBuffer = (unsigned char *)malloc( (size_t)statSize );
			if (!statBuffer)
				return;
			unsigned long   stayedSize = statSize;
			for (unsigned long i = 0; i < pr->nReceiverConnected; ++i ) {
				const unsigned long   connectedIndex = pr->receiverConnectedIndex[i];
				if (connectedIndex >= maxParts)
					continue;
				VS_StreamsRouter_Participant   *connectedPr = part[connectedIndex];
				if (!connectedPr || !pr->senderConnectedFlag[connectedIndex])
					continue;
				statIndex += connectedPr->statCalc.FormSndStatistics(reinterpret_cast<stream::StreamStatistics*>(statBuffer + statIndex), stayedSize);
				stayedSize = statSize - statIndex;
				if (stayedSize < sizeof(stream::StreamStatistics))
					break;
			}
		}
		unsigned char* crb = 0;
		uint32_t crbs = 0;
		if (scrypter.IsValid() && statIndex) {
			crbs = statIndex*2+128;
			crb = (unsigned char*)malloc(statIndex*2+16);
			if (!scrypter.Encrypt(statBuffer, statIndex, crb, &crbs))
				crbs = 0;
		}
		const unsigned char * bf = crbs ? crb : statBuffer;
		unsigned long length = crbs ? crbs : statIndex;
		stream::FrameHeader& writeHead = pr->writeHead;
		writeHead.length = (unsigned short)length;
		writeHead.tick_count = sr->currTm;
		writeHead.track = stream::Track::old_command;
		writeHead.cksum = stream::GetFrameBodyChecksum(bf, length);
		pr->Write(bf, length);

		if (crb) free(crb);
		if (statBuffer)	free(statBuffer);
	}
	// end VS_StreamsRouter_Conference::ProcessingStatistics

	inline void FillMonitorStruct( VS_StreamsMonitor::SmReply::Conference &conference )
	{
		conference.type = SM_TYPE_PERIODIC_CONFERENCE;
		strncpy( conference.conferenceName, conferenceName, sizeof(conference.conferenceName) );
		conference.conferenceName[sizeof(conference.conferenceName) - 1] = 0;
		conference.createdDate = createdDate;
		conference.maxParticipants = maxParts;
		conference.nParticipants = nParts;
		conference.absenceMs = maxSilence;
	}
	// end VS_StreamsRouter_Conference::FillMonitorStruct

	inline void FillParticipantMonitorStruct( const unsigned long index,
								VS_StreamsMonitor::SmReply::Participant &participant )
	{
		VS_StreamsRouter_Participant   *pr = part[index];
		pr->FillMonitorStruct( participant );
		memset( participant.attachedParticipantName, 0, sizeof(participant.attachedParticipantName) );
/*
		const unsigned long   connectedIndex = pr->receiverConnectedIndex;
		if (connectedIndex < maxParts)
		{	VS_StreamsRouter_Participant   *connPr = part[connectedIndex];
			if (connPr)
			{	strncpy( participant.attachedParticipantName, connPr->participantName, sizeof(participant.attachedParticipantName) );
				participant.attachedParticipantName[sizeof(participant.attachedParticipantName) - 1] = 0;
		}	}
*/
	}
	// end VS_StreamsRouter_Conference::FillParticipantMonitorStruct
};
// end VS_StreamsRouter_Conference struct

//////////////////////////////////////////////////////////////////////////////////////////

const char   trHandlerNamePrefix[] = "Handler Of Streams Router. Endpoint: ";

struct VS_StreamsRouter_Implementation : public VS_StreamsRouter,
                                         public VS_StreamsRouter_ImplementationBase,
                                         public VS_StreamsRouter_SetConnection,
										 public VS_StreamsMonitor
{
	VS_StreamsRouter_Implementation( void )
		: isInit(false), tickMs(VS_SR_TICK_MS), maxConfs(0), nConfs(0), maxIndConfs(0)
		, conf(0), streamsHandler(0), ocp(0), icp(0), ocp_cond(0), icp_cond(0), mcp(0)
		, smRequest(0), smReply(0), smReadState(0), statePeriodic(0)
		, flagMcpConnect(0), flagMcpWrite(0), smReadSize(0)
		, indexConfCount(0), indexPartCount(0), acs(0)
		, hiocp(0), hthr(0), hthr_cond(0), MAX_ITERS(5), STAT_BUF(0x4000)
		, _sVideo(4000), _fVideo(4000),threadId(0)
	{
		Stop();
		buf = reinterpret_cast<stream::StreamStatistics*>(new char[STAT_BUF]);
		UpdateBitrateConfig();
	}
	// end VS_StreamsRouter_Implementation::VS_StreamsRouter_Implementation

	~VS_StreamsRouter_Implementation( void )
	{
		Stop();
		_conferences.clear(); delete[] buf; buf = 0;
	}
	// end VS_StreamsRouter_Implementation::~VS_StreamsRouter_Implementation

	vs::fast_recursive_mutex mtx;
	vs::fast_mutex mtx_read;
	bool   isInit;
	const unsigned long   tickMs;
	unsigned long   maxConfs, nConfs, maxIndConfs;
	VS_StreamsRouter_Conference   **conf;
	char   endpointName[VS_ACS_MAX_SIZE_ENDPOINT_NAME + 1];
	char   handlerName[VS_ACS_MAX_SIZE_HANDLER_NAME + 1];
	VS_StreamsHandler   *streamsHandler;
	VS_ConnectionMsg   *ocp, *icp, *ocp_cond, *icp_cond;	VS_ConnectionByte   *mcp;
	SmRequest   *smRequest;		SmReply   *smReply;
	unsigned   smReadState, statePeriodic;		bool   flagMcpConnect, flagMcpWrite;
	unsigned long   smReadSize, indexConfCount, indexPartCount;
	VS_AccessConnectionSystem   *acs;
	stream::ConferencesConditionsSplitter ccs;
	HANDLE   hiocp, hthr, hthr_cond;
	DWORD	threadId;



	// refactored from bitrate calculation defines
	const long MAX_ITERS;
	const long STAT_BUF;
	/* таймаут на начало/остановку видео */
	const long _sVideo;
	const long _fVideo;

	enum VS_SR_Cmd { vs_sr_unknown = 0, vs_sr_start_router,
		vs_sr_create_conference, vs_sr_set_mcc, vs_sr_remove_conference,
		vs_sr_add_participant, vs_sr_add_unconnected_participant, vs_sr_add_mcc,
		vs_sr_remove_participant, vs_sr_connect_participant,
		vs_sr_connect_participant_sender, vs_sr_connect_participant_receiver,
		vs_sr_set_participant_tracks, vs_sr_set_participant_sender_tracks,
		vs_sr_set_participant_receiver_tracks, vs_sr_disconnect_participant,
		vs_sr_disconnect_participant_concrete, vs_sr_disconnect_participant_sender,
		vs_sr_disconnect_participant_sender_concrete, vs_sr_disconnect_participant_receiver,
		vs_sr_disconnect_participant_receiver_concrete, vs_sr_set_connection, vs_sr_get_statistics, vs_sr_connect_to_frame_sink,
		vs_sr_set_participant_caps,
		vs_sr_systemload_participant, vs_sr_framesizemb_participant };
	union ControlInquiry
	{	struct CntrlInquiry
		{	VS_SR_Cmd   cmd;
		};	// end CntrlInquiry struct
		struct CreateConference
		{	VS_SR_Cmd   cmd;
			char   conferenceName[VS_STREAMS_MAX_SIZE_CONFERENCE_NAME + 1];
			char   sslKey[VS_STREAMS_MAX_SIZE_SSLKEY+1];
			unsigned long   maxParticipants;
			unsigned long   maxSilenceMs;
			bool   formLogs;
			int type;
		};	// end CreateConference struct
		struct RemoveConference
		{	VS_SR_Cmd   cmd;
			char   conferenceName[VS_STREAMS_MAX_SIZE_CONFERENCE_NAME + 1];
		};	// end RemoveConference struct
		struct AddParticipant
		{	VS_SR_Cmd   cmd;
			char   conferenceName[VS_STREAMS_MAX_SIZE_CONFERENCE_NAME + 1];
			char   participantName[VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME + 1];
			char   connectedParticipantName[VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME + 1];
			stream::Buffer* sndBuffer;
			bool   flagBufferDelete;
			bool   formLogs;
			unsigned long   maxSilenceMs;
			unsigned char   mtracks[256];
		};	// end AddParticipant struct
		struct AddUnconnectedParticipant
		{	VS_SR_Cmd   cmd;
			char   conferenceName[VS_STREAMS_MAX_SIZE_CONFERENCE_NAME + 1];
			char   participantName[VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME + 1];
			stream::Buffer* sndBuffer;
			bool   flagBufferDelete;
			bool   formLogs;
			unsigned long   maxSilenceMs;
		};	// end AddUnconnectedParticipant struct
		struct RemoveParticipant
		{	VS_SR_Cmd   cmd;
			char   conferenceName[VS_STREAMS_MAX_SIZE_CONFERENCE_NAME + 1];
			char   participantName[VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME + 1];
		};	// end RemoveParticipant struct
		struct ConnectParticipant
		{	VS_SR_Cmd   cmd;
			char   conferenceName[VS_STREAMS_MAX_SIZE_CONFERENCE_NAME + 1];
			char   participantName[VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME + 1];
			char   connectedParticipantName[VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME + 1];
			unsigned char   mtracks[256];
		};	// end ConnectParticipant struct
		struct DisconnectParticipant
		{	VS_SR_Cmd   cmd;
			char   conferenceName[VS_STREAMS_MAX_SIZE_CONFERENCE_NAME + 1];
			char   participantName[VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME + 1];
		};	// end DisconnectParticipant struct
		struct DisconnectParticipantConcrete
		{	VS_SR_Cmd   cmd;
			char   conferenceName[VS_STREAMS_MAX_SIZE_CONFERENCE_NAME + 1];
			char   participantName[VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME + 1];
			char   connectedParticipantName[VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME + 1];
		};	// end DisconnectParticipantConcrete struct
		struct SetConnection
		{	VS_SR_Cmd   cmd;
			char   conferenceName[VS_STREAMS_MAX_SIZE_CONFERENCE_NAME + 1];
			char   participantName[VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME + 1];
			char   connectedParticipantName[VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME + 1];
			unsigned   type;
			VS_ConnectionSock   *conn;
			unsigned char   mtracks[32];
		};	// end SetConnection struct
		struct ConnectToFrameSink
		{
			VS_SR_Cmd	cmd;
			ConnectToSinkParams	*params;
			char   conferenceName[VS_STREAMS_MAX_SIZE_CONFERENCE_NAME + 1];
		};
		struct SetParticipantCaps
		{
			VS_SR_Cmd				cmd;
			char					conferenceName[VS_STREAMS_MAX_SIZE_CONFERENCE_NAME + 1];
			char					participantName[VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME + 1];
			const void				*caps_buf;
			unsigned long			buf_sz;
		};
		struct SetSystemLoadParticipant
		{	VS_SR_Cmd   cmd;
			std::vector<stream::ParticipantLoadInfo> *load;
		};
		struct SetFrameSizeMBParticipant
		{	VS_SR_Cmd   cmd;
			char   conferenceName[VS_STREAMS_MAX_SIZE_CONFERENCE_NAME + 1];
			char   participantNameTo[VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME + 1];
			std::vector<stream::ParticipantFrameSizeInfo> *mb;
		};	// end SetFrameSizeMBParticipant struct

		CntrlInquiry					cntrlInquiry;
		CreateConference				createConference;
		RemoveConference				removeConference;
		AddParticipant					addParticipant;
		AddUnconnectedParticipant		addUnconnectedParticipant;
		RemoveParticipant				removeParticipant;
		ConnectParticipant				connectParticipant;
		DisconnectParticipant			disconnectParticipant;
		DisconnectParticipantConcrete	disconnectParticipantConcrete;
		SetConnection					setConnection;
		ConnectToFrameSink				connectToFrameSink;
		SetParticipantCaps				setPartiipantCaps;
		SetSystemLoadParticipant		setSystemLoadParticipant;
		SetFrameSizeMBParticipant		setFrameSizeMBParticipant;
	};	// end ControlInquiry struct
	union ControlResponse
	{	struct CntrlResponse
		{	VS_SR_Cmd   cmd;
			bool	res;
		};	// end CntrlResponse struct
		struct GetStatistics
		{	VS_SR_Cmd   cmd;
			bool	res;
			stream::RouterStatistics stat;
		};	// end GetStatistics struct
		CntrlResponse	cntrlResponse;
		GetStatistics	getStatistics;
	};	// end ControlResponse struct
	ControlInquiry   ci;
	ControlResponse   cr;

	inline void ResetToConnectMcp( void )
	{
		flagMcpConnect = flagMcpWrite = false;
		if (mcp)
		{	if (!mcp->IsRW())	DeleteConn( mcp );
			else {	mcp->SetOvReadFields( VS_SR_CONNECTION_DEAD_READ, (VS_ACS_Field)mcp );
					mcp->SetOvWriteFields( VS_SR_CONNECTION_DEAD_WRITE, (VS_ACS_Field)mcp );
					mcp->Disconnect();
			}
			mcp = 0;	}
		if (smRequest) {	delete smRequest;	smRequest = 0;		}
		if (smReply) {		delete smReply;		smReply = 0;	}
		bool   againFlag = false;
		auto   nameServer = g_tr_endpoint_name;
go_again:
		if (nameServer.empty())	return;
		VS_FilterPath(nameServer.begin(), nameServer.end(), nameServer.begin());
		char   *namePipe = (char *)malloc( 512 );
		if (!namePipe)	return;
		mcp = new VS_ConnectionByte;
		if (!mcp) {		free( (void *)namePipe );	return;		}
		sprintf( namePipe, "%s%s%s\\%s", VS_PipeDir, VS_Servers_PipeDir, nameServer.c_str(), VS_SrPrefixMonPipe );
		if (!mcp->Create( namePipe, vs_pipe_type_duplex ) || !mcp->SetIOCP( hiocp ))
		{	delete mcp;		mcp = 0;	free( (void *)namePipe );	return;		}
		free( (void *)namePipe );
		mcp->SetOvReadFields( (const VS_ACS_Field)VS_SR_MONITOR_CONNECT );
		if (!mcp->Connect())
		{
			if (mcp->State() != vs_pipe_state_connected || againFlag)
			{	delete mcp;		mcp = 0;	return;		}
			mcp->SetOvWriteFields( (const VS_ACS_Field)VS_SR_MONITOR_WRITE );
			mcp->SetOvReadFields( (const VS_ACS_Field)VS_SR_MONITOR_READ );
			smRequest = new SmRequest;
			if (!smRequest) {	delete mcp;		mcp = 0;	return;		}
			smReply = new SmReply;
			if (!smReply)
			{	delete smRequest;	smRequest = 0;	delete mcp;		mcp = 0;	return;		}
			if (!mcp->Read( (void *)&smRequest->type, sizeof(smRequest->type) ))
			{	delete smReply;	smReply = 0;	delete smRequest;	smRequest = 0;
				delete mcp;		mcp = 0;	againFlag = true;	goto go_again;	}
			smReadState = 0;	smReadSize = sizeof(smRequest->type);
			flagMcpConnect = true;
	}	}
	// end VS_StreamsRouter_Implementation::ResetToConnectMcp

	inline void WriteMcp( const void *buf, const unsigned long size )
	{
		if (!mcp->Write( buf, size )) {		ResetToConnectMcp();	return;		}
		flagMcpWrite = true;
	}
	// end VS_StreamsRouter_Implementation::WriteMcp

	inline unsigned long GetFreeConferenceIndex( void )
	{
		for (unsigned long i = 0; i < maxConfs; ++i)	if (!conf[i])	return i;
		return ~0;
	}
	// end VS_StreamsRouter_Implementation::GetConferenceIndex

	inline unsigned long GetConferenceIndex( const char *conferenceName )
	{
		for (unsigned long i = 0; i <= maxIndConfs; ++i)
			if (conf[i] && !strcmp( conf[i]->conferenceName, conferenceName ))	return i;
		return ~0;
	}
	// end VS_StreamsRouter_Implementation::GetConferenceIndex

	inline void DeleteConference( const unsigned long index )
	{
		delete conf[index];		conf[index] = 0;	--nConfs;
		if (index >= maxIndConfs)
		{	maxIndConfs = index;	while (maxIndConfs && !conf[--maxIndConfs]);	}
	}
	// end VS_StreamsRouter_Implementation::DeleteConference

	inline bool GoCreateConference( const unsigned long size )
	{
		if (size != sizeof(ci.createConference))	return false;
		const char *conferenceName = ci.createConference.conferenceName;
		const char * key = ci.createConference.sslKey;
		const unsigned long   maxParticipants = ci.createConference.maxParticipants;
		const unsigned long   maxSilenceMs = ci.createConference.maxSilenceMs;
		const bool   formLogs = ci.createConference.formLogs;
		const int type = ci.createConference.type;
		if (!*conferenceName || strlen( conferenceName ) > VS_STREAMS_MAX_SIZE_CONFERENCE_NAME
				|| !maxParticipants || maxParticipants > VS_SR_MAX_PARTICIPANTS
				|| maxSilenceMs < VS_SR_MIN_SILENCE_MS)		return false;
		cr.cntrlResponse.cmd = vs_sr_create_conference;		cr.cntrlResponse.res = false;
		if (GetConferenceIndex( conferenceName ) >= maxConfs)
		{	const unsigned long   index = GetFreeConferenceIndex();
			if (index < maxConfs)
			{	VS_StreamsRouter_Conference   *cf = new VS_StreamsRouter_Conference(this, index, conferenceName, key, maxParticipants, maxSilenceMs, type, formLogs);
				if (cf)
				{	if (cf->isValid)
					{	conf[index] = cf;	++nConfs;
						if (index > maxIndConfs)	maxIndConfs = index;
						cr.cntrlResponse.res = true;
						Cond_CreateConference( conferenceName );
					}
					else	delete cf;
		}	}	}
		return icp->Write( (const void *)&cr, sizeof(cr.cntrlResponse) );
	}
	// end VS_StreamsRouter_Implementation::GoCreateConference

	inline bool GoRemoveConference( const unsigned long size )
	{
		if (size != sizeof(ci.removeConference))	return false;
		const char   *conferenceName = ci.removeConference.conferenceName;
		if (!*conferenceName || strlen( conferenceName ) > VS_STREAMS_MAX_SIZE_CONFERENCE_NAME)
			return false;
		const unsigned long   index = GetConferenceIndex( conferenceName );
		if (index < maxConfs)	DeleteConference( index );
		return true;
	}
	// end VS_StreamsRouter_Implementation::GoRemoveConference

	inline bool GoAddParticipant( const unsigned long size )
	{
		if (size != sizeof(ci.addParticipant))	return false;
		const char   *conferenceName = ci.addParticipant.conferenceName,
						*participantName = ci.addParticipant.participantName,
						*connectedParticipantName = ci.addParticipant.connectedParticipantName;
		stream::Buffer* sndBuffer = ci.addParticipant.sndBuffer;
		const bool   flagBufferDelete = ci.addParticipant.flagBufferDelete,
						formLogs = ci.addParticipant.formLogs;
		const unsigned long   maxSilenceMs = ci.addParticipant.maxSilenceMs;
		const unsigned char   *mtracks = ci.addParticipant.mtracks;
		if (!*conferenceName || strlen( conferenceName ) > VS_STREAMS_MAX_SIZE_CONFERENCE_NAME
			|| !*participantName || strlen( participantName ) > VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME
			|| strlen( connectedParticipantName ) > VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME
			|| !sndBuffer || maxSilenceMs < VS_SR_MIN_SILENCE_MS)	return false;
		cr.cntrlResponse.cmd = vs_sr_add_participant;	cr.cntrlResponse.res = false;
		const unsigned long   index = GetConferenceIndex( conferenceName );
		if (index < maxConfs)
		{	VS_StreamsRouter_Conference   *cf = conf[index];
			if (cf->AddParticipant( participantName, sndBuffer, flagBufferDelete, formLogs, maxSilenceMs ))
			{	if (cf->ConnectParticipant( participantName, connectedParticipantName )
						&& cf->SetParticipantTracks( participantName, connectedParticipantName, mtracks ))
					cr.cntrlResponse.res = true;
				else	cf->DeleteParticipant( participantName );
		}	}
		return icp->Write( (const void *)&cr, sizeof(cr.cntrlResponse) );
	}
	// end VS_StreamsRouter_Implementation::GoAddParticipant

	inline bool GoAddUnconnectedParticipant( const unsigned long size )
	{
		if (size != sizeof(ci.addUnconnectedParticipant))	return false;
		const char   *conferenceName = ci.addUnconnectedParticipant.conferenceName,
						*participantName = ci.addUnconnectedParticipant.participantName;
		stream::Buffer* sndBuffer = ci.addUnconnectedParticipant.sndBuffer;
		const bool   flagBufferDelete = ci.addUnconnectedParticipant.flagBufferDelete,
						formLogs = ci.addUnconnectedParticipant.formLogs;
		const unsigned long   maxSilenceMs = ci.addUnconnectedParticipant.maxSilenceMs;
		if (!*conferenceName || strlen( conferenceName ) > VS_STREAMS_MAX_SIZE_CONFERENCE_NAME
			|| !*participantName || strlen( participantName ) > VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME
			|| !sndBuffer || maxSilenceMs < VS_SR_MIN_SILENCE_MS)	return false;
		cr.cntrlResponse.cmd = vs_sr_add_unconnected_participant;	cr.cntrlResponse.res = false;
		const unsigned long   index = GetConferenceIndex( conferenceName );
		if (index < maxConfs)	cr.cntrlResponse.res = conf[index]->AddParticipant( participantName, sndBuffer, flagBufferDelete, formLogs, maxSilenceMs );
		return icp->Write( (const void *)&cr, sizeof(cr.cntrlResponse) );
	}
	// end VS_StreamsRouter_Implementation::GoAddUnconnectedParticipant

	inline bool GoRemoveParticipant( const unsigned long size )
	{
		if (size != sizeof(ci.removeParticipant))	return false;
		const char   *conferenceName = ci.removeParticipant.conferenceName,
						*participantName = ci.removeParticipant.participantName;
		if (!*conferenceName || strlen( conferenceName ) > VS_STREAMS_MAX_SIZE_CONFERENCE_NAME
				|| !*participantName || strlen( participantName ) > VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME)
			return false;
		const unsigned long   index = GetConferenceIndex( conferenceName );
		if (index < maxConfs)	conf[index]->DeleteParticipant( participantName );
		return true;
	}
	// end VS_StreamsRouter_Implementation::GoRemoveParticipant

	inline bool GoConnectParticipant( const unsigned long size )
	{
		if (size != sizeof(ci.connectParticipant))		return false;
		const char   *conferenceName = ci.connectParticipant.conferenceName,
						*participantName = ci.connectParticipant.participantName,
						*connectedParticipantName = ci.connectParticipant.connectedParticipantName;
		const unsigned char   *mtracks = ci.connectParticipant.mtracks;
		if (!*conferenceName || strlen( conferenceName ) > VS_STREAMS_MAX_SIZE_CONFERENCE_NAME
				|| !*participantName || strlen( participantName ) > VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME
				|| !*connectedParticipantName || strlen( connectedParticipantName ) > VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME)
			return false;
		cr.cntrlResponse.cmd = vs_sr_connect_participant;	cr.cntrlResponse.res = false;
		const unsigned long   index = GetConferenceIndex( conferenceName );
		if (index < maxConfs)
		{	VS_StreamsRouter_Conference   *cf = conf[index];
			if (cf->ConnectParticipant( participantName, connectedParticipantName ))
			{	if (cf->SetParticipantTracks( participantName, connectedParticipantName, mtracks ))
					cr.cntrlResponse.res = true;
				else	cf->DisconnectParticipant( participantName, connectedParticipantName );
		}	}
		return icp->Write( (const void *)&cr, sizeof(cr.cntrlResponse) );
	}
	// end VS_StreamsRouter_Implementation::GoConnectParticipant

	inline bool GoConnectParticipantSender( const unsigned long size )
	{
		if (size != sizeof(ci.connectParticipant))		return false;
		const char   *conferenceName = ci.connectParticipant.conferenceName,
						*participantName = ci.connectParticipant.participantName,
						*connectedParticipantName = ci.connectParticipant.connectedParticipantName;
		const unsigned char   *mtracks = ci.connectParticipant.mtracks;
		if (!*conferenceName || strlen( conferenceName ) > VS_STREAMS_MAX_SIZE_CONFERENCE_NAME
				|| !*participantName || strlen( participantName ) > VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME
				|| !*connectedParticipantName || strlen( connectedParticipantName ) > VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME)
			return false;
		cr.cntrlResponse.cmd = vs_sr_connect_participant_sender;	cr.cntrlResponse.res = false;
		const unsigned long   index = GetConferenceIndex( conferenceName );
		if (index < maxConfs)
		{	VS_StreamsRouter_Conference   *cf = conf[index];
			if (cf->ConnectParticipantSender( participantName, connectedParticipantName ))
			{	if (cf->SetParticipantSenderTracks( participantName, connectedParticipantName, mtracks ))
					cr.cntrlResponse.res = true;
				else	cf->DisconnectParticipantSender( participantName, connectedParticipantName );
		}	}
		return icp->Write( (const void *)&cr, sizeof(cr.cntrlResponse) );
	}
	// end VS_StreamsRouter_Implementation::GoConnectParticipantSender

	inline bool GoConnectParticipantReceiver( const unsigned long size )
	{
		if (size != sizeof(ci.connectParticipant))		return false;
		const char   *conferenceName = ci.connectParticipant.conferenceName,
						*participantName = ci.connectParticipant.participantName,
						*connectedParticipantName = ci.connectParticipant.connectedParticipantName;
		const unsigned char   *mtracks = ci.connectParticipant.mtracks;
		if (!*conferenceName || strlen( conferenceName ) > VS_STREAMS_MAX_SIZE_CONFERENCE_NAME
				|| !*participantName || strlen( participantName ) > VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME
				|| !*connectedParticipantName || strlen( connectedParticipantName ) > VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME)
			return false;
		cr.cntrlResponse.cmd = vs_sr_connect_participant_receiver;	cr.cntrlResponse.res = false;
		const unsigned long   index = GetConferenceIndex( conferenceName );
		if (index < maxConfs)
		{	VS_StreamsRouter_Conference   *cf = conf[index];
			if (cf->ConnectParticipantReceiver( participantName, connectedParticipantName ))
			{	if (cf->SetParticipantReceiverTracks( participantName, connectedParticipantName, mtracks ))
					cr.cntrlResponse.res = true;
				else	cf->DisconnectParticipantReceiver( participantName, connectedParticipantName );
		}	}
		return icp->Write( (const void *)&cr, sizeof(cr.cntrlResponse) );
	}
	// end VS_StreamsRouter_Implementation::GoConnectParticipantReceiver

	inline bool GoSetParticipantTracks( const unsigned long size )
	{
		if (size != sizeof(ci.connectParticipant))		return false;
		const char   *conferenceName = ci.connectParticipant.conferenceName,
						*participantName = ci.connectParticipant.participantName,
						*connectedParticipantName = ci.connectParticipant.connectedParticipantName;
		const unsigned char   *mtracks = ci.connectParticipant.mtracks;
		if (!*conferenceName || strlen( conferenceName ) > VS_STREAMS_MAX_SIZE_CONFERENCE_NAME
				|| !*participantName || strlen( participantName ) > VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME
				|| !*connectedParticipantName || strlen( connectedParticipantName ) > VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME)
			return false;
		cr.cntrlResponse.cmd = vs_sr_set_participant_tracks;	cr.cntrlResponse.res = false;
		const unsigned long   index = GetConferenceIndex( conferenceName );
		if (index < maxConfs)
		{	VS_StreamsRouter_Conference   *cf = conf[index];
			if (cf->IsConnectParticipant( participantName, connectedParticipantName ))
				cr.cntrlResponse.res = cf->SetParticipantTracks( participantName, connectedParticipantName, mtracks );
		}
		return icp->Write( (const void *)&cr, sizeof(cr.cntrlResponse) );
	}
	// end VS_StreamsRouter_Implementation::GoSetParticipantTracks

	inline bool GoSetParticipantSenderTracks( const unsigned long size )
	{
		if (size != sizeof(ci.connectParticipant))		return false;
		const char   *conferenceName = ci.connectParticipant.conferenceName,
						*participantName = ci.connectParticipant.participantName,
						*connectedParticipantName = ci.connectParticipant.connectedParticipantName;
		const unsigned char   *mtracks = ci.connectParticipant.mtracks;
		if (!*conferenceName || strlen( conferenceName ) > VS_STREAMS_MAX_SIZE_CONFERENCE_NAME
				|| !*participantName || strlen( participantName ) > VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME
				|| !*connectedParticipantName || strlen( connectedParticipantName ) > VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME)
			return false;
		cr.cntrlResponse.cmd = vs_sr_set_participant_sender_tracks;		cr.cntrlResponse.res = false;
		const unsigned long   index = GetConferenceIndex( conferenceName );
		if (index < maxConfs)
		{	VS_StreamsRouter_Conference   *cf = conf[index];
			if (cf->IsConnectParticipantSender( participantName, connectedParticipantName ))
				cr.cntrlResponse.res = cf->SetParticipantSenderTracks( participantName, connectedParticipantName, mtracks );
		}
		return icp->Write( (const void *)&cr, sizeof(cr.cntrlResponse) );
	}
	// end VS_StreamsRouter_Implementation::GoSetParticipantSenderTracks

	inline bool GoSetParticipantReceiverTracks( const unsigned long size )
	{
		if (size != sizeof(ci.connectParticipant))		return false;
		const char   *conferenceName = ci.connectParticipant.conferenceName,
						*participantName = ci.connectParticipant.participantName,
						*connectedParticipantName = ci.connectParticipant.connectedParticipantName;
		const unsigned char   *mtracks = ci.connectParticipant.mtracks;
		if (!*conferenceName || strlen( conferenceName ) > VS_STREAMS_MAX_SIZE_CONFERENCE_NAME
				|| !*participantName || strlen( participantName ) > VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME
				|| !*connectedParticipantName || strlen( connectedParticipantName ) > VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME)
			return false;
		cr.cntrlResponse.cmd = vs_sr_set_participant_receiver_tracks;	cr.cntrlResponse.res = false;
		const unsigned long   index = GetConferenceIndex( conferenceName );
		if (index < maxConfs)
		{	VS_StreamsRouter_Conference   *cf = conf[index];
			if (cf->IsConnectParticipantReceiver( participantName, connectedParticipantName ))
				cr.cntrlResponse.res = cf->SetParticipantReceiverTracks( participantName, connectedParticipantName, mtracks );
		}
		return icp->Write( (const void *)&cr, sizeof(cr.cntrlResponse) );
	}
	// end VS_StreamsRouter_Implementation::GoSetParticipantReceiverTracks

	inline bool GoDisconnectParticipant( const unsigned long size )
	{
		if (size != sizeof(ci.disconnectParticipant))	return false;
		const char   *conferenceName = ci.disconnectParticipant.conferenceName,
						*participantName = ci.disconnectParticipant.participantName;
		if (!*conferenceName || strlen( conferenceName ) > VS_STREAMS_MAX_SIZE_CONFERENCE_NAME
				|| !*participantName || strlen( participantName ) > VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME)
			return false;
		const unsigned long   index = GetConferenceIndex( conferenceName );
		if (index < maxConfs)	conf[index]->DisconnectParticipant( participantName );
		return true;
	}
	// end VS_StreamsRouter_Implementation::GoDisconnectParticipant

	inline bool GoDisconnectParticipantConcrete( const unsigned long size )
	{
		if (size != sizeof(ci.disconnectParticipantConcrete))	return false;
		const char   *conferenceName = ci.disconnectParticipantConcrete.conferenceName,
						*participantName = ci.disconnectParticipantConcrete.participantName,
						*connectedParticipantName = ci.disconnectParticipantConcrete.connectedParticipantName;
		if (!*conferenceName || strlen( conferenceName ) > VS_STREAMS_MAX_SIZE_CONFERENCE_NAME
				|| !*participantName || strlen( participantName ) > VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME
				|| !*connectedParticipantName || strlen( connectedParticipantName ) > VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME)
			return false;
		const unsigned long   index = GetConferenceIndex( conferenceName );
		if (index < maxConfs)	conf[index]->DisconnectParticipant( participantName, connectedParticipantName );
		return true;
	}
	// end VS_StreamsRouter_Implementation::GoDisconnectParticipantConcrete

	inline bool GoDisconnectParticipantSender( const unsigned long size )
	{
		if (size != sizeof(ci.disconnectParticipant))	return false;
		const char   *conferenceName = ci.disconnectParticipant.conferenceName,
						*participantName = ci.disconnectParticipant.participantName;
		if (!*conferenceName || strlen( conferenceName ) > VS_STREAMS_MAX_SIZE_CONFERENCE_NAME
				|| !*participantName || strlen( participantName ) > VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME)
			return false;
		const unsigned long   index = GetConferenceIndex( conferenceName );
		if (index < maxConfs)	conf[index]->DisconnectParticipantSender( participantName );
		return true;
	}
	// end VS_StreamsRouter_Implementation::GoDisconnectParticipantSender

	inline bool GoDisconnectParticipantSenderConcrete( const unsigned long size )
	{
		if (size != sizeof(ci.disconnectParticipantConcrete))	return false;
		const char   *conferenceName = ci.disconnectParticipantConcrete.conferenceName,
						*participantName = ci.disconnectParticipantConcrete.participantName,
						*connectedParticipantName = ci.disconnectParticipantConcrete.connectedParticipantName;
		if (!*conferenceName || strlen( conferenceName ) > VS_STREAMS_MAX_SIZE_CONFERENCE_NAME
				|| !*participantName || strlen( participantName ) > VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME
				|| !*connectedParticipantName || strlen( connectedParticipantName ) > VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME)
			return false;
		const unsigned long   index = GetConferenceIndex( conferenceName );
		if (index < maxConfs)	conf[index]->DisconnectParticipantSender( participantName, connectedParticipantName );
		return true;
	}
	// end VS_StreamsRouter_Implementation::GoDisconnectParticipantSenderConcrete

	inline bool GoDisconnectParticipantReceiver( const unsigned long size )
	{
		if (size != sizeof(ci.disconnectParticipant))	return false;
		const char   *conferenceName = ci.disconnectParticipant.conferenceName,
						*participantName = ci.disconnectParticipant.participantName;
		if (!*conferenceName || strlen( conferenceName ) > VS_STREAMS_MAX_SIZE_CONFERENCE_NAME
				|| !*participantName || strlen( participantName ) > VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME)
			return false;
		const unsigned long   index = GetConferenceIndex( conferenceName );
		if (index < maxConfs)	conf[index]->DisconnectParticipantReceiver( participantName );
		return true;
	}
	// end VS_StreamsRouter_Implementation::GoDisconnectParticipantReceiver

	inline bool GoDisconnectParticipantReceiverConcrete( const unsigned long size )
	{
		if (size != sizeof(ci.disconnectParticipantConcrete))	return false;
		const char   *conferenceName = ci.disconnectParticipantConcrete.conferenceName,
						*participantName = ci.disconnectParticipantConcrete.participantName,
						*connectedParticipantName = ci.disconnectParticipantConcrete.connectedParticipantName;
		if (!*conferenceName || strlen( conferenceName ) > VS_STREAMS_MAX_SIZE_CONFERENCE_NAME
				|| !*participantName || strlen( participantName ) > VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME)
			return false;
		const unsigned long   index = GetConferenceIndex( conferenceName );
		if (index < maxConfs)	conf[index]->DisconnectParticipantReceiver( participantName, connectedParticipantName );
		return true;
	}
	// end VS_StreamsRouter_Implementation::GoDisconnectParticipantReceiverConcrete

	inline bool GoSetConnection( const unsigned long size )
	{
		if (size != sizeof(ci.setConnection))	return false;
		const char   *conferenceName = ci.setConnection.conferenceName,
					*participantName = ci.setConnection.participantName,
					*connectedParticipantName = ci.setConnection.connectedParticipantName;
		const unsigned   type = ci.setConnection.type;
		VS_ConnectionSock   *conn = ci.setConnection.conn;
		const unsigned char   *mtracks = ci.setConnection.mtracks;
		if (!*conferenceName || strlen( conferenceName ) > VS_STREAMS_MAX_SIZE_CONFERENCE_NAME
				|| !*participantName || strlen( participantName ) > VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME
				|| strlen( connectedParticipantName ) > VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME
				|| (type != VS_ACS_LIB_SENDER && type != VS_ACS_LIB_RECEIVER) || !conn)
			return false;
		const unsigned long   index = GetConferenceIndex( conferenceName );
		if (index >= maxConfs)		DeleteConn( conn );
		else {	conn->SetOvFildIOCP( (const void *)hiocp );
				conf[index]->SetConnection( participantName, type, conn, mtracks );	}
		return true;
	}
	// end VS_StreamsRouter_Implementation::GoSetConnection

	inline bool GoGetStatisticsAct(stream::RouterStatistics& stat)
	{
		const unsigned __int64   diffSendBytes = ( sendBytes - lastSendBytes ) * 1000;
		lastSendBytes = sendBytes;
		const unsigned __int64   diffReceiveBytes = ( receiveBytes - lastReceiveBytes ) * 1000;
		lastReceiveBytes = receiveBytes;
		const unsigned long   diffTm = currTm - lastBitrateTm, addDiffTm = ( diffTm + 1 ) / 2;
		lastBitrateTm = currTm;
		unsigned long   streams = 0;
		for (unsigned long i = 0; i <= maxIndConfs; ++i)
			if (conf[i])	streams += conf[i]->nParts * 2;
		stat.streams = streams;
		stat.out_bytes = out_bytes += lastSendBytes;
		stat.in_bytes = in_bytes += lastReceiveBytes;
		stat.out_byterate = diffTm ? (float)(( diffSendBytes + addDiffTm ) / diffTm ) : 0;
		stat.in_byterate = diffTm ? (float)(( diffReceiveBytes + addDiffTm ) / diffTm ) : 0;
		return true;
	}
	// end VS_StreamsRouter_Implementation::GoGetStatisticsAct

	inline bool GoGetStatistics( const unsigned long size )
	{
		if (size != sizeof(ci.cntrlInquiry))	return false;
		cr.getStatistics.cmd = vs_sr_get_statistics;
		cr.getStatistics.res = GoGetStatisticsAct( cr.getStatistics.stat );
		return icp->Write( (const void *)&cr, sizeof(cr.getStatistics) );
	}
	// end VS_StreamsRouter_Implementation::GoGetStatistics

	inline bool GoConnectToFrameSink(const unsigned long size)
	{
		if(size != sizeof(ci.connectToFrameSink) || !ci.connectToFrameSink.params)
			return false;
		const char *conf_name = ci.connectToFrameSink.conferenceName;
		unsigned long index = GetConferenceIndex(conf_name);
		cr.cntrlResponse.cmd = vs_sr_connect_to_frame_sink;
		cr.cntrlResponse.res = false;
		if(index!=-1)
		{
			ci.connectToFrameSink.params->res_conn = conf[index]->m_fireFrameReceived.connect(ci.connectToFrameSink.params->slot);
			cr.cntrlResponse.res = true;
		}
		return icp->Write( (const void *)&cr, sizeof(cr.cntrlResponse) );
	}

	inline bool GoSetParticipantCaps(const unsigned long size)
	{
		if(size != sizeof(ci.setPartiipantCaps))
			return false;
		cr.cntrlResponse.cmd = vs_sr_set_participant_caps;
		cr.cntrlResponse.res = false;
		unsigned long confIndex = GetConferenceIndex(ci.setPartiipantCaps.conferenceName);
		if( confIndex < maxConfs )
			cr.cntrlResponse.res = conf[confIndex]->SetParticipantCaps(ci.setPartiipantCaps.participantName,ci.setPartiipantCaps.caps_buf,ci.setPartiipantCaps.buf_sz);
		return icp->Write( (const void *)&cr, sizeof(cr.cntrlResponse) );
	}
	inline bool GoSetSystemLoadParticipant( const unsigned long size )
	{
		if (size != sizeof(ci.setSystemLoadParticipant))	return false;
		for (auto &it : *ci.setSystemLoadParticipant.load)
			VS_MainSVCStatistics::UpdateSystemLoad(it.conference_name.c_str(), it.participant_name.c_str(), it.load);
		delete ci.setSystemLoadParticipant.load;
		return true;
	}
	// end VS_StreamsRouter_Implementation::GoSetSystemLoadParticipant

	inline bool GoSetFrameSizeMBParticipant( const unsigned long size )
	{
		if (size != sizeof(ci.setFrameSizeMBParticipant))	return false;
		const char *conferenceName = ci.setFrameSizeMBParticipant.conferenceName,
			*participantNameTo = ci.setFrameSizeMBParticipant.participantNameTo;
		VS_MainSVCStatistics::UpdateFrameSizeMB(conferenceName, participantNameTo, *ci.setFrameSizeMBParticipant.mb);
		delete ci.setFrameSizeMBParticipant.mb;
		return true;
	}
	// end VS_StreamsRouter_Implementation::GoSetFrameSizeMBParticipant



	inline bool GoControlInquiry( const unsigned long size )
	{
		switch (ci.cntrlInquiry.cmd)
		{
		case vs_sr_create_conference :
			if (!GoCreateConference( size ))	return false;						break;
		case vs_sr_remove_conference :
			if (!GoRemoveConference( size ))	return false;						break;
		case vs_sr_add_participant :
			if (!GoAddParticipant( size ))		return false;						break;
		case vs_sr_add_unconnected_participant :
			if (!GoAddUnconnectedParticipant( size ))	return false;				break;
		case vs_sr_remove_participant :
			if (!GoRemoveParticipant( size ))	return false;						break;
		case vs_sr_connect_participant :
			if (!GoConnectParticipant( size ))		return false;					break;
		case vs_sr_connect_participant_sender :
			if (!GoConnectParticipantSender( size ))	return false;				break;
		case vs_sr_connect_participant_receiver :
			if (!GoConnectParticipantReceiver( size ))		return false;			break;
		case vs_sr_set_participant_tracks :
			if (!GoSetParticipantTracks( size ))	return false;					break;
		case vs_sr_set_participant_sender_tracks :
			if (!GoSetParticipantSenderTracks( size ))	return false;				break;
		case vs_sr_set_participant_receiver_tracks :
			if (!GoSetParticipantReceiverTracks( size ))	return false;			break;
		case vs_sr_disconnect_participant :
			if (!GoDisconnectParticipant( size ))	return false;					break;
		case vs_sr_disconnect_participant_concrete :
			if (!GoDisconnectParticipantConcrete( size ))	return false;			break;
		case vs_sr_disconnect_participant_sender :
			if (!GoDisconnectParticipantSender( size ))		return false;			break;
		case vs_sr_disconnect_participant_sender_concrete :
			if (!GoDisconnectParticipantSenderConcrete( size ))		return false;	break;
		case vs_sr_disconnect_participant_receiver :
			if (!GoDisconnectParticipantReceiver( size ))	return false;			break;
		case vs_sr_disconnect_participant_receiver_concrete :
			if (!GoDisconnectParticipantReceiverConcrete( size ))	return false;	break;
		case vs_sr_set_connection :
			if (!GoSetConnection( size ))	return false;							break;
		case vs_sr_get_statistics :
			if (!GoGetStatistics( size ))	return false;							break;
		case vs_sr_connect_to_frame_sink :
			if(!GoConnectToFrameSink(size)) return false;							break;
		case vs_sr_set_participant_caps:
			if(!GoSetParticipantCaps( size )) return false;							break;
		case vs_sr_systemload_participant:
			if (!GoSetSystemLoadParticipant( size ))	return false;				break;
		case vs_sr_framesizemb_participant:
			if (!GoSetFrameSizeMBParticipant( size ))	return false;				break;
		default :	return false;
		}
		memset( (void *)&ci, 0, sizeof(ci) );
		return !icp->IsWrite() ? icp->Read( (void *)&ci, sizeof(ControlInquiry) ) : true;
	}
	// end VS_StreamsRouter_Implementation::GoControlInquiry

	inline void GoMonitorInquiry( const unsigned long size )
	{
		if (size != smReadSize)
		{
			ResetToConnectMcp();	return;
		}
		switch (smRequest->type)
		{
		case SM_TYPE_UNKNOW :	break;
		default:	ResetToConnectMcp();
	}	}
	// end VS_TransportRouter_Implementation::GoMonitorInquiry

	inline bool GoControlWrite( unsigned long bTrans, VS_Overlapped &ov )
	{
		if (ov.error)	return false;
		if (icp->SetWriteResult( bTrans, &ov ) < 0)		return false;
		return icp->Read( (void *)&ci, sizeof(ci) );
	}
	// end VS_StreamsRouter_Implementation::GoControlWrite

	inline bool GoControlRead( unsigned long bTrans, VS_Overlapped &ov )
	{
		if (ov.error)	return false;
		int   ret = icp->SetReadResult( bTrans, &ov, 0, true );
		if (ret < (int)sizeof(ci.cntrlInquiry.cmd))		return false;
		return GoControlInquiry( (const unsigned long)ret );
	}
	// end VS_StreamsRouter_Implementation::GoControlRead

	inline void GoSndConnectionWrite( unsigned long bTrans, VS_Overlapped &ov )
	{
		const unsigned long   index = (const unsigned long)ov.field2;
		if (index >= maxConfs)
		{	/* Here it will be necessary to throw off in TRACE */	return;		}
		VS_StreamsRouter_Conference   *cf = conf[index];
		if (!cf)
		{	/* Here it will be necessary to throw off in TRACE */	return;		}
		cf->SndWrite( bTrans, ov );
	}
	// end VS_StreamsRouter_Implementation::GoSndConnectionWrite

	inline void GoSndConnectionRead( unsigned long bTrans, VS_Overlapped &ov )
	{
		const unsigned long   index = (const unsigned long)ov.field2;
		if (index >= maxConfs)
		{	/* Here it will be necessary to throw off in TRACE */	return;		}
		VS_StreamsRouter_Conference   *cf = conf[index];
		if (!cf)
		{	/* Here it will be necessary to throw off in TRACE */	return;		}
		cf->SndRead( bTrans, ov );
	}
	// end VS_StreamsRouter_Implementation::GoSndConnectionRead

	inline void GoRcvConnectionWrite( unsigned long bTrans, VS_Overlapped &ov )
	{
		const unsigned long   index = (const unsigned long)ov.field2;
		if (index >= maxConfs)
		{	/* Here it will be necessary to throw off in TRACE */	return;		}
		VS_StreamsRouter_Conference   *cf = conf[index];
		if (!cf)
		{	/* Here it will be necessary to throw off in TRACE */	return;		}
		cf->RcvWrite( bTrans, ov );
	}
	// end VS_StreamsRouter_Implementation::GoRcvConnectionWrite

	inline void GoRcvConnectionRead( unsigned long bTrans, VS_Overlapped &ov )
	{
		const unsigned long   index = (const unsigned long)ov.field2;
		if (index >= maxConfs)
		{	/* Here it will be necessary to throw off in TRACE */	return;		}
		VS_StreamsRouter_Conference   *cf = conf[index];
		if (!cf)
		{	/* Here it will be necessary to throw off in TRACE */	return;		}
		cf->RcvRead( bTrans, ov );
	}
	// end VS_StreamsRouter_Implementation::GoRcvConnectionRead

	inline void GoConnectionDeadWrite( unsigned long bTrans, VS_Overlapped &ov )
	{
		VS_ConnectionSock   *conn = (VS_ConnectionSock *)ov.field2;
		if (!conn)
		{
			return;
		}
		conn->SetWriteResult( bTrans, &ov );
		if (!conn->IsRead())
		{
			DeleteConn( conn , true );
		}
	}
	// end VS_StreamsRouter_Implementation::GoConnectionWrite

	inline void GoConnectionDeadRead( unsigned long bTrans, VS_Overlapped &ov )
	{
		VS_ConnectionSock   *conn = (VS_ConnectionSock *)ov.field2;
		if (!conn)
		{
			return;
		}
		conn->SetReadResult( bTrans, &ov );
		if (!conn->IsWrite())
		{
			DeleteConn( conn , true );
		}
	}
	// end VS_StreamsRouter_Implementation::GoConnectionRead

	inline void GoMonitorWrite( unsigned long bTrans, VS_Overlapped &ov )
	{
		if (mcp->SetWriteResult( bTrans, &ov ) < 0)		ResetToConnectMcp();
		flagMcpWrite = false;
	}
	// end VS_StreamsRouter_Implementation::GoMonitorWrite

	inline void GoMonitorRead( unsigned long bTrans, VS_Overlapped &ov )
	{
		GoMonitorInquiry( (const unsigned long)mcp->SetReadResult( bTrans, &ov ));
	}
	// end VS_StreamsRouter_Implementation::GoMonitorRead

	inline void GoMonitorConnect( unsigned long bTrans, VS_Overlapped &ov )
	{
		if (mcp->SetConnectResult( bTrans, &ov ))
		{
			mcp->SetOvWriteFields( (const VS_ACS_Field)VS_SR_MONITOR_WRITE );
			mcp->SetOvReadFields( (const VS_ACS_Field)VS_SR_MONITOR_READ );
			smRequest = new SmRequest;
			if (!smRequest) {	ResetToConnectMcp();	return;		}
			smReply = new SmReply;
			if (!smReply) {		ResetToConnectMcp();	return;		}
			if (!mcp->Read( (void *)&smRequest->type, sizeof(smRequest->type) ))
			{	ResetToConnectMcp();	return;		}
			smReadState = 0;	smReadSize = sizeof(smRequest->type);
			flagMcpConnect = true;
	}	}
	// end VS_StreamsRouter_Implementation::GoMonitorConnect

	inline bool RunHandle( unsigned long bTrans, VS_Overlapped &ov )
	{
		switch ((unsigned long)ov.field1)
		{
		case VS_SR_SND_CONNECTION_WRITE  : GoSndConnectionWrite( bTrans, ov );	return true;
		case VS_SR_SND_CONNECTION_READ   : GoSndConnectionRead( bTrans, ov );	return true;
		case VS_SR_RCV_CONNECTION_WRITE  : GoRcvConnectionWrite( bTrans, ov );	return true;
		case VS_SR_RCV_CONNECTION_READ   : GoRcvConnectionRead( bTrans, ov );	return true;
		case VS_SR_CONNECTION_DEAD_WRITE : GoConnectionDeadWrite( bTrans, ov );	return true;
		case VS_SR_CONNECTION_DEAD_READ  : GoConnectionDeadRead( bTrans, ov );	return true;
		case VS_SR_CONTROL_WRITE         : return GoControlWrite( bTrans, ov );
		case VS_SR_CONTROL_READ          : return GoControlRead( bTrans, ov );
		case VS_SR_MONITOR_WRITE         : GoMonitorWrite( bTrans, ov );        return true;
		case VS_SR_MONITOR_READ          : GoMonitorRead( bTrans, ov );         return true;
		case VS_SR_MONITOR_CONNECT       : GoMonitorConnect( bTrans, ov );      return true;
		default :	/* Here it will be necessary to throw off in TRACE */		return true;
	}	}
	// end VS_StreamsRouter_Implementation::RunHandle

	/* d78 структура логического participant'a, по которому собирается статистика */
	struct logpart {
		logpart() {};
		logpart(const char *pn) {
			participantName = pn; skip = -9; checked = true; video = false;
			allFramesBuffer = allWriteBytesBand = allWriteFramesBand = 0;
			_calc_allFramesBuffer = _calc_allWriteBytesBand = _calc_allWriteFramesBand = 0;
			iterates = 0; lastVideoStartTime = lastVideoTimeStamp = 0;
		}
		virtual ~logpart() {
		}
		void __check(const bool check) {
			checked = check;
		}
		std::string participantName;	// имя логического participant'а
		signed long skip;				// счетчик пропусков (для статистики)
		signed long allFramesBuffer;    // статистика participant'a
		signed long allWriteBytesBand;  // статистика participant'a
		signed long allWriteFramesBand; // статистика participant'a
		signed long iterates;			// текущее количество наборов статистики
		bool checked;					// используется как флаг для удаления.
		bool video;						// указывает на присутствие видеотрека
		signed long long lastVideoStartTime;	// указывает на время начала передачи видео
		signed long long lastVideoTimeStamp;	// указывает на время последней положительно проверки на присутствие видео
		signed long _calc_allFramesBuffer;		// расчитанная статистика participant'a
		signed long _calc_allWriteBytesBand;	// расчитанная статистика participant'a
		signed long _calc_allWriteFramesBand;	// расчитанная статистика participant'a
	};

	/* d78 структура физического participant'a */
	struct phypart {
		phypart(){};
		phypart(const char *cn, const char *pn, stream::ConferencesConditions* cc) {
			conferenceName = cn; participantName = pn; _cc = cc;
			lastSetBitrate = maxSendBand = maxRecvBand = pLastBitrate = pBitUpHelper = 0x7FFFFFFF;
			checked = true; ___RCount = lastWrtBitrate = upBitrate = brate = phyAFB = 0; lastRCount = 0;
			lastOpTime = __confDeltaTime; __changed = false; phyBrokerLoad = 0.0;
		}
		virtual ~phypart() {
			channels.clear();
		}
		void __checkAll(const bool check) {
			checked = check;
			std::map<std::string, logpart>::iterator _ichi;
			for (_ichi = channels.begin(); _ichi != channels.end(); ++_ichi) {
				(*_ichi).second.__check(check);
			}
		}
		/* d78 расчет нового битрейта */
		signed long __calcBitrate(const float brokerLoad, const signed long brokerBand = 0x7FFFFFFF) {
			if (pLastBitrate == 0x7FFFFFFF) {
				pLastBitrate = brokerBand;
				return brokerBand;
			}
			if (pLastBitrate < brokerBand) {
				pLastBitrate = brokerBand;
			}
			signed long cBitrate = pLastBitrate;
			bool __doLoadConnection = false;
			if (brokerLoad > 25) {
				float koef = 1.f/(1.f + brokerLoad/200.f);
				if (koef < 0.5)
					koef = 0.5;
				long br = (long)(brokerBand*koef + 0.5);
				if (pLastBitrate > br) {
					cBitrate = br;
					lastOpTime = __confDeltaTime;
				}
			} else if (brokerLoad > __broker_load) {
				if (pLastBitrate > ((brokerBand * 7) / 8)) {
					cBitrate = (signed long)((float)pLastBitrate / (1. + brokerLoad / 2. / 100.));
					lastOpTime = __confDeltaTime;
				}
			} else if (brokerLoad >  1) {
				cBitrate = pLastBitrate;
			}
			return cBitrate;
		}
		/* d78 расчет нового битрейта */
		signed long __tryUpBitrate(const float brokerLoad, const signed long brokerBand = 384, const signed long totalBand = 0x7FFFFFFF) {
		  if (pBitUpHelper == 0x7FFFFFFF) { return 0; }
		  signed long __newUpBitrate = 0; signed long cBitrate = pBitUpHelper; signed long long CurrTime = __confDeltaTime;
		  if ((brokerLoad > 1) || (upBitrate == -1)) { upBitrate = -1; return 0; } else // если все плохо битрейт не поднимать
			   if ((CurrTime - lastOpTime) < 33000) {
				   if (totalBand > brokerBand) {
					   __newUpBitrate = (signed long)((float)pBitUpHelper * 1.06 + 1.);
					   if (__newUpBitrate > totalBand) {
						   __newUpBitrate = totalBand;
					   }
				   } else { upBitrate = -1; return 0; }
			   } else {
				   __newUpBitrate = (signed long)((float)pBitUpHelper * 1.06 + 1.);
			   }
			if (upBitrate) {
				if (__newUpBitrate < upBitrate) {
					upBitrate = __newUpBitrate;
				}
			} else { upBitrate = __newUpBitrate; }
			return upBitrate;
		}
		void __killUnchecked(bool logging) {
			std::map<std::string, logpart>::iterator _ichn;
			for (_ichn = channels.begin(); _ichn != channels.end();) {
				if (!(*_ichn).second.checked) {
					if (logging) {
						if (::__log_scr) {
							printf("\nKILL : logic participant %s", (*_ichn).second.participantName.c_str());
						}
					}
					_ichn = channels.erase(_ichn);
				} else {
					++ _ichn;
				}
			}
			return;
		}
		std::string conferenceName;	// имя конференции
		std::string participantName;// имя физического participant'а
		stream::ConferencesConditions* _cc; // управление скоростью
		signed long maxSendBand;	// максимальная пропускная способность на передачу данного participant'a (задает Костя С.)
		signed long maxRecvBand;	// максимальная пропускная способность на прием данного participant'a (расчитывается по статистике)
		signed long pLastBitrate;	// последняя расчитанная пропускная способность конференции на прием
		signed long pBitUpHelper;   // тоже самое, что и pLastBitrate, но используется для разделения поднятия скорости от опускания
		signed long ___RCount;		// количество receiver'ов
		long long	lastRCount;     // используется для определения изменений видеоканала
		signed long lastSetBitrate;	// последнее установленное значение bitrate'а
		signed long lastWrtBitrate;	// последний bitrate, переданный Косте С.
		signed long long lastOpTime;// время выполнения поледней операции
		signed long upBitrate;		// если можно поднять битрейт, то это значение >0,
									// если -1 это сигнал о том что партисипанту заблокировано подняти битрейта
									// в текущей итерации расчетов. 0 - значение неопределенности, не изменяющее на скорость
		signed long brate;			// входящая скорость ото всех ОУ
		signed long phyAFB;			// количество фреймов в буфере
		float phyBrokerLoad;		// расчитаная загрузка текущего партисипанта.
		std::map<std::string, logpart> channels;// логические participant'ы, к которым передаются данные. текущая накапливаемая статистика
		bool checked;				// используется при изменении конференции
		bool __changed;				// используется для определения изменений скорости
	};

	/* d78 структура конференции */
	struct fullconferencestat {
		std::string conferenceName;			// Имя конференции
		bool cStatDone;						// true - расчет статистики завершен
		bool cvRestricted;					// true - видео у одного part уже отключено
		signed long cMaxRBand;				// суммарная скорость приема данных для всех participnt'ов конференции
		bool cChecked;						// внутренний флаг проверки. Используется при создании списка конференций
		bool cChanged;						// флаг измененний в конференции. Используется для построения списка participant'ов
		signed long cTotalCalc;				// количество participant'ов в конференции. Используется для определения изменений.
		signed long cStatTicks;				// количество тиков сбора статистики. если рано 5 - то прошло 2500 мсек и производится перерачсет битрейта
		std::map<std::string, phypart> pctable;	// информация о физическом participant'e
		bool cEnableLog;					// флаг логгирования данной конференции

		fullconferencestat() {};
		fullconferencestat(const char *cn, bool enable_log)
			: conferenceName(cn), cStatDone(false), cvRestricted(false),
			cMaxRBand(0), cChecked(true), cChanged(false), cTotalCalc(-1),
			cStatTicks(0), cEnableLog(enable_log) {};
		virtual ~fullconferencestat() {	pctable.clear(); }

		void cCheckAll(const bool check)
		{
			std::map<std::string, phypart>::iterator _pctablei;
			_pctablei = pctable.begin();
			while (_pctablei != pctable.end()) {
				(*_pctablei).second.__checkAll(check);
				_pctablei ++;
			}
		}
		void cApplyBitrateChanges() {
			std::map<std::string, phypart>::iterator _pctablei;
			_pctablei = pctable.begin();
			while (_pctablei != pctable.end()) {
				signed long _minbitrate = __min_bitrate;
				if ((*_pctablei).second.lastSetBitrate != 0x7FFFFFFF) {
				if ((*_pctablei).second.lastSetBitrate < _minbitrate) {
					(*_pctablei).second.lastSetBitrate = _minbitrate;
				}
				if ((*_pctablei).second.lastSetBitrate > __max_bitrate) {
					(*_pctablei).second.lastSetBitrate = __max_bitrate;
				}
				if ((*_pctablei).second.upBitrate > __max_bitrate) {
					(*_pctablei).second.upBitrate = __max_bitrate;
				}
				if ((*_pctablei).second.upBitrate == -1) {
					(*_pctablei).second.upBitrate = 0;
				}
				if ((*_pctablei).second.upBitrate) {
					(*_pctablei).second.lastSetBitrate = (*_pctablei).second.upBitrate;
					(*_pctablei).second.upBitrate = 0;
				}
				if ((*_pctablei).second.lastWrtBitrate) {
					if ((*_pctablei).second.lastWrtBitrate < (*_pctablei).second.lastSetBitrate) {
						if (((float)(*_pctablei).second.lastWrtBitrate * 1.06 + 1.) < ((*_pctablei).second.lastSetBitrate - 1)) {
							(*_pctablei).second.lastSetBitrate = (*_pctablei).second.lastWrtBitrate;
						}
					}
					if ((*_pctablei).second.lastWrtBitrate > (*_pctablei).second.lastSetBitrate) {
						if (((float)(*_pctablei).second.lastWrtBitrate * __bit_variance) > ((*_pctablei).second.lastSetBitrate)) {
							(*_pctablei).second.lastSetBitrate = (signed long)((float)(*_pctablei).second.lastWrtBitrate * __bit_variance);
						}
					}
				}
				(*_pctablei).second.__changed = ((*_pctablei).second.lastSetBitrate != (*_pctablei).second.lastWrtBitrate);
				if ((*_pctablei).second.__changed) {
					if (cEnableLog) {
						if (::__log_scr) {
							printf("\nSR bitrate changed %s %ld ", (*_pctablei).second.participantName.c_str(), (*_pctablei).second.lastSetBitrate);
						}
					}
					(*_pctablei).second.pBitUpHelper = (*_pctablei).second.lastSetBitrate;
					if ((*_pctablei).second.lastSetBitrate < (*_pctablei).second.lastWrtBitrate) {
						(*_pctablei).second.lastOpTime = __confDeltaTime;
					}
					(*_pctablei).second._cc->RestrictBitrate((*_pctablei).second.conferenceName.c_str(), (*_pctablei).second.participantName.c_str(), (*_pctablei).second.lastSetBitrate);
					(*_pctablei).second.__changed = false;
					(*_pctablei).second.lastWrtBitrate = (*_pctablei).second.lastSetBitrate;
				} }
				_pctablei ++;
			}
		}
		void cKillUnChecked()
		{
			std::map<std::string, phypart>::iterator _ipci;
			for (_ipci = pctable.begin(); _ipci != pctable.end();) {
				if (!(*_ipci).second.checked) {
					if (cEnableLog) {
						if (::__log_scr) {
							printf("\nKILL : physic participant %s", (*_ipci).second.participantName.c_str());
						}
					}
					_ipci = pctable.erase(_ipci);
				} else {
					(*_ipci).second.__killUnchecked(cEnableLog);
					++ _ipci;
				}
			}
		}
		signed long cCalcReceiverBandwidth(std::string &pName) {
			if (pctable.empty()) { return -1; }
			signed long _rBandwidth = 0; signed long _rCount = 0;
			std::map<std::string, phypart>::iterator _phyi;
			std::map<std::string, phypart>::iterator _desi;
			_desi = pctable.find(pName);
			if (_desi == pctable.end()) { return -1; }
			_phyi = pctable.begin();
			while (_phyi != pctable.end()) {
				if ((*_phyi).second.channels.empty() || ((*_phyi).second.participantName == pName)) {
					_phyi ++; continue; // exclude myself and not connected participants from staticitics
				}
				std::map<std::string, logpart>::iterator _logi;
				_logi = (*_phyi).second.channels.begin();
				while (_logi != (*_phyi).second.channels.end()) {
					if ((*_logi).second.video) {
						if ((*_logi).second.participantName.find(pName) == 0) {
							_rBandwidth += (*_logi).second._calc_allWriteBytesBand;
							_rCount ++;
						}
					}_logi ++;
				} _phyi ++;
			}
			_rBandwidth = _rBandwidth >> 7; // (* 8 / 1024)
			if (_rCount) {
				if ((*_desi).second.maxRecvBand == 0x7FFFFFFF) {
					(*_desi).second.maxRecvBand = _rBandwidth;
				} else {
					(*_desi).second.maxRecvBand = (((*_desi).second.maxRecvBand * 4) + _rBandwidth) / 5;
				}
				(*_desi).second.___RCount = _rCount;
				if (cEnableLog) {
					if (::__log_scr) {
						printf("\nbandwidth %s %ld", (*_desi).second.participantName.c_str(), (*_desi).second.maxRecvBand);
					}
				}
				return ((*_desi).second.maxRecvBand);
			} else {
				return -1;
			}
		}
		void __doBitrateGoUpstairs() {
			std::map<std::string, logpart>::iterator _ichn, _tchn;
			std::map<std::string, phypart>::iterator _iphy, _tphy, __tphy;

			if (cEnableLog) {
				if (::__log_scr) {
					printf("\nSR Conference Stat: %s", conferenceName.c_str());
				}
			}

			for (_iphy = pctable.begin(); _iphy != pctable.end(); ++ _iphy ) {
				std::vector<std::string> _targets; std::vector<std::string>::iterator _istarget;
				for (_ichn = (*_iphy).second.channels.begin(); _ichn != (*_iphy).second.channels.end(); ++ _ichn ) {
					std::string _tp = (*_ichn).first.substr(0, (*_ichn).first.find("-<%%>-"));
					_tphy = pctable.find(_tp);
					if (_tphy == pctable.end()) { continue; }
					_targets.push_back(_tp);
				}
				for (_istarget = _targets.begin(); _istarget != _targets.end(); ++ _istarget) {
					_tphy = pctable.find(*_istarget);
					signed long __band = (*_tphy).second.maxRecvBand; float __bload = (*_tphy).second.phyBrokerLoad;
					signed long __bitrate = 0;
					for (__tphy = pctable.begin(); __tphy != pctable.end(); ++ __tphy) {
						std::string _lp = (*_tphy).first + "-<%%>-" + (*__tphy).first;
						_tchn = (*__tphy).second.channels.find(_lp);
						if (_tchn == (*__tphy).second.channels.end()) { continue; }
						if (!(*_tchn).second.video) { continue; }
						__bitrate	+= (*__tphy).second.lastWrtBitrate;
					}
					(*_tphy).second.brate = __bitrate;
					if (__bitrate) {
						(*_iphy).second.__tryUpBitrate(__bload, __bitrate, __band);
					}
				}
				if (cEnableLog) {
					if (::__log_scr) {
						printf("\nSR %16.16s: aFB=%5ld  Bl=%5.1f Br=%4ld Band=%ld", (*_iphy).second.participantName.c_str(), (*_iphy).second.phyAFB, (*_iphy).second.phyBrokerLoad, (*_iphy).second.brate, (*_iphy).second.maxRecvBand);
					}
				}
			}
		}
	};

	std::map<std::string, fullconferencestat> _conferences;	// все конференции
	std::map<std::string, fullconferencestat>::iterator _ci;// итератор по конференциям
	stream::StreamStatistics* buf;		// буфер для сбора статистики

	/* d78 прочитать конфигурацию реестра */
	inline void UpdateBitrateConfig() {
		VS_RegistryKey key(true, CONFIGURATION_KEY);
		key.GetValue(&__min_bitrate, sizeof(__min_bitrate), VS_REG_INTEGER_VT, "Min Client Bitrate");
		key.GetValue(&__min_bitstep, sizeof(__min_bitstep), VS_REG_INTEGER_VT, "Min Bitrate Step");
		key.GetValue(&__max_bitrate, sizeof(__max_bitrate), VS_REG_INTEGER_VT, "Max Client Bitrate");
		key.GetValue(&__log_scr, sizeof(__log_scr), VS_REG_INTEGER_VT, "Debug Bitrate Screen");
		key.GetValue(&__bitrate_false, sizeof(__bitrate_false), VS_REG_INTEGER_VT, "Disable Bitrate Changes");
		long useSVC = 1;
		key.GetValue(&useSVC, 4, VS_REG_INTEGER_VT, "UseSVC");
		if (useSVC)
			__bitrate_false = 1;
		key.GetValue(&__enable_QoS, sizeof(__enable_QoS), VS_REG_INTEGER_VT, "QoS Value");
		key.GetValue(&__broker_load, sizeof(__broker_load), VS_REG_INTEGER_VT, "Load Threshold");
		signed long __variance(0);
		key.GetValue(&__variance, sizeof(__variance), VS_REG_INTEGER_VT, "Bitrate Variance Percent");
		if (__variance > 0) { __bit_variance = (float)__variance / (float) 100.0; }
	}

	/* d78 получить список конференций */
	inline void MakeConferencesList() {
		unsigned long size = nConfs; _ci = _conferences.begin();
		while (_ci != _conferences.end()) {
			(*_ci).second.cChecked = false;
			_ci ++;
		}
		if (size > 0) {
			unsigned long i = 0, j = 0;
			while (i < size) {
				if (conf[j])  {
					_ci = _conferences.find(conf[j]->conferenceName);
					if (_ci != _conferences.end()) {
						if ((*_ci).second.cTotalCalc != conf[j]->nParts) {
							(*_ci).second.cTotalCalc = conf[j]->nParts;
							(*_ci).second.cChanged = true;
						} (*_ci).second.cChecked = true;
					} else {
						const bool c_enable_log = (2 != conf[j]->limitParts);
						_conferences[conf[j]->conferenceName] = fullconferencestat(conf[j]->conferenceName, c_enable_log);
						if (c_enable_log) {
							if (::__log_scr) {
								printf("\nCREATE : conference %s", conf[j]->conferenceName);
							}
						}
					} i ++; j ++;
				} else {
					j ++;
				}
			}
		}
		for (_ci = _conferences.begin(); _ci != _conferences.end();) {
			if ((*_ci).second.cChecked == false) {
				(*_ci).second.pctable.clear();
				if ((*_ci).second.cEnableLog) {
					if (::__log_scr) {
						printf("\nKILL : conference %s", (*_ci).second.conferenceName.c_str());
					}
				}
				_ci = _conferences.erase(_ci);
			} else {
				(*_ci).second.cChecked = false;
				++ _ci;
			}
		} return;
	};

	/* d78 получить список participant'ов */
	inline void MakeConferenceParticipants(fullconferencestat &cstat)
	{
		if (cstat.cChanged) {
			signed long index = GetConferenceIndex(cstat.conferenceName.c_str());
			VS_StreamsRouter_Conference *vssrc = NULL;
			if (index != -1) {
				vssrc = conf[index];
			}
			if (!vssrc) {
				return;
			}
			signed long size = vssrc->nParts;
			signed long i = 0; signed long j = 0;
			cstat.cCheckAll(false);
			while (i < size) {
				if (vssrc->part[j]) {
					if (strstr(vssrc->part[j]->participantName, "-<%%>-") != NULL) {
						char *dest = strstr(vssrc->part[j]->participantName, "-<%%>-") + 6;
						std::map<std::string, phypart>::iterator _phyi = (*_ci).second.pctable.find(dest);
						if (_phyi !=  cstat.pctable.end()) {
							std::map<std::string, logpart>::iterator _logich;
							(*_phyi).second.checked = true;
							_logich = (*_phyi).second.channels.find(vssrc->part[j]->participantName);
							if (_logich == (*_phyi).second.channels.end()) {
								(*_phyi).second.channels[vssrc->part[j]->participantName] = logpart(vssrc->part[j]->participantName);
								if (cstat.cEnableLog) {
									if (::__log_scr) {
										printf("\nCREATE : logic participant %s", vssrc->part[j]->participantName);
									}
								}
							} else {
								(*_logich).second.checked = true;
							}
						} else {
							phypart _phy(cstat.conferenceName.c_str(), dest, &ccs);
							(*_ci).second.pctable[dest] = _phy;
							if (cstat.cEnableLog) {
								if (::__log_scr) {
									printf("\nCREATE : physic participant %s", dest);
								}
							}
							continue;
						}
					} else {
						char *dest = vssrc->part[j]->participantName;
						std::map<std::string, phypart>::iterator _phyi = (*_ci).second.pctable.find(dest);
						if (_phyi == (*_ci).second.pctable.end()) {
							phypart _phy(cstat.conferenceName.c_str(), dest, &ccs);
							(*_ci).second.pctable[dest] = _phy;
							if (cstat.cEnableLog) {
								if (::__log_scr) {
									printf("\nCREATE : physic participant %s", dest);
								}
							}
						} else { (*_phyi).second.checked = true; }
					} i ++; j ++;
				} else { j ++; }
			}
			cstat.cChanged = false;
			cstat.cKillUnChecked();
			cstat.cCheckAll(false);
		}
	};

	/* d78 сбор статистики для participant'а */
	inline bool CollectParticipantStatistics(const char *conference, logpart &part)
	{
		int index = GetConferenceIndex(conference);
		if (index == -1) {
			return false;
		}
		VS_StreamsRouter_Conference *vssrc = conf[index];
		if (!vssrc) {
			return false;
		}
		index = vssrc->GetParticipantIndex(part.participantName.c_str());
		if (index == -1) {
			return false;
		}
		VS_StreamsRouter_Participant *vssrp = vssrc->part[index];
		if (!vssrp) {
			return false;
		}
		memset(buf, 0, STAT_BUF * sizeof(char));
		vssrp->statCalc.FormSndStatistics(buf, STAT_BUF, &part.video);
		if (part.video) {
			if (!part.lastVideoStartTime) {
				part.lastVideoStartTime = __confDeltaTime;
				part.lastVideoTimeStamp = __confDeltaTime;
			} else {
				part.lastVideoTimeStamp = __confDeltaTime;
			}
			part.video &= ((part.lastVideoTimeStamp - part.lastVideoStartTime) > _sVideo);
		} else {
			if ((__confDeltaTime - part.lastVideoTimeStamp) > _fVideo) {
				part.lastVideoStartTime = 0;
			} else {
				part.video = true;
			}
		}
		if (part.skip <= 0) { // skip statictics gathering if needed (first start or restart)
			part.allFramesBuffer	= buf->allFramesBuffer;
			part.allWriteBytesBand	= buf->allWriteBytesBand;
			part.allWriteFramesBand	= buf->allWriteFramesBand;
			if ((part.allWriteBytesBand > 0) || (part.video == false)) {
				part.skip ++;
			}
		} else if (part.iterates < (MAX_ITERS - 1)) { // make staticstics gathering
			part.allFramesBuffer	+= buf->allFramesBuffer;
			part.allWriteBytesBand	+= buf->allWriteBytesBand;
			part.allWriteFramesBand	+= buf->allWriteFramesBand;
			part.iterates ++;
		}
		if (part.iterates == (MAX_ITERS - 1)) {
			part._calc_allFramesBuffer	  = part.allFramesBuffer / MAX_ITERS;
			part._calc_allWriteBytesBand  = part.allWriteBytesBand / MAX_ITERS;
			part._calc_allWriteFramesBand = part.allWriteFramesBand / MAX_ITERS;
			if (part._calc_allWriteFramesBand > 0) {
				part.allWriteFramesBand = (part.allWriteFramesBand * 4 + part._calc_allWriteFramesBand) / 5;
			}
			if (part.video) {
				part._calc_allWriteFramesBand = part.allWriteFramesBand;
			} else {
				part._calc_allFramesBuffer = 2;		// make brokerLoad = 2
				part._calc_allWriteFramesBand = 1;	// for disable bitrate changes at __calcBitrate
			}
			part.skip = 0; part.iterates = 0; return true;	// calculation success finish
		}
		return false;
	};

	inline void OptimizeConferenceBitrate(fullconferencestat &cstat) {
		if (cstat.pctable.empty()) {return;}
		if (cstat.cChanged) {cstat.cChanged = false; return;}
		signed long __old_minimal = __min_bitrate;
		cstat.cvRestricted = false;
		std::map<std::string, phypart>::iterator _phyi;
		_phyi = cstat.pctable.begin();
//		if ((*_phyi).second.channels.empty()) { return; }
		{
			std::map<std::string, phypart>::iterator _temp;
			_temp = cstat.pctable.begin();
			for (unsigned long i = 0; i < cstat.pctable.size(); i++) {
				(*_temp).second.__changed = false;
				_temp ++;
			}
		}
		while (_phyi != cstat.pctable.end()) {
			std::map<std::string, phypart>::iterator _peer;
			_peer = cstat.pctable.begin();
			std::vector<std::string> __peers;
			std::vector<std::string> __peers_wrt;
			signed long __lastSetBitrate  = 0; // для расчета старой полосы
			signed long __lastCallBitrate = 0; // для расчета если превышен brokerLoad
			signed long __allFramesBuffer = 0;
			signed long __allWriteFramesBand = 0;
			signed long __minLastWriteBitrate = 0x7FFFFFFF;
			while (_peer != cstat.pctable.end()) {
				if ((*_peer).second.participantName != (*_phyi).second.participantName) {
					std::map<std::string, logpart>::iterator _logi;
					std::string _peerName = (*_phyi).second.participantName + "-<%%>-" + (*_peer).second.participantName;
					_logi = (*_peer).second.channels.find(_peerName);
					if (_logi !=  (*_peer).second.channels.end()) {
						if ((*_logi).second.video) {
							__allFramesBuffer += (*_logi).second._calc_allFramesBuffer;
							__allWriteFramesBand += (*_logi).second._calc_allWriteFramesBand;
							__lastSetBitrate += (*_peer).second.lastWrtBitrate;
							if (((*_peer).second.lastWrtBitrate < __minLastWriteBitrate) && (*_peer).second.lastWrtBitrate) {
								__minLastWriteBitrate = (*_peer).second.lastWrtBitrate;
							}
							__peers.push_back(_peerName);
							if ((*_peer).second.lastWrtBitrate) {
								__peers_wrt.push_back(_peerName);
							}
						}
					}
				} _peer ++;
			}
			/* если bandwidth не известен, не изменяем битрейт */
			if ((*_phyi).second.maxRecvBand == 0x7FFFFFFF) {
				__lastSetBitrate = 0;
			}
			/* если данные не передаются, то обход деления на 0 и brokerLoad будет очень большим */
			if (!__allWriteFramesBand) {
				__allWriteFramesBand = 1;
			}
			/* если есть расчитанный битрейт, то он будет использоваться для расчетов при 5% загрузке */
			if ((__lastSetBitrate) && ((*_phyi).second.pLastBitrate!= 0x7FFFFFFF)) {
				(*_phyi).second.pLastBitrate = __lastSetBitrate;
			}
			const float __brLoad = (float) __allFramesBuffer / (float) __allWriteFramesBand;
			(*_phyi).second.phyAFB = __allFramesBuffer;
			(*_phyi).second.phyBrokerLoad = __brLoad;
			signed long __newSetBitrate = 0;
			if (__brLoad > __broker_load) {
				__lastSetBitrate = __lastCallBitrate = cstat.cCalcReceiverBandwidth((*_phyi).second.participantName);
				if (__lastSetBitrate) {
					__newSetBitrate = (*_phyi).second.__calcBitrate(__brLoad, __lastSetBitrate);
				}
			} else { __lastSetBitrate = 0; }
			signed long __nPairs = 0; signed long __mPairs = 0;
			if (__lastCallBitrate) {
				__mPairs = (signed long)__peers.size();
			} else {
				__mPairs = (signed long)__peers_wrt.size();
			}
			if ((__mPairs) && (__newSetBitrate)) {
				__nPairs = __newSetBitrate / __min_bitstep;
				if ((__nPairs > __mPairs)
					|| (__newSetBitrate >= __lastSetBitrate)) {
					__nPairs = __mPairs;
				}
			}
			if (__nPairs) {
				signed long __newBitrate = __newSetBitrate / __nPairs;
				if (__newBitrate > __max_bitrate) {
					__newBitrate = __max_bitrate;
				}
				if ((__newBitrate < __min_bitrate) && (!cstat.cvRestricted) && (0x7FFFFFFF != __minLastWriteBitrate)) {
					(*_phyi).second._cc->RestrictBitrate((*_phyi).second.conferenceName.c_str(), (*_phyi).second.participantName.c_str(), -1);
					if (cstat.cEnableLog) {
						if (::__log_scr) {
							printf("\nVIDEO : channel disabled at %s, %ld", (*_phyi).second.participantName.c_str(), __newBitrate);
						}
					}
					cstat.cvRestricted = true;
					__newBitrate = __min_bitrate;
				}
				__min_bitrate = __old_minimal;
				for (signed long i = 0; i < __nPairs; i++) {
					std::string __base;
					if (__lastCallBitrate) {
						__base = __peers[i].substr(__peers[i].find("-<%%>-") + 6);
					} else {
						__base = __peers_wrt[i].substr(__peers[i].find("-<%%>-") + 6);
					}
					std::map<std::string, phypart>::iterator __tartget;
					__tartget = cstat.pctable.find(__base);
					if (__tartget != cstat.pctable.end()) {
						if ((*__tartget).second.checked) {
							if ((*__tartget).second.lastSetBitrate > __newBitrate) {
								(*__tartget).second.lastSetBitrate = __newBitrate;
								(*__tartget).second.checked = true;
								(*__tartget).second.__changed = true;
							}
						} else {
							if (((*__tartget).second.lastWrtBitrate) || __lastCallBitrate) {
								(*__tartget).second.lastSetBitrate = __newBitrate;
								(*__tartget).second.checked = true;
								(*__tartget).second.__changed = true;
							}
						}
					}
				}
			} _phyi ++;
		} cstat.cCheckAll(false); cstat.__doBitrateGoUpstairs(); cstat.cApplyBitrateChanges(); return;
	}

	inline void ProcessingPeriodic( void )
	{
		if (!mcp)	ResetToConnectMcp();
		if (flagMcpWrite)	return;
		switch (statePeriodic)
		{
		case 0 :
			{
				if (flagMcpConnect)
				{
					smReply->type = SM_TYPE_PERIODIC_START;
					WriteMcp( (const void *)smReply, sizeof(smReply->type) );
				}
				statePeriodic = 1;		return;
			}
		case 1 :
			{
				if (flagMcpConnect)
				{
					SmReply::StartConferences   &startConferences = smReply->startConferences;
					startConferences.type = SM_TYPE_PERIODIC_START_CONFERENCES;
					startConferences.maxConferences = maxConfs;
					WriteMcp( (const void *)smReply, sizeof(smReply->startConferences) );
				}
				statePeriodic = 2;	indexConfCount = ~0;	return;
			}
		case 2 :
			{
				while (++indexConfCount <= maxIndConfs)
				{
					VS_StreamsRouter_Conference   *cf = conf[indexConfCount];
					if (!cf)	continue;
					if (!cf->ProcessingPeriodic())
					{	DeleteConference( indexConfCount );		continue;	}
					if (flagMcpConnect)
					{
						cf->FillMonitorStruct( smReply->conference );
						WriteMcp( (const void *)smReply, sizeof(smReply->conference) );
					}
					statePeriodic = 3;		return;
				}
				statePeriodic = 6;
                return;
			}
		case 3 :
			{
				if (flagMcpConnect)
				{
					smReply->type = SM_TYPE_PERIODIC_START_PARTICIPANTS;
					WriteMcp( (const void *)smReply, sizeof(smReply->type) );
				}
				statePeriodic = 4;	indexPartCount = ~0;	return;
			}
		case 4 :
			{
				VS_StreamsRouter_Conference   *cf = conf[indexConfCount];
				if (cf)
				{
					while (++indexPartCount <= cf->maxIndParts)
					{
						VS_StreamsRouter_Participant   *pr = cf->part[indexPartCount];
						if (!pr)	continue;
						if (pr->ProcessingPeriodic())
						{
							if (cf->type == VS_Conference_Type::CT_PRIVATE && pr->CalculateBandwidth()) {
								for (unsigned long i = 0; i < pr->nReceiverConnected; i++) {
									auto sender = cf->part[pr->receiverConnectedIndex[i]];
									ccs.RestrictBitrate(cf->conferenceName, sender->participantName, pr->bandwidthInfo.restrictBitrate);
								}
							}
							if (flagMcpConnect)
							{
								cf->FillParticipantMonitorStruct( indexPartCount, smReply->participant );
								WriteMcp( (const void *)smReply, sizeof(smReply->participant) );
						}	}
						else	cf->DeleteParticipant( indexPartCount );
						return;
					}
				}
				statePeriodic = 5;	return;
			}
		case 5 :
			{
				if (flagMcpConnect)
				{
					smReply->type = SM_TYPE_PERIODIC_STOP_PARTICIPANTS;
					WriteMcp( (const void *)smReply, sizeof(smReply->type) );
				}
				statePeriodic = 2;	return;
			}
		case 6 :
			{
				if (flagMcpConnect)
				{
					smReply->type = SM_TYPE_PERIODIC_STOP_CONFERENCES;
					WriteMcp( (const void *)smReply, sizeof(smReply->type) );
				}
				statePeriodic = 7;	return;
			}
		case 7 :
			{
				if (flagMcpConnect)
				{
					smReply->type = SM_TYPE_PERIODIC_STOP;
					WriteMcp( (const void *)smReply, sizeof(smReply->type) );
				}
				statePeriodic = 8;	return;
			}
		case 8 :
			{
				PeriodicDeleteConn();
				statePeriodic = 0;	return;
			}
		default :
			{
			statePeriodic = 0;	return;
	}	}	}
	// end VS_StreamsRouter_Implementation::ProcessingPeriodic

	inline void ProcessingTick( void )
	{
		VS_MainSVCStatistics::UpdateStatistics(&ccs);
		__confDeltaTime = VS_GetTickCount64() - __confStartTime;
		if (__bitrate_false > 0) {
			if (__bitrate_false > 10) {
				UpdateBitrateConfig(); // refresh config once in 5 seconds
			} else {
				__bitrate_false ++;
			}
			return; // no bitrate changes if indicated in registry
		}
		if (__bitrate_false <= -10) {
			UpdateBitrateConfig(); // refresh config once in 5 seconds
		} else {
			__bitrate_false --;
		}
		MakeConferencesList();
		_ci = _conferences.begin();
		while (_ci != _conferences.end()) {
			MakeConferenceParticipants((*_ci).second);
			std::map<std::string, phypart>::iterator _phyi;
			if ((*_ci).second.pctable.empty()) {
				_ci ++; continue;
			}
			_phyi = (*_ci).second.pctable.begin();
			while (_phyi != (*_ci).second.pctable.end()) {
				std::map<std::string, logpart>::iterator _logi;
				if ((*_phyi).second.channels.empty()) {
					_phyi ++; continue;
				}
				_logi = (*_phyi).second.channels.begin();
				while (_logi != (*_phyi).second.channels.end()) {
					CollectParticipantStatistics((*_ci).second.conferenceName.c_str(), (*_logi).second);
					_logi ++;
				} _phyi ++;
			}
			if ((*_ci).second.cStatTicks >= 10) {
				OptimizeConferenceBitrate((*_ci).second);
				(*_ci).second.cStatTicks = 0;
			} else { (*_ci).second.cStatTicks ++; }
			_ci ++;
		}
	}
	// end VS_StreamsRouter_Implementation::ProcessingTick

	inline void Thread( void )
	{
		threadId = GetCurrentThreadId();
		cr.cntrlResponse.cmd = vs_sr_start_router;		cr.cntrlResponse.res = true;
		if (icp->Write( (const void *)&cr, sizeof(cr.cntrlResponse) ))
		{
			DWORD   trans, error, n_repeat_error = 0, repeat_error = 0,
						mills = tickMs, old_tick = GetTickCount(), tmp_tick;
			ULONG_PTR   key;	VS_Overlapped   *pov;
			while (1)
			{
				trans = 0;	key = 0;	pov = 0;	error = 0;
				const BOOL   res = GetQueuedCompletionStatus( hiocp, &trans, &key, (LPOVERLAPPED *)&pov, (!statePeriodic || flagMcpWrite) ? mills : 0 );
				tmp_tick = GetTickCount();	currDiffTm = tmp_tick - old_tick;
				currTm += currDiffTm;		sumCurrDiffTm += currDiffTm;
				if (!res)
				{
					error = GetLastError();
					if (pov == 0)
					switch (error)
					{
					case WAIT_TIMEOUT :
					case ERROR_SEM_TIMEOUT :	n_repeat_error = 0;		goto go_continue;
					}
					if (!pov)
					{
						if (!n_repeat_error)
						{
							repeat_error = error;	++n_repeat_error;
						}
						else if (repeat_error == error)
						{
							if (++n_repeat_error >= VS_SR_REPEAT_ERROR_NUMBER)
							{
								goto go_return;
						}	}
						else	n_repeat_error = 0;
						goto go_continue;
					}
					else	pov->error = error;
					n_repeat_error = 0;
				}
				if (!RunHandle( (unsigned long)trans, *pov ))
				{
					goto go_return;
				}
go_continue:	if (statePeriodic)		ProcessingPeriodic();
				if (sumCurrDiffTm >= tickMs)
				{
					tickDiffTm = sumCurrDiffTm;		sumCurrDiffTm = 0;
					tickTm += tickDiffTm;			mills = tickMs;
					if (!statePeriodic)		ProcessingPeriodic();
					ProcessingTick();
				}
				else	mills -= currDiffTm;
				old_tick = tmp_tick;
		}	}
go_return:
		icp->Close();
	}
	// end VS_StreamsRouter_Implementation::Thread

	static unsigned __stdcall Thread( void *arg )
	{
		vs::SetThreadName("StreamRouter");
		((VS_StreamsRouter_Implementation *)arg)->Thread();
		_endthreadex( 0 );		return 0;
	}
	// end VS_StreamsRouter_Implementation::Thread

	//////////////////////////////////////////////////////////////////////////////////////

	enum VS_SR_Cond_Cmd { vs_sr_cond_unknown = 0,
				vs_sr_cond_create_conference, vs_sr_cond_add_participant,
				vs_sr_cond_remove_participant, vs_sr_cond_remove_conference,
				vs_sr_cond_set_participant_caps};
	union CondInstruction
	{	struct CreateConference
		{	VS_SR_Cond_Cmd   cmd;
			char   conferenceName[VS_STREAMS_MAX_SIZE_CONFERENCE_NAME + 1];
		};	// end CreateConference struct
		struct AddParticipant
		{	VS_SR_Cond_Cmd   cmd;
			char   conferenceName[VS_STREAMS_MAX_SIZE_CONFERENCE_NAME + 1];
			char   participantName[VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME + 1];
		};	// end AddParticipant struct
		struct RemoveParticipant
		{	VS_SR_Cond_Cmd   cmd;
			char   conferenceName[VS_STREAMS_MAX_SIZE_CONFERENCE_NAME + 1];
			char   participantName[VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME + 1];
			stream::ParticipantStatistics partStat;
		};	// end RemoveParticipant struct
		struct RemoveConference
		{	VS_SR_Cond_Cmd   cmd;
			char   conferenceName[VS_STREAMS_MAX_SIZE_CONFERENCE_NAME + 1];
			stream::ConferenceStatistics confStat;
		};	// end RemoveConference struct
		struct SetParticipantCaps
		{
			VS_SR_Cond_Cmd   cmd;
			char   conferenceName[VS_STREAMS_MAX_SIZE_CONFERENCE_NAME + 1];
			char   participantName[VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME + 1];
			void		*caps_buf;
			unsigned long	buf_sz;
		};
		VS_SR_Cond_Cmd   cmd;
		CreateConference				createConference;
		AddParticipant					addParticipant;
		RemoveParticipant				removeParticipant;
		RemoveConference				removeConference;
		SetParticipantCaps				setPartCaps;
	};	// end CondInstruction struct

	inline void Cond_Write( const void *buffer, const unsigned long size )
	{
		unsigned long   mills = 60000;
		if (ocp_cond && (!ocp_cond->Write( buffer, size )
							|| ocp_cond->GetWriteResult( mills ) != (int)size))
		{
			delete ocp_cond;	ocp_cond = 0;
		}
	}
	// end VS_StreamsRouter_Implementation::Cond_Write

	inline void Cond_CreateConference( const char *conferenceName )
	{
		/**
			Call in stream router thread
		**/
		ccs.CreateConference(conferenceName);

		////CondInstruction::CreateConference   createConference;
		////memset( (void *)&createConference, 0, sizeof(createConference) );
		////createConference.cmd = vs_sr_cond_create_conference;
		////strcpy( createConference.conferenceName, conferenceName );
		////Cond_Write( (const void *)&createConference, sizeof(createConference) );
	}
	// end VS_StreamsRouter_Implementation::Cond_CreateConference

	inline void Cond_AddParticipant( const char *conferenceName, const char *participantName )
	{
		/**
			Call in stream router thread
		**/

		ccs.AddParticipant(conferenceName, participantName);
		////CondInstruction::AddParticipant   addParticipant;
		////memset( (void *)&addParticipant, 0, sizeof(addParticipant) );
		////addParticipant.cmd = vs_sr_cond_add_participant;
		////strcpy( addParticipant.conferenceName, conferenceName );
		////strcpy( addParticipant.participantName, participantName );
		////Cond_Write( (const void *)&addParticipant, sizeof(addParticipant) );
	}
	// end VS_StreamsRouter_Implementation::Cond_AddParticipant

	inline void Cond_RemoveParticipant(const char *conferenceName, const char *participantName, const stream::ParticipantStatistics& partStat)
	{
		/**
			Call in stream router thread
		**/

		ccs.RemoveParticipant(conferenceName, participantName, partStat);
		////CondInstruction::RemoveParticipant   removeParticipant;
		////memset( (void *)&removeParticipant, 0, sizeof(removeParticipant) );
		////removeParticipant.cmd = vs_sr_cond_remove_participant;
		////strcpy( removeParticipant.conferenceName, conferenceName );
		////strcpy( removeParticipant.participantName, participantName );
		////removeParticipant.partStat = partStat;
		////Cond_Write( (const void *)&removeParticipant, sizeof(removeParticipant) );
	}
	// end VS_StreamsRouter_Implementation::Cond_RemoveParticipant

	inline void Cond_RemoveConference(const char *conferenceName, const stream::ConferenceStatistics& confStat)
	{
		/**
			Call in stream router thread
		**/

		ccs.RemoveConference(conferenceName, confStat);
		////CondInstruction::RemoveConference   removeConference;
		////memset( (void *)&removeConference, 0, sizeof(removeConference) );
		////removeConference.cmd = vs_sr_cond_remove_conference;
		////strcpy( removeConference.conferenceName, conferenceName );
		////removeConference.confStat = confStat;
		////Cond_Write( (const void *)&removeConference, sizeof(removeConference) );
	}
	// end VS_StreamsRouter_Implementation::Cond_RemoveConference
	inline void Cond_SetParticipantCaps(const char *conferenceName, const char *partName, const void *caps_buf,const unsigned long buf_sz)
	{
		/**
			Call in stream router thread
		**/
		ccs.SetParticipantCaps(conferenceName, partName, caps_buf, buf_sz);
		////void * copy_caps_buf = malloc(buf_sz);
		////memcpy(copy_caps_buf,caps_buf,buf_sz);
		////CondInstruction::SetParticipantCaps setPartCaps;
		////memset(&setPartCaps,0,sizeof(setPartCaps));
		////setPartCaps.cmd = vs_sr_cond_set_participant_caps;
		////strcpy(setPartCaps.conferenceName,conferenceName);
		////strcpy(setPartCaps.participantName,partName);
		////setPartCaps.caps_buf = copy_caps_buf;
		////setPartCaps.buf_sz = buf_sz;
		////Cond_Write( (const void *)&setPartCaps,sizeof(setPartCaps));
	}
	inline void ThreadConditionsCallbacks( void )
	{
		bool   flagContinue = true;
		unsigned long   mills;
		CondInstruction   condInstruction;
		while (flagContinue)
		{
			mills = INFINITE;
			if ( !icp_cond->Read( (void *)&condInstruction, sizeof(condInstruction) )
					|| icp_cond->GetReadResult( mills, 0, true ) <= (int)sizeof(condInstruction.cmd))
			{
				flagContinue = false;
			}
			else
			{	switch (condInstruction.cmd)
				{
				case vs_sr_cond_create_conference :
					ccs.CreateConference(condInstruction.createConference.conferenceName);
					break;
				case vs_sr_cond_add_participant :
					ccs.AddParticipant(condInstruction.addParticipant.conferenceName, condInstruction.addParticipant.participantName);
					break;
				case vs_sr_cond_remove_participant :
					ccs.RemoveParticipant(condInstruction.removeParticipant.conferenceName, condInstruction.removeParticipant.participantName, condInstruction.removeParticipant.partStat);
					break;
				case vs_sr_cond_remove_conference :
					ccs.RemoveConference(condInstruction.removeConference.conferenceName, condInstruction.removeConference.confStat);
					break;
				case vs_sr_cond_set_participant_caps:
					ccs.SetParticipantCaps(condInstruction.setPartCaps.conferenceName, condInstruction.setPartCaps.participantName, condInstruction.setPartCaps.caps_buf, condInstruction.setPartCaps.buf_sz);
					free(condInstruction.setPartCaps.caps_buf);
					break;
				default :
					flagContinue = false;
	}	}	}	}
	// end VS_StreamsRouter_Implementation::ThreadConditionsCallbacks

	static unsigned __stdcall ThreadConditionsCallbacks( void *arg )
	{	((VS_StreamsRouter_Implementation *)arg)->ThreadConditionsCallbacks();	_endthreadex( 0 );	return 0;	}
	// end VS_StreamsRouter_Implementation::ThreadConditionsCallbacks

	//////////////////////////////////////////////////////////////////////////////////////

	inline bool CntrlSR( const ControlInquiry *cI, const unsigned long sI,
							ControlResponse *cR = 0, const unsigned long sR = 0,
							unsigned long mills = 600000 )
	{
		std::unique_lock<decltype(mtx)> l(mtx);
		if (!isInit)
			return false;

		assert(cI != nullptr);
		assert(sI != 0);
		if (!ocp->Write(cI, sI) || sI != ocp->GetWriteResult(mills))
		{
			dprint1("SR: Error writing to \"ocp\". System Error: %lu.", GetLastError());
			return false;
		}

		if (!sR) // No response is expected, we are done.
			return true;

		std::lock_guard<decltype(mtx_read)> l_read(mtx_read);
		// Release the main mutex to allow other threads to send async requests while we are waiting for our response.
		l.unlock();
		assert(cR != nullptr);
		if (!ocp->Read(cR, sR) || sR != ocp->GetReadResult(mills) || cI->cntrlInquiry.cmd != cR->cntrlResponse.cmd)
		{
			dprint1("SR: Error reading from \"ocp\". System Error: %lu.", GetLastError());
			return false;
		}
		return true;
	}
	// end VS_StreamsRouter_Implementation::CntrlSR

	/////////////////////////////////////////////////////////////////////////////////////

	inline bool InitAct( const char *endpointName, VS_AccessConnectionSystem *acs,
							VS_TlsHandler* tlsHandler, stream::ConferencesConditions *ccs,
							const unsigned long maxConferences)
	{
		if (isInit)		return false;
		logDir = vs::GetLogDirectory() + stream::c_streams_logs_directory_name;
		boost::system::error_code ec;
		boost::filesystem::create_directories(logDir, ec);
		if (ec)
		{
			dstream0 << "Can't create directory '" << logDir << "': " << ec.message();
			return false;
		}
		currDiffTm = currTm = 0;
		VS_StreamsRouter_Implementation::acs = acs;
		VS_StreamsRouter_Implementation::ccs.ConnectToSplitter(ccs);
		VS_StreamsRouter_Implementation::statePeriodic = false;
		size_t   sz = maxConferences * sizeof(VS_StreamsRouter_Conference *);
		conf = (VS_StreamsRouter_Conference **)malloc( sz );
		if (!conf) {	ShutdownAct();	return false;	}
		memset( (void *)conf, 0, sz );
		maxConfs = maxConferences;
		if (!(hiocp = CreateIoCompletionPort( INVALID_HANDLE_VALUE, 0, 0, 0 )))
		{	ShutdownAct();	return false;	}
		////if (!(icp_cond = new VS_ConnectionMsg) || !icp_cond->SetSizeBuffers( 0, 65536 )
		////	|| !icp_cond->Create( vs_pipe_type_inbound )
		////	|| !icp_cond->CreateOvReadEvent()) {	ShutdownAct();	return false;	}
		////if (!(ocp_cond = new VS_ConnectionMsg)
		////	|| !ocp_cond->Open( icp_cond, vs_pipe_type_outbound )
		////	|| !ocp_cond->CreateOvWriteEvent()) {	ShutdownAct();	return false;	}
		////uintptr_t   th = _beginthreadex( 0, 0, ThreadConditionsCallbacks, (void *)this, 0, 0 );
		////if (!th || th == -1L) {		ShutdownAct();	return false;	}
		////hthr_cond = (HANDLE)th;
		if (!(ocp = new VS_ConnectionMsg) || !ocp->Create( vs_pipe_type_duplex )
			|| !ocp->CreateOvReadEvent() || !ocp->CreateOvWriteEvent())
		{	ShutdownAct();	return false;	}
		if (!(icp = new VS_ConnectionMsg) || !icp->Open( ocp, vs_pipe_type_duplex )
			|| !icp->SetIOCP( hiocp )) {	ShutdownAct();	return false;	}
		icp->SetOvWriteFields( (const VS_ACS_Field)VS_SR_CONTROL_WRITE );
		icp->SetOvReadFields( (const VS_ACS_Field)VS_SR_CONTROL_READ );
		ResetToConnectMcp();
		uintptr_t th = _beginthreadex( 0, 0, Thread, (void *)this, 0, 0 );
		if ( !th || th == -1L) {	ShutdownAct();	return false;	}
		hthr = (HANDLE)th;	SetThreadPriority( hthr, THREAD_PRIORITY_ABOVE_NORMAL );
		ControlResponse::CntrlResponse	 cntrlResponse;		memset( (void *)&cntrlResponse, 0, sizeof(cntrlResponse) );
		unsigned long   mills = 120000;
		if (!ocp->Read( (void *)&cntrlResponse, sizeof(cntrlResponse) )
			|| ocp->GetReadResult( mills ) != sizeof(cntrlResponse)
			|| cntrlResponse.cmd != vs_sr_start_router || !cntrlResponse.res)
		{	ShutdownAct();	return false;	}
		strcpy( VS_StreamsRouter_Implementation::endpointName, endpointName );
		streamsHandler = new VS_StreamsHandler( VS_StreamsRouter_Implementation::endpointName, this );
		if (!streamsHandler || !streamsHandler->IsValid())
		{	ShutdownAct();	return false;	}
		sprintf( handlerName, "%s%s", trHandlerNamePrefix, VS_StreamsRouter_Implementation::endpointName );
		if (!acs->AddHandler( handlerName, streamsHandler ))
		{	ShutdownAct();	return false;	}
		if (tlsHandler && !tlsHandler->AddHandler( handlerName, streamsHandler ))
		{	ShutdownAct();	return false;	}
		VS_MainSVCStatistics::CleanStatDirectory();
		return isInit = true;
	}
	// end VS_StreamsRouter_Implementation::InitAct

	bool Init(const char *endpointName, VS_AccessConnectionSystem *acs,
						VS_TlsHandler* tlsHandler, stream::ConferencesConditions* ccs,
						const unsigned long maxConferences) override
	{
		if (!endpointName || !*endpointName || strlen( endpointName ) > VS_ACS_MAX_SIZE_ENDPOINT_NAME
				|| !acs || !ccs || !maxConferences || maxConferences > VS_SR_MAX_CONFERENCES )
			return false;
		std::lock_guard<decltype(mtx)> l(mtx);
		return InitAct(endpointName, acs, tlsHandler, ccs, maxConferences);
	}
	// end VS_StreamsRouter_Implementation::Init

	bool IsInit() override
	{
		std::lock_guard<decltype(mtx)> l(mtx);
		return isInit;
	}
	// end VS_StreamsRouter_Implementation::IsInit

	inline void ShutdownThreadAct( HANDLE &hthr, VS_ConnectionMsg *&ocp, VS_ConnectionMsg *&icp )
	{
		if (ocp) {		delete ocp;		ocp = 0;	}
		if (hthr)
		{
			auto ret = WaitForSingleObject(hthr, VS_SR_TIMEOUT_THREAD_SHUTDOWN);
			if (ret == WAIT_OBJECT_0)
				CloseHandle( hthr );
			else if (ret == WAIT_TIMEOUT)
				dprint1("Wait for main thread termination timed out");
			else if (ret == WAIT_FAILED)
				dprint1("Wait for main thread termination failed: gle=%lu", GetLastError());
			hthr = 0;
		}
		if (icp) {		delete icp;		icp = 0;	}
	}
	// end VS_StreamsRouter_Implementation::ShutdownThreadAct

	inline void ShutdownAct( void )
	{
		if (streamsHandler)
		{
			if (acs)	acs->RemoveHandler( handlerName );
			delete streamsHandler;	streamsHandler = 0;
		}
		ShutdownThreadAct( hthr, ocp, icp );
		ShutdownThreadAct( hthr_cond, ocp_cond, icp_cond );
		if (hiocp) {	CloseHandle( hiocp );	hiocp = 0;	}
		if (conf)
		{
			for (unsigned long i = 0; i < maxConfs; ++i)
				if (conf[i]) {		delete conf[i];		conf[i] = 0;	}
			free( (void *)conf );	conf = 0;
		}
		memset( (void *)endpointName, 0, sizeof(endpointName) );
		memset( (void *)handlerName, 0, sizeof(handlerName) );
		RemoveAllDeleteConns();
		acs = 0;	isInit = false;
	}
	// end VS_StreamsRouter_Implementation::ShutdownAct

	void Stop() override
	{
		std::lock_guard<decltype(mtx)> l(mtx);
		ShutdownAct();
	}
	// end VS_StreamsRouter_Implementation::Stop

	bool AddConferencesCondition(stream::ConferencesConditions* ccs) override
	{
		return VS_StreamsRouter_Implementation::ccs.ConnectToSplitter(ccs);
	}

	void RemoveConferencesCondition(stream::ConferencesConditions* ccs) override
	{
		VS_StreamsRouter_Implementation::ccs.DisconnectFromSplitter(ccs);
	}

	bool CreateConference(string_view conferenceName, VS_Conference_Type conferenceType, const char* sslKey, unsigned maxParticipants, bool formLogs, std::chrono::steady_clock::duration max_silence) override
	{
		const unsigned long maxSilenceMs = std::chrono::duration_cast<std::chrono::milliseconds>(max_silence).count();
		if (conferenceName.empty() || conferenceName.size() > VS_STREAMS_MAX_SIZE_CONFERENCE_NAME
				|| !maxParticipants || maxParticipants > VS_SR_MAX_PARTICIPANTS || maxSilenceMs < VS_SR_MIN_SILENCE_MS)
				return false;
		if (sslKey && strlen(sslKey) > VS_STREAMS_MAX_SIZE_SSLKEY)
				return false;
		bool   ret = false, flagCcsDelete = false;
		ControlInquiry::CreateConference   createConference;
		memset( (void *)&createConference, 0, sizeof(createConference) );
		createConference.cmd = vs_sr_create_conference;
		strcpy( createConference.conferenceName, conferenceName );
		if (sslKey)
			strcpy(createConference.sslKey, sslKey );
		createConference.maxParticipants = maxParticipants;
		createConference.formLogs = formLogs;
		createConference.maxSilenceMs = maxSilenceMs;
		createConference.type = conferenceType;
		ControlResponse::CntrlResponse   cntrlResponse;		memset( (void *)&cntrlResponse, 0, sizeof(cntrlResponse) );
		if (CntrlSR( (ControlInquiry *)&createConference, sizeof(createConference), (ControlResponse *)&cntrlResponse, sizeof(cntrlResponse) ))
			ret = cntrlResponse.res;
		return ret;
	}
	// end VS_StreamsRouter_Implementation::CreateConference

	void RemoveConference(string_view conferenceName) override
	{
		if (conferenceName.empty() || conferenceName.size() > VS_STREAMS_MAX_SIZE_CONFERENCE_NAME)
			return;
		ControlInquiry::RemoveConference   removeConference;
		memset( (void *)&removeConference, 0, sizeof(removeConference) );
		removeConference.cmd = vs_sr_remove_conference;
		strcpy( removeConference.conferenceName, conferenceName );
		CntrlSR( (ControlInquiry *)&removeConference, sizeof(removeConference) );
	}
	// end VS_StreamsRouter_Implementation::RemoveConference

	bool AddParticipant(string_view conferenceName, string_view participantName,
	                    string_view connectedParticipantName, stream::Buffer* sndBuffer,
	                    bool formLogs, std::chrono::steady_clock::duration max_silence,
	                    const stream::Track* tracks, unsigned nTracks) override
	{
		const unsigned long maxSilenceMs = std::chrono::duration_cast<std::chrono::milliseconds>(max_silence).count();
		if (conferenceName.empty() || conferenceName.size() > VS_STREAMS_MAX_SIZE_CONFERENCE_NAME
			|| participantName.empty() || participantName.size() > VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME
			|| connectedParticipantName.size() > VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME
			|| maxSilenceMs < VS_SR_MIN_SILENCE_MS)		return false;
		bool   ret = false, flagBufferDelete = false;
		if (!sndBuffer) {	sndBuffer = new stream::DefaultBuffer;
							if (!sndBuffer)		return ret;
							flagBufferDelete = true;	}
		ControlInquiry::AddParticipant   addParticipant;
		memset( (void *)&addParticipant, 0, sizeof(addParticipant) );
		addParticipant.cmd = vs_sr_add_participant;
		strcpy( addParticipant.conferenceName, conferenceName );
		strcpy( addParticipant.participantName, participantName );
		if (!connectedParticipantName.empty())
			strcpy( addParticipant.connectedParticipantName, connectedParticipantName );
		addParticipant.sndBuffer = sndBuffer;
		addParticipant.flagBufferDelete = flagBufferDelete;
		addParticipant.formLogs = formLogs;
		addParticipant.maxSilenceMs = maxSilenceMs;
		{
			uint8_t ftracks[256];
			if (!stream::TracksToFTracks(ftracks, tracks, nTracks))
				return false;
			stream::FTracksToMTracks(addParticipant.mtracks, ftracks);
		}
		ControlResponse::CntrlResponse   cntrlResponse;		memset( (void *)&cntrlResponse, 0, sizeof(cntrlResponse) );
		if (CntrlSR( (ControlInquiry *)&addParticipant, sizeof(addParticipant), (ControlResponse *)&cntrlResponse, sizeof(cntrlResponse) ))
			ret = cntrlResponse.res;
		if (!ret && flagBufferDelete)	delete sndBuffer;
		return ret;
	}
	// end VS_StreamsRouter_Implementation::AddParticipant

	bool AddParticipant(string_view conferenceName, string_view participantName,
	                    stream::Buffer* sndBuffer, bool formLogs,
	                    std::chrono::steady_clock::duration max_silence) override
	{
		const unsigned long maxSilenceMs = std::chrono::duration_cast<std::chrono::milliseconds>(max_silence).count();
		if (conferenceName.empty() || conferenceName.size() > VS_STREAMS_MAX_SIZE_CONFERENCE_NAME
			|| participantName.empty() || participantName.size() > VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME
			|| maxSilenceMs < VS_SR_MIN_SILENCE_MS)		return false;
		bool   ret = false, flagBufferDelete = false;
		if (!sndBuffer) {	sndBuffer = new stream::DefaultBuffer;
							if (!sndBuffer)		return ret;
							flagBufferDelete = true;	}
		ControlInquiry::AddUnconnectedParticipant   addUnconnectedParticipant;
		memset( (void *)&addUnconnectedParticipant, 0, sizeof(addUnconnectedParticipant) );
		addUnconnectedParticipant.cmd = vs_sr_add_unconnected_participant;
		strcpy( addUnconnectedParticipant.conferenceName, conferenceName );
		strcpy( addUnconnectedParticipant.participantName, participantName );
		addUnconnectedParticipant.sndBuffer = sndBuffer;
		addUnconnectedParticipant.flagBufferDelete = flagBufferDelete;
		addUnconnectedParticipant.formLogs = formLogs;
		addUnconnectedParticipant.maxSilenceMs = maxSilenceMs;
		ControlResponse::CntrlResponse   cntrlResponse;		memset( (void *)&cntrlResponse, 0, sizeof(cntrlResponse) );
		if (CntrlSR( (ControlInquiry *)&addUnconnectedParticipant, sizeof(addUnconnectedParticipant), (ControlResponse *)&cntrlResponse, sizeof(cntrlResponse) ))
			ret = cntrlResponse.res;
		if (!ret && flagBufferDelete)	delete sndBuffer;
		return ret;
	}
	// end VS_StreamsRouter_Implementation::AddParticipant

	void RemoveParticipant(string_view conferenceName, string_view participantName) override
	{
		if (conferenceName.empty() || conferenceName.size() > VS_STREAMS_MAX_SIZE_CONFERENCE_NAME
			|| participantName.empty() || participantName.size() > VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME)
			return;
		ControlInquiry::RemoveParticipant   removeParticipant;
		memset( (void *)&removeParticipant, 0, sizeof(removeParticipant) );
		removeParticipant.cmd = vs_sr_remove_participant;
		strcpy (removeParticipant.conferenceName, conferenceName );
		strcpy (removeParticipant.participantName, participantName );
		CntrlSR( (ControlInquiry *)&removeParticipant, sizeof(removeParticipant) );
	}
	// end VS_StreamsRouter_Implementation::RemoveParticipant

	inline bool ConnectSetParticipant(VS_SR_Cmd cmd, string_view conferenceName, string_view participantName, string_view connectedParticipantName, const stream::Track* tracks, unsigned nTracks)
	{
		unsigned char   ftracks[256] = { 0 };
		if (conferenceName.empty() || conferenceName.size() > VS_STREAMS_MAX_SIZE_CONFERENCE_NAME
			|| participantName.empty() || participantName.size() > VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME
			|| connectedParticipantName.empty() || connectedParticipantName.size() > VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME
			|| !stream::TracksToFTracks(ftracks, tracks, nTracks))
			return false;
		bool   ret = false;
		ControlInquiry::ConnectParticipant   connectParticipant;
		memset( (void *)&connectParticipant, 0, sizeof(connectParticipant) );
		connectParticipant.cmd = cmd;
		strcpy( connectParticipant.conferenceName, conferenceName );
		strcpy( connectParticipant.participantName, participantName );
		strcpy( connectParticipant.connectedParticipantName, connectedParticipantName );
		stream::FTracksToMTracks(connectParticipant.mtracks, ftracks);
		ControlResponse::CntrlResponse   cntrlResponse;	memset( (void *)&cntrlResponse, 0, sizeof(cntrlResponse) );
		if (CntrlSR( (ControlInquiry *)&connectParticipant, sizeof(connectParticipant), (ControlResponse *)&cntrlResponse, sizeof(cntrlResponse) ))
			ret = cntrlResponse.res;
		return ret;
	}
	// end VS_StreamsRouter_Implementation::ConnectParticipant

	bool ConnectParticipant(string_view conferenceName, string_view participantName, string_view connectedParticipantName, const stream::Track* tracks, unsigned nTracks) override
	{	return ConnectSetParticipant( vs_sr_connect_participant, conferenceName, participantName, connectedParticipantName, tracks, nTracks );		}
	// end VS_StreamsRouter_Implementation::ConnectParticipant

	bool ConnectParticipantReceiver(string_view conferenceName, string_view participantName, string_view connectedParticipantName, const stream::Track* tracks, unsigned nTracks) override
	// NOTE: Meaning of 'receiver' and 'sender' is swapped in the implementation.
	{	return ConnectSetParticipant( vs_sr_connect_participant_sender, conferenceName, participantName, connectedParticipantName, tracks, nTracks );	}
	// end VS_StreamsRouter_Implementation::ConnectParticipantReceiver

	bool ConnectParticipantSender(string_view conferenceName, string_view participantName, string_view connectedParticipantName, const stream::Track* tracks, unsigned nTracks) override
	// NOTE: Meaning of 'receiver' and 'sender' is swapped in the implementation.
	{	return ConnectSetParticipant( vs_sr_connect_participant_receiver, conferenceName, participantName, connectedParticipantName, tracks, nTracks );	}
	// end VS_StreamsRouter_Implementation::ConnectParticipantSender

	inline bool DisconnectParticipant(VS_SR_Cmd cmd, string_view conferenceName, string_view participantName)
	{
		if (conferenceName.empty() || conferenceName.size() > VS_STREAMS_MAX_SIZE_CONFERENCE_NAME
			|| participantName.empty() || participantName.size() > VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME)
			return false;
		bool   ret = false;
		ControlInquiry::DisconnectParticipant   disconnectParticipant;
		memset( (void *)&disconnectParticipant, 0, sizeof(disconnectParticipant) );
		disconnectParticipant.cmd = cmd;
		strcpy( disconnectParticipant.conferenceName, conferenceName );
		strcpy( disconnectParticipant.participantName, participantName );
		ControlResponse::CntrlResponse   cntrlResponse;		memset( (void *)&cntrlResponse, 0, sizeof(cntrlResponse) );
		if (CntrlSR( (ControlInquiry *)&disconnectParticipant, sizeof(disconnectParticipant), (ControlResponse *)&cntrlResponse, sizeof(cntrlResponse) ))
			ret = cntrlResponse.res;
		return ret;
	}
	// end VS_StreamsRouter_Implementation::DisconnectParticipant

	bool SetParticipantTracks(string_view conferenceName, string_view participantName, string_view connectedParticipantName, const stream::Track* tracks, unsigned nTracks) override
	{	return ConnectSetParticipant( vs_sr_set_participant_tracks, conferenceName, participantName, connectedParticipantName, tracks, nTracks );	}
	// end VS_StreamsRouter_Implementation::SetParticipantTracks

	bool SetParticipantReceiverTracks(string_view conferenceName, string_view participantName, string_view connectedParticipantName, const stream::Track* tracks, unsigned nTracks) override
	// NOTE: Meaning of 'receiver' and 'sender' is swapped in the implementation.
	{	return ConnectSetParticipant( vs_sr_set_participant_sender_tracks, conferenceName, participantName, connectedParticipantName, tracks, nTracks );	}
	// end VS_StreamsRouter_Implementation::SetParticipantReceiverTracks

	bool SetParticipantSenderTracks(string_view conferenceName, string_view participantName, string_view connectedParticipantName, const stream::Track* tracks, unsigned nTracks) override
	// NOTE: Meaning of 'receiver' and 'sender' is swapped in the implementation.
	{	return ConnectSetParticipant( vs_sr_set_participant_receiver_tracks, conferenceName, participantName, connectedParticipantName, tracks, nTracks );	}
	// end VS_StreamsRouter_Implementation::SetParticipantSenderTracks

	bool DisconnectParticipant(string_view conferenceName, string_view participantName) override
	{	return DisconnectParticipant( vs_sr_disconnect_participant, conferenceName, participantName );	}
	// end VS_StreamsRouter_Implementation::DisconnectParticipant

	inline bool DisconnectParticipant(VS_SR_Cmd cmd, string_view conferenceName, string_view participantName, string_view connectedParticipantName)
	{
		if (conferenceName.empty() || conferenceName.size() > VS_STREAMS_MAX_SIZE_CONFERENCE_NAME
			|| participantName.empty() || participantName.size() > VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME
			|| connectedParticipantName.empty() || connectedParticipantName.size() > VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME)
			return false;
		bool   ret = false;
		ControlInquiry::DisconnectParticipantConcrete   disconnectParticipantConcrete;
		memset( (void *)&disconnectParticipantConcrete, 0, sizeof(disconnectParticipantConcrete) );
		disconnectParticipantConcrete.cmd = cmd;
		strcpy( disconnectParticipantConcrete.conferenceName, conferenceName );
		strcpy( disconnectParticipantConcrete.participantName, participantName );
		strcpy( disconnectParticipantConcrete.connectedParticipantName, connectedParticipantName );
		ControlResponse::CntrlResponse   cntrlResponse;		memset( (void *)&cntrlResponse, 0, sizeof(cntrlResponse) );
		if (CntrlSR( (ControlInquiry *)&disconnectParticipantConcrete, sizeof(disconnectParticipantConcrete), (ControlResponse *)&cntrlResponse, sizeof(cntrlResponse) ))
			ret = cntrlResponse.res;
		return ret;
	}
	// end VS_StreamsRouter_Implementation::DisconnectParticipant

	bool DisconnectParticipant(string_view conferenceName, string_view participantName, string_view connectedParticipantName) override
	{	return DisconnectParticipant( vs_sr_disconnect_participant_concrete, conferenceName, participantName, connectedParticipantName );	}
	// end VS_StreamsRouter_Implementation::DisconnectParticipant

	bool DisconnectParticipantReceiver(string_view conferenceName, string_view participantName) override
	// NOTE: Meaning of 'receiver' and 'sender' is swapped in the implementation.
	{	return DisconnectParticipant( vs_sr_disconnect_participant_sender, conferenceName, participantName );	}
	// end VS_StreamsRouter_Implementation::DisconnectParticipantReceiver

	bool DisconnectParticipantReceiver(string_view conferenceName, string_view participantName, string_view connectedParticipantName) override
	// NOTE: Meaning of 'receiver' and 'sender' is swapped in the implementation.
	{	return DisconnectParticipant( vs_sr_disconnect_participant_sender_concrete, conferenceName, participantName, connectedParticipantName );	}
	// end VS_StreamsRouter_Implementation::DisconnectParticipantReceiver

	bool DisconnectParticipantSender(string_view conferenceName, string_view participantName) override
	// NOTE: Meaning of 'receiver' and 'sender' is swapped in the implementation.
	{	return DisconnectParticipant( vs_sr_disconnect_participant_receiver, conferenceName, participantName );		}
	// end VS_StreamsRouter_Implementation::DisconnectParticipantSender

	bool DisconnectParticipantSender(string_view conferenceName, string_view participantName, string_view connectedParticipantName) override
	// NOTE: Meaning of 'receiver' and 'sender' is swapped in the implementation.
	{	return DisconnectParticipant( vs_sr_disconnect_participant_receiver_concrete, conferenceName, participantName, connectedParticipantName );	}
	// end VS_StreamsRouter_Implementation::DisconnectParticipantSender

	inline void SetConnection( const char *conferenceName, const char *participantName,
									const char *connectedParticipantName,
									const unsigned type, VS_ConnectionSock *conn,
									const unsigned char *mtracks )
	{
		if (!conferenceName || !*conferenceName
				|| strlen( conferenceName ) > VS_STREAMS_MAX_SIZE_CONFERENCE_NAME
				|| !participantName || !*participantName
				|| strlen( participantName ) > VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME
				|| (connectedParticipantName
					&& strlen( connectedParticipantName ) > VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME)
				|| (type != VS_ACS_LIB_SENDER && type != VS_ACS_LIB_RECEIVER)
				|| (mtracks && (*mtracks & 1)) || !conn)
		{	if (conn)	DeleteConn( conn );		return;		}
		ControlInquiry::SetConnection   setConnection;
		memset( (void *)&setConnection, 0, sizeof(setConnection) );
		setConnection.cmd = vs_sr_set_connection;
		strcpy( setConnection.conferenceName, conferenceName );
		strcpy( setConnection.participantName, participantName );
		strcpy( setConnection.connectedParticipantName, connectedParticipantName );
		setConnection.type = type;		setConnection.conn = conn;
		if (!mtracks) {		memset( (void *)setConnection.mtracks, 0xFF, sizeof(setConnection.mtracks) );
							*setConnection.mtracks = 0xFE;		}
		else	memcpy( (void *)setConnection.mtracks, (const void *)mtracks, sizeof(setConnection.mtracks) );
		CntrlSR( (ControlInquiry *)&setConnection, sizeof(setConnection) );
	}
	// end VS_StreamsRouter_Implementation::SetConnection

	bool GetStatistics(stream::RouterStatistics& stat) override
	{
		bool   ret = false;
		ControlInquiry::CntrlInquiry   cntrlInquiry;
		memset( (void *)&cntrlInquiry, 0, sizeof(cntrlInquiry) );
		cntrlInquiry.cmd = vs_sr_get_statistics;
		ControlResponse::GetStatistics   getStatistics;
		memset( (void *)&getStatistics, 0, sizeof(getStatistics) );
		if (CntrlSR( (ControlInquiry *)&cntrlInquiry, sizeof(cntrlInquiry), (ControlResponse *)&getStatistics, sizeof(getStatistics) ))
		{	if (getStatistics.res)
			{
				stat = getStatistics.stat;
				ret = true;
		}	}
		return ret;
	}
	// end VS_StreamsRouter_Implementation::GetStatistics

	bool SetParticipantCaps(string_view conferenceName, string_view participantName, const void* caps_buf, size_t buf_sz)
	{
		if (conferenceName.empty() || conferenceName.size() > VS_STREAMS_MAX_SIZE_CONFERENCE_NAME
			|| participantName.empty() || participantName.size() > VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME
			|| buf_sz == 0)
			return false;
		ControlInquiry::SetParticipantCaps setPartCaps;
		memset(&setPartCaps,0,sizeof(setPartCaps));
		setPartCaps.cmd = vs_sr_set_participant_caps;
		strcpy(setPartCaps.conferenceName,conferenceName);
		strcpy(setPartCaps.participantName,participantName);
		setPartCaps.caps_buf = caps_buf;
		setPartCaps.buf_sz = buf_sz;

		ControlResponse::CntrlResponse response;
		memset(&response,0,sizeof(response));
		bool ret(false);
		if(CntrlSR( (ControlInquiry*)&setPartCaps,sizeof(setPartCaps),(ControlResponse*)&response,sizeof(response)))
			ret = response.res;
		return ret;
	}

	boost::signals2::connection ConnectToFrameSink(string_view conferenceName, const stream::FrameReceivedSignalType::slot_type& slot) override
	{
		boost::signals2::connection res;
		if (conferenceName.empty() || conferenceName.size() > VS_STREAMS_MAX_SIZE_CONFERENCE_NAME)
			return res;
		if(threadId == GetCurrentThreadId())
		{
			unsigned long index = GetConferenceIndex(std::string(conferenceName).c_str());
			if(index!=-1)
				res = conf[index]->m_fireFrameReceived.connect(slot);
			return res;
		}

		ConnectToSinkParams params(slot);

		ControlInquiry::ConnectToFrameSink	connectToFrameSink;
		memset(&connectToFrameSink,0,sizeof(connectToFrameSink));
		connectToFrameSink.cmd = vs_sr_connect_to_frame_sink;
		strcpy(connectToFrameSink.conferenceName, conferenceName);
		connectToFrameSink.params = &params;
		ControlResponse::CntrlResponse response;
		memset(&response,0,sizeof(response));
		if(CntrlSR( (ControlInquiry*)&connectToFrameSink,sizeof(connectToFrameSink),(ControlResponse*)&response,sizeof(response)))
		{
			if(response.res)
				res = params.res_conn;
		}
		return res;
	}

	void SetParticipantSystemLoad(std::vector<stream::ParticipantLoadInfo>&& load) override
	{
		if (load.size() == 0)
			return;
		ControlInquiry::SetSystemLoadParticipant   sysLoadParticipant;
		memset( (void *)&sysLoadParticipant, 0, sizeof(sysLoadParticipant) );
		sysLoadParticipant.cmd = vs_sr_systemload_participant;
		sysLoadParticipant.load = new std::vector<stream::ParticipantLoadInfo>(std::move(load));
		CntrlSR( (ControlInquiry *)&sysLoadParticipant, sizeof(sysLoadParticipant) );
	}

	void SetParticipantFrameSizeMB(const char* conferenceName, const char* participantNameTo, std::vector<stream::ParticipantFrameSizeInfo>&& mb) override
	{
		if (!conferenceName || !*conferenceName || strlen( conferenceName ) > VS_STREAMS_MAX_SIZE_CONFERENCE_NAME
				|| !participantNameTo || !*participantNameTo || strlen( participantNameTo ) > VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME)
			return;
		ControlInquiry::SetFrameSizeMBParticipant   frameSizeMBParticipant;
		memset( (void *)&frameSizeMBParticipant, 0, sizeof(frameSizeMBParticipant) );
		frameSizeMBParticipant.cmd = vs_sr_framesizemb_participant;
		strcpy (frameSizeMBParticipant.conferenceName, conferenceName );
		strcpy (frameSizeMBParticipant.participantNameTo, participantNameTo );
		frameSizeMBParticipant.mb = new std::vector<stream::ParticipantFrameSizeInfo>(std::move(mb));
		CntrlSR( (ControlInquiry *)&frameSizeMBParticipant, sizeof(frameSizeMBParticipant) );
	}
	// end VS_StreamsRouter_Implementation::SetParticipantFrameMBps

};
// end VS_StreamsRouter_Implementation struct

//////////////////////////////////////////////////////////////////////////////////////////

VS_StreamsRouter* VS_StreamsRouter::Create()
{
	return new VS_StreamsRouter_Implementation;
}

#endif
