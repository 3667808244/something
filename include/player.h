#pragma once

#include <bitset>
#include <cstddef>
#include <random>

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

		void _fire(Scene &scene);
		float _current_spread() const;
		void _start_reload();
		// 霰弹：一次射击消耗的弹药 / 实际射击间隔 / 本次弹丸数（含概率补足）
		int _ammo_per_shot() const;
		float _shot_interval() const;
		int _roll_pellet_count();

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

		// 按升级层数重算全部数值（当前血量按同一比例回复，向上取整）
		void apply_modifiers(const UpgradeModifiers &mods);
		// 击杀回调：汲取在这里结算；精英怪额外多回一点
		void on_enemy_killed(bool elite);

		void draw(PIMAGE image) override;
		void update(float delta, Scene &scene) override;
		void handle_keyboard(key_msg msg) override;
		void handle_mouse(mouse_msg msg) override;
};
