#pragma once

#include <cstdint>
#include <optional>
#include <random>
#include <vector>

#include "spirit.h"

class Enemy;

class Bullet : public Spirit {
	protected:
		int _window_width, _window_height;
		float _x, _y, _vel_x, _vel_y;
		bool _hit;
		int _damage;
		int _explosive;   // 爆裂弹层数
		int _pierce;      // 剩余可穿透的敌人数
		std::vector<std::uint64_t> _hit_enemies;
		std::optional<std::mt19937> _rng;  // 只有爆裂弹需要随机数，按需构造

		bool _already_hit(std::uint64_t serial) const;
		void _explode(Scene &scene, float x, float y, const Enemy *direct);

	public:
		Bullet(float x, float y, float dir_x, float dir_y, float speed, int damage, int explosive, int pierce,
			   int window_width, int window_height);

		void draw(PIMAGE image) override;
		void update(float delta, Scene &scene) override;
		bool is_invalid() const override;
};
