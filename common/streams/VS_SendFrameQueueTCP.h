#pragma once

#include "std/cpplib/VS_Map.h"
#include "VS_SendFrameQueueBase.h"
#include "std-generic/cpplib/VS_Container.h"

class VS_SentFrame
{
public:
	unsigned char	m_track;
	unsigned char	m_slayer;
	bool			m_last;
	VS_BinBuff		m_Buff;
	VS_SentFrame() : m_track(0), m_slayer(0), m_last(false) {};
};

class VS_SendFrameQueueTCP : public VS_SendFrameQueueBase
{
	VS_Map			m_queue;			///< Sort and store packets
	uint32_t		m_PrevVideoTime;	///< Time of previous video frame
	unsigned short	m_PrioritySeq[256];	///< Counter for all priorities
	bool			m_bEnableSVC;
	int				m_QueueBytes;
	int				m_widthBaseLayer;
	int				m_heightBaseLayer;

	static void* Factory(const void* upd) {
		return new VS_SentFrame(*(VS_SentFrame*)upd);
	}

	static void Destructor(void* upd) {
		delete (VS_SentFrame*)upd;
	}

public:
	VS_SendFrameQueueTCP(bool useSVC);
	bool AddFrame(int track, int Size, unsigned char *pBuffer, int iPriority) override;
	bool AddFrame(int track, int size, unsigned char *pBuffer, int iPriority, const FrameQueueInfo &info, const stream::SVCHeader *h) override;
	int GetFrame(unsigned char* &pBuffer, int &size, unsigned char &track, unsigned char &slayer) override;
	int GetSize() override;
	int GetBytes() override;
	bool MarkFirstAsSend() override;
	void EraseMedia() override;
	void EraseVideo() override;
	void EraseAudio() override;
};
