#include "rect.h"

#include <cmath>

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

void Rect::update(float delta [[maybe_unused]], Scene &scene [[maybe_unused]]) {}

void Rect::handal_keybord(key_msg msg [[maybe_unused]]) {}

void Rect::handal_mouse(mouse_msg msg [[maybe_unused]]) {}

bool Rect::is_invalid() const { return false; }
