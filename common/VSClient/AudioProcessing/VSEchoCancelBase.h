/*
 * File:   VSEchoCancelBase.h
 * Author: dront78
 *
 * Created on 8 Октябрь 2012 г., 15:27
 */

#ifndef VSECHOCANCELBASE_H
#define	VSECHOCANCELBASE_H

#include <sys/types.h>
#include <stdint.h>

namespace webrtc {
	class AudioBuffer;
}

class VSEchoCancelBase {
public:
    // constructor
	VSEchoCancelBase() {}

    // destructor
	virtual ~VSEchoCancelBase() {}
    // init echo canselltion module with specified parametrs.
    // If the function succeeds, the return value is zero

    virtual void Init(uint32_t frequency) = 0;

    // release echo cansellation module,
    virtual void Release() = 0;

    // echo cancellation
    // far_end	- far end signal (from other client)
    // near_end - near end signal (from capture device)
    // out		- calcellated near_end signal
    // samples	- number of samples
    // If the function succeeds, the return value is zero
    virtual void Cancellate(webrtc::AudioBuffer *ra, webrtc::AudioBuffer *ca, bool *stream_has_echo) = 0;

private:

};

class VSEchoCancelStub : public VSEchoCancelBase
{

public:

	VSEchoCancelStub() {

	}

	virtual ~VSEchoCancelStub() {

	}

	virtual void Init(uint32_t /*frequency*/) override {

	}

	virtual void Release() override {

	}

	virtual void Cancellate(webrtc::AudioBuffer *ra, webrtc::AudioBuffer *ca, bool *stream_has_echo) override {

	}

private:

};

#endif	/* VSECHOCANCELBASE_H */

