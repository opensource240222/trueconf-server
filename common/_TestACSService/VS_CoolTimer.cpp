#include <stdio.h>
#include "VS_CoolTimer.h"

VS_CoolTimer::VS_CoolTimer(void)
{
	if (!QueryPerformanceFrequency(&m_freq)) throw -1;
	printf("\nClibrating performance... done at %ld", m_freq);
	memset(&m_counter, 0xff, 256 * sizeof(LARGE_INTEGER));
	memset(&m_internal, 0xff, 256 * sizeof(LARGE_INTEGER));
	memset(&m_rez, 0, 256 * sizeof(float));
	memset(&m_cnt, 0, 256 * sizeof(unsigned long));
}

VS_CoolTimer::~VS_CoolTimer(void)
{
}

void VS_CoolTimer::Start(char number)
{
	QueryPerformanceCounter(&m_internal[number]);
}

void VS_CoolTimer::Shot(char number)
{
	QueryPerformanceCounter(&m_counter[number]);
}

float VS_CoolTimer::Get(char number, float prec)
{
	return prec * ((float)(m_counter[number].QuadPart - m_internal[number].QuadPart)) / ((float)m_freq.QuadPart);
}

float VS_CoolTimer::GetWith(char number, char with)
{
	return ((float)(m_counter[number].QuadPart - m_internal[with].QuadPart)) / ((float)m_freq.QuadPart);
}

void VS_CoolTimer::Reset(char number)
{
	memset(&m_counter[number], 0xff, sizeof(LARGE_INTEGER));
	memset(&m_internal[number], 0xff, sizeof(LARGE_INTEGER));
	m_rez[number] = (float).0;
	m_cnt[number] = 0;
}

void VS_CoolTimer::ResetAll()
{
	memset(&m_counter, 0xff, 256 * sizeof(LARGE_INTEGER));
	memset(&m_internal, 0xff, 256 * sizeof(LARGE_INTEGER));
	memset(&m_rez, 0, 256 * sizeof(float));
	memset(&m_cnt, 0, 256 * sizeof(unsigned long));
}

void VS_CoolTimer::cAdd(char number, float prec)
{
	float x = VS_CoolTimer::Get(number, prec);
	if (x > 0) {
		++m_cnt[number];
		m_rez[number] += x;
	}
}

float VS_CoolTimer::cRez(char number)
{
	return m_rez[number] / (float) m_cnt[number];
}