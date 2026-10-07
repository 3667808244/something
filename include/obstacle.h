#pragma once

#include "rect.h"

class Obstacle : public Rect {
	public:
		Obstacle(float center_x, float center_y, float width, float height);

		float segment_hit_t(float x0, float y0, float x1, float y1) const;

		void draw(PIMAGE image) override;
};
