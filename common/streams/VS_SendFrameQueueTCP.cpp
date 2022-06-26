#include "VS_SendFrameQueueTCP.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "Protocol.h"
#include "ParseSvcStream.h"
#include "assert.h"

#include <algorithm>
#include <chrono>

VS_SendFrameQueueTCP::VS_SendFrameQueueTCP(bool useSVC)
{
	m_queue.SetDataFactory(Factory, Destructor);
	m_queue.SetKeyFactory(VS_Map::Int64Factory, VS_Map::Int64Destructor);
	m_queue.SetPredicate(VS_Map::Int64Predicate);

	SetLimitSize(VIDEODATASIZE_MAX);

	m_PrevVideoTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
	memset(m_PrioritySeq, 0, sizeof(m_PrioritySeq));

	m_bEnableSVC = useSVC;
	m_QueueBytes = 0;
	m_widthBaseLayer = -1;
	m_heightBaseLayer = -1;
}

bool VS_SendFrameQueueTCP::AddFrame(int track, int Size, unsigned char *pBuffer, int iPriority)
{
	if (iPriority > FRAME_PRIORITY_SHARE || FRAME_PRIORITY_COMMAND > iPriority)
		return false;
	if (Size < 0)
		return false;
	int setSize = 0;
	if (track == 2) {
		unsigned char VideoFrameCounter = pBuffer[0];
		unsigned char frameType = pBuffer[1] == 0;
		uint32_t CurrVideoTime = *(uint32_t*)(pBuffer + 2);
		uint32_t DiffTime = CurrVideoTime - m_PrevVideoTime;
		m_PrevVideoTime = CurrVideoTime;
		if (DiffTime > 1000)
			DiffTime = 1000;
		pBuffer += 6;
		Size -= 6;
		int layerSize = 0;
		int shiftSize = 0;
		stream::SVCHeader svchdr;
		svchdr.spatial = 0;
		const auto svc_hdr_size = m_bEnableSVC ? sizeof(stream::SVCHeader) : 0u;
		while ((pBuffer = ParseSvcStream(pBuffer, Size, m_bEnableSVC, &layerSize, &svchdr, &shiftSize))) {
			*(unsigned char*)(pBuffer - 5) = frameType;
			*(uint32_t*)(pBuffer - 4) = DiffTime;
			int itSize = layerSize + 5;
			const unsigned n_slices = (itSize + m_LimitSize - 1) / m_LimitSize;
			assert(n_slices > 0);
			assert(n_slices <= 256);
			for (unsigned i = 0; i < n_slices; ++i) {
				int size = std::min(itSize, m_LimitSize);
				itSize -= size;
				VS_SentFrame sf;
				uint64_t seq = iPriority;
				seq = (seq << 32) | m_PrioritySeq[iPriority]++;
				sf.m_track = track;
				sf.m_slayer = svchdr.spatial;
				sf.m_Buff.SetSize(size + sizeof(stream::SliceHeader) + svc_hdr_size);
				unsigned char* cbuff = (unsigned char*)sf.m_Buff.Buffer();
				memcpy(cbuff, (pBuffer - 5) + m_LimitSize * i, size);
				const auto sh = reinterpret_cast<stream::SliceHeader*>(cbuff + size);
				sh->id = n_slices - 1 - i;
				sh->first_id = n_slices - 1;
				sh->frame_counter = VideoFrameCounter;
				sf.m_last = (sh->id == 0);
				memcpy(cbuff + size + sizeof(stream::SliceHeader), &svchdr, svc_hdr_size);
				m_queue.Insert(&seq, &sf);
				setSize += (size + sizeof(stream::SliceHeader) + svc_hdr_size);
			}
			pBuffer += layerSize;
			Size -= shiftSize;
		}
	} else {
		if (track == 1) {
			pBuffer += 4;
			Size -= 4;
		}
		VS_SentFrame sf;
		uint64_t seq = iPriority;
		seq = (seq << 32) | m_PrioritySeq[iPriority]++;
		sf.m_track = track;
		sf.m_Buff.Set(pBuffer, Size);
		m_queue.Insert(&seq, &sf);
		setSize = Size;
	}
	m_QueueBytes += setSize;
	return true;
}

