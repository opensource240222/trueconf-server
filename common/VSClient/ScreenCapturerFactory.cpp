#include "ScreenCapturerFactory.h"
#include "ScreenCapturerGDI.h"
#include "ScreenCapturerDesktopDuplication.h"
#include "versionhelpers.h"

ScreenCapturer * ScreenCapturerFactory::Create(CapturerType::Type type)
{
	ScreenCapturer *sc(nullptr);
	switch (type)
	{
		case CapturerType::GDI:
		{
			sc = new ScreenCapturerGDI();
			break;
		}
		case CapturerType::DEFAULT:
		{
			if (IsWindows8OrGreater()) {
				sc = new ScreenCapturerDesktopDuplication();
				if (sc) {
					if (!sc->EnumerateOutputs()) {
						delete sc;
						sc = nullptr;
					}
					else {
						sc->Reset();
					}
				}
			}
			if (!sc) {
				sc = new ScreenCapturerGDI();
			}
			break;
		}
	}
	return sc;
}

bool ScreenCapturerFactory::IsAvailable(CapturerType::Type type)
{
	bool res = false;

	switch (type)
	{
	case CapturerType::DEFAULT:
	case CapturerType::GDI:
		res = true;
		break;
	case CapturerType::MIRROR_DRIVER:
		res = false;
		break;
	}

	return res;
}
