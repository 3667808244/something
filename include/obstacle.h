#pragma once

#include "rect.h"

class Obstacle : public Rect {
	public:
		Obstacle(float x, float y, float width, float height);

		float segment_hit_t(float x0, float y0, float x1, float y1) const;
		bool intersects_segment(float x0, float y0, float x1, float y1) const;

		virtual void draw(PIMAGE image);
};
