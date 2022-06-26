#include "VS_SendFrameQueueNhp.h"
#include "../std/cpplib/VS_RegistryKey.h"
#include "Command.h"

#include <algorithm>
#include <memory>

typedef std::list<VS_PartitionData>::iterator iterPartData;

class Greater
{
	int m_priority;
public:
	Greater(int priority) {m_priority = priority;};
	bool operator () (VS_PartitionData& partdata)
	{
		return (partdata.priority() > m_priority);
	};
};

class FindPacket
{
	unsigned short	m_id_packet;
	int				m_type_data;
public:
	FindPacket(int SeqId, int typeData) {m_id_packet = SeqId, m_type_data = typeData;};
	bool operator () (VS_PartitionData& partdata)
	{
		return (partdata.id_packet() == m_id_packet && partdata.type_data() == m_type_data);
	};
};

bool Remove(VS_PartitionData partdata)
{
	return (partdata.type_data() == NHPH_DT_AUDIO || partdata.type_data() == NHPH_DT_VIDEO);
}

bool RemoveAudio(VS_PartitionData partdata)
{
	return (partdata.type_data() == NHPH_DT_AUDIO);
}

bool RemoveVideo(VS_PartitionData partdata)
{
	return (partdata.type_data() == NHPH_DT_VIDEO);
}

/**
 **************************************************************************
 * \brief Constructor
 ****************************************************************************/
VS_SendFrameQueueNhp::VS_SendFrameQueueNhp(bool useSVC)
{
	m_queue_lenght = QKEY_TIME_INTERVAL * 3;
	m_id_frame = 0;
	m_resending = 0;
	m_id_cpacket = 0;
	m_id_apacket = 0;
	m_id_vpacket = 0;
	m_id_dpacket = 0;
	memset(&m_nullpartdata, 0, sizeof(VS_PartitionData));

	SetLimitSize(VIDEODATASIZE_DEFAULT);
}

/**
 **************************************************************************
 * \brief Destructor
 ****************************************************************************/
VS_SendFrameQueueNhp::~VS_SendFrameQueueNhp()
{
	m_queue_lenght = 0;
	m_id_frame = 0;
	m_resending = 0;
	m_id_cpacket = 0;
	m_id_apacket = 0;
	m_id_vpacket = 0;
	m_id_dpacket = 0;
	m_senddata_l.clear();
	m_preparedata_l.clear();
	m_tmppartdata_l.clear();
}

/**
 **************************************************************************
 * \brief Copy reference to first packet from send queue, return queue size
 ****************************************************************************/
int VS_SendFrameQueueNhp::GetFrame(unsigned char* &pBuffer, int &size, unsigned char &track, unsigned char &slayer)
{
	int lenght = m_preparedata_l.size();
	if (lenght > 0) {
		size = m_preparedata_l.front().GetSize();
		track = m_preparedata_l.front().type_data();
		pBuffer = m_preparedata_l.front().GetData();
		//DTRACE(VSTM_NHP_SND, "get packet : type = %d (%d, %d, %d, %d, %d), l = %d",
		//		track, m_preparedata_l.front().is_resending(), m_preparedata_l.front().priority(), m_preparedata_l.front().id_packet(),
		//		m_preparedata_l.front().id_frame(), m_preparedata_l.front().is_last(), size);
	}
	slayer = 0;
	return lenght;
}

/**
 **************************************************************************
 * \brief Return queue size
 ****************************************************************************/
int	VS_SendFrameQueueNhp::GetSize()
{
	return m_preparedata_l.size();
}

 int VS_SendFrameQueueNhp::GetBytes()
 {
	 return 0;
 }

/**
 **************************************************************************
 * \brief Erase first packet from send queue, put it to resend queue
 ****************************************************************************/
