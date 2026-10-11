#include "spawn_effect.h"

#include <algorithm>

#include "define.h"

constexpr float spawn_effect_duration = 0.28f;
constexpr float spawn_effect_ring_max = 22.0f;

SpawnEffect::SpawnEffect(float x, float y) {
	_x = x;
	_y = y;
	_life = spawn_effect_duration;
}

void SpawnEffect::draw(PIMAGE image) {
	float progress = std::clamp(1.0f - _life / spawn_effect_duration, 0.0f, 1.0f);

	// 外环：从中心扩散出去
	setlinecolor(rgb(80, 240, 255), image);
	setlinewidth(2.0f, image);
	circlef(_x, _y, 4.0f + spawn_effect_ring_max * progress, image);

	// 内层实心圆：快速收缩，强调"刚从这里冒出来"
	setfillcolor(rgb(200, 255, 255), image);
	fillcirclef(_x, _y, 7.0f * (1.0f - progress), image);
	setlinewidth(1.0f, image);
}

void SpawnEffect::update(float delta, Scene &scene [[maybe_unused]]) { _life -= delta; }

bool SpawnEffect::is_invalid() const { return _life <= 0.0f; }
