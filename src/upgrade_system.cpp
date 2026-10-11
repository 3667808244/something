#include "upgrade_system.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cwchar>
#include <vector>

#include "define.h"
#include "player.h"
#include "progression.h"
#include "random_seed.h"
#include "scene.h"

namespace {

constexpr int card_width = 170;
constexpr int card_height = 216;
constexpr int card_gap = 16;
constexpr int card_panel_margin = 14;
constexpr int card_title_height = 58;  // 面板上方留给标题的高度
constexpr int card_top_offset = 24;
constexpr int history_font_height = 13;
constexpr int history_max_lines = 12;
constexpr unsigned upgrade_seed_salt = 0x0badc0deu;
const wchar_t *ui_font = L"Microsoft YaHei";

}  // namespace

UpgradeSystem::UpgradeSystem(int window_width, int window_height)
	: _window_width(window_width), _window_height(window_height), _rng(make_random_seed(upgrade_seed_salt)) {}

bool UpgradeSystem::is_choosing() const { return _choosing; }

const UpgradeModifiers &UpgradeSystem::modifiers() const { return _modifiers; }

const std::vector<UpgradeSystem::Choice> &UpgradeSystem::history() const { return _history; }

UpgradeTier UpgradeSystem::_tier_for_level(int level) const {
	return (level > 0 && level % 5 == 0) ? UpgradeTier::Advanced : UpgradeTier::Common;
}

int UpgradeSystem::_stack_count(UpgradeId id) const {
	int count = 0;
	for (const Choice &choice : _history) {
		if (choice.id == id) {
			++count;
		}
	}
	return count;
}

bool UpgradeSystem::_collect_candidates(UpgradeTier tier, std::vector<UpgradeId> &out) const {
	const std::vector<UpgradeId> &pool = upgrade_pool(tier);
	const std::deque<UpgradeId> &recent = _recent_picks[static_cast<std::size_t>(tier)];

	// 排除规则：普通池排除最近 1 次选中的，高级池排除最近 2 次选中的
	std::size_t exclude_count = tier == UpgradeTier::Advanced ? 2 : 1;
	std::vector<UpgradeId> excluded;
	for (std::size_t i = 0; i < exclude_count && i < recent.size(); ++i) {
		excluded.push_back(recent[recent.size() - 1 - i]);
	}

	auto collect = [&](bool ignore_exclusions) {
		out.clear();
		for (UpgradeId id : pool) {
			// 达到上限的升级永不再发，层数不会溢出
			const UpgradeDef &def = upgrade_def(id);
			if (def.max_stacks > 0 && _stack_count(id) >= def.max_stacks) {
				continue;
			}
			if (!ignore_exclusions) {
				bool skipped = false;
				for (UpgradeId excluded_id : excluded) {
					if (excluded_id == id) {
						skipped = true;
					}
				}
				if (skipped) {
					continue;
				}
			}
			out.push_back(id);
		}
	};

	collect(false);
	// 池子被排除规则挤到不足三张时放宽排除
	if (out.size() < offer_count) {
		collect(true);
	}
	return !out.empty();
}

void UpgradeSystem::_open_cards(int level) {
	UpgradeTier tier = _tier_for_level(level);
	std::vector<UpgradeId> candidates;
	if (!_collect_candidates(tier, candidates) && tier == UpgradeTier::Advanced) {
		// 高级升级已全部点满：该等级改发普通卡
		tier = UpgradeTier::Common;
		_collect_candidates(tier, candidates);
	}

	std::shuffle(candidates.begin(), candidates.end(), _rng);
	_offered_size = std::min(offer_count, candidates.size());
	for (std::size_t i = 0; i < _offered_size; ++i) {
		_offered[i] = candidates[i];
	}

	_offered_tier = tier;
	_hover = -1;
	_pending_pick = -1;
	_choosing = _offered_size > 0;
}

void UpgradeSystem::_advance_cards() {
	// 正常情况下开一批卡就停下；若某个等级确实无卡可发（池子被点满），
	// 丢弃该等级继续处理下一笔，避免 _pending_levels 永远推不动
	while (!_choosing && !_pending_levels.empty()) {
		_open_cards(_pending_levels.front());
		if (!_choosing) {
			_pending_levels.pop_front();
		}
	}
}

void UpgradeSystem::_pick(std::size_t index, Scene &scene) {
	if (!_choosing || index >= _offered_size) {
		return;
	}

	int level = _pending_levels.empty() ? _known_level : _pending_levels.front();
	_history.push_back({ level, _offered[index] });
	if (!_pending_levels.empty()) {
		_pending_levels.pop_front();
	}

	_recompute();
	_apply(scene);

	_choosing = false;
	_offered_size = 0;
	_hover = -1;
	_pending_pick = -1;

	_advance_cards();
}

