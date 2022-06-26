#ifndef _VS_COOLTIMER_H_ 
#define _VS_COOLTIMER_H_

#pragma once

#include <windows.h>

// simgle QueryPerformanceFrequency timer class

enum tm_timers {
	tm_connect = 0,
	tm_handshake = 1
};

class VS_CoolTimer
{
private:
	LARGE_INTEGER m_freq;
	LARGE_INTEGER m_counter[256];
	LARGE_INTEGER m_internal[256];
	float m_rez[256];
	unsigned long m_cnt[256];
public:
	VS_CoolTimer(void);
	void Start(char number = 0);
	void Shot(char number = 0);
	float Get(char number = 0, float prec = 1000.0);
	float GetWith(char number = 0, char with = 0);
	void Reset(char number = 0);
	void ResetAll();
	void  cAdd(char number = 0, float prec = 1000.0);
	float cRez(char number = 0);
	virtual ~VS_CoolTimer(void);
};

#endif