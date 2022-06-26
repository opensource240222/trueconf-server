#ifndef SCREEN_CAPTURER_TYPES_H
#define SCREEN_CAPTURER_TYPES_H

namespace CapturerType
{
	enum Type
	{
		DEFAULT, // Win8 or higher - Desktop Duplication, otherwise GDI
		GDI,
		MIRROR_DRIVER
	};
}

#endif
