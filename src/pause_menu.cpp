#include "pause_menu.h"

#include <cstddef>
#include <cwchar>
#include <string>
#include <vector>

#include "define.h"

namespace {

const wchar_t *ui_font = L"Microsoft YaHei";

constexpr int panel_width = 600;
constexpr int panel_height = 430;
constexpr int button_width = 190;
constexpr int button_height = 40;
constexpr int button_gap = 12;
constexpr int button_first_y = 100;  // 相对面板顶部

// 右侧两列：升级层数 / 当前属性
constexpr int stack_x = 268;
constexpr int stack_step = 16;
constexpr int stat_x = 428;
constexpr int stat_value_x = 508;
constexpr int stats_header_y = 104;
constexpr int stats_first_y = 132;

std::wstring format_stat(const wchar_t *pattern, double value) {
	wchar_t buffer[64];
	std::swprintf(buffer, 64, pattern, value);
	return std::wstring(buffer);
}

}  // namespace

PauseMenu::PauseMenu(int window_width, int window_height)
	: _window_width(window_width), _window_height(window_height) {
	_layout();
}

void PauseMenu::_layout() {
	_buttons.clear();
	const int panel_left = (_window_width - panel_width) / 2;
	const int panel_top = (_window_height - panel_height) / 2;
	for (int i = 0; i < 4; ++i) {
		UiButton button;
		button.id = i;
		button.left = panel_left + 25;
		button.top = panel_top + button_first_y + i * (button_height + button_gap);
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

void PauseMenu::_draw_stats(PIMAGE image, const PauseInfo &info) {
	setbkmode(TRANSPARENT, image);
	setfont(15, 0, ui_font, image);
	settextcolor(rgb(215, 228, 245), image);
	outtextxy(stack_x, stats_header_y, L"升级层数", image);
	outtextxy(stat_x, stats_header_y, L"当前属性", image);

	setfont(13, 0, ui_font, image);

	// 左列：已经选过的升级与层数，没选的自动跳过
	int y = stats_first_y;
	bool any = false;
	const UpgradeTier tiers[] = { UpgradeTier::Common, UpgradeTier::Advanced };
	for (UpgradeTier tier : tiers) {
		for (UpgradeId id : upgrade_pool(tier)) {
			const int stacks = upgrade_stack_count(info.modifiers, id);
			if (stacks <= 0) {
				continue;
			}
			any = true;
			const UpgradeDef &def = upgrade_def(id);
			const std::wstring line = std::wstring(def.name) + L" ×" + std::to_wstring(stacks);
			settextcolor(def.tier == UpgradeTier::Advanced ? rgb(240, 200, 120) : rgb(170, 205, 240), image);
			outtextxy(stack_x, y, line.c_str(), image);
			y += stack_step;
		}
	}
	if (!any) {
		settextcolor(rgb(140, 150, 166), image);
		outtextxy(stack_x, stats_first_y, L"尚未选择任何升级", image);
	}

	// 右列：玩家当前属性
	struct Row {
		const wchar_t *name;
		std::wstring value;
	};
	std::vector<Row> rows;
	rows.push_back({ L"生命", std::to_wstring(info.health) + L" / " + std::to_wstring(info.max_health) });
	rows.push_back({ L"护甲", std::to_wstring(info.armor) + L" / " + std::to_wstring(info.max_armor) });
	rows.push_back({ L"弹匣", std::to_wstring(info.magazine) + L" 发" });
	rows.push_back({ L"子弹伤害", std::to_wstring(info.bullet_damage) });
	rows.push_back({ L"射击间隔", format_stat(L"%.3f s", info.fire_interval) });
	rows.push_back({ L"移动速度", format_stat(L"%.0f", info.move_speed) });
	rows.push_back({ L"换弹时间", format_stat(L"%.2f s", info.reload_duration) });
	rows.push_back({ L"汲取回血", info.lifesteal > 0 ? L"+" + std::to_wstring(info.lifesteal) : std::wstring(L"—") });
	rows.push_back({ L"经验加成",
					 info.experience_gain > 1.0f ? format_stat(L"+%.0f%%", (info.experience_gain - 1.0f) * 100.0f)
														 : std::wstring(L"—") });
	rows.push_back({ L"拾取半径", format_stat(L"%.0f", info.pickup_radius) });

	settextcolor(rgb(200, 212, 228), image);
	int row_y = stats_first_y;
	for (const Row &row : rows) {
		outtextxy(stat_x, row_y, row.name, image);
		outtextxy(stat_value_x, row_y, row.value.c_str(), image);
		row_y += stack_step;
	}
}

void PauseMenu::draw(PIMAGE image, const PauseInfo &info) {
	if (!_open) {
		return;
	}
	ui_draw_dim(image, _window_width, _window_height, 160);

	const int panel_left = (_window_width - panel_width) / 2;
	const int panel_top = (_window_height - panel_height) / 2;
	ui_draw_panel(image, panel_left, panel_top, panel_width, panel_height);
	ui_draw_text_center(image, _window_width / 2, panel_top + 14, 22, ui_font, rgb(235, 242, 252), L"已暂停");

	for (std::size_t i = 0; i < _buttons.size(); ++i) {
		ui_draw_button(image, _buttons[i], _highlight == static_cast<int>(i) || _hover == static_cast<int>(i));
	}

	_draw_stats(image, info);

	ui_draw_text_center(image, _window_width / 2, panel_top + panel_height - 30, 12, ui_font, rgb(130, 150, 175),
												L"W/S 选择    Enter 确认    Esc 继续");
	ui_draw_cursor(image);
}
