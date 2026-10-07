#include "actor.h"

#include <algorithm>
#include <cmath>

#include "obstacle.h"
#include "platform.h"
#include "scene.h"

// 必须高于 enemy_chase_speed(110)，否则玩家贴住敌人后无法脱身
constexpr float contact_speed_limit = 150.0f;

Actor::Actor(float center_x, float center_y, float width, float height, int window_width, int window_height)
	: Rect(center_x, center_y, width, height) {
	_window_width = window_width;
	_window_height = window_height;
	_speed_x = 0.0f;
	_speed_y = 0.0f;
	_on_ground = false;
}

float Actor::speed_x() const { return _speed_x; }

float Actor::speed_y() const { return _speed_y; }

bool Actor::on_ground() const { return _on_ground; }

bool Actor::overlaps(const Actor &other) const {
	return std::abs(_x - other._x) < half_width() + other.half_width() &&
		   std::abs(_y - other._y) < half_height() + other.half_height();
}

float Actor::segment_hit_t(float x0, float y0, float x1, float y1) const {
	return Rect::segment_aabb_hit_t(left(), top(), right(), bottom(), x0, y0, x1, y1);
}

// 只压水平速度：把整条速度向量一起缩放会连起跳速度也压掉，玩家贴住敌人后就跳不出去；
// 而且敌人每帧都会重新赋值水平速度，这个阻尼对它们本来就不生效。
void Actor::resolve_contacts(Scene &scene) {
	scene.each<Actor>([&](Actor &other) {
		if (&other == this || !overlaps(other)) {
			return;
		}

		if (std::abs(_speed_x) > contact_speed_limit) {
			_speed_x = std::copysign(contact_speed_limit, _speed_x);
		}
	});
}

bool Actor::resolve_world(Scene &scene, float previous_y, bool drop_through) {
	float half_w = half_width();
	float half_h = half_height();
	bool blocked = false;

	_on_ground = false;

	scene.each<Obstacle>([&](Obstacle &obstacle) {
		float dx = _x - obstacle.center_x();
		float dy = _y - obstacle.center_y();
		float overlap_x = (half_w + obstacle.half_width()) - std::abs(dx);
		float overlap_y = (half_h + obstacle.half_height()) - std::abs(dy);
		if (overlap_x <= 0.0f || overlap_y <= 0.0f) {
			return;
		}

		if (overlap_x < overlap_y) {
			_x += dx > 0.0f ? overlap_x : -overlap_x;
			_speed_x = 0.0f;
			blocked = true;
		} else {
			_y += dy > 0.0f ? overlap_y : -overlap_y;
			_speed_y = 0.0f;
			if (dy < 0.0f) {
				_on_ground = true;
			}
		}
	});

	if (!drop_through) {
		float previous_bottom = previous_y + half_h;
		float bottom = _y + half_h;
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
			if (_x + half_w <= platform.left() || _x - half_w >= platform.right()) {
				return;
			}
			_y = platform.top() - half_h;
			_speed_y = 0.0f;
			_on_ground = true;
		});
	}

	float clamped_x = std::clamp(_x, half_w, static_cast<float>(_window_width) - half_w);
	if (clamped_x != _x) {
		_x = clamped_x;
		blocked = true;
	}

	float ground = static_cast<float>(_window_height) - half_h;
	float ceiling = half_h;
	if (_y >= ground) {
		_y = ground;
		_speed_y = 0.0f;
		_on_ground = true;
	} else if (_y <= ceiling) {
		_y = ceiling;
		_speed_y = std::max(_speed_y, 0.0f);
	}

	return blocked;
}
