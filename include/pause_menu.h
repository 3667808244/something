#pragma once

#include <vector>

#include "ege/graphics.h"
#include "ui.h"
#include "upgrade.h"

enum class PauseAction { None, Resume, Restart, BackToStart, Quit };

// 暂停界面展示用的数据快照：由 App 从 Player / UpgradeSystem 收集后传进来
struct PauseInfo {
	UpgradeModifiers modifiers;  // 各升级叠了几层
	int health = 0;
	int max_health = 0;
	int armor = 0;
	int max_armor = 0;
	int magazine = 0;
	int bullet_damage = 0;
	float fire_interval = 0.0f;
	float move_speed = 0.0f;
	float reload_duration = 0.0f;
	int lifesteal = 0;
	float experience_gain = 1.0f;
	float pickup_radius = 0.0f;
};

// 暂停界面：左侧按钮（继续 / 重新开始 / 返回开始界面 / 退出游戏），
// 右侧显示已选升级的层数和玩家当前属性。
class PauseMenu {
	public:
	PauseMenu(int window_width, int window_height);

	void open();
	void close();
	bool is_open() const;

	void handle_keyboard(key_msg msg);
	void handle_mouse(mouse_msg msg);
	PauseAction take_action();

	void draw(PIMAGE image, const PauseInfo &info);

	private:
	void _layout();
	void _activate(int index);
	void _draw_stats(PIMAGE image, const PauseInfo &info);

	int _window_width;
	int _window_height;
	bool _open = false;
	int _highlight = 0;
	int _hover = -1;
	PauseAction _pending = PauseAction::None;
	std::vector<UiButton> _buttons;
};
