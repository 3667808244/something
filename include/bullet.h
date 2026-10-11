#pragma once

#include <cstdint>
#include <optional>
#include <random>
#include <vector>

#include "spirit.h"

class Enemy;
class Obstacle;

// 一发子弹的数值：由 Player 按当前升级组装后传进来
struct BulletStats {
	int damage = 0;
	int explosive = 0;
	int pierce = 0;
	int ricochet = 0;		    // 剩余可反弹次数（跳弹）
	float elite_bonus = 0.0f;	// 对精英敌人的额外伤害倍率（精英猎手）
};

class Bullet : public Spirit {
	protected:
	int _window_width, _window_height;
	float _x, _y, _vel_x, _vel_y;
	bool _hit;
	int _damage;
	int _explosive;	 // 爆裂弹层数
	int _pierce;	 // 剩余可穿透的敌人数
	int _ricochet;	 // 剩余可反弹次数
	float _elite_bonus;
	float _bounce_cooldown;	 // 反弹后的短暂冷却，避免贴着同一面墙反复弹
	std::vector<std::uint64_t> _hit_enemies;
	std::optional<std::mt19937> _rng;	// 只有爆裂弹需要随机数，按需构造

	bool _already_hit(std::uint64_t serial) const;
	// 对这一只敌人实际造成多少伤害（精英怪吃精英猎手加成）
	int _damage_against(const Enemy &enemy) const;
	void _explode(Scene &scene, float x, float y, const Enemy *direct);
	// 撞到障碍物时尝试反弹；返回 true 表示已经弹开、子弹继续飞
	bool _try_bounce(const Obstacle &obstacle, float hit_x, float hit_y);

	public:
	Bullet(float x, float y, float dir_x, float dir_y, float speed, const BulletStats &stats, int window_width,
			   int window_height);

	void draw(PIMAGE image) override;
	void update(float delta, Scene &scene) override;
	bool is_invalid() const override;
};
