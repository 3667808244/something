#include "obstacle.h"

#include <algorithm>
#include <cmath>
#include <utility>

#include "define.h"

Obstacle::Obstacle(float x, float y, float width, float height) : Rect(x, y, width, height) {}

float Obstacle::segment_hit_t(float x0, float y0, float x1, float y1) const {
	float t_min = 0.0f;
	float t_max = 1.0f;

	float dx = x1 - x0;
	if (std::abs(dx) < 1e-6f) {
		if (x0 < left() || x0 > right()) {
			return -1.0f;
		}
	} else {
		float t1 = (left() - x0) / dx;
		float t2 = (right() - x0) / dx;
		if (t1 > t2) {
			std::swap(t1, t2);
		}
		t_min = std::max(t_min, t1);
		t_max = std::min(t_max, t2);
		if (t_min > t_max) {
			return -1.0f;
		}
	}

	float dy = y1 - y0;
	if (std::abs(dy) < 1e-6f) {
		if (y0 < top() || y0 > bottom()) {
			return -1.0f;
		}
	} else {
		float t1 = (top() - y0) / dy;
		float t2 = (bottom() - y0) / dy;
		if (t1 > t2) {
			std::swap(t1, t2);
		}
		t_min = std::max(t_min, t1);
		t_max = std::min(t_max, t2);
		if (t_min > t_max) {
			return -1.0f;
		}
	}

	return t_min;
}

bool Obstacle::intersects_segment(float x0, float y0, float x1, float y1) const {
	return segment_hit_t(x0, y0, x1, y1) >= 0.0f;
}

void Obstacle::draw(PIMAGE image) {
	setfillcolor(rgb(110, 110, 120), image);
	bar(static_cast<int>(left()), static_cast<int>(top()), static_cast<int>(right()), static_cast<int>(bottom()), image);
}
