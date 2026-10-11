#pragma once

#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "ege/graphics.h"
#include "frame_timer.h"
#include "scene.h"
#include "spirit.h"

class App {
	protected:
		PIMAGE _target;	 // EGE 的默认绘制目标（nullptr 即窗口）
		FrameTimer _frame_timer;
		int _window_width, _window_height;
		Scene _root_scene;

		void _update(float delta);

		void _draw();

		void _draw_hud();

		void _draw_crosshair();

		void _handle_keyboard(key_msg msg);

		void _handle_mouse(mouse_msg msg);

		// 是否有升级卡正在等待选择（是则暂停物理、输入只给升级系统）
		bool _is_choosing();

	public:
		App(int window_width, int window_height, const std::optional<std::string> &title = std::nullopt);

		~App();

		void run();

		SpiritId add_spirit(std::unique_ptr<Spirit> spirit);

		template <typename T, typename... Args>
		SpiritId add_spirit(Args &&...args) {
			return _root_scene.add_spirit<T>(std::forward<Args>(args)...);
		}

		// 预留接口：目前无调用点
		bool remove_spirit(SpiritId id);

		Spirit *find_spirit(SpiritId id);

		template <typename T>
		T *find_spirit(SpiritId id) {
			return _root_scene.find_spirit<T>(id);
		}

		Scene &root_scene();
};
