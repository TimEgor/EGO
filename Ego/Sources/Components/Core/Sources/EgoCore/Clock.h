#pragma once

#include <chrono>

namespace ego
{
	using ClockCounter = std::chrono::high_resolution_clock;
	using ClockTimePoint = ClockCounter::time_point;
	using Millisecond = std::chrono::milliseconds;

	namespace Clock
	{
		inline ClockTimePoint GetCurrentTimePoint() { return ClockCounter::now(); }
		
	    template <typename T>
		inline T CalcTimePointDelta(const ClockTimePoint& t1, const ClockTimePoint& t2) { return std::chrono::duration<T>(t1 - t2).count(); }
	};
}