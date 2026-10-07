#include "player.h"

#include <algorithm>
#include <chrono>
#include <cmath>

#include "bullet.h"
#include "define.h"
#include "ege/graphics.h"
#include "obstacle.h"
#include "platform.h"
#include "scene.h"

constexpr float speed_max = 300.0f;
constexpr float acceleration = 1200.0f;
constexpr float collision_box_size = 20.0f;
constexpr float jump_speed = 420.0f;
constexpr float fall_speed_max = 900.0f;
constexpr float bullet_speed = 600.0f;
constexpr float fire_interval = 0.1f;
constexpr int magazine_capacity = 24;
constexpr float reload_duration = 1.2f;
constexpr float deg_to_rad = 3.14159265358979323846f / 180.0f;
constexpr float spread_min_deg = 1.0f;
constexpr float spread_max_deg = 8.0f;
constexpr float spread_sigma_scale = 3.0f;
constexpr float spread_smooth_rate = 6.0f;

Player::Player(int window_width, int window_height)
    : _rng(static_cast<std::mt19937::result_type>(std::random_device{}()) ^
           static_cast<std::mt19937::result_type>(std::chrono::steady_clock::now().time_since_epoch().count())) {
	_window_height = window_height;
	_window_width = window_width;
	_x = static_cast<float>(window_width) / 2;
	_y = static_cast<float>(window_height) / 2;
	_speed_x = 0;
	_speed_y = 0;
	_on_ground = false;
	_jump_requested = false;
	_jump_held = false;
	_drop_through = false;
	_aim_x = _x;
	_aim_y = _y;
	_fire_cooldown = 0;
	_move_ratio = 0.0f;
	_firing = false;
	_ammo = magazine_capacity;
	_reload_timer = 0.0f;
	_reloading = false;
}

void Player::draw(PIMAGE image) {
	setfillcolor(rgb(255, 0, 0), image);
	bar(static_cast<int>(_x - collision_box_size / 2), static_cast<int>(_y - collision_box_size / 2),
		static_cast<int>(_x + collision_box_size / 2), static_cast<int>(_y + collision_box_size / 2), image);
}

void Player::update(float delta, Scene &scene) {
	float ax = 0.0f;
	if (_move_flags.test(MOVE_LEFT)) ax -= 1.0f;
	if (_move_flags.test(MOVE_RIGHT)) ax += 1.0f;

	if (ax != 0.0f) {
		_speed_x += ax * acceleration * delta;
	} else {
		float drop = acceleration * delta;
		if (_speed_x > 0.0f) {
			_speed_x = std::max(0.0f, _speed_x - drop);
		} else if (_speed_x < 0.0f) {
			_speed_x = std::min(0.0f, _speed_x + drop);
		}
	}
	_speed_x = std::clamp(_speed_x, -speed_max, speed_max);

	_speed_y += scene.gravity() * delta;
	if (_jump_requested && _on_ground) {
		_speed_y = -jump_speed;
	}
	_jump_requested = false;
	_speed_y = std::min(_speed_y, fall_speed_max);

	float previous_y = _y;

	_x += _speed_x * delta;
	_y += _speed_y * delta;

	_x = std::clamp(_x, collision_box_size / 2, _window_width - collision_box_size / 2);

	_on_ground = false;

	float half = collision_box_size / 2.0f;
	scene.each<Obstacle>([&](Obstacle &obstacle) {
		float dx = _x - obstacle.center_x();
		float dy = _y - obstacle.center_y();
		float overlap_x = (half + obstacle.half_width()) - std::abs(dx);
		float overlap_y = (half + obstacle.half_height()) - std::abs(dy);
		if (overlap_x <= 0.0f || overlap_y <= 0.0f) {
			return;
		}

		if (overlap_x < overlap_y) {
			_x += dx > 0.0f ? overlap_x : -overlap_x;
			_speed_x = 0.0f;
		} else {
			_y += dy > 0.0f ? overlap_y : -overlap_y;
			_speed_y = 0.0f;
			if (dy < 0.0f) {
				_on_ground = true;
			}
		}
	});

	if (!_drop_through) {
		float previous_bottom = previous_y + half;
		float bottom = _y + half;
		scene.each<Platform>([&](Platform &platform) {
			if (_speed_y <= 0.0f) {
				return;
			}
			if (previous_bottom > platform.top() + 1e-3f) {
				return;
			}
			if (bottom < platform.top()) {
				return;
			}
			if (_x + half <= platform.left() || _x - half >= platform.right()) {
				return;
			}
			_y = platform.top() - half;
			_speed_y = 0.0f;
			_on_ground = true;
		});
	}

	float ground = _window_height - collision_box_size / 2;
	float ceiling = collision_box_size / 2;
	if (_y >= ground) {
		_y = ground;
		_speed_y = 0.0f;
		_on_ground = true;
	} else if (_y <= ceiling) {
		_y = ceiling;
		_speed_y = std::max(_speed_y, 0.0f);
	}

	float move_speed = std::sqrt(_speed_x * _speed_x + _speed_y * _speed_y);
	float target_ratio = std::clamp(move_speed / speed_max, 0.0f, 1.0f);
	_move_ratio += (target_ratio - _move_ratio) * (1.0f - std::exp(-spread_smooth_rate * delta));

	_fire_cooldown -= delta;
	if (_fire_cooldown < 0.0f) {
		_fire_cooldown = 0.0f;
	}

	if (_reloading) {
		_reload_timer -= delta;
		if (_reload_timer <= 0.0f) {
			_reload_timer = 0.0f;
			_reloading = false;
			_ammo = magazine_capacity;
		}
	} else if (_firing) {
		if (_fire_cooldown <= 0.0f) {
			if (_ammo > 0) {
				_fire(scene);
				--_ammo;
				_fire_cooldown = fire_interval;
			} else {
				_start_reload();
			}
		}
	}
}

