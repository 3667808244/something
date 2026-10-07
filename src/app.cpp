#include "app.h"

#include <algorithm>
#include <cmath>
#include <optional>
#include <string>
#include <utility>

#include "define.h"
#include "ege/graphics.h"
#include "player.h"

constexpr int crosshair_arm = 8;
constexpr int crosshair_gap = 3;

constexpr int hud_pip_width = 6;
constexpr int hud_pip_height = 14;
constexpr int hud_pip_gap = 4;
constexpr int hud_pip_x = 12;
constexpr int hud_pip_y = 12;
constexpr int hud_health_x = 12;
constexpr int hud_health_y = 34;
constexpr int hud_health_width = 160;
constexpr int hud_health_height = 10;

// 单帧最多消化的真实时间，避免窗口被拖动/系统卡顿后物理一次跨出好几秒
constexpr float max_frame_time = 0.25f;
// 固定物理步长：900px/s 的落速下一步只走 7.5px，不会穿过 12~20px 的薄平台
constexpr float physics_step = 1.0f / 120.0f;

App::App(int window_width, int window_height, const std::optional<std::string> &title) {
	_target = nullptr;
	_window_width = window_width;
	_window_height = window_height;
	setinitmode(INIT_RENDERMANUAL | INIT_NOFORCEEXIT);
	initgraph(window_width, window_height);
	showmouse(0);
	if (title.has_value()) {
		setcaption(title->c_str());
	}
}

App::~App() {
	_root_scene.clear();
	_root_scene.purge();
	showmouse(1);
	closegraph();
}

void App::run() {
	_frame_timer.reset();
	float accumulator = 0.0f;
	while (is_run() && !_root_scene.is_invalid()) {
		while (kbmsg()) {
			_handle_keyboard(getkey());
		}
		while (mousemsg()) {
			_handle_mouse(getmouse());
		}

		accumulator += std::min(_frame_timer.tick(), max_frame_time);
		while (accumulator >= physics_step && !_root_scene.is_invalid()) {
			_update(physics_step);
			accumulator -= physics_step;
		}

		cleardevice(_target);
		_draw();
		delay_fps(60);
	}
}

void App::_update(float delta) { _root_scene.update(delta, _root_scene); }

void App::_draw() {
	_root_scene.draw(_target);
	_draw_hud();
	_draw_crosshair();
}

void App::_draw_hud() {
	_root_scene.each<Player>([&](Player &player) {
		int capacity = player.magazine_size();
		int filled = player.ammo();
		color_t fill_color = rgb(255, 220, 80);

		if (player.is_reloading()) {
			filled = static_cast<int>(std::lround(static_cast<float>(capacity) * player.reload_progress()));
			fill_color = rgb(255, 140, 0);
		}

		for (int i = 0; i < capacity; ++i) {
			int x = hud_pip_x + i * (hud_pip_width + hud_pip_gap);
			setfillcolor(i < filled ? fill_color : rgb(70, 70, 80), _target);
			bar(x, hud_pip_y, x + hud_pip_width, hud_pip_y + hud_pip_height, _target);
		}

		float ratio = static_cast<float>(player.health()) / static_cast<float>(player.max_health());
		setfillcolor(rgb(60, 60, 70), _target);
		bar(hud_health_x, hud_health_y, hud_health_x + hud_health_width, hud_health_y + hud_health_height, _target);
		setfillcolor(ratio > 0.5f ? rgb(80, 200, 90) : rgb(230, 60, 60), _target);
		bar(hud_health_x, hud_health_y, hud_health_x + static_cast<int>(hud_health_width * ratio),
			hud_health_y + hud_health_height, _target);
	});
}

void App::_draw_crosshair() {
	int x = 0, y = 0;
	mousepos(&x, &y);
	if (x < 0 || x >= _window_width || y < 0 || y >= _window_height) {
		return;
	}

	setlinecolor(rgb(0, 255, 0), _target);
	line(x - crosshair_gap - crosshair_arm, y, x - crosshair_gap, y, _target);
	line(x + crosshair_gap, y, x + crosshair_gap + crosshair_arm, y, _target);
	line(x, y - crosshair_gap - crosshair_arm, x, y - crosshair_gap, _target);
	line(x, y + crosshair_gap, x, y + crosshair_gap + crosshair_arm, _target);
}

void App::_handle_keyboard(key_msg msg) { _root_scene.handle_keyboard(msg); }

void App::_handle_mouse(mouse_msg msg) { _root_scene.handle_mouse(msg); }

SpiritId App::add_spirit(std::unique_ptr<Spirit> spirit) { return _root_scene.add_spirit(std::move(spirit)); }

bool App::remove_spirit(SpiritId id) { return _root_scene.remove_spirit(id); }

Spirit *App::find_spirit(SpiritId id) { return _root_scene.find_spirit(id); }

Scene &App::root_scene() { return _root_scene; }