void UpgradeSystem::_recompute() {
	_modifiers = UpgradeModifiers{};
	for (std::size_t i = 0; i < upgrade_tier_count; ++i) {
		_recent_picks[i].clear();
	}

	for (const Choice &choice : _history) {
		upgrade_add_stack(_modifiers, choice.id);
		std::deque<UpgradeId> &recent = _recent_picks[static_cast<std::size_t>(upgrade_def(choice.id).tier)];
		recent.push_back(choice.id);
		while (recent.size() > 2) {
			recent.pop_front();
		}
	}

#ifndef NDEBUG
	// F3 调试：把当前记录和累计修改器打到控制台，便于验证回滚（release 无控制台，直接编译掉）
	if (_show_history) {
		std::printf("[upgrade] level=%d history=%zu mods{fire=%d mag=%d hp=%d spd=%d dmg=%d aoe=%d leech=%d "
					"magnet=%d pierce=%d thorns=%d exp=%d}\n",
					_known_level, _history.size(), _modifiers.fire_rate, _modifiers.magazine, _modifiers.max_health,
					_modifiers.move_speed, _modifiers.bullet_damage, _modifiers.explosive, _modifiers.lifesteal,
					_modifiers.magnet, _modifiers.pierce, _modifiers.thorns, _modifiers.experience_gain);
		for (const Choice &choice : _history) {
			std::printf("  Lv%d id=%d\n", choice.level, static_cast<int>(choice.id));
		}
		std::fflush(stdout);
	}
#endif
}

void UpgradeSystem::_apply(Scene &scene) {
	scene.each<Player>([&](Player &player) { player.apply_modifiers(_modifiers); });
}

void UpgradeSystem::modal_update(Scene &scene) {
	if (_pending_pick >= 0) {
		_pick(static_cast<std::size_t>(_pending_pick), scene);
	}
}

void UpgradeSystem::update(float delta, Scene &scene) {
	(void)delta;

	if (_pending_pick >= 0) {
		_pick(static_cast<std::size_t>(_pending_pick), scene);
	}

	int level = 1;
	scene.each<Progression>([&](Progression &progression) { level = progression.level(); });

	if (!_known_level_valid) {
		_known_level_valid = true;
		_known_level = level;
	}

	if (level != _known_level) {
		int delta_level = level - _known_level;
		if (delta_level > 0) {
			for (int i = 0; i < delta_level; ++i) {
				_pending_levels.push_back(_known_level + 1 + i);
			}
		} else {
			for (int i = 0; i < -delta_level; ++i) {
				// 降级：先取消还没选的卡，其次回滚最近一次已选升级
				if (!_pending_levels.empty()) {
					_pending_levels.pop_back();
					if (_pending_levels.empty()) {
						_choosing = false;
						_offered_size = 0;
					}
				} else if (!_history.empty()) {
					_history.pop_back();
				}
			}
		}
		_known_level = level;
		_recompute();
		_apply(scene);
	}

	_advance_cards();
}

void UpgradeSystem::_card_rect(std::size_t index, int &left, int &top, int &width, int &height) const {
	int count = static_cast<int>(offer_count);
	int total = card_width * count + card_gap * (count - 1);
	left = (_window_width - total) / 2 + static_cast<int>(index) * (card_width + card_gap);
	top = (_window_height - card_height) / 2 + card_top_offset;
	width = card_width;
	height = card_height;
}

void UpgradeSystem::draw_overlay(PIMAGE image) {
	if (_choosing && _offered_size > 0) {
		int l0, t0, w0, h0, l1, t1, w1, h1;
		_card_rect(0, l0, t0, w0, h0);
		_card_rect(_offered_size - 1, l1, t1, w1, h1);

		setfillcolor(rgb(10, 14, 22), image);
		fillroundrect(l0 - card_panel_margin, t0 - card_title_height, l1 + w1 + card_panel_margin,
					  t0 + h0 + card_panel_margin, 16, image);
		setlinecolor(rgb(72, 86, 110), image);
		roundrect(l0 - card_panel_margin, t0 - card_title_height, l1 + w1 + card_panel_margin,
				  t0 + h0 + card_panel_margin, 16, image);

		UpgradeTier tier = _offered_tier;
		const wchar_t *title = tier == UpgradeTier::Advanced ? L"高级升级 · 三选一" : L"选择一项升级";

		setfont(19, 0, ui_font, image);
		setbkmode(TRANSPARENT, image);
		settextcolor(tier == UpgradeTier::Advanced ? rgb(255, 224, 150) : rgb(235, 242, 252), image);
		int title_width = textwidth(title, image);
		int center_x = (l0 + l1 + w1) / 2;
		outtextxy(center_x - title_width / 2, t0 - card_title_height + 18, title, image);

		for (std::size_t i = 0; i < _offered_size; ++i) {
			int left, top, width, height;
			_card_rect(i, left, top, width, height);
			const UpgradeDef &def = upgrade_def(_offered[i]);
			upgrade_draw_card(image, def, static_cast<int>(i), _stack_count(def.id), left, top, width, height,
							  _hover == static_cast<int>(i));
		}
	}

	if (_choosing) {
		_draw_cursor(image);
	}

	if (_show_history) {
		_draw_history(image);
	}
}