void Player::handal_keybord(key_msg msg) {
	if (msg.key == key_A) {
		if (msg.msg == ege::key_msg_down) {
			_move_flags.set(MOVE_LEFT);
		} else if (msg.msg == ege::key_msg_up) {
			_move_flags.reset(MOVE_LEFT);
		}
	} else if (msg.key == key_D) {
		if (msg.msg == ege::key_msg_down) {
			_move_flags.set(MOVE_RIGHT);
		} else if (msg.msg == ege::key_msg_up) {
			_move_flags.reset(MOVE_RIGHT);
		}
	} else if (msg.key == key_W) {
		if (msg.msg == ege::key_msg_down) {
			if (!_jump_held) {
				_jump_requested = true;
				_jump_held = true;
			}
		} else if (msg.msg == ege::key_msg_up) {
			_jump_held = false;
		}
	} else if (msg.key == key_R) {
		if (msg.msg == ege::key_msg_down && !_reloading && _ammo < magazine_capacity) {
			_start_reload();
		}
	} else if (msg.key == key_S) {
		if (msg.msg == ege::key_msg_down) {
			_drop_through = true;
		} else if (msg.msg == ege::key_msg_up) {
			_drop_through = false;
		}
	}
}

void Player::handal_mouse(mouse_msg msg) {
	_aim_x = static_cast<float>(msg.x);
	_aim_y = static_cast<float>(msg.y);

	if (msg.is_left()) {
		if (msg.is_down()) {
			_firing = true;
			_fire_cooldown = 0.0f;
		} else if (msg.is_up()) {
			_firing = false;
		}
	}
}

void Player::_fire(Scene &scene) {
	float dx = _aim_x - _x;
	float dy = _aim_y - _y;
	float len = std::sqrt(dx * dx + dy * dy);
	if (len < 1e-4f) {
		dx = 1.0f;
		dy = 0.0f;
		len = 1.0f;
	}
	dx /= len;
	dy /= len;

	// 正态散射：spread 为 3σ 包络，并夹取到 ±spread 以避免极端离群
	float spread = _current_spread();
	std::normal_distribution<float> dist(0.0f, spread / spread_sigma_scale);
	float angle = std::clamp(dist(_rng), -spread, spread);

	float cos_a = std::cos(angle);
	float sin_a = std::sin(angle);
	float rx = dx * cos_a - dy * sin_a;
	float ry = dx * sin_a + dy * cos_a;

	scene.add_spirit<Bullet>(_x, _y, rx, ry, bullet_speed, _window_width, _window_height);
}

float Player::_current_spread() const {
	return (spread_min_deg + (spread_max_deg - spread_min_deg) * _move_ratio) * deg_to_rad;
}

void Player::_start_reload() {
	_reloading = true;
	_reload_timer = reload_duration;
}

int Player::ammo() const { return _ammo; }

int Player::magazine_size() const { return magazine_capacity; }

bool Player::is_reloading() const { return _reloading; }

float Player::reload_progress() const { return _reloading ? 1.0f - _reload_timer / reload_duration : 0.0f; }

bool Player::is_invalid() const { return false; }