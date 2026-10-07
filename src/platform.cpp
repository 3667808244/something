#include "platform.h"

#include "define.h"

Platform::Platform(float x, float y, float width, float height) : Rect(x, y, width, height) {}

void Platform::draw(PIMAGE image) {
	setfillcolor(rgb(100, 160, 100), image);
	bar(static_cast<int>(left()), static_cast<int>(top()), static_cast<int>(right()), static_cast<int>(bottom()), image);
}
