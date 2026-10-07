#pragma once

#include "rect.h"

class Platform : public Rect {
	public:
		Platform(float center_x, float center_y, float width, float height);

		void draw(PIMAGE image) override;
};
