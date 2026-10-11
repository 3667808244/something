#include "explosion_effect.h"

#include <algorithm>

#include "define.h"

constexpr float explosion_duration = 0.30f;

ExplosionEffect::ExplosionEffect(float x, float y, float radius) {
	_x = x;
	_y = y;
	_radius = radius;
	_life = explosion_duration;
}

void ExplosionEffect::draw(PIMAGE image) {
	float progress = std::clamp(1.0f - _life / explosion_duration, 0.0f, 1.0f);
	float ring = _radius * progress;

	setlinecolor(rgb(255, 170, 60), image);
	setlinewidth(3.0f, image);
	circlef(_x, _y, ring, image);

	setlinecolor(rgb(255, 90, 40), image);
	setlinewidth(1.5f, image);
	circlef(_x, _y, ring * 0.65f, image);

	setfillcolor(rgb(255, 225, 150), image);
	fillcirclef(_x, _y, _radius * 0.28f * (1.0f - progress), image);
	setlinewidth(1.0f, image);
}

void ExplosionEffect::update(float delta, Scene &scene [[maybe_unused]]) { _life -= delta; }

bool ExplosionEffect::is_invalid() const { return _life <= 0.0f; }
