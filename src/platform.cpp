#include "platform.h"

#include "define.h"

Platform::Platform(float center_x, float center_y, float width, float height)
    : Rect(center_x, center_y, width, height) {}

void Platform::draw(PIMAGE image) {
	setfillcolor(rgb(100, 160, 100), image);
	bar(static_cast<int>(left()), static_cast<int>(top()), static_cast<int>(right()), static_cast<int>(bottom()), image);
}
