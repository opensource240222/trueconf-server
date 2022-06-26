/**
 **************************************************************************
 * \file SpeexEchoCancel.cpp
 * (c) 2002-2007 Visicron Inc.  http://www.visicron.net/
 * \brief Speex Echo Cancellation processing
 *
 * \b Project Audio
 * \date 30.08.2007
 *
 ****************************************************************************/

/****************************************************************************
 * Includes
 ****************************************************************************/

#include "SpeexEchoCancel.h"
#include "../../VSClient/VS_ApplicationInfo.h"
#include "../../std/cpplib/VS_RegistryKey.h"

#include <speex/speex_echo.h>
#include <speex/speex_preprocess.h>

/****************************************************************************
 * Defines
 ****************************************************************************/

/****************************************************************************
 * Classes
 ****************************************************************************/

VS_SpeexEchoCancel::VS_SpeexEchoCancel()
{
	m_frame_size = 0;
	m_st = 0;
	m_den = 0;
	m_far_tmp = 0;
	m_num_spk = 0;
	m_type = AEC_SPEEX;

#ifdef TEST_FUNC_RESPONSE
	m_is_dump = 0;
	VS_RegistryKey key(true, REG_CurrentConfiguratuon);
	key.GetValue(&m_is_dump, sizeof(int), VS_REG_INTEGER_VT, "DumpAEC");
	m_num_inc = 0;
	m_num_incf = 0;
	m_dump_time = 0;
	m_filter_response = 0;
#endif

}

VS_SpeexEchoCancel::~VS_SpeexEchoCancel()
{
	Release();
}

void VS_SpeexEchoCancel::Init(int frequency)
{
	Release();
	m_frame_size = (frequency / 8000) * 80; /// 10 ms
	m_den = speex_preprocess_state_init(m_frame_size, frequency);
	long noise = -12;
	speex_preprocess_ctl(m_den, SPEEX_PREPROCESS_SET_NOISE_SUPPRESS, &noise);
	noise = -60;
	speex_preprocess_ctl(m_den, SPEEX_PREPROCESS_SET_ECHO_SUPPRESS, &noise);
	noise = -40;
	speex_preprocess_ctl(m_den, SPEEX_PREPROCESS_SET_ECHO_SUPPRESS_ACTIVE, &noise);
	noise = 1;
	speex_preprocess_ctl(m_den, SPEEX_PREPROCESS_SET_VAD, &noise);
	noise = 0;
	VS_RegistryKey key(true, REG_CurrentConfiguratuon);
	key.GetValue(&noise, 4, VS_REG_INTEGER_VT, "SpeexAGC");
	speex_preprocess_ctl(m_den, SPEEX_PREPROCESS_SET_AGC, &noise);
}

void VS_SpeexEchoCancel::Init(int frequency, int num_mic, int num_spk)
{
	if (m_far_tmp) free(m_far_tmp); m_far_tmp = 0;
	if (m_st) speex_echo_state_destroy(m_st); m_st = 0;
	if (num_spk > 0) {
		m_frame_size = (frequency / 8000) * 80; /// 10 ms

		int len = 0;
		VS_RegistryKey key(true, REG_CurrentConfiguratuon);
		key.GetValue(&len, 4, VS_REG_INTEGER_VT, "AECFilterLen");
		if (len == 0) {
			len = frequency / 10;
		} else {
			len = len * frequency / 1000;
		}

		m_st = speex_echo_state_init_mc(m_frame_size, len, num_mic, num_spk);
		speex_echo_ctl(m_st, SPEEX_ECHO_SET_SAMPLING_RATE, &frequency);
		m_num_spk = num_spk;
		m_far_tmp = (short*)malloc(m_frame_size*sizeof(short)*m_num_spk);

#ifdef TEST_FUNC_RESPONSE
		if (m_is_dump != 0) {
			int sw = 0;
			speex_echo_ctl(m_st, SPEEX_ECHO_GET_IMPULSE_RESPONSE_SIZE, &sw);
			m_filter_response = (int*)malloc(sw * sizeof(int));
			memset(m_filter_response, 0, sw * sizeof(int));
		}
#endif

	}
}

void VS_SpeexEchoCancel::Release()
{
	m_frame_size = 0;
	if (m_st) speex_echo_state_destroy(m_st); m_st = 0;
	if (m_den) speex_preprocess_state_destroy(m_den); m_den = 0;
	if (m_far_tmp) free(m_far_tmp); m_far_tmp = 0;

#ifdef TEST_FUNC_RESPONSE
	if (m_filter_response) free(m_filter_response); m_filter_response = 0;
#endif

}

#ifdef TEST_FUNC_RESPONSE

void VS_SpeexEchoCancel::DumpResponseFilter()
{
	int sw = 0;
	speex_echo_ctl(m_st, SPEEX_ECHO_GET_IMPULSE_RESPONSE_SIZE, &sw);
	speex_echo_ctl(m_st, SPEEX_ECHO_GET_IMPULSE_RESPONSE, m_filter_response);
	if (m_num_inc == 0 || m_num_inc % 4 == 0) {
		char buff[128];
		sprintf(buff, "fr_%d.txt", m_num_incf);
		FILE *fst = fopen(buff, "w");
		for (int i = 0; i < m_frame_size; i++) {
			for (int j = 0; j < sw / m_frame_size; j++) {
				fprintf(fst, "%9d,", m_filter_response[i+m_frame_size*j]);
			}
			fprintf(fst, "\n");
		}
		fclose(fst);
		m_num_incf++;
	}
	m_num_inc++;
}

#endif

void VS_SpeexEchoCancel::Cancellate(short *far_end, short *near_end, short *echo, int samples)
{
	int i, k, j;
	if (!m_st) {
		memcpy(echo, near_end, samples*2);
		speex_preprocess_ctl(m_den, SPEEX_PREPROCESS_SET_ECHO_STATE, 0);
		for (i = 0; i < samples; i += m_frame_size) {
			speex_preprocess_run(m_den, echo+i);
		}
	} else {
		speex_preprocess_ctl(m_den, SPEEX_PREPROCESS_SET_ECHO_STATE, m_st);
		for (i = 0; i < samples; i += m_frame_size) {
			if (m_num_spk != 1) {
				short *pf  = far_end + i;
				for (k = 0; k < m_frame_size; k++) {
					for (j = 0; j < m_num_spk; j++) {
						m_far_tmp[k*m_num_spk+j] = pf[samples*j+k];
					}
				}
				speex_echo_cancellation(m_st, near_end+i, m_far_tmp, echo+i);
			} else {
				speex_echo_cancellation(m_st, near_end+i, far_end+i, echo+i);
			}
			speex_preprocess_run(m_den, echo+i);

#ifdef TEST_FUNC_RESPONSE
			if (m_is_dump != 0) {
				int ctime = timeGetTime();
				if (m_dump_time == 0) m_dump_time = ctime;
				//if (ctime - m_dump_time > 1000) {
					DumpResponseFilter();
					m_dump_time = ctime;
				//}
			}
#endif

		}
	}
}

/*
void VS_SpeexEchoCancel::Preprocess(short* in, int samples)
{
	speex_preprocess_ctl(m_den, SPEEX_PREPROCESS_SET_ECHO_STATE, 0);
	for (int i = 0; i<samples; i+=m_frame_size)
		speex_preprocess_run(m_den, in+i);
}
*/
