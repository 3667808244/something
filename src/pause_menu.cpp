#include "pause_menu.h"

#include <cstddef>

#include "define.h"

namespace {

const wchar_t *ui_font = L"Microsoft YaHei";

constexpr int button_width = 220;
constexpr int button_height = 40;
constexpr int button_gap = 12;
constexpr int panel_width = 300;
constexpr int panel_height = 340;

}  // namespace

PauseMenu::PauseMenu(int window_width, int window_height)
	: _window_width(window_width), _window_height(window_height) {
	_layout();
}

void PauseMenu::_layout() {
	_buttons.clear();
	const int panel_top = (_window_height - panel_height) / 2;
	const int first_y = panel_top + 74;
	for (int i = 0; i < 4; ++i) {
		UiButton button;
		button.id = i;
		button.left = (_window_width - button_width) / 2;
		button.top = first_y + i * (button_height + button_gap);
		button.width = button_width;
		button.height = button_height;
		_buttons.push_back(button);
	}
	_buttons[0].label = L"继续";
	_buttons[1].label = L"重新开始";
	_buttons[2].label = L"返回开始界面";
	_buttons[3].label = L"退出游戏";
}

void PauseMenu::open() {
	_open = true;
	_hover = -1;
	_pending = PauseAction::None;
}

void PauseMenu::close() {
	_open = false;
	_hover = -1;
	_pending = PauseAction::None;
}

bool PauseMenu::is_open() const { return _open; }

PauseAction PauseMenu::take_action() {
	const PauseAction action = _pending;
	_pending = PauseAction::None;
	return action;
}

void PauseMenu::_activate(int index) {
	switch (index) {
		case 0: _pending = PauseAction::Resume; break;
		case 1: _pending = PauseAction::Restart; break;
		case 2: _pending = PauseAction::BackToStart; break;
		case 3: _pending = PauseAction::Quit; break;
		default: break;
	}
}

void PauseMenu::handle_keyboard(key_msg msg) {
	if (!_open || msg.msg != ege::key_msg_down) {
		return;
	}
	const int count = static_cast<int>(_buttons.size());
	if (count <= 0) {
		return;
	}
	if (msg.key == key_W || msg.key == key_up) {
		_highlight = (_highlight + count - 1) % count;
	} else if (msg.key == key_S || msg.key == key_down) {
		_highlight = (_highlight + 1) % count;
	} else if (msg.key == key_enter || msg.key == key_space) {
		_activate(_highlight);
	} else if (msg.key == key_esc) {
		_pending = PauseAction::Resume;
	}
}

void PauseMenu::handle_mouse(mouse_msg msg) {
	if (!_open) {
		return;
	}
	_hover = -1;
	for (std::size_t i = 0; i < _buttons.size(); ++i) {
		if (_buttons[i].contains(msg.x, msg.y)) {
			_hover = static_cast<int>(i);
			break;
		}
	}
	if (!(msg.is_left() && msg.is_down()) || _hover < 0) {
		return;
	}
	_highlight = _hover;
	_activate(_hover);
}

void PauseMenu::draw(PIMAGE image) {
	if (!_open) {
		return;
	}
	ui_draw_dim(image, _window_width, _window_height, 160);

	const int panel_left = (_window_width - panel_width) / 2;
	const int panel_top = (_window_height - panel_height) / 2;
	ui_draw_panel(image, panel_left, panel_top, panel_width, panel_height);
	ui_draw_text_center(image, _window_width / 2, panel_top + 24, 22, ui_font, rgb(235, 242, 252), L"已暂停");

	for (std::size_t i = 0; i < _buttons.size(); ++i) {
		ui_draw_button(image, _buttons[i], _highlight == static_cast<int>(i) || _hover == static_cast<int>(i));
	}

	ui_draw_text_center(image, _window_width / 2, panel_top + panel_height - 34, 12, ui_font, rgb(130, 150, 175),
												L"W/S 选择    Enter 确认    Esc 继续");
	ui_draw_cursor(image);
}
