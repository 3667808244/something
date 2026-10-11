#include "obstacle.h"

#include "define.h"

Obstacle::Obstacle(float center_x, float center_y, float width, float height)
	: Rect(center_x, center_y, width, height) {}

void Obstacle::draw(PIMAGE image) {
	setfillcolor(rgb(110, 110, 120), image);
	bar(static_cast<int>(left()), static_cast<int>(top()), static_cast<int>(right()), static_cast<int>(bottom()),
		image);
}
