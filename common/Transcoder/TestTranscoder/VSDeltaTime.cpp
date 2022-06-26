#include "VSDeltaTime.h"

void VSDeltaTime::Tick()
{
	delta = std::chrono::steady_clock::now() - timePoint;
	timePoint = std::chrono::steady_clock::now();
}

std::chrono::milliseconds VSDeltaTime::DeltaMs()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(delta);
}

float VSDeltaTime::DeltaMsF()
{
	return float(std::chrono::duration_cast<std::chrono::nanoseconds>(delta).count()) / std::nano::den * 1000.f;
}
