#pragma once
#include "ege/graphics.h"

class Scene;

class Spirit {
	public:
		virtual ~Spirit() = default;
		virtual void draw(PIMAGE image) = 0;
		virtual void update(float delta, Scene &scene) = 0;
		// 键鼠回调默认空实现，只有真正需要处理的子类才覆写
		virtual void handle_keyboard(key_msg msg [[maybe_unused]]) {}
		virtual void handle_mouse(mouse_msg msg [[maybe_unused]]) {}
		virtual bool is_invalid() const = 0;
};
