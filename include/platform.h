#pragma once

#include "rect.h"

class Platform : public Rect {
	public:
		Platform(float x, float y, float width, float height);

		virtual void draw(PIMAGE image);
};
