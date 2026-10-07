#pragma once

#include <bitset>
#include <cstddef>
#include <random>

#include "spirit.h"

constexpr std::size_t MOVE_RIGHT = 0;
constexpr std::size_t MOVE_LEFT = 1;

class Player : public Spirit {
	protected:
		int _window_width, _window_height;
		float _x, _y, _speed_x, _speed_y;
		std::bitset<2> _move_flags;
		float _aim_x, _aim_y;
		float _fire_cooldown;
		float _move_ratio;
		bool _firing;
		bool _on_ground;
		bool _jump_requested;
		bool _jump_held;
		bool _drop_through;
		int _ammo;
		float _reload_timer;
		bool _reloading;
		std::mt19937 _rng;

		void _fire(Scene &scene);
		float _current_spread() const;
		void _start_reload();

	public:
		Player(int _window_width, int _window_height);

		int ammo() const;
		int magazine_size() const;
		bool is_reloading() const;
		float reload_progress() const;

		virtual void draw(PIMAGE image);
		virtual void update(float delta, Scene &scene);
		virtual void handal_keybord(key_msg msg);
		virtual void handal_mouse(mouse_msg msg);
		virtual bool is_invalid() const;
};