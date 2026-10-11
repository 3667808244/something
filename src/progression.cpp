#include "progression.h"

#include <algorithm>
#include <string>

#include "define.h"

namespace {

constexpr int progression_base_requirement = 20;  // 1 -> 2 级所需经验
constexpr int progression_step = 15;              // 每升一级需求递增
constexpr float progression_flash_duration = 1.2f;
constexpr int progression_flash_font_height = 18;
constexpr int progression_flash_y = 84;

}  // namespace

Progression::Progression(int window_width) {
	_experience = 0;
	_level = 1;
	_experience_to_next = progression_base_requirement;
	_level_up_flash = 0.0f;
	_level_down_flash = 0.0f;
	_window_width = window_width;
}

void Progression::_refresh_requirement() {
	_experience_to_next = progression_base_requirement + progression_step * (_level - 1);
}

void Progression::add_experience(int amount) {
	if (amount <= 0) {
		return;
	}

	_experience += amount;
	while (_experience >= _experience_to_next) {
		_experience -= _experience_to_next;
		++_level;
		_refresh_requirement();
		_level_up_flash = progression_flash_duration;
	}
}

void Progression::lose_level() {
	if (_level <= 1) {
		// 已经是最低等级，没有可扣的等级
		return;
	}
	--_level;
	_experience = 0;
	_refresh_requirement();
	_level_down_flash = progression_flash_duration;
}

int Progression::level() const { return _level; }

int Progression::experience() const { return _experience; }

int Progression::experience_to_next() const { return _experience_to_next; }

float Progression::progress() const {
	if (_experience_to_next <= 0) {
		return 1.0f;
	}
	return std::clamp(static_cast<float>(_experience) / static_cast<float>(_experience_to_next), 0.0f, 1.0f);
}

void Progression::update(float delta, Scene &scene [[maybe_unused]]) {
	_level_up_flash = std::max(0.0f, _level_up_flash - delta);
	_level_down_flash = std::max(0.0f, _level_down_flash - delta);
}

void Progression::draw(PIMAGE image [[maybe_unused]]) {}

void Progression::draw_overlay(PIMAGE image) {
	if (_level_up_flash <= 0.0f && _level_down_flash <= 0.0f) {
		return;
	}

	setfont(progression_flash_font_height, 0, "Consolas", image);
	setbkmode(TRANSPARENT, image);

	std::string text;
	if (_level_down_flash > 0.0f) {
		// 掉级优先显示，用红色和升级区分
		bool bright = static_cast<int>(_level_down_flash * 12.0f) % 2 == 0;
		settextcolor(bright ? rgb(255, 120, 100) : rgb(180, 70, 60), image);
		text = "LEVEL DOWN  Lv " + std::to_string(_level);
	} else {
		// 用亮度闪烁代替 alpha 淡出
		bool bright = static_cast<int>(_level_up_flash * 12.0f) % 2 == 0;
		settextcolor(bright ? rgb(150, 255, 230) : rgb(90, 190, 175), image);
		text = "LEVEL UP  Lv " + std::to_string(_level);
	}

	int width = textwidth(text.c_str(), image);
	ege_outtextxy(static_cast<float>(_window_width / 2 - width / 2), static_cast<float>(progression_flash_y),
				  text.c_str(), image);
}

bool Progression::is_invalid() const { return false; }
