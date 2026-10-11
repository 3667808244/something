#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "ege/graphics.h"
#include "level_catalog.h"
#include "ui.h"

enum class StartAction { None, StartGame, Quit };

// 开始界面：主菜单 / 选择地图 / 操作说明三页
class StartScreen {
	public:
	StartScreen(int window_width, int window_height);

	void set_levels(std::vector<LevelOption> levels, std::size_t default_index);
	void set_error(const std::string &message);
	const LevelOption &selected_level() const;

	void handle_keyboard(key_msg msg);
	void handle_mouse(mouse_msg msg);
	StartAction take_action();

	void draw(PIMAGE image);

	private:
	enum class Page { Menu, LevelSelect, Help };

	void _layout();
	void _activate_menu(int index);
	int _visible_levels() const;
	UiButton _level_button_at(int index) const;
	void _draw_menu(PIMAGE image);
	void _draw_level_select(PIMAGE image);
	void _draw_help(PIMAGE image);

	int _window_width;
	int _window_height;
	std::vector<LevelOption> _levels;
	std::size_t _selected = 0;
	Page _page = Page::Menu;
	int _menu_highlight = 0;
	int _level_highlight = 0;
	int _level_scroll = 0;
	int _hover = -1;
	StartAction _pending = StartAction::None;
	std::string _error;

	std::vector<UiButton> _menu_buttons;
	UiButton _back_button;
};