bool VS_SendFrameQueueNhp::MarkFirstAsSend()
{
	bool ret = false;
	if (!m_preparedata_l.empty()) {
		AnalisysResendQueue(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
		if (m_preparedata_l.front().is_resending() < 3) {
			m_preparedata_l.front().SetResendTime(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
			m_senddata_l.push_back(m_preparedata_l.front());
		}
		ret = ((m_preparedata_l.front().is_last() == NHPH_TP_LAST || m_preparedata_l.front().is_last() == NHPH_TP_ONEONLY) &&
				m_preparedata_l.front().is_resending() == 0);

		//DTRACE(VSTM_NHP_SND, "mark as first packet : ret = %d type = %d (%d, %d, %d, %d, %d), l = %d",
		//		ret, m_preparedata_l.front().type_data(), m_preparedata_l.front().is_resending(), m_preparedata_l.front().priority(), m_preparedata_l.front().id_packet(),
		//		m_preparedata_l.front().id_frame(), m_preparedata_l.front().is_last(), m_preparedata_l.front().GetSize());

		m_preparedata_l.pop_front();
	}
	return ret;
}

/**
 **************************************************************************
 * \brief Erase all audio and video packets
 ****************************************************************************/
void VS_SendFrameQueueNhp::EraseMedia()
{
	if (!m_preparedata_l.empty()) m_preparedata_l.remove_if(Remove);
	if (!m_senddata_l.empty()) m_senddata_l.remove_if(Remove);
}

/**
 **************************************************************************
 * \brief Erase all audio and video packets
 ****************************************************************************/
void VS_SendFrameQueueNhp::EraseVideo()
{
	if (!m_preparedata_l.empty()) m_preparedata_l.remove_if(RemoveVideo);
	if (!m_senddata_l.empty()) m_senddata_l.remove_if(RemoveVideo);
}

/**
 **************************************************************************
 * \brief Erase all audio  packets
 ****************************************************************************/
void VS_SendFrameQueueNhp::EraseAudio()
{
	if (!m_preparedata_l.empty()) m_preparedata_l.remove_if(RemoveAudio);
	if (!m_senddata_l.empty()) m_senddata_l.remove_if(RemoveAudio);
}

/**
 **************************************************************************
 * \brief Move packet from receiver queue to send queue, decreasing
 *		  it iPriority value
 ****************************************************************************/
bool VS_SendFrameQueueNhp::ResendPackect(unsigned short SeqId, int dataType)
{
	bool ret = false;
	iterPartData part_iter = find_if(m_senddata_l.begin(), m_senddata_l.end(), FindPacket(SeqId, dataType));
	if (part_iter != m_senddata_l.end()) {
		part_iter->IncResend();
		part_iter->DecPriority();
		std::list<VS_PartitionData> tmplist(1, *part_iter);
		ret = PushPartData(tmplist, part_iter->priority());

		//DTRACE(VSTM_NHP_SND, "resend packet : ret = %d type = %d (%d, %d, %d, %d, %d), l = %d",
		//		ret, dataType, part_iter->is_resending(), part_iter->priority(), SeqId, part_iter->id_frame(), part_iter->is_last(), part_iter->GetSize());
	}
	return ret;
}

/**
 **************************************************************************
 * \brief Set time-to-live for packet in resend queue
 ****************************************************************************/
void VS_SendFrameQueueNhp::SetTimeLenth(int msec)
{
	if (msec >= 0) {
		m_queue_lenght = msec;
	}
}

/**
 **************************************************************************
 * \brief Cutting frame and add packets in queue with priority,
 *		  return 'true' if successfully
 ****************************************************************************/
bool VS_SendFrameQueueNhp::AddFrame(int track, int size, unsigned char *pBuffer, int iPriority)
{
	stream::Command cmd;
	if (track==255 && size==1) {
		cmd.RequestKeyFrame();
		pBuffer = reinterpret_cast<unsigned char*>(&cmd);
		size = cmd.Size();
	}

	bool ret = false;
	unsigned char type_frame = 0, *pbuff = pBuffer;
	unsigned int time_stamp = 0, dsize = 0;
	int nhp_dt = 0;

	switch (track)
	{
	case 0x01:
		{
			VS_NhpAudioHeader ah;
			memcpy(&ah, pBuffer, sizeof(VS_NhpAudioHeader));
			time_stamp = ah.TimeStamp;
			nhp_dt = NHPH_DT_AUDIO;
			m_id_packet = &m_id_apacket;
			dsize = sizeof(VS_NhpAudioHeader);
			break;
		}
	case 0x02:
		{
			VS_NhpVideoHeader vh;
			memcpy(&vh, pBuffer, sizeof(VS_NhpVideoHeader));
			time_stamp = vh.TimeStamp;
			type_frame = vh.FrameType;
			m_id_frame = vh.FrameId;
			m_id_packet = &m_id_vpacket;
			nhp_dt = NHPH_DT_VIDEO;
			dsize = sizeof(VS_NhpVideoHeader);
			break;
		}
	case 0x05:
		{
			m_id_packet = &m_id_dpacket;
			nhp_dt = NHPH_DT_DATA;
			break;
		}
	case 0xfe: case 0xff:
		{
			m_id_packet = &m_id_cpacket;
			nhp_dt = NHPH_DT_CMND;
			break;
		}
	default:
		{
			nhp_dt = -1;
			return ret;
		}
	}

	pbuff = pBuffer + dsize;
	size -= dsize;

	if (size >= 0) {
		ret = CuttingOtherFrame(pbuff, size, nhp_dt, time_stamp, type_frame, iPriority);
	}

/*	if (size >= 0) {
		if (nhp_dt == NHPH_DT_VIDEO) {
			ret = CuttingRTPFrame(pbuff, size, nhp_dt, time_stamp, type_frame, iPriority);
		} else {
			ret = CuttingOtherFrame(pbuff, size, nhp_dt, time_stamp, type_frame, iPriority);
		}
	}*/

	//DTRACE(VSTM_NHP_SND, "add frame to queue : ret = %d, t = %u, type = %d (%d, %d), l = %d, ct = %u",
	//	   ret, time_stamp, nhp_dt, (nhp_dt == NHPH_DT_VIDEO) ? type_frame : 0, (nhp_dt == NHPH_DT_VIDEO) ? m_id_frame : 0, size, timeGetTime());

	return ret;
}

bool VS_SendFrameQueueNhp::AddFrame(int track, int size, unsigned char *pBuffer, int iPriority, const FrameQueueInfo &info, const stream::SVCHeader *h)
{
	stream::Command cmd;
	if (track == 255 && size == 1) {
		cmd.RequestKeyFrame();
		pBuffer = reinterpret_cast<unsigned char*>(&cmd);
		size = cmd.Size();
	}

	bool ret = false;
	int nhp_dt = 0;

	switch (track) {
	case 0x01:
	{
		nhp_dt = NHPH_DT_AUDIO;
		m_id_packet = &m_id_apacket;
		break;
	}
	case 0x02:
	{
		m_id_frame = info.counter;
		m_id_packet = &m_id_vpacket;
		nhp_dt = NHPH_DT_VIDEO;
		break;
	}
	case 0x05:
	{
		m_id_packet = &m_id_dpacket;
		nhp_dt = NHPH_DT_DATA;
		break;
	}
	case 0xfe: case 0xff:
	{
		m_id_packet = &m_id_cpacket;
		nhp_dt = NHPH_DT_CMND;
		break;
	}
	default:
	{
		nhp_dt = -1;
		return ret;
	}
	}

	if (size >= 0) {
		ret = CuttingOtherFrame(pBuffer, size, nhp_dt, info.timestamp, info.type, iPriority);
	}

	return ret;
}

/**
 **************************************************************************
 * \brief Encode packet header
 ****************************************************************************/
int VS_SendFrameQueueNhp::EncodePacketHeader(VS_PartitionData partdata, unsigned char *header)
{
	int size_header = 0;
	unsigned char *ptr = header;

	VS_NhpFirstHeader fh;
	memset(&fh, 0, sizeof(VS_NhpFirstHeader));
	fh.version = NHPH_VER_ALFA;
	fh.dataType = partdata.type_data();
	fh.resendNum = partdata.is_resending();
	fh.typeOfPart = partdata.is_last();
	fh.SeqId = partdata.id_packet();
	fh.DataFormat = 0;
	memcpy(ptr, &fh, sizeof(VS_NhpFirstHeader));
	size_header += sizeof(VS_NhpFirstHeader);
	ptr += sizeof(VS_NhpFirstHeader);

	switch (fh.dataType)
	{
	case NHPH_DT_CMND:
		{
			break;
		}
	case NHPH_DT_AUDIO:
		{
			VS_NhpAudioHeader *ah = (VS_NhpAudioHeader*)ptr;
			ah->TimeStamp = partdata.time_stamp();
			size_header += sizeof(VS_NhpAudioHeader);
			break;
		}
	case NHPH_DT_VIDEO:
		{
			VS_NhpVideoHeader *vh = (VS_NhpVideoHeader*)ptr;
			vh->FrameType = partdata.type_frame();
			vh->FrameId = partdata.id_frame();
			vh->TimeStamp = partdata.time_stamp();
			size_header += sizeof(VS_NhpVideoHeader);
			break;
		}
	case NHPH_DT_DATA:
		{
			break;
		}
	default:
		{
			size_header = -1;
			break;
		}
	}

	return size_header;
}

/**
 **************************************************************************
 * \brief Create packet from data
 ****************************************************************************/
void VS_SendFrameQueueNhp::CreatePacket(VS_PartitionData partdata, unsigned char* ptr_frame, unsigned char* ptr_packet, int &sizep)
{
	int size_data = sizep;
	sizep = 0;
	if (ptr_packet) {
		sizep = EncodePacketHeader(partdata, ptr_packet);
		if (sizep > 0) {
			memcpy(ptr_packet + sizep, ptr_frame, size_data);
			sizep += size_data;
		}
	}
}

/**
 **************************************************************************
 * \brief Add packet information in queue
 ****************************************************************************/
bool VS_SendFrameQueueNhp::PushPartData(std::list<VS_PartitionData>& partdata, int iPriority)
{
	bool ret = false;
	iterPartData part_iter = find_if(m_preparedata_l.begin(), m_preparedata_l.end(), Greater(iPriority));
	m_preparedata_l.splice(part_iter, partdata);
	ret = true;
	return ret;
}

/**
 **************************************************************************
 * \brief Cutting frame
 ****************************************************************************/
bool VS_SendFrameQueueNhp::CuttingOtherFrame(unsigned char *pBuffer, int size, int track, unsigned int time_stamp,
											 unsigned char type_frame, int iPriority)
{
	bool ret = false;
	int i = 0, j = 0;
	unsigned char is_last = NHPH_TP_LAST,
				  *packet = new unsigned char [size + sizeof(VS_NhpVideoHeader) + sizeof(VS_NhpFirstHeader)];
	int loop = 0, delta = 0, sizep = 0;

	unsigned char *ptr_fr = pBuffer;

	if (track == NHPH_DT_VIDEO) {
		loop = size / m_LimitSize;
		delta = size - loop * m_LimitSize;
		for (i = 0; i < loop; i++) {
			is_last = NHPH_TP_INBOUND;
			if (i == 0) {
				if ((loop == 1) && (delta == 0)) is_last = NHPH_TP_ONEONLY;
				else is_last = NHPH_TP_FIRST;
			} else {
				if (i == (loop - 1) && (delta == 0)) is_last = NHPH_TP_LAST;
			}
			m_tmppartdata_l.push_back(m_nullpartdata);
			m_tmppartdata_l.back().SetAttribute(0, *m_id_packet, m_id_frame, is_last, track, type_frame,
												time_stamp, iPriority);
			sizep = m_LimitSize;
			CreatePacket(m_tmppartdata_l.back(), ptr_fr, packet, sizep);
			m_tmppartdata_l.back().SetData(packet, sizep);
			ptr_fr += m_LimitSize;
			(*m_id_packet)++;
		}
		//if (delta) {
		if (delta || size == 0) {
			if (loop == 0) is_last = NHPH_TP_ONEONLY;
			else is_last = NHPH_TP_LAST;
			m_tmppartdata_l.push_back(m_nullpartdata);
			m_tmppartdata_l.back().SetAttribute(0, *m_id_packet, m_id_frame, is_last, track, type_frame,
												time_stamp, iPriority);
			sizep = delta;
			CreatePacket(m_tmppartdata_l.back(), ptr_fr, packet, sizep);
			m_tmppartdata_l.back().SetData(packet, sizep);
			(*m_id_packet)++;
		}
	} else {
		m_tmppartdata_l.push_back(m_nullpartdata);
		m_tmppartdata_l.back().SetAttribute(0, *m_id_packet, m_id_frame, is_last, track, type_frame,
											time_stamp, iPriority);
		sizep = size;
		CreatePacket(m_tmppartdata_l.back(), ptr_fr, packet, sizep);
		m_tmppartdata_l.back().SetData(packet, sizep);
		(*m_id_packet)++;
	}
	ret = PushPartData(m_tmppartdata_l, iPriority);
	if (packet) delete [] packet; packet = 0;

	return ret;
}

/**
 **************************************************************************
 * \brief Cutting H264 frame to RTP frames
 ****************************************************************************/
bool VS_SendFrameQueueNhp::CuttingRTPFrame(unsigned char *pBuffer, int size, int track, unsigned int time_stamp,
										   unsigned char type_frame, int iPriority)
{
	bool ret = false;
	unsigned char is_last = NHPH_TP_FIRST,
				  *packet = new unsigned char [size + sizeof(VS_NhpVideoHeader) + sizeof(VS_NhpFirstHeader)];
	int start_pos = -1, end_pos = -1, j = 0;
	unsigned long ind = 0;

	unsigned char *ptr_fr = pBuffer;

	if (size > 0) {
		auto out = std::make_unique<unsigned char[]>(size);
		while(is_last != NHPH_TP_LAST && is_last != NHPH_TP_ONEONLY) {
			for(; ind + 3 < (unsigned long)size; ind++){
				if(ptr_fr[ind] == 0 && ptr_fr[ind+1] == 0 && ptr_fr[ind+2] == 1) // startcode found
					break;
			}
			end_pos = ind;
			ind += 3;
			is_last = (ind == size) + 2 * (j == 0);
			if (start_pos == -1) {
				start_pos = ind;
				continue; // first startcode
			}
			if (is_last == NHPH_TP_LAST || is_last == NHPH_TP_ONEONLY)
				end_pos += 3;
			int nalSize = end_pos - start_pos;
			unsigned char *src = ptr_fr + start_pos;
			int i = 0;
			for (; i < nalSize - 2;i++) {
				/// SMirnovK: Polycom expect format
				//if (src[i] == 0 && src[i+1]==0 && src[i+2]==3) {
				//	out[i++] = 0;
				//	out[i++] = 0;
				//	src++;
				//	nalSize--;
				//}
				out[i] = src[i];
			}
			out[i] = src[i];
			i++;
			if (src[i])
			{
				out[i] = src[i];
				i++;
			}

			/// SMirnovK: Polycom not understand NALs type of 9
			if ((out[0] & 0x1f) != 9) {
				m_tmppartdata_l.push_back(m_nullpartdata);
				m_tmppartdata_l.back().SetAttribute(0, *m_id_packet, m_id_frame, is_last, track, type_frame,
													time_stamp, iPriority);
				CreatePacket(m_tmppartdata_l.back(), out.get(), packet, i);
				m_tmppartdata_l.back().SetData(packet, i);
				(*m_id_packet)++;
			}
			start_pos = ind;
			j++;
		}
	} else {
		int i = 0;
		m_tmppartdata_l.push_back(m_nullpartdata);
		m_tmppartdata_l.back().SetAttribute(0, *m_id_packet, m_id_frame, NHPH_TP_ONEONLY, track, type_frame,
											time_stamp, iPriority);
		CreatePacket(m_tmppartdata_l.back(), NULL, packet, i);
		m_tmppartdata_l.back().SetData(packet, i);
		(*m_id_packet)++;
	}
	ret = PushPartData(m_tmppartdata_l, iPriority);
	if (packet) delete [] packet; packet = 0;

	return ret;
}

/**
 **************************************************************************
 * \brief Live packet from resend queue decision
 ****************************************************************************/
void VS_SendFrameQueueNhp::AnalisysResendQueue(unsigned int time)
{
	unsigned int rtime = 0;
	while (!m_senddata_l.empty()) {
		rtime = m_senddata_l.front().time_resend();
		if (time - rtime < (unsigned int)m_queue_lenght) break;
		m_senddata_l.pop_front();
	}
}