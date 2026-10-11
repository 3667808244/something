#include "pickup.h"

#include <algorithm>
#include <cmath>

#include "player.h"
#include "scene.h"

namespace {

constexpr float pickup_default_lifetime = 15.0f;
constexpr float pickup_fall_speed_max = 900.0f;
constexpr float pickup_air_friction = 1.5f;    // 每秒衰减系数
constexpr float pickup_ground_friction = 8.0f;
constexpr float pickup_magnet_scale = 3.0f;    // 吸附范围 = 玩家拾取半径 * 该系数
constexpr float pickup_magnet_accel = 1600.0f;
constexpr float pickup_magnet_speed_max = 560.0f;
constexpr float pickup_blink_time = 3.0f;
constexpr float pickup_blink_hz = 6.0f;

}  // namespace

Pickup::Pickup(float x, float y, float width, float height, int value, int window_width, int window_height)
	: Actor(x, y, width, height, window_width, window_height) {
	_value = value;
	_life = pickup_default_lifetime;
	_collected = false;
}

int Pickup::value() const { return _value; }

bool Pickup::is_collected() const { return _collected; }

bool Pickup::is_solid() const { return false; }

bool Pickup::blink_hidden() const {
	if (_life > pickup_blink_time) {
		return false;
	}
	return static_cast<int>(_life * pickup_blink_hz) % 2 == 0;
}

void Pickup::update(float delta, Scene &scene) {
	float previous_y = _y;

	_speed_y += scene.gravity() * delta;
	_speed_y = std::min(_speed_y, pickup_fall_speed_max);

	_x += _speed_x * delta;
	_y += _speed_y * delta;

	// 复用 Actor 的落地/平台/窗口边界处理
	resolve_world(scene, previous_y, false);

	// 落地后摩擦停住；空中也给一点阻力，让掉落时散开的速度很快收敛
	float friction = _on_ground ? pickup_ground_friction : pickup_air_friction;
	_speed_x -= _speed_x * std::min(1.0f, friction * delta);

	_life -= delta;

	// 吸附与拾取：范围全部来自玩家的 pickup_radius()
	scene.each<Player>([&](Player &player) {
		if (_collected) {
			return;
		}

		float dx = player.center_x() - _x;
		float dy = player.center_y() - _y;
		float distance_sq = dx * dx + dy * dy;
		float radius = player.pickup_radius();

		if (distance_sq <= radius * radius) {
			apply(scene, player);
			_collected = true;
			return;
		}

		float magnet = radius * pickup_magnet_scale;
		if (distance_sq <= magnet * magnet && distance_sq > 1e-6f) {
			float distance = std::sqrt(distance_sq);
			_speed_x += dx / distance * pickup_magnet_accel * delta;
			_speed_y += dy / distance * pickup_magnet_accel * delta;

			float speed = std::sqrt(_speed_x * _speed_x + _speed_y * _speed_y);
			if (speed > pickup_magnet_speed_max) {
				float scale = pickup_magnet_speed_max / speed;
				_speed_x *= scale;
				_speed_y *= scale;
			}
		}
	});
}

bool Pickup::is_invalid() const { return _collected || _life <= 0.0f; }
