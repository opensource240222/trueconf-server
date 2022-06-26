#ifndef SCREEN_CAPTURER_FACTORY_H
#define SCREEN_CAPTURER_FACTORY_H

#include "ScreenCapturer.h"
#include "ScreenCapturerTypes.h"

class ScreenCapturerFactory
{
public:

	static ScreenCapturer * Create(CapturerType::Type type);
	static bool IsAvailable(CapturerType::Type type);
};

#endif
