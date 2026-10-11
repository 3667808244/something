#pragma once

#include <cstdint>
#include <random>

#include "actor.h"

// 普通敌人的基础数值：精英怪由 Spawner 按倍数从这两个值换算
inline constexpr int enemy_base_health = 3;
inline constexpr int enemy_base_exp_value = 5;

// 一只敌人的数值：普通怪和精英怪只在这份数据上不同
struct EnemyStats {
	int max_health = enemy_base_health;
	int exp_value = enemy_base_exp_value;
	bool elite = false;
};

class Enemy : public Actor {
	protected:
	int _max_health;
	int _health;
	int _exp_value;
	bool _elite;
	bool _dead;
	float _flash_timer;
	bool _player_nearby;
	float _direction;
	bool _wandering;
	float _wander_center_x;
	float _wander_radius;
	float _wander_target_x;
	bool _death_handled;
	std::uint64_t _serial;
	std::mt19937 _rng;

	// 在游荡区域内均匀随机取一个目标点；avoid_direction 非零时只在反方向那半边取
	float _pick_wander_target(float avoid_direction);
	// 死亡处理：掉经验球 + 通知玩家（汲取）。敌人一死就不再进 update，
	// 所以只能在伤害结算当帧调用。
	void _on_death(Scene &scene);

	public:
	Enemy(float x, float y, int window_width, int window_height, const EnemyStats &stats = EnemyStats{});

	void take_damage(int amount, Scene &scene);
	// 预留接口：目前无调用点
	int health() const;
	bool is_dead() const;
	bool is_elite() const;
	// 唯一序号：子弹穿透时用来记录已经打过的敌人
	std::uint64_t serial() const;

	void draw(PIMAGE image) override;
	void update(float delta, Scene &scene) override;
	bool is_invalid() const override;
};
