#include "bullet.h"

#include <cmath>

#include "define.h"
#include "hit_effect.h"
#include "obstacle.h"
#include "scene.h"

constexpr float bullet_length = 12.0f;
constexpr float bullet_width = 3.0f;
constexpr float bullet_gravity_scale = 0.5f;

Bullet::Bullet(float x, float y, float dir_x, float dir_y, float speed, int window_width, int window_height) {
	_window_width = window_width;
	_window_height = window_height;
	_x = x;
	_y = y;
	_vel_x = dir_x * speed;
	_vel_y = dir_y * speed;
	_hit = false;
}

void Bullet::draw(PIMAGE image) {
	float len = std::sqrt(_vel_x * _vel_x + _vel_y * _vel_y);
	float dx = 1.0f;
	float dy = 0.0f;
	if (len > 1e-4f) {
		dx = _vel_x / len;
		dy = _vel_y / len;
	}

	float hx = dx * bullet_length / 2.0f;
	float hy = dy * bullet_length / 2.0f;
	float px = -dy * bullet_width / 2.0f;
	float py = dx * bullet_width / 2.0f;

	ege_point points[4] = {
		{_x + hx + px, _y + hy + py},
		{_x + hx - px, _y + hy - py},
		{_x - hx - px, _y - hy - py},
		{_x - hx + px, _y - hy + py},
	};

	setfillcolor(rgb(255, 255, 0), image);
	ege_fillpoly(4, points, image);
}

void Bullet::update(float delta, Scene &scene) {
	float previous_x = _x;
	float previous_y = _y;

	_vel_y += scene.gravity() * bullet_gravity_scale * delta;
	_x += _vel_x * delta;
	_y += _vel_y * delta;

	float hit_t = -1.0f;
	scene.each<Obstacle>([&](Obstacle &obstacle) {
		float t = obstacle.segment_hit_t(previous_x, previous_y, _x, _y);
		if (t >= 0.0f && (hit_t < 0.0f || t < hit_t)) {
			hit_t = t;
		}
	});

	if (hit_t >= 0.0f) {
		_hit = true;
		scene.add_spirit<HitEffect>(previous_x + (_x - previous_x) * hit_t, previous_y + (_y - previous_y) * hit_t);
	}
}

void Bullet::handal_keybord(key_msg msg [[maybe_unused]]) {}

void Bullet::handal_mouse(mouse_msg msg [[maybe_unused]]) {}

bool Bullet::is_invalid() const {
	if (_hit) {
		return true;
	}

	float margin = bullet_length / 2.0f;
	return _x < -margin || _x > static_cast<float>(_window_width) + margin ||
	       _y < -margin || _y > static_cast<float>(_window_height) + margin;
}
