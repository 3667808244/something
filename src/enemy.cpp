#include "enemy.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

#include "define.h"
#include "player.h"
#include "random_seed.h"
#include "scene.h"

constexpr float enemy_size = 20.0f;
constexpr float enemy_fall_speed_max = 900.0f;
constexpr float enemy_chase_speed = 110.0f;
constexpr float enemy_wander_enter = 80.0f;
constexpr float enemy_wander_exit = 110.0f;
constexpr float enemy_wander_width_base = 40.0f;          // |dy| = 0 时的游荡总宽度
constexpr float enemy_wander_width_per_y = 1.0f / 3.0f;   // |dy| 每 1px 增加的宽度（即每 3px 加 1px）
constexpr float enemy_wander_width_max = 120.0f;          // 游荡总宽度上限
constexpr float enemy_wander_speed = 55.0f;
constexpr float enemy_wander_arrive = 4.0f;
constexpr int enemy_max_health = 3;
constexpr float enemy_flash_duration = 0.12f;
constexpr int enemy_bar_width = 24;
constexpr int enemy_bar_height = 3;
constexpr int enemy_bar_offset_y = 6;
constexpr float enemy_health_bar_range = 160.0f;
constexpr unsigned enemy_seed_salt = 2654435761u;

Enemy::Enemy(float x, float y, int window_width, int window_height)
    : Actor(x, y, enemy_size, enemy_size, window_width, window_height),
      // 混入出生 x，避免同一批创建的敌人因为时钟分辨率相同而拿到同一颗种子
      _rng(make_random_seed(static_cast<std::uint64_t>(static_cast<unsigned>(x)) * enemy_seed_salt)) {
	_health = enemy_max_health;
	_dead = false;
	_flash_timer = 0.0f;
	_player_nearby = false;
	_direction = 1.0f;
	_wandering = false;
	_wander_center_x = x;
	_wander_radius = enemy_wander_width_base / 2.0f;
	_wander_target_x = x;
}

void Enemy::take_damage(int amount) {
	if (_dead) {
		return;
	}

	_health -= amount;
	_flash_timer = enemy_flash_duration;

	if (_health <= 0) {
		_health = 0;
		_dead = true;
	}
}

int Enemy::health() const { return _health; }

bool Enemy::is_dead() const { return _dead; }

float Enemy::_pick_wander_target(float avoid_direction) {
	float lo = std::max(half_width(), _wander_center_x - _wander_radius);
	float hi = std::min(static_cast<float>(_window_width) - half_width(), _wander_center_x + _wander_radius);

	if (avoid_direction > 0.0f) {
		hi = std::min(hi, _x - enemy_wander_arrive);
	} else if (avoid_direction < 0.0f) {
		lo = std::max(lo, _x + enemy_wander_arrive);
	}

	if (hi <= lo) {
		return _wander_center_x;
	}

	std::uniform_real_distribution<float> dist(lo, hi);
	return dist(_rng);
}

void Enemy::draw(PIMAGE image) {
	setfillcolor(_flash_timer > 0.0f ? rgb(255, 255, 255) : rgb(180, 80, 220), image);
	bar(static_cast<int>(left()), static_cast<int>(top()), static_cast<int>(right()), static_cast<int>(bottom()), image);

	// 非满血 或 玩家在范围内，二者满足其一就画（并集）
	if (!_player_nearby && _health >= enemy_max_health) {
		return;
	}

	// 头顶血条：底色 + 按当前血量填充
	int bar_left = static_cast<int>(center_x()) - enemy_bar_width / 2;
	int bar_top = static_cast<int>(top()) - enemy_bar_offset_y;
	setfillcolor(rgb(40, 40, 48), image);
	bar(bar_left, bar_top, bar_left + enemy_bar_width, bar_top + enemy_bar_height, image);

	int filled = enemy_bar_width * _health / enemy_max_health;
	if (filled > 0) {
		setfillcolor(rgb(90, 210, 90), image);
		bar(bar_left, bar_top, bar_left + filled, bar_top + enemy_bar_height, image);
	}
}

void Enemy::update(float delta, Scene &scene) {
	if (_flash_timer > 0.0f) {
		_flash_timer -= delta;
	}

	float previous_y = _y;

	bool has_target = false;
	float dx = 0.0f;
	float dy = 0.0f;
	scene.each<Player>([&](Player &player) {
		dx = player.center_x() - _x;
		dy = player.center_y() - _y;
		has_target = true;
	});

	// 玩家是否在范围内（血条显示条件之一）
	_player_nearby = has_target && std::sqrt(dx * dx + dy * dy) <= enemy_health_bar_range;

	// 距离近时在当前位置附近的小区域里游荡；走远才转为追逐（带迟滞避免反复切换）
	if (!has_target) {
		_wandering = false;
	} else if (_wandering) {
		if (std::abs(dx) > enemy_wander_exit) {
			_wandering = false;
		}
	} else if (std::abs(dx) < enemy_wander_enter) {
		_wandering = true;
		_wander_center_x = _x;
	}

	if (_wandering) {
		// 游荡范围随高度差放大：总宽度 = 40 + |dy|/3，封顶 120，半径取一半
		float wander_width = enemy_wander_width_base + std::abs(dy) * enemy_wander_width_per_y;
		wander_width = std::min(wander_width, enemy_wander_width_max);
		_wander_radius = wander_width / 2.0f;

		if (std::abs(_x - _wander_target_x) < enemy_wander_arrive) {
			_wander_target_x = _pick_wander_target(0.0f);
		}
		_direction = _wander_target_x > _x ? 1.0f : -1.0f;
		_speed_x = _direction * enemy_wander_speed;
	} else if (has_target) {
		if (std::abs(dx) > 1.0f) {
			_direction = dx > 0.0f ? 1.0f : -1.0f;
		}
		_speed_x = _direction * enemy_chase_speed;
	} else {
		_speed_x = 0.0f;
	}

	_speed_y += scene.gravity() * delta;
	_speed_y = std::min(_speed_y, enemy_fall_speed_max);

	_x += _speed_x * delta;
	_y += _speed_y * delta;

	bool blocked = resolve_world(scene, previous_y, false);
	resolve_contacts(scene);

	// 被挡住就在反方向那半边重新取目标，避免贴着墙来回抖
	if (blocked && _wandering) {
		_wander_target_x = _pick_wander_target(_direction);
	}
}

bool Enemy::is_invalid() const { return _dead; }
