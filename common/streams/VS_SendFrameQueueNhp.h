#pragma once

#include <list>
#include "VS_SendFrameQueueBase.h"
#include "VS_PartitionData.h"

#define QKEY_TIME_INTERVAL			(2500)
#define QBITRATE_TIME_INTERVAL		(1000)
#define QLAG_TIME_INTERVAL			(5000)
#define BITRATE_MIN					(20)
#define	STUFF_DELAY					(15)

class VS_SendFrameQueueNhp : public VS_SendFrameQueueBase
{
	int						m_resending, m_queue_lenght;
	unsigned char			m_id_frame;
	unsigned short			*m_id_packet, m_id_cpacket, m_id_apacket, m_id_vpacket, m_id_dpacket;
	VS_PartitionData		m_nullpartdata;
	std::list<VS_PartitionData>  m_preparedata_l, m_senddata_l, m_tmppartdata_l;

	bool					CuttingOtherFrame(unsigned char *pBuffer, int size, int track, unsigned int time_stamp,
		unsigned char type_frame, int iPriority);
	bool					CuttingRTPFrame(unsigned char *pBuffer, int size, int track, unsigned int time_stamp,
		unsigned char type_frame, int iPriority);
	bool					PushPartData(std::list<VS_PartitionData>& partdata, int iPriority);
	int						EncodePacketHeader(VS_PartitionData partdata, unsigned char *header);
	void					CreatePacket(VS_PartitionData partdata, unsigned char* ptr_frame, unsigned char* ptr_packet, int &sizep);
	void					AnalisysResendQueue(unsigned int time);
public:
	VS_SendFrameQueueNhp(bool useSVC);
	~VS_SendFrameQueueNhp();
	bool					AddFrame(int track, int size, unsigned char *pBuffer, int iPriority) override;
	bool					AddFrame(int track, int size, unsigned char *pBuffer, int iPriority, const FrameQueueInfo &info, const stream::SVCHeader *h) override;
	int						GetFrame(unsigned char* &pBuffer, int &size, unsigned char &track, unsigned char &slayer) override;
	int						GetSize() override;
	int						GetBytes() override;
	bool					MarkFirstAsSend() override;
	void					EraseMedia() override;
	void					EraseVideo() override;
	void					EraseAudio() override;
	bool					ResendPackect(unsigned short SeqId, int dataType);
	void					SetTimeLenth(int msec);
};
