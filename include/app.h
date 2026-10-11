#pragma once

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "ege/graphics.h"
#include "frame_timer.h"
#include "level.h"
#include "level_catalog.h"
#include "pause_menu.h"
#include "scene.h"
#include "spirit.h"
#include "start_screen.h"

class App {
	protected:
	enum class State { StartScreen, Playing };

	PIMAGE _target;	 // EGE 的默认绘制目标（nullptr 即窗口）
	FrameTimer _frame_timer;
	int _window_width, _window_height;
	Scene _root_scene;

	State _state = State::StartScreen;
	StartScreen _start_screen;
	PauseMenu _pause_menu;
	std::vector<LevelOption> _level_options;
	LevelOption _current_level;
	LevelData _level_data;

	void _update(float delta);

	void _draw();

	void _draw_hud();

	void _draw_crosshair();

	void _handle_keyboard(key_msg msg);

	void _handle_mouse(mouse_msg msg);

	// 是否有升级卡正在等待选择
	bool _is_choosing();

	// 世界是否冻结：不在游戏内 / 暂停菜单打开 / 正在选卡
	bool _world_paused();

	// 载入并进入指定关卡；失败时把错误留在开始界面上并返回 false
	bool _start_level(const LevelOption &option);
	void _rebuild_scene();
	void _return_to_start_screen();

	public:
	App(int window_width, int window_height, const std::optional<std::string> &title = std::nullopt);

	~App();

	void set_level_options(std::vector<LevelOption> options, std::size_t default_index);

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
