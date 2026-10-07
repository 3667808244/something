#include "hit_effect.h"

#include <algorithm>

#include "define.h"

constexpr float hit_effect_duration = 0.18f;
constexpr float hit_effect_ring_max = 12.0f;

HitEffect::HitEffect(float x, float y) {
	_x = x;
	_y = y;
	_life = hit_effect_duration;
}

void HitEffect::draw(PIMAGE image) {
	float progress = std::clamp(1.0f - _life / hit_effect_duration, 0.0f, 1.0f);

	int green = static_cast<int>(255.0f * (1.0f - progress));
	setlinecolor(rgb(255, green, 0), image);
	setlinewidth(2.0f, image);
	circlef(_x, _y, 2.0f + hit_effect_ring_max * progress, image);
	setlinewidth(1.0f, image);

	setfillcolor(rgb(255, 255, 200), image);
	fillcirclef(_x, _y, 3.0f * (1.0f - progress), image);
}

void HitEffect::update(float delta, Scene &scene [[maybe_unused]]) { _life -= delta; }

void HitEffect::handal_keybord(key_msg msg [[maybe_unused]]) {}

void HitEffect::handal_mouse(mouse_msg msg [[maybe_unused]]) {}

bool HitEffect::is_invalid() const { return _life <= 0.0f; }