bool VS_SendFrameQueueTCP::AddFrame(int track, int size, unsigned char *pBuffer, int iPriority, const FrameQueueInfo &info, const stream::SVCHeader *h)
{
	if (iPriority > FRAME_PRIORITY_SHARE || FRAME_PRIORITY_COMMAND > iPriority) {
		return false;
	}
	if (size < 0) {
		return false;
	}
	int32_t packetsBytes(0);
	if (track == 2) {
		const auto frame_info_hdr_size = sizeof(uint8_t) + sizeof(uint32_t); /* type frame + timestamp */
		const auto svc_hdr_size = h ? sizeof(stream::SVCHeader) : 0u;
		uint32_t diff = std::min<uint32_t>(1000, info.interval);
		m_PrevVideoTime = info.timestamp;
		int32_t lenght = size + frame_info_hdr_size;
		const unsigned n_slices = (lenght + m_LimitSize - 1) / m_LimitSize;
		uint8_t *buffer = pBuffer;
		for (unsigned i = 0; i < n_slices; ++i) {
			int32_t packetSize = std::min(lenght, m_LimitSize);
			int32_t packetSizeFull = packetSize + sizeof(stream::SliceHeader) + svc_hdr_size;
			lenght -= packetSize;
			packetsBytes += packetSizeFull;
			VS_SentFrame sf;
			uint64_t seq = iPriority;
			seq = (seq << 32) | m_PrioritySeq[iPriority]++;
			sf.m_track = track;
			sf.m_slayer = (h) ? h->spatial : 0;
			sf.m_Buff.SetSize(packetSizeFull);
			uint8_t *data = (uint8_t*) sf.m_Buff.Buffer();
			if (i == 0) {
				/// write in 1st packet : type frame + timestamp = 5 bytes
				*reinterpret_cast<uint8_t*>(data) = (info.type == 0);
				*reinterpret_cast<uint32_t*>(data + 1) = diff;
				data += frame_info_hdr_size;
				packetSize -= frame_info_hdr_size;
			}
			memcpy(data, buffer, packetSize);
			const auto sh = reinterpret_cast<stream::SliceHeader*>(data + packetSize);
			sh->id = n_slices - 1 - i;
			sh->first_id = n_slices - 1;
			sh->frame_counter = info.counter;
			sf.m_last = (sh->id == 0);
			memcpy(data + packetSize + sizeof(stream::SliceHeader), h, svc_hdr_size);
			m_queue.Insert(&seq, &sf);
			buffer += packetSize;
		}
	}
	else {
		VS_SentFrame sf;
		uint64_t seq = iPriority;
		seq = (seq << 32) | m_PrioritySeq[iPriority]++;
		sf.m_track = track;
		sf.m_Buff.Set(pBuffer, size);
		m_queue.Insert(&seq, &sf);
		packetsBytes = size;
	}
	m_QueueBytes += packetsBytes;
	return true;
}

int VS_SendFrameQueueTCP::GetFrame(unsigned char* &pBuffer, int &size, unsigned char &track, unsigned char &slayer)
{
	if (m_queue.Empty())
		return 0;
	VS_Map::Iterator i = m_queue.Begin();
	VS_SentFrame &frame = *(VS_SentFrame *)(*i).data;
	size = frame.m_Buff.Size();
	track = frame.m_track;
	slayer = frame.m_slayer;
	pBuffer = (unsigned char*)frame.m_Buff.Buffer();
	return m_queue.Size();
}

int VS_SendFrameQueueTCP::GetSize() {
	return m_queue.Size();
}

int VS_SendFrameQueueTCP::GetBytes() {
	return m_QueueBytes;
}

bool VS_SendFrameQueueTCP::MarkFirstAsSend() {
	if (m_queue.Empty())
		return false;
	VS_Map::Iterator i = m_queue.Begin();
	VS_SentFrame &frame = *(VS_SentFrame *)(*i).data;
	bool res = frame.m_track != 2 || frame.m_last;
	m_QueueBytes -= frame.m_Buff.Size();
	m_queue.Erase(m_queue.Begin());
	return res;
}

void VS_SendFrameQueueTCP::EraseMedia() {
	VS_Map::Iterator i = m_queue.Begin();
	while (i!=m_queue.End()) {
		VS_SentFrame *frame = (VS_SentFrame *)(*i).data;
		if (frame->m_track==1 || frame->m_track==2) {
			m_QueueBytes -= frame->m_Buff.Size();
			i = m_queue.Erase(i);
		} else {
			++i;
		}
	}
}

void VS_SendFrameQueueTCP::EraseVideo() {
	VS_Map::Iterator i = m_queue.Begin();
	while (i!=m_queue.End()) {
		VS_SentFrame *frame = (VS_SentFrame *)(*i).data;
		if (frame->m_track==2) {
			m_QueueBytes -= frame->m_Buff.Size();
			i = m_queue.Erase(i);
		} else {
			++i;
		}
	}
}

void VS_SendFrameQueueTCP::EraseAudio() {
	VS_Map::Iterator i = m_queue.Begin();
	while (i!=m_queue.End()) {
		VS_SentFrame *frame = (VS_SentFrame *)(*i).data;
		if (frame->m_track==1) {
			m_QueueBytes -= frame->m_Buff.Size();
			i = m_queue.Erase(i);
		} else {
			++i;
		}
	}
}
