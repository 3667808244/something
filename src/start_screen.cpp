#include "start_screen.h"

#include <algorithm>
#include <string>
#include <utility>

#include "define.h"

namespace {

const wchar_t *ui_font = L"Microsoft YaHei";

constexpr int menu_button_width = 240;
constexpr int menu_button_height = 42;
constexpr int menu_button_gap = 14;
constexpr int menu_first_y = 208;

constexpr int level_button_width = 360;
constexpr int level_button_height = 32;
constexpr int level_button_gap = 8;
constexpr int level_first_y = 176;

const wchar_t *help_lines[] = {
	L"A / D        左右移动",
	L"W            跳跃",
	L"S            从单向平台向下穿过",
	L"鼠标         瞄准",
	L"鼠标左键     射击（按住连发）",
	L"R            换弹",
	L"1 / 2 / 3    升级选卡",
	L"Esc          暂停 / 返回",
	L"F1           显示 / 隐藏刷怪点",
	L"F2           叠加调试信息",
	L"F3           显示升级记录",
};

}  // namespace

StartScreen::StartScreen(int window_width, int window_height)
	: _window_width(window_width), _window_height(window_height) {
	_layout();
}

void StartScreen::set_levels(std::vector<LevelOption> levels, std::size_t default_index) {
	_levels = std::move(levels);
	if (_levels.empty()) {
		_levels.push_back({ std::string(), L"内置地图" });
	}
	if (default_index >= _levels.size()) {
		default_index = 0;
	}
	_selected = default_index;
	_level_highlight = static_cast<int>(default_index);
	_level_scroll = 0;
	_layout();
}

void StartScreen::set_error(const std::string &message) { _error = message; }

const LevelOption &StartScreen::selected_level() const { return _levels[_selected]; }

StartAction StartScreen::take_action() {
	const StartAction action = _pending;
	_pending = StartAction::None;
	return action;
}

void StartScreen::_layout() {
	_menu_buttons.clear();
	for (int i = 0; i < 4; ++i) {
		UiButton button;
		button.id = i;
		button.left = (_window_width - menu_button_width) / 2;
		button.top = menu_first_y + i * (menu_button_height + menu_button_gap);
		button.width = menu_button_width;
		button.height = menu_button_height;
		_menu_buttons.push_back(button);
	}
	_menu_buttons[0].label = L"开始游戏";
	_menu_buttons[1].label = L"选择地图";
	_menu_buttons[2].label = L"操作说明";
	_menu_buttons[3].label = L"退出";

	_back_button.id = 0;
	_back_button.label = L"返回";
	_back_button.width = 140;
	_back_button.height = 36;
	_back_button.left = (_window_width - _back_button.width) / 2;
	_back_button.top = _window_height - 72;
}

int StartScreen::_visible_levels() const {
	const int available = _back_button.top - level_first_y - 8;
	const int row = level_button_height + level_button_gap;
	return std::max(1, available / row);
}

UiButton StartScreen::_level_button_at(int index) const {
	UiButton button;
	button.id = index;
	button.left = (_window_width - level_button_width) / 2;
	button.top = level_first_y + (index - _level_scroll) * (level_button_height + level_button_gap);
	button.width = level_button_width;
	button.height = level_button_height;
	button.label = _levels[static_cast<std::size_t>(index)].label;
	return button;
}

void StartScreen::_activate_menu(int index) {
	if (index == 0) {
		_pending = StartAction::StartGame;
	} else if (index == 1) {
		_page = Page::LevelSelect;
		_error.clear();
		_level_highlight = static_cast<int>(_selected);
		_level_scroll = std::max(0, _level_highlight - _visible_levels() + 1);
		_hover = -1;
	} else if (index == 2) {
		_page = Page::Help;
		_hover = -1;
	} else if (index == 3) {
		_pending = StartAction::Quit;
	}
}

void StartScreen::handle_keyboard(key_msg msg) {
	if (msg.msg != ege::key_msg_down) {
		return;
	}

	if (_page == Page::Menu) {
		const int count = static_cast<int>(_menu_buttons.size());
		if (count <= 0) {
			return;
		}
		if (msg.key == key_W || msg.key == key_up) {
			_menu_highlight = (_menu_highlight + count - 1) % count;
		} else if (msg.key == key_S || msg.key == key_down) {
			_menu_highlight = (_menu_highlight + 1) % count;
		} else if (msg.key == key_enter || msg.key == key_space) {
			_activate_menu(_menu_highlight);
		} else if (msg.key == key_esc) {
			_pending = StartAction::Quit;
		}
	} else if (_page == Page::LevelSelect) {
		const int count = static_cast<int>(_levels.size());
		if (count <= 0) {
			return;
		}
		if (msg.key == key_W || msg.key == key_up) {
			_level_highlight = (_level_highlight + count - 1) % count;
		} else if (msg.key == key_S || msg.key == key_down) {
			_level_highlight = (_level_highlight + 1) % count;
		} else if (msg.key == key_enter || msg.key == key_space) {
			_selected = static_cast<std::size_t>(_level_highlight);
			_page = Page::Menu;
			// 选完地图后光标回到「开始游戏」，免得再按 Enter 又打开列表
			_menu_highlight = 0;
		} else if (msg.key == key_esc) {
			_page = Page::Menu;
		}
		const int visible = _visible_levels();
		if (_level_highlight < _level_scroll) {
			_level_scroll = _level_highlight;
		} else if (_level_highlight >= _level_scroll + visible) {
			_level_scroll = _level_highlight - visible + 1;
		}
	} else {
		if (msg.key == key_esc || msg.key == key_enter || msg.key == key_space) {
			_page = Page::Menu;
		}
	}
}

