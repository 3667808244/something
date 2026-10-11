#pragma once

#include <vector>

#include "ege/graphics.h"
#include "ui.h"

enum class PauseAction { None, Resume, Restart, BackToStart, Quit };

// 暂停界面：继续 / 重新开始 / 返回开始界面 / 退出游戏
class PauseMenu {
	public:
	PauseMenu(int window_width, int window_height);

	void open();
	void close();
	bool is_open() const;

	void handle_keyboard(key_msg msg);
	void handle_mouse(mouse_msg msg);
	PauseAction take_action();

	void draw(PIMAGE image);

	private:
	void _layout();
	void _activate(int index);

	int _window_width;
	int _window_height;
	bool _open = false;
	int _highlight = 0;
	int _hover = -1;
	PauseAction _pending = PauseAction::None;
	std::vector<UiButton> _buttons;
};
