#include "bullet.h"

#include <cmath>
#include <vector>

#include "define.h"
#include "enemy.h"
#include "explosion_effect.h"
#include "hit_effect.h"
#include "obstacle.h"
#include "random_seed.h"
#include "scene.h"

constexpr float bullet_length = 12.0f;
constexpr float bullet_width = 3.0f;
constexpr float bullet_gravity_scale = 0.5f;
constexpr float explosive_radius = 30.0f;
constexpr float explosive_chance_per_stack = 0.15f;

Bullet::Bullet(float x, float y, float dir_x, float dir_y, float speed, int damage, int explosive, int pierce,
			   int window_width, int window_height)
	: _rng(make_random_seed(static_cast<std::uint64_t>(static_cast<unsigned>(x)))) {
	_window_width = window_width;
	_window_height = window_height;
	_x = x;
	_y = y;
	_vel_x = dir_x * speed;
	_vel_y = dir_y * speed;
	_hit = false;
	_damage = damage;
	_explosive = explosive;
	_pierce = pierce;
}

bool Bullet::_already_hit(std::uint64_t serial) const {
	for (std::uint64_t hit : _hit_enemies) {
		if (hit == serial) {
			return true;
		}
	}
	return false;
}

void Bullet::_explode(Scene &scene, float x, float y, const Enemy *direct) {
	if (_explosive <= 0) {
		return;
	}

	std::uniform_real_distribution<float> roll(0.0f, 1.0f);
	if (roll(_rng) >= explosive_chance_per_stack * static_cast<float>(_explosive)) {
		return;
	}

	// 先收集再结算，避免在遍历敌人的同时改动场景
	std::vector<Enemy *> victims;
	scene.each<Enemy>([&](Enemy &enemy) {
		if (&enemy == direct) {
			return;
		}
		float dx = enemy.center_x() - x;
		float dy = enemy.center_y() - y;
		if (dx * dx + dy * dy <= explosive_radius * explosive_radius) {
			victims.push_back(&enemy);
		}
	});

	for (Enemy *victim : victims) {
		victim->take_damage(_damage, scene);
	}

	scene.add_spirit<ExplosionEffect>(x, y, explosive_radius);
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
		{ _x + hx + px, _y + hy + py },
		{ _x + hx - px, _y + hy - py },
		{ _x - hx - px, _y - hy - py },
		{ _x - hx + px, _y - hy + py },
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

	float obstacle_t = -1.0f;
	scene.each<Obstacle>([&](Obstacle &obstacle) {
		float t = obstacle.segment_hit_t(previous_x, previous_y, _x, _y);
		if (t >= 0.0f && (obstacle_t < 0.0f || t < obstacle_t)) {
			obstacle_t = t;
		}
	});

	// 已经穿透过的敌人不再重复结算
	float enemy_t = -1.0f;
	Enemy *enemy_hit = nullptr;
	scene.each<Enemy>([&](Enemy &enemy) {
		if (_already_hit(enemy.serial())) {
			return;
		}
		float t = enemy.segment_hit_t(previous_x, previous_y, _x, _y);
		if (t >= 0.0f && (enemy_t < 0.0f || t < enemy_t)) {
			enemy_t = t;
			enemy_hit = &enemy;
		}
	});

	// 取更近的那个：障碍物优先，避免隔着墙打到敌人
	float hit_t = -1.0f;
	bool hit_obstacle = false;
	if (obstacle_t >= 0.0f && (enemy_t < 0.0f || obstacle_t <= enemy_t)) {
		hit_t = obstacle_t;
		hit_obstacle = true;
	} else if (enemy_t >= 0.0f) {
		hit_t = enemy_t;
	}

	if (hit_t < 0.0f) {
		return;
	}

	float hit_x = previous_x + (_x - previous_x) * hit_t;
	float hit_y = previous_y + (_y - previous_y) * hit_t;
	scene.add_spirit<HitEffect>(hit_x, hit_y);

	if (hit_obstacle) {
		_hit = true;
		return;
	}

	enemy_hit->take_damage(_damage, scene);
	_hit_enemies.push_back(enemy_hit->serial());
	_explode(scene, hit_x, hit_y, enemy_hit);

	if (_pierce > 0) {
		--_pierce;
	} else {
		_hit = true;
	}
}

bool Bullet::is_invalid() const {
	if (_hit) {
		return true;
	}

	float margin = bullet_length / 2.0f;
	return _x < -margin || _x > static_cast<float>(_window_width) + margin || _y < -margin ||
		   _y > static_cast<float>(_window_height) + margin;
}
