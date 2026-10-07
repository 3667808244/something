#include "rect.h"

#include <algorithm>
#include <cmath>
#include <utility>

Rect::Rect(float x, float y, float width, float height) {
	_x = x;
	_y = y;
	_width = width;
	_height = height;
}

float Rect::left() const { return _x; }
float Rect::top() const { return _y; }
float Rect::right() const { return _x + _width; }
float Rect::bottom() const { return _y + _height; }
float Rect::center_x() const { return _x + _width / 2.0f; }
float Rect::center_y() const { return _y + _height / 2.0f; }
float Rect::half_width() const { return _width / 2.0f; }
float Rect::half_height() const { return _height / 2.0f; }

bool Rect::intersects(float cx, float cy, float half_w, float half_h) const {
	return std::abs(cx - center_x()) < half_w + half_width() &&
	       std::abs(cy - center_y()) < half_h + half_height();
}

float Rect::segment_aabb_hit_t(float left, float top, float right, float bottom, float x0, float y0, float x1,
                               float y1) {
	float t_min = 0.0f;
	float t_max = 1.0f;

	float dx = x1 - x0;
	if (std::abs(dx) < 1e-6f) {
		if (x0 < left || x0 > right) {
			return -1.0f;
		}
	} else {
		float t1 = (left - x0) / dx;
		float t2 = (right - x0) / dx;
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
		if (y0 < top || y0 > bottom) {
			return -1.0f;
		}
	} else {
		float t1 = (top - y0) / dy;
		float t2 = (bottom - y0) / dy;
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

void Rect::update(float delta [[maybe_unused]], Scene &scene [[maybe_unused]]) {}

void Rect::handal_keybord(key_msg msg [[maybe_unused]]) {}

void Rect::handal_mouse(mouse_msg msg [[maybe_unused]]) {}

bool Rect::is_invalid() const { return false; }
