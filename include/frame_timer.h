#pragma once

#include <chrono>

class FrameTimer {
		using _clock_t = std::chrono::steady_clock;
		_clock_t::time_point _last = _clock_t::now();

	public:
		float tick();
		void reset();
};