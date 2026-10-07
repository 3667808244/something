#pragma once
#include "ege/graphics.h"

class Scene;

class Spirit {
	public:
		virtual ~Spirit() = default;
		virtual void draw(PIMAGE image) = 0;
		virtual void update(float delta, Scene &scene) = 0;
		virtual void handal_keybord(key_msg msg) = 0;
		virtual void handal_mouse(mouse_msg msg) = 0;
		virtual bool is_invalid() const = 0;
};