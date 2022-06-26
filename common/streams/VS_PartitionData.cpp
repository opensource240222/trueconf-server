#include "VS_PartitionData.h"

VS_PartitionData::VS_PartitionData()
{
	m_size = 0;
	m_is_resending = 1;
	m_id_packet = 0;
	m_id_frame = 0;
	m_is_last = 0;
	m_type_data = 0;
	m_priority = 0;
	m_rtime = 0;
}

VS_PartitionData::~VS_PartitionData()
{
	m_size = 0;
	m_is_resending = 1;
	m_id_packet = 0;
	m_id_frame = 0;
	m_is_last = 0;
	m_type_data = 0;
	m_type_frame = 0;
	m_time_stamp = 0;
	m_priority = 0;
	m_rtime = 0;
}

void VS_PartitionData::SetData(unsigned char *in, int size)
{
	m_data.Set(in, size);
	m_size = m_data.Size();
}

void VS_PartitionData::SetAttribute(int is_resending, int id_packet, int id_frame, int is_last, int type_data,
									int type_frame, int time_stamp, int priority)
{
	m_is_resending = is_resending;
	m_id_packet = id_packet;
	m_id_frame = id_frame;
	m_is_last = is_last;
	m_type_data = type_data;
	m_type_frame = type_frame;
	m_time_stamp = time_stamp;
	m_priority = priority;
}

void VS_PartitionData::DecPriority()
{
	m_priority--;
	if (m_priority < 1) m_priority = 1;
}

void VS_PartitionData::IncResend()
{
	m_is_resending++;
	VS_NhpFirstHeader* h = (VS_NhpFirstHeader*)(m_data.Buffer());
	if (h) h->resendNum = m_is_resending;
}