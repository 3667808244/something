#pragma once

#include <random>

#include "actor.h"

class Enemy : public Actor {
	protected:
		int _health;
		bool _dead;
		float _flash_timer;
		bool _player_nearby;
		float _direction;
		bool _wandering;
		float _wander_center_x;
		float _wander_radius;
		float _wander_target_x;
		std::mt19937 _rng;

		// 在游荡区域内均匀随机取一个目标点；avoid_direction 非零时只在反方向那半边取
		float _pick_wander_target(float avoid_direction);

	public:
		Enemy(float x, float y, int window_width, int window_height);

		void take_damage(int amount);
		int health() const;
		bool is_dead() const;

		virtual void draw(PIMAGE image);
		virtual void update(float delta, Scene &scene);
		virtual bool is_invalid() const;
};
