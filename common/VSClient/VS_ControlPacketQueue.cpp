
#include "VS_StreamPacketManagment.h"
#include "../streams/Client/VS_StreamClientSender.h"
#include "SecureLib/VS_StreamCrypter.h"

unsigned char ConvertTrack(unsigned char track);

VS_ControlPacketQueue::VS_ControlPacketQueue()
{
	m_sendFrameQueue = 0;
	m_streamCrypter = 0;
	m_tmp_frame = 0;
	m_bound_bytes_ms = 0;
	m_period_time = 0;
	m_calc_t = m_dt_wait = 0.0;

#ifdef WRITE_PQC_STAT
	m_file = 0;
#endif

	m_start_t = 0;
	m_is_init = false;
}

bool VS_ControlPacketQueue::Init(VS_SendFrameQueueBase* pFrameQueue, const boost::shared_ptr<VS_StreamClientSender> & pStreamSnd, VS_StreamCrypter* pStreamCrypt, int B, int T)
{
	Release();

	m_sendFrameQueue = pFrameQueue;
	m_streamSender = pStreamSnd;
	m_streamCrypter = pStreamCrypt;
	m_tmp_frame = (unsigned char*)malloc(0x20000);
	if (T < 0) T = 0;
	if (T > 1000) T = 1000;
	m_period_time = T;
	m_bound_bytes_ms = (B * 1000.0 / 8.0) / 1000.0;
	m_start_t = timeGetTime();

#ifdef WRITE_PQC_STAT
	m_file = fopen("log_PQC.txt", "w");
	fprintf(m_file, "B = %d, T = %d, Bms = %8.2f \n", B, T, m_bound_bytes_ms);
#endif

	m_is_init = true;

	//ActivateThread(this); // uncomment for thread implementation

	return true;
}

void VS_ControlPacketQueue::Release()
{
	m_sendFrameQueue = 0;
	m_streamSender.reset();
	m_streamCrypter = 0;
	m_bound_bytes_ms = 0;
	m_period_time = 0;
	m_calc_t = m_dt_wait = 0.0;
	m_start_t = 0;
	if (m_tmp_frame) free(m_tmp_frame); m_tmp_frame = 0;
	m_is_init = false;

#ifdef WRITE_PQC_STAT
	if (m_file) fclose(m_file); m_file = 0;
#endif

	//DesactivateThread(); // uncomment for thread implementation
}

VS_ControlPacketQueue::~VS_ControlPacketQueue()
{
	Release();
}

DWORD VS_ControlPacketQueue::Loop(LPVOID hEvDie)
{
	int cur_t;
	int res;
	HANDLE handles = hEvDie;
	DWORD waitRes;
	DWORD exitReason = 0;

	while (true) {
		waitRes = WaitForSingleObject(handles, (int)(m_dt_wait + 0.5));

		switch (waitRes)
		{
		case WAIT_OBJECT_0 + 0:
			exitReason = 1;
			break;
		case WAIT_FAILED:
			exitReason = 3;
			break;
		}
		if (exitReason) break;

		cur_t = timeGetTime();
		res = ProcessSendQueueThread(cur_t);
	}

	return NOERROR;
}

int	VS_ControlPacketQueue::ProcessSendQueueThread(int cur_t)
{
	unsigned char *buff;
	unsigned char track, slayer;
	unsigned long t_out = 20;
	int size, lqueue, res;
	double dt;

	if (m_calc_t == 0.0) m_calc_t = cur_t;
	if (cur_t >= m_calc_t) {
		dt = cur_t - m_calc_t;
		if (dt > m_period_time) dt = m_period_time;
		m_dt_wait = 0;
		while (true) {
			res = -1;
			lqueue = m_sendFrameQueue->GetFrame(buff, size, track, slayer);
			if (lqueue > 0)
				res = m_streamSender->SendFrame(buff, size, static_cast<stream::Track>(track), &t_out);
			if (res == -1) break;
			m_dt_wait += (double)size / m_bound_bytes_ms;
			m_sendFrameQueue->MarkFirstAsSend();
			if (m_dt_wait - dt > 0) break;
		}
		m_dt_wait -= dt;
		if (m_dt_wait <= 0) m_dt_wait = 0;
		else if (m_dt_wait <= 10) m_dt_wait = 10;
		m_calc_t = cur_t + m_dt_wait;
	}

	return res;
}

int	VS_ControlPacketQueue::ProcessSendQueue(int cur_t, unsigned char* &pBuffer, unsigned char &track, unsigned char &slayer, int &size, bool is_nhp)
{
	unsigned long t_out = 20;
	int lqueue, res;
	double dt;

	res = -4;

	if (m_calc_t == 0.0) m_calc_t = cur_t;

#ifdef WRITE_PQC_STAT
	fprintf(m_file, "c_t = %7d   clc_t = %8.1f   ", cur_t - m_start_t, m_calc_t - m_start_t);
#endif

	if (cur_t >= m_calc_t) {
		dt = cur_t - m_calc_t;
		if (dt > m_period_time) dt = m_period_time;
		m_dt_wait = 0;
		lqueue = m_sendFrameQueue->GetFrame(pBuffer, size, track, slayer);

#ifdef WRITE_PQC_STAT
		fprintf(m_file, "dt = %5.1f   lq = %3d   ", dt, lqueue);
#endif

		if (is_nhp) track = ConvertTrack(track);
		if (lqueue <= 0) {

#ifdef WRITE_PQC_STAT
			fprintf(m_file, "\n");
#endif

			return -4;
		}

		if (size > 0 && m_streamCrypter->IsValid()) {
			uint32_t encsize = 0x10000;
			if (m_streamCrypter->Encrypt(pBuffer, size, m_tmp_frame, &encsize))
				res = m_streamSender->SendFrame(m_tmp_frame, encsize, static_cast<stream::Track>(track), &t_out);
			else
				res = m_streamSender->SendFrame(pBuffer, size, static_cast<stream::Track>(track), &t_out);
		}
		else {
			res = m_streamSender->SendFrame(pBuffer, size, static_cast<stream::Track>(track), &t_out);
		}

		if (res != -1) {
			m_dt_wait += (double)size / m_bound_bytes_ms;
			m_dt_wait -= dt;
			if (m_dt_wait <= 10) {
				if (res >= 0) res = -5;
				res -= 3;
			}
		}

#ifdef WRITE_PQC_STAT
		fprintf(m_file, "sz = %4d   dt_w = %5.1f   res = %d", size, m_dt_wait, res);
#endif

		m_calc_t = cur_t + m_dt_wait;
	}

#ifdef WRITE_PQC_STAT
	fprintf(m_file, "\n");
#endif

	return res;
}