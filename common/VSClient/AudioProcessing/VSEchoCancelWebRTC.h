/*
 * File:   VSEchoCancelWebRtc.h
 * Author: dront78
 *
 * Created on 8 Октябрь 2012 г., 15:28
 */

#ifndef VSECHOCANCELWEBRTC_H
#define	VSECHOCANCELWEBRTC_H

#include "VSEchoCancelBase.h"

class VSEchoCancelWebRTC : public VSEchoCancelBase
{

public:

    VSEchoCancelWebRTC();
    virtual ~VSEchoCancelWebRTC();
    virtual void Init(uint32_t frequency) override;
    virtual void Release() override;
	virtual void Cancellate(webrtc::AudioBuffer *ra, webrtc::AudioBuffer *ca, bool *stream_has_echo) override;

private:

	int				m_frame_size;
	void			*m_st;

};

#endif	/* VSECHOCANCELWEBRTC_H */

