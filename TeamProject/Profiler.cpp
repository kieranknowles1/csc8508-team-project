#include "Profiler.h"

#include "Debug.h"

#include <sstream>

namespace NCL::CSC8503 {
	void Profiler::beginFrame()
	{
		activeSection.clear();
		times.clear();
		lastSectionEnd = Clock::now();
	}
	void Profiler::endFrame()
	{
		recordElapsed();
	}

	void Profiler::printTimes()
	{
		Vector2 position(0, 0.1);
		for (auto& section : times) {
			std::stringstream str;
			str << section.first << ": " << std::chrono::duration_cast<std::chrono::microseconds>(section.second).count() / 1000.0f << "ms";
			Debug::Print(str.str(), position);
			position.y += 0.05;
		}
	}

	void Profiler::startSection(std::string_view name)
	{
		recordElapsed();
		activeSection = name;
	}
	void Profiler::recordElapsed()
	{
		auto now = Clock::now();
		if (!activeSection.empty()) {
			times.emplace(activeSection, now - lastSectionEnd);
		}
		lastSectionEnd = now;
	}
}
