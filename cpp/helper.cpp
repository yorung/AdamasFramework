#include "stdafx.h"
#include "font_man.h"

#include <random>
#include <set>

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

AFName afToUniqueName(const char* name)
{
	static std::set<std::shared_ptr<std::string>> tbl;
	auto it = std::find_if(tbl.begin(), tbl.end(), [name](const std::shared_ptr<std::string>& storedName) { return !strcmp(storedName.get()->c_str(), name); });
	if (it != tbl.end())
	{
		return (*it).get()->c_str();
	}
	std::shared_ptr<std::string> str(new std::string(name));
	tbl.insert(str);
	return str.get()->c_str();
}

AFProfiler afProfiler;

void AFProfiler::Print()
{
	float y = 60;
	for (auto it : results)
	{
		fontMan.DrawString(Vec2(20, y), 20, SPrintf("%s: %f", it.first, it.second), 0xffffffff);
		y += 20;
	}
	results.clear();
}