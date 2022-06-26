#pragma once

#include "std-generic/cpplib/VS_Container.h"

#include "VSClient/VS_NhpHeaders.h"

class VS_PartitionData
{
	VS_BinBuff		m_data;
	int	m_size, m_is_resending, m_id_packet, m_id_frame, m_is_last, m_type_data, m_type_frame, m_priority;
	unsigned int	m_time_stamp, m_rtime;
public:
	VS_PartitionData();
	~VS_PartitionData();
	void SetData(unsigned char *in, int size);
	void SetAttribute(int is_resending, int id_packet, int id_frame, int is_last, int type_data = NHPH_DT_VIDEO,
		int type_frame = 0, int time_stamp = 0, int priority = 0);
	void SetResendTime(unsigned int time) {
		m_rtime = time;
	};
	void DecPriority();
	void IncResend();
	int	is_resending() {
		return m_is_resending;
	};
	int	id_packet() {
		return m_id_packet;
	};
	int	id_frame() {
		return m_id_frame;
	};
	int	is_last() {
		return m_is_last;
	};
	int	type_data() {
		return m_type_data;
	};
	int	type_frame() {
		return m_type_frame;
	};
	unsigned int	time_stamp() {
		return m_time_stamp;
	};
	unsigned int	time_resend() {
		return m_rtime;
	};
	int	priority() {
		return m_priority;
	};
	unsigned char*	GetData() {
		return (unsigned char*)m_data.Buffer();
	};
	int	GetSize() {
		return m_size;
	};
};
