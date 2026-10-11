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
		bool _loot_dropped;
		std::mt19937 _rng;

		// 在游荡区域内均匀随机取一个目标点；avoid_direction 非零时只在反方向那半边取
		float _pick_wander_target(float avoid_direction);
		// 死亡时掉出经验球；敌人一死就不再进 update，所以只能在伤害结算当帧调用
		void _drop_loot(Scene &scene);

	public:
		Enemy(float x, float y, int window_width, int window_height);

		void take_damage(int amount, Scene &scene);
		int health() const;
		bool is_dead() const;

		void draw(PIMAGE image) override;
		void update(float delta, Scene &scene) override;
		bool is_invalid() const override;
};
