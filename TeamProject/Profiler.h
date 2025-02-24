#pragma once

#include <string_view>
#include <chrono>
#include <map>

namespace NCL::CSC8503 {
	class Profiler {
	public:
		using Clock = std::chrono::high_resolution_clock;

		void beginFrame();
		void endFrame();
		void printTimes();

		// Begin recording a new section
		void startSection(std::string_view name);
	private:
		// Record the elapsed time for the current section
		void recordElapsed();

		std::map<std::string, Clock::duration> times;
		std::string activeSection;
		Clock::time_point lastSectionEnd;
	};
}
