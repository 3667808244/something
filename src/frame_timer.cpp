#include "frame_timer.h"

float FrameTimer::tick() {
	auto now = _clock_t::now();
	float dt = std::chrono::duration<float>(now - _last).count();
	_last = now;
	return dt;
}

void FrameTimer::reset() { _last = _clock_t::now(); }