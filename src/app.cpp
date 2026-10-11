#include "app.h"

#include <algorithm>
#include <cmath>
#include <cwchar>
#include <optional>
#include <string>
#include <utility>

#include "define.h"
#include "ege/graphics.h"
#include "level_loader.h"
#include "player.h"
#include "progression.h"
#include "spawner.h"
#include "upgrade_system.h"

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
constexpr int hud_armor_x = 12;
constexpr int hud_armor_y = 46;
constexpr int hud_armor_width = 160;
constexpr int hud_armor_height = 5;
constexpr int hud_armor_font_height = 13;
constexpr int hud_xp_x = 12;
constexpr int hud_xp_y = 60;
constexpr int hud_xp_width = 160;
constexpr int hud_xp_height = 6;
constexpr int hud_label_gap = 8;
constexpr int hud_value_font_height = 14;

// 单帧最多消化的真实时间，避免窗口被拖动/系统卡顿后物理一次跨出好几秒
constexpr float max_frame_time = 0.25f;
// 固定物理步长：900px/s 的落速下一步只走 7.5px，不会穿过 12~20px 的薄平台
constexpr float physics_step = 1.0f / 120.0f;

App::App(int window_width, int window_height, const std::optional<std::string> &title)
	: _start_screen(window_width, window_height), _pause_menu(window_width, window_height) {
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

void App::set_level_options(std::vector<LevelOption> options, std::size_t default_index) {
	_level_options = std::move(options);
	if (_level_options.empty()) {
		_level_options.push_back({ std::string(), L"内置地图" });
	}
	_start_screen.set_levels(_level_options, default_index);
}

bool App::_start_level(const LevelOption &option) {
	LevelData data;
	if (option.path.empty()) {
		data = builtin_level();
	} else {
		LevelLoadError error;
		if (!load_level_from_file(option.path, data, error)) {
			if (error.line > 0) {
				_start_screen.set_error(option.path + ":" + std::to_string(error.line) + ": " + error.message);
			} else {
				_start_screen.set_error(option.path + ": " + error.message);
			}
			return false;
		}
	}

	_current_level = option;
	_level_data = data;
	_rebuild_scene();
	_start_screen.set_error(std::string());
	_pause_menu.close();
	_state = State::Playing;
	return true;
}

void App::_rebuild_scene() {
	_root_scene.clear();
	_root_scene.purge();

	// 绘制顺序 = 插入顺序：几何 -> 刷怪点 -> 玩家。
	// 刷怪点标记因此落在几何之上、玩家和敌人之下，不会挡住实体。
	build_level(_root_scene, _level_data);
	_root_scene.add_spirit<Spawner>(_level_data.spawn_points, _window_width, _window_height, SpawnerConfig{});
	_root_scene.add_spirit<Player>(_window_width, _window_height, _level_data.player_x, _level_data.player_y);
	// 经验/等级系统；放在玩家之后，升级提示文字盖在其它实体之上
	_root_scene.add_spirit<Progression>(_window_width);
	// 升级卡系统：跟随等级发卡；覆盖层由 App 在 HUD 之后绘制
	_root_scene.add_spirit<UpgradeSystem>(_window_width, _window_height);

	// 立刻把 _pending 里的实体挪进 _spirits，保证进入游戏的第一帧就能画出来
	_root_scene.purge();
}

void App::_return_to_start_screen() {
	_root_scene.clear();
	_root_scene.purge();
	_pause_menu.close();
	_start_screen.set_error(std::string());
	_state = State::StartScreen;
}

void App::run() {
	_frame_timer.reset();
	float accumulator = 0.0f;
	while (is_run() && !_root_scene.is_invalid()) {
		// 每帧都取一次时间：暂停/选卡/开始界面时也刷新计时基准，恢复后不会一次性补帧
		const float frame_time = _frame_timer.tick();

		while (kbmsg()) {
			key_msg msg = getkey();
			if (_state == State::StartScreen) {
				_start_screen.handle_keyboard(msg);
			} else if (msg.msg == ege::key_msg_up) {
				// 抬起始终广播：暂停/选卡时松开 A/D 也要能清掉 _move_flags
				_handle_keyboard(msg);
				_pause_menu.handle_keyboard(msg);
			} else if (_is_choosing()) {
				// 选卡优先：此时 Esc 不开暂停，必须先选卡
				_root_scene.each<UpgradeSystem>([&](UpgradeSystem &system) { system.handle_keyboard(msg); });
			} else if (_pause_menu.is_open()) {
				_pause_menu.handle_keyboard(msg);
			} else if (msg.key == key_esc) {
				_pause_menu.open();
			} else {
				_handle_keyboard(msg);
			}
		}
		while (mousemsg()) {
			mouse_msg msg = getmouse();
			if (_state == State::StartScreen) {
				_start_screen.handle_mouse(msg);
			} else if (msg.is_up()) {
				// 同理：抬起广播出去清掉 _firing，同时让菜单刷新悬停
				_handle_mouse(msg);
				_pause_menu.handle_mouse(msg);
			} else if (_is_choosing()) {
				_root_scene.each<UpgradeSystem>([&](UpgradeSystem &system) { system.handle_mouse(msg); });
			} else if (_pause_menu.is_open()) {
				_pause_menu.handle_mouse(msg);
			} else {
				_handle_mouse(msg);
			}
		}

		if (_state == State::StartScreen) {
			const StartAction action = _start_screen.take_action();
			if (action == StartAction::StartGame) {
				_start_level(_start_screen.selected_level());
			} else if (action == StartAction::Quit) {
				_root_scene.finish();
			}
		} else if (_pause_menu.is_open()) {
			const PauseAction action = _pause_menu.take_action();
			if (action == PauseAction::Resume) {
				_pause_menu.close();
			} else if (action == PauseAction::Restart) {
				accumulator = 0.0f;
				if (!_start_level(_current_level)) {
					// 重开失败（例如地图文件被删/改坏）：回开始界面，让错误显示出来
					_return_to_start_screen();
				}
			} else if (action == PauseAction::BackToStart) {
				accumulator = 0.0f;
				_return_to_start_screen();
			} else if (action == PauseAction::Quit) {
				_root_scene.finish();
			}
		}

		if (_world_paused()) {
			// 暂停物理；升级系统仍然要处理已经排队的选卡
			accumulator = 0.0f;
			if (_is_choosing()) {
				_root_scene.each<UpgradeSystem>([&](UpgradeSystem &system) { system.modal_update(_root_scene); });
			}
		} else {
			accumulator += std::min(frame_time, max_frame_time);
			while (accumulator >= physics_step && !_root_scene.is_invalid()) {
				_update(physics_step);
				accumulator -= physics_step;
				if (_world_paused()) {
					// 这一帧中途弹卡/暂停了，剩下的时间不再推进
					accumulator = 0.0f;
					break;
				}
			}
		}

		cleardevice(_target);
		_draw();
		delay_fps(60);
	}
}

void App::_update(float delta) { _root_scene.update(delta, _root_scene); }

void App::_draw() {
	if (_state == State::StartScreen) {
		_start_screen.draw(_target);
		return;
	}

	_root_scene.draw(_target);
	// 等级提示画在所有实体之上：动态生成的敌人/子弹是追加进场景的，会盖住普通绘制层
	_root_scene.each<Progression>([&](Progression &progression) { progression.draw_overlay(_target); });
	_draw_hud();
	if (!_is_choosing() && !_pause_menu.is_open()) {
		_draw_crosshair();
	}
	// 升级卡覆盖层
	_root_scene.each<UpgradeSystem>([&](UpgradeSystem &system) { system.draw_overlay(_target); });
	// 暂停菜单压在最上面：顺便把玩家的属性快照一起交给它画
	if (_pause_menu.is_open()) {
		PauseInfo info;
		_root_scene.each<Player>([&](Player &player) {
			info.health = player.health();
			info.max_health = player.max_health();
			info.armor = static_cast<int>(player.armor());
			info.max_armor = player.max_armor();
			info.magazine = player.magazine_size();
			info.bullet_damage = player.bullet_damage();
			info.fire_interval = player.fire_interval();
			info.move_speed = player.speed_limit();
			info.reload_duration = player.reload_duration();
			info.lifesteal = player.lifesteal_per_kill();
			info.experience_gain = player.experience_gain();
			info.pickup_radius = player.pickup_radius();
			info.jump_velocity = player.jump_velocity();
			info.invulnerable_duration = player.invulnerable_duration();
			info.armor_regen_delay = player.armor_regen_delay();
			info.armor_regen_rate = player.armor_regen_rate();
			info.ammo_on_kill = player.ammo_on_kill();
		});
		_root_scene.each<UpgradeSystem>([&](UpgradeSystem &system) { info.modifiers = system.modifiers(); });
		_pause_menu.draw(_target, info);
	}
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

		// 弹匣容量随升级无限增长：pip 放不下时改用压缩长条 + 数字，避免画出屏幕
		int max_pips = std::max(1, (_window_width - hud_pip_x - 8) / (hud_pip_width + hud_pip_gap));
		if (capacity <= max_pips) {
			for (int i = 0; i < capacity; ++i) {
				int x = hud_pip_x + i * (hud_pip_width + hud_pip_gap);
				setfillcolor(i < filled ? fill_color : rgb(70, 70, 80), _target);
				bar(x, hud_pip_y, x + hud_pip_width, hud_pip_y + hud_pip_height, _target);
			}
		} else {
			setfillcolor(rgb(70, 70, 80), _target);
			bar(hud_pip_x, hud_pip_y, hud_pip_x + hud_health_width, hud_pip_y + hud_pip_height, _target);
			int ammo_width = capacity > 0 ? static_cast<int>(static_cast<float>(hud_health_width) *
																																										static_cast<float>(filled) / static_cast<float>(capacity))
																												 : 0;
			if (ammo_width > 0) {
				setfillcolor(fill_color, _target);
				bar(hud_pip_x, hud_pip_y, hud_pip_x + ammo_width, hud_pip_y + hud_pip_height, _target);
			}
			setfont(hud_value_font_height, 0, "Consolas", _target);
			setbkmode(TRANSPARENT, _target);
			settextcolor(rgb(230, 240, 250), _target);
			std::string ammo_text = std::to_string(filled) + "/" + std::to_string(capacity);
			outtextxy(hud_pip_x + hud_health_width + hud_label_gap, hud_pip_y + 1, ammo_text.c_str(), _target);
		}

		float ratio = std::clamp(static_cast<float>(player.health()) / static_cast<float>(player.max_health()), 0.0f,
															1.0f);
		setfillcolor(rgb(60, 60, 70), _target);
		bar(hud_health_x, hud_health_y, hud_health_x + hud_health_width, hud_health_y + hud_health_height, _target);
		setfillcolor(ratio > 0.5f ? rgb(80, 200, 90) : rgb(230, 60, 60), _target);
		bar(hud_health_x, hud_health_y, hud_health_x + static_cast<int>(hud_health_width * ratio),
				hud_health_y + hud_health_height, _target);

		// 血条右侧的数字
		setfont(hud_value_font_height, 0, "Consolas", _target);
		setbkmode(TRANSPARENT, _target);
		settextcolor(rgb(220, 235, 245), _target);
		std::string health_text = std::to_string(player.health()) + "/" + std::to_string(player.max_health());
		outtextxy(hud_health_x + hud_health_width + hud_label_gap, hud_health_y - 4, health_text.c_str(), _target);

		// 护甲条：点了护甲升级才显示
		if (player.max_armor() > 0) {
			float armor_ratio = player.armor() / static_cast<float>(player.max_armor());
			setfillcolor(rgb(40, 54, 76), _target);
			bar(hud_armor_x, hud_armor_y, hud_armor_x + hud_armor_width, hud_armor_y + hud_armor_height, _target);
			setfillcolor(rgb(90, 170, 235), _target);
			bar(hud_armor_x, hud_armor_y, hud_armor_x + static_cast<int>(hud_armor_width * armor_ratio),
				hud_armor_y + hud_armor_height, _target);

			wchar_t armor_text[32];
			std::swprintf(armor_text, 32, L"甲 %d/%d", static_cast<int>(player.armor()), player.max_armor());
			setfont(hud_armor_font_height, 0, L"Microsoft YaHei", _target);
			settextcolor(rgb(170, 210, 245), _target);
			outtextxy(hud_armor_x + hud_armor_width + hud_label_gap, hud_armor_y - 6, armor_text, _target);
		}
	});

	// 经验条 + 等级：Lv 数字画在条的右侧
	_root_scene.each<Progression>([&](Progression &progression) {
		setfillcolor(rgb(50, 60, 78), _target);
		bar(hud_xp_x, hud_xp_y, hud_xp_x + hud_xp_width, hud_xp_y + hud_xp_height, _target);

		int filled = static_cast<int>(static_cast<float>(hud_xp_width) * progression.progress());
		if (filled > 0) {
			setfillcolor(rgb(90, 220, 235), _target);
			bar(hud_xp_x, hud_xp_y, hud_xp_x + filled, hud_xp_y + hud_xp_height, _target);
		}

		setfont(hud_value_font_height, 0, "Consolas", _target);
		setbkmode(TRANSPARENT, _target);
		settextcolor(rgb(200, 240, 255), _target);
		std::string label = "Lv " + std::to_string(progression.level());
		outtextxy(hud_xp_x + hud_xp_width + hud_label_gap, hud_xp_y - 4, label.c_str(), _target);
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

bool App::_is_choosing() {
	bool choosing = false;
	_root_scene.each<UpgradeSystem>([&](UpgradeSystem &system) {
		if (system.is_choosing()) {
			choosing = true;
		}
	});
	return choosing;
}

bool App::_world_paused() { return _state != State::Playing || _pause_menu.is_open() || _is_choosing(); }

SpiritId App::add_spirit(std::unique_ptr<Spirit> spirit) { return _root_scene.add_spirit(std::move(spirit)); }

bool App::remove_spirit(SpiritId id) { return _root_scene.remove_spirit(id); }

Spirit *App::find_spirit(SpiritId id) { return _root_scene.find_spirit(id); }

Scene &App::root_scene() { return _root_scene; }
