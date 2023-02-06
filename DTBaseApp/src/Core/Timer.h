#pragma once
#include <chrono>

namespace DT
{
	class Timer
	{
	public:
		Timer()
		{
			Reset();
		}
		void Reset()
		{
			m_Start = clock::now();
		}
		float Mark()
		{
			float elapsed = ElapsedSeconds();
			Reset();
			return elapsed;
		}
		uint64 ElapsedNanoseconds() const
		{
			clock::time_point now = clock::now();
			std::chrono::nanoseconds nanoseconds = now - m_Start;
			return nanoseconds.count();
		}
		float ElapsedMicroseconds() const { return float(ElapsedNanoseconds() * 0.001); }
		float ElapsedMilliseconds() const { return float(ElapsedNanoseconds() * 0.001 * 0.001); }
		float ElapsedSeconds()      const { return float(ElapsedNanoseconds() * 0.001 * 0.001 * 0.001); }
	private:
		using clock = std::chrono::high_resolution_clock;

		clock::time_point m_Start;
	};
}