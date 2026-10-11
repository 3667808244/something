#pragma once

#include <bitset>
#include <cstddef>
#include <random>

#include "actor.h"

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

		void _fire(Scene &scene);
		float _current_spread() const;
		void _start_reload();

	public:
		Player(int window_width, int window_height, float spawn_x, float spawn_y);

		int ammo() const;
		int magazine_size() const;
		bool is_reloading() const;
		float reload_progress() const;

		int health() const;
		int max_health() const;
		bool is_invulnerable() const;
		float pickup_radius() const;

		void draw(PIMAGE image) override;
		void update(float delta, Scene &scene) override;
		void handle_keyboard(key_msg msg) override;
		void handle_mouse(mouse_msg msg) override;
};
