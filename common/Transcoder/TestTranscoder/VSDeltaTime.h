#pragma once

#include <chrono>

class VSDeltaTime
{
public:
	void Tick();

	std::chrono::milliseconds DeltaMs();
	float DeltaMsF();

private:
	std::chrono::steady_clock::time_point timePoint = std::chrono::steady_clock::now();
	std::chrono::steady_clock::duration delta = std::chrono::steady_clock::duration(0);
};
