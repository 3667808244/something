#include "app.h"

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

App::App(int window_width, int window_height, const std::optional<std::string> &title) {
	_window = nullptr;
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
	while (is_run() && !_root_scene.is_invalid()) {
		while (kbmsg()) {
			_handal_keybord(getkey());
		}
		while (mousemsg()) {
			_handal_mouse(getmouse());
		}
		_update(_frame_timer.tick());
		cleardevice(_window);
		_draw();
		delay_fps(60);
	}
}

void App::_update(float delta) { _root_scene.update(delta, _root_scene); }

void App::_draw() {
	_root_scene.draw(_window);
	_draw_hud();
	_draw_crosshair();
}

void App::_draw_hud() {
	_root_scene.each<Player>([&](Player &player) {
		int capacity = player.magazine_size();
		int filled = player.ammo();
		color_t fill_color = rgb(255, 220, 80);

		if (player.is_reloading()) {
			filled = static_cast<int>(static_cast<float>(capacity) * player.reload_progress() + 0.5f);
			fill_color = rgb(255, 140, 0);
		}

		for (int i = 0; i < capacity; ++i) {
			int x = hud_pip_x + i * (hud_pip_width + hud_pip_gap);
			setfillcolor(i < filled ? fill_color : rgb(70, 70, 80), _window);
			bar(x, hud_pip_y, x + hud_pip_width, hud_pip_y + hud_pip_height, _window);
		}
	});
}

void App::_draw_crosshair() {
	int x = 0, y = 0;
	mousepos(&x, &y);
	if (x < 0 || x >= _window_width || y < 0 || y >= _window_height) {
		return;
	}

	setlinecolor(rgb(0, 255, 0), _window);
	line(x - crosshair_gap - crosshair_arm, y, x - crosshair_gap, y, _window);
	line(x + crosshair_gap, y, x + crosshair_gap + crosshair_arm, y, _window);
	line(x, y - crosshair_gap - crosshair_arm, x, y - crosshair_gap, _window);
	line(x, y + crosshair_gap, x, y + crosshair_gap + crosshair_arm, _window);
}

void App::_handal_keybord(key_msg msg) { _root_scene.handal_keybord(msg); }

void App::_handal_mouse(mouse_msg msg) { _root_scene.handal_mouse(msg); }

SpiritId App::add_spirit(std::unique_ptr<Spirit> spirit) { return _root_scene.add_spirit(std::move(spirit)); }

bool App::remove_spirit(SpiritId id) { return _root_scene.remove_spirit(id); }

Spirit *App::find_spirit(SpiritId id) { return _root_scene.find_spirit(id); }

Scene &App::root_scene() { return _root_scene; }
