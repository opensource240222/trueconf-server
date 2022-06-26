#include "VS_CorePinAnalyser.h"

VS_CoreFlag::VS_CoreFlag()
	: m_flag(0)
{
}

VS_CoreFlag::~VS_CoreFlag()
{
}

void VS_CoreFlag::SetFlag(unsigned long flag, unsigned long option, unsigned long ext)
{
	unsigned long newBits = m_flag ^ flag;
	unsigned long bitChecker(1), counter(0);
	while (newBits) {
		if (newBits & 1) {
			bitChecker = 1 << counter;
			if (flag & bitChecker) {
				//OnRaise(static_cast<core::pin>(counter), option, ext);  // bit number
				OnRaise(static_cast<core::pin>(bitChecker), option, ext); // bit->long
			} else if (m_flag & bitChecker) {
				//OnFall(static_cast<core::pin>(counter), option, ext);
				OnFall(static_cast<core::pin>(bitChecker), option, ext);
			}
		}
		++counter;
		newBits >>= 1;
	}

	m_flag = flag;
#ifdef _DEBUG
	printf("\n*flag::pin*\n");
#endif
}

bool VS_CoreFlag::GetPin(core::pin pin)
{
	return ((pin & m_flag) == pin);
}

void VS_CoreFlag::SetPin(core::pin pin, bool enable)
{
	unsigned long n_flag = enable ? m_flag | pin : m_flag & !pin;
	SetFlag(n_flag);
}
