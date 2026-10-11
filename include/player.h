#pragma once

#include <bitset>
#include <cstddef>
#include <random>
#include <vector>

#include "actor.h"
#include "upgrade.h"

constexpr std::size_t MOVE_RIGHT = 0;
constexpr std::size_t MOVE_LEFT = 1;

class Player : public Actor {
	protected:
	std::bitset<2> _move_flags;
	float _aim_x, _aim_y;
	float _fire_cooldown;
	float _move_ratio;
	bool _firing;
	bool _jump_requested;
	bool _jump_held;
	int _air_jumps;  // 剩余空中跳次数（二段跳）
	bool _drop_through;
	int _ammo;
	float _reload_timer;
	bool _reloading;
	int _health;
	float _invulnerable_timer;
	float _spawn_x, _spawn_y;
	float _pickup_radius;
	std::mt19937 _rng;

	// 受升级影响的数值：apply_modifiers() 每次都从基础值整体重算
	int _max_health;
	float _fire_interval;
	int _magazine_capacity;
	float _speed_limit;
	int _bullet_damage;
	int _explosive_stacks;
	int _pierce_stacks;
	int _lifesteal_per_kill;
	int _thorns_stacks;
	float _experience_gain;
	float _armor;
	int _armor_max;
	float _armor_regen_timer;  // 脱战计时：受击清零，攒够就开始回甲
	float _reload_duration;
	int _shotgun_stacks;
	float _jump_velocity;
	float _invulnerable_duration;
	float _armor_regen_delay;
	float _armor_regen_rate;
	int _ammo_on_kill;
	int _air_jumps_max;
	int _ricochet_stacks;
	float _elite_damage_bonus;
	int _drone_count;
	float _drone_angle;
	std::vector<float> _drone_timers;

	void _fire(Scene &scene);
	float _current_spread() const;
	void _start_reload();
	// 霰弹：一次射击消耗的弹药 / 实际射击间隔 / 本次弹丸数（含概率补足）
	int _ammo_per_shot() const;
	float _shot_interval() const;
	int _roll_pellet_count();
	// 僚机：环绕、索敌、自动开火
	void _update_drones(float delta, Scene &scene);

	public:
	Player(int window_width, int window_height, float spawn_x, float spawn_y);

	int ammo() const;
	int magazine_size() const;
	bool is_reloading() const;
	float reload_progress() const;

	int health() const;
	int max_health() const;
	// 预留接口：目前无调用点
	bool is_invulnerable() const;
	float pickup_radius() const;
	float experience_gain() const;
	float armor() const;
	int max_armor() const;

	// 暂停界面「当前属性」用
	float fire_interval() const;
	int bullet_damage() const;
	float speed_limit() const;
	float reload_duration() const;
	int lifesteal_per_kill() const;
	float jump_velocity() const;
	float invulnerable_duration() const;
	float armor_regen_delay() const;
	float armor_regen_rate() const;
	int ammo_on_kill() const;

	// 按升级层数重算全部数值（当前血量按同一比例回复，向上取整）
	void apply_modifiers(const UpgradeModifiers &mods);
	// 击杀回调：汲取与击杀补弹在这里结算；精英怪回得更多
	void on_enemy_killed(bool elite);

	// 朝 angle 方向打出一发子弹（含全部子弹升级）；本体和僚机共用
	void fire_bullet(Scene &scene, float x, float y, float angle);

	void draw(PIMAGE image) override;
	void update(float delta, Scene &scene) override;
	void handle_keyboard(key_msg msg) override;
	void handle_mouse(mouse_msg msg) override;
};