void StartScreen::handle_mouse(mouse_msg msg) {
	_hover = -1;
	const bool on_back = _page != Page::Menu && _back_button.contains(msg.x, msg.y);

	if (on_back) {
		_hover = -2;
	} else if (_page == Page::Menu) {
		for (std::size_t i = 0; i < _menu_buttons.size(); ++i) {
			if (_menu_buttons[i].contains(msg.x, msg.y)) {
				_hover = static_cast<int>(i);
				break;
			}
		}
	} else if (_page == Page::LevelSelect) {
		const int visible = _visible_levels();
		const int last = std::min(static_cast<int>(_levels.size()), _level_scroll + visible);
		for (int i = _level_scroll; i < last; ++i) {
			if (_level_button_at(i).contains(msg.x, msg.y)) {
				_hover = i;
				break;
			}
		}
	}

	if (!(msg.is_left() && msg.is_down())) {
		return;
	}

	if (on_back) {
		_page = Page::Menu;
		_hover = -1;
		return;
	}

	if (_page == Page::Menu && _hover >= 0) {
		_menu_highlight = _hover;
		_activate_menu(_hover);
		_hover = -1;
	} else if (_page == Page::LevelSelect && _hover >= 0) {
		_level_highlight = _hover;
		_selected = static_cast<std::size_t>(_hover);
		_page = Page::Menu;
		_menu_highlight = 0;
		_hover = -1;
	}
}

void StartScreen::_draw_menu(PIMAGE image) {
	for (std::size_t i = 0; i < _menu_buttons.size(); ++i) {
		ui_draw_button(image, _menu_buttons[i], _menu_highlight == static_cast<int>(i));
	}

	if (!_levels.empty()) {
		const std::wstring text = std::wstring(L"当前地图：") + _levels[_selected].label;
		const int map_y = menu_first_y + 4 * (menu_button_height + menu_button_gap) + 6;
		ui_draw_text_center(image, _window_width / 2, map_y, 14, ui_font, rgb(170, 190, 215), text.c_str());
	}
}

void StartScreen::_draw_level_select(PIMAGE image) {
	ui_draw_text_center(image, _window_width / 2, 148, 17, ui_font, rgb(215, 228, 245), L"选择地图");

	const int visible = _visible_levels();
	const int last = std::min(static_cast<int>(_levels.size()), _level_scroll + visible);
	for (int i = _level_scroll; i < last; ++i) {
		UiButton button = _level_button_at(i);
		if (static_cast<std::size_t>(i) == _selected) {
			button.label = std::wstring(L"● ") + button.label;
		}
		ui_draw_button(image, button, _level_highlight == i || _hover == i);
	}

	if (static_cast<int>(_levels.size()) > visible) {
		const std::wstring more = std::to_wstring(_level_scroll + 1) + L"-" + std::to_wstring(last) + L" / " +
																std::to_wstring(_levels.size());
		ui_draw_text_center(image, _window_width / 2, _back_button.top - 22, 12, ui_font, rgb(130, 150, 175), more.c_str());
	}

	ui_draw_button(image, _back_button, _hover == -2);
}

void StartScreen::_draw_help(PIMAGE image) {
	const int count = static_cast<int>(sizeof(help_lines) / sizeof(help_lines[0]));
	const int line_height = 19;
	const int panel_width = 420;
	const int panel_top = 140;
	const int panel_height = count * line_height + 46;
	const int panel_left = (_window_width - panel_width) / 2;

	ui_draw_panel(image, panel_left, panel_top, panel_width, panel_height);
	ui_draw_text_center(image, _window_width / 2, panel_top + 12, 16, ui_font, rgb(215, 228, 245), L"操作说明");

	setfont(13, 0, ui_font, image);
	setbkmode(TRANSPARENT, image);
	settextcolor(rgb(200, 212, 228), image);
	int y = panel_top + 44;
	for (int i = 0; i < count; ++i) {
		outtextxy(panel_left + 28, y, help_lines[i], image);
		y += line_height;
	}

	ui_draw_button(image, _back_button, _hover == -2);
}

void StartScreen::draw(PIMAGE image) {
	setfillcolor(rgb(10, 13, 20), image);
	bar(0, 0, _window_width, _window_height, image);

	ui_draw_text_center(image, _window_width / 2, 64, 36, ui_font, rgb(235, 242, 252), L"something");
	ui_draw_text_center(image, _window_width / 2, 114, 15, ui_font, rgb(150, 170, 195), L"EGE 2D 平台射击");

	if (_page == Page::Menu) {
		_draw_menu(image);
	} else if (_page == Page::LevelSelect) {
		_draw_level_select(image);
	} else {
		_draw_help(image);
	}

	const wchar_t *hint =
		(_page == Page::Menu) ? L"W/S 选择    Enter 确认    Esc 退出" : L"W/S 选择    Enter 确认    Esc 返回";
	ui_draw_text_center(image, _window_width / 2, _window_height - 26, 13, ui_font, rgb(120, 138, 162), hint);

	if (!_error.empty()) {
		const std::wstring wide = ui_utf8_to_wide(_error);
		ui_draw_text_center(image, _window_width / 2, _window_height - 50, 13, ui_font, rgb(255, 120, 100), wide.c_str());
	}

	ui_draw_cursor(image);
}
