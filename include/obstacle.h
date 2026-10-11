#pragma once

#include "rect.h"

class Obstacle : public Rect {
	public:
		Obstacle(float center_x, float center_y, float width, float height);

		void draw(PIMAGE image) override;
};