void UpgradeSystem::_draw_cursor(PIMAGE image) {
	int mouse_x = 0;
	int mouse_y = 0;
	mousepos(&mouse_x, &mouse_y);

	static const int arrow[7][2] = {
		{ 0, 0 }, { 0, 17 }, { 4, 13 }, { 8, 21 }, { 11, 19 }, { 7, 11 }, { 12, 11 },
	};

	ege_point points[7];
	for (int i = 0; i < 7; ++i) {
		points[i].x = static_cast<float>(mouse_x + arrow[i][0]);
		points[i].y = static_cast<float>(mouse_y + arrow[i][1]);
	}

	setfillcolor(rgb(255, 255, 255), image);
	ege_fillpoly(7, points, image);

	setlinecolor(rgb(20, 22, 28), image);
	for (int i = 0; i < 7; ++i) {
		int j = (i + 1) % 7;
		line(mouse_x + arrow[i][0], mouse_y + arrow[i][1], mouse_x + arrow[j][0], mouse_y + arrow[j][1], image);
	}
}

void UpgradeSystem::_draw_history(PIMAGE image) {
	std::size_t count = std::min(_history.size(), static_cast<std::size_t>(history_max_lines));
	std::size_t first = _history.size() - count;

	setfont(history_font_height, 0, ui_font, image);
	setbkmode(TRANSPARENT, image);

	int x = 12;
	int line_step = history_font_height + 3;
	int y = _window_height - 24 - static_cast<int>(count) * line_step;
	if (y < 76) {
		y = 76;
	}

	settextcolor(rgb(210, 220, 235), image);
	outtextxy(x, y - line_step - 4, L"升级记录 (F3)", image);

	wchar_t line[64];
	for (std::size_t i = 0; i < count; ++i) {
		const Choice &choice = _history[first + i];
		const UpgradeDef &def = upgrade_def(choice.id);
		std::swprintf(line, 64, L"Lv%d  %ls", choice.level, def.name);
		settextcolor(def.tier == UpgradeTier::Advanced ? rgb(240, 200, 120) : rgb(170, 205, 240), image);
		outtextxy(x, y + static_cast<int>(i) * line_step, line, image);
	}
}

void UpgradeSystem::handle_keyboard(key_msg msg) {
	if (msg.key == key_f3) {
		if (msg.msg == ege::key_msg_down) {
			if (!_f3_held) {
				_show_history = !_show_history;
				_f3_held = true;
			}
		} else if (msg.msg == ege::key_msg_up) {
			_f3_held = false;
		}
	}

	if (!_choosing || msg.msg != ege::key_msg_down) {
		return;
	}

	int index = -1;
	if (msg.key == key_1) {
		index = 0;
	} else if (msg.key == key_2) {
		index = 1;
	} else if (msg.key == key_3) {
		index = 2;
	}

	if (index >= 0 && static_cast<std::size_t>(index) < _offered_size) {
		_pending_pick = index;
		_hover = index;
	}
}

void UpgradeSystem::handle_mouse(mouse_msg msg) {
	if (!_choosing) {
		return;
	}

	int hovered = -1;
	for (std::size_t i = 0; i < _offered_size; ++i) {
		int left, top, width, height;
		_card_rect(i, left, top, width, height);
		if (msg.x >= left && msg.x < left + width && msg.y >= top && msg.y < top + height) {
			hovered = static_cast<int>(i);
		}
	}
	_hover = hovered;

	if (hovered >= 0 && msg.is_left() && msg.is_down()) {
		_pending_pick = hovered;
	}
}

void UpgradeSystem::draw(PIMAGE image [[maybe_unused]]) {}

bool UpgradeSystem::is_invalid() const { return false; }
