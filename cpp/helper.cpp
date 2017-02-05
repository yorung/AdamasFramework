#include "stdafx.h"

#include <random>

float Random()
{
	static std::mt19937 seed{ std::random_device()() };
	return std::uniform_real_distribution<float>(0.0, 1.0)(seed);
}

double GetTime()
{
	static auto start = std::chrono::high_resolution_clock::now();
	auto now = std::chrono::high_resolution_clock::now();
	return std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1, 1>>>(now - start).count();
}

void _afVerify(const char* file, const char* func, int line, const char* command, bool ok)
{
	if (ok)
	{
		return;
	}
	aflog("afVerify Fatal: %s %d %s %s", file, line, func, command);
	*(uint32_t*)(4) = 1;	// crash
}
