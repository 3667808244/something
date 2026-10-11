#include "player.h"

#include <algorithm>
#include <cmath>

#include "bullet.h"
#include "define.h"
#include "ege/graphics.h"
#include "enemy.h"
#include "progression.h"
#include "random_seed.h"
#include "scene.h"

constexpr float player_size = 20.0f;
constexpr float base_speed_max = 300.0f;
constexpr float acceleration = 1200.0f;
constexpr float jump_speed = 420.0f;
constexpr float fall_speed_max = 900.0f;
constexpr float bullet_speed = 600.0f;
constexpr float base_fire_interval = 0.1f;
constexpr float min_fire_interval = 0.02f;
constexpr int base_magazine_capacity = 24;
constexpr float base_reload_duration = 1.2f;
constexpr float deg_to_rad = 3.14159265358979323846f / 180.0f;
constexpr float spread_min_deg = 1.0f;
constexpr float spread_max_deg = 8.0f;
constexpr float spread_sigma_scale = 3.0f;
constexpr float spread_smooth_rate = 6.0f;
constexpr int base_max_health = 100;
constexpr int contact_damage = 20;
constexpr float base_invulnerable_duration = 1.0f;
constexpr float base_pickup_radius = 26.0f;
constexpr int base_bullet_damage = 1;

// 新普通升级
constexpr float jump_power_increment = 0.08f;		   // 每层 +8% 跳跃初速
constexpr float invulnerability_increment = 0.12f;	   // 每层 +0.12s 受击无敌
constexpr float armor_regen_delay_decrement = 0.30f;   // 每层 -0.3s 脱战延迟
constexpr float armor_regen_delay_min = 0.5f;		   // 延迟下限
constexpr float armor_regen_rate_increment = 1.0f;	   // 每层 +1/s 回甲速度
constexpr int ammo_on_kill_per_stack = 2;			   // 每层击杀补 2 发

// 普通升级：卡片上写多少就加多少（线性，不做边际衰减）
constexpr float fire_rate_increment = 0.10f;   // 每层 +10% 射速
constexpr int magazine_increment = 4;          // 每层 +4 发
constexpr int health_increment = 15;           // 每层 +15 生命
constexpr float move_speed_increment = 0.08f;  // 每层 +8% 移速
constexpr int damage_increment = 1;            // 每层 +1 伤害

// 换弹速度：换弹时长 = 基础 / (1 + 每层 12%)
constexpr float reload_speed_increment = 0.12f;

// 护甲：每层 +20 上限，最多抵消 80% 伤害，脱战后缓慢回复
constexpr int armor_per_stack = 20;
constexpr float armor_absorb_ratio = 0.8f;
constexpr float base_armor_regen_delay = 3.5f;
constexpr float base_armor_regen_rate = 4.0f;

// 高级升级
constexpr float magnet_bonus_per_stack = 0.40f;
constexpr float experience_bonus_per_stack = 0.25f;
constexpr float thorns_chance_per_stack = 0.20f;
constexpr int lifesteal_per_stack = 3;
// 击杀精英怪时，在普通汲取回血之外额外回复
constexpr int elite_lifesteal_bonus = 9;
constexpr float elite_hunter_bonus_per_stack = 0.50f;  // 每层对精英 +50% 伤害

// 僚机
constexpr float drone_orbit_radius = 30.0f;
constexpr float drone_orbit_speed = 2.2f;	// rad/s
constexpr float drone_fire_interval = 1.2f;	// s
constexpr float drone_range = 240.0f;
constexpr float pi = 3.14159265358979323846f;

// 霰弹：一次消耗多枚弹药，弹丸数 = 消耗 * 1.1（小数按概率补 1），并固定降速
constexpr float shotgun_ammo_factor = 1.1f;
constexpr float shotgun_interval_scale = 1.15f;
constexpr float shotgun_cone_deg = 9.0f;

Player::Player(int window_width, int window_height, float spawn_x, float spawn_y)
	: Actor(spawn_x, spawn_y, player_size, player_size, window_width, window_height), _rng(make_random_seed()) {
	_jump_requested = false;
	_jump_held = false;
	_drop_through = false;
	_aim_x = _x;
	_aim_y = _y;
	_fire_cooldown = 0;
	_move_ratio = 0.0f;
	_firing = false;
	_reload_timer = 0.0f;
	_reloading = false;
	_invulnerable_timer = 0.0f;
	_spawn_x = spawn_x;
	_spawn_y = spawn_y;

	// 基础数值，apply_modifiers() 之后会按升级层数整体重算
	_max_health = base_max_health;
	_health = base_max_health;
	_fire_interval = base_fire_interval;
	_magazine_capacity = base_magazine_capacity;
	_ammo = base_magazine_capacity;
	_speed_limit = base_speed_max;
	_bullet_damage = base_bullet_damage;
	_explosive_stacks = 0;
	_pierce_stacks = 0;
	_lifesteal_per_kill = 0;
	_thorns_stacks = 0;
	_experience_gain = 1.0f;
	_pickup_radius = base_pickup_radius;
	_armor = 0.0f;
	_armor_max = 0;
	_armor_regen_timer = 0.0f;
	_reload_duration = base_reload_duration;
	_shotgun_stacks = 0;
	_jump_velocity = jump_speed;
	_invulnerable_duration = base_invulnerable_duration;
	_armor_regen_delay = base_armor_regen_delay;
	_armor_regen_rate = base_armor_regen_rate;
	_ammo_on_kill = 0;
	_air_jumps = 0;
	_air_jumps_max = 0;
	_ricochet_stacks = 0;
	_elite_damage_bonus = 0.0f;
	_drone_count = 0;
	_drone_angle = 0.0f;
}

void Player::apply_modifiers(const UpgradeModifiers &mods) {
	// 射速：间隔 = 基础 / (1 + 每层 10% 射速)
	_fire_interval = std::max(
		min_fire_interval, base_fire_interval / (1.0f + fire_rate_increment * static_cast<float>(mods.fire_rate)));

	_magazine_capacity = base_magazine_capacity + magazine_increment * mods.magazine;

	int previous_max_health = _max_health;
	_max_health = base_max_health + health_increment * mods.max_health;
	if (_max_health > previous_max_health && previous_max_health > 0) {
		// 生命值升级：当前血量按同一比例提升，向上取整
		float scaled = static_cast<float>(_health) * static_cast<float>(_max_health) /
					   static_cast<float>(previous_max_health);
		_health = std::min(_max_health, static_cast<int>(std::ceil(scaled)));
	} else {
		_health = std::min(_health, _max_health);
	}

	_speed_limit = base_speed_max * (1.0f + move_speed_increment * static_cast<float>(mods.move_speed));

	_bullet_damage = base_bullet_damage + damage_increment * mods.bullet_damage;

	_ammo = std::min(_ammo, _magazine_capacity);

	_pickup_radius = base_pickup_radius * (1.0f + magnet_bonus_per_stack * static_cast<float>(mods.magnet));
	_explosive_stacks = mods.explosive;
	_pierce_stacks = mods.pierce;
	_lifesteal_per_kill = lifesteal_per_stack * mods.lifesteal;
	_thorns_stacks = mods.thorns;
	_experience_gain = 1.0f + experience_bonus_per_stack * static_cast<float>(mods.experience_gain);

	_reload_duration = base_reload_duration / (1.0f + reload_speed_increment * static_cast<float>(mods.reload_speed));
	_shotgun_stacks = mods.shotgun;

	_jump_velocity = jump_speed * (1.0f + jump_power_increment * static_cast<float>(mods.jump_power));
	_invulnerable_duration =
		base_invulnerable_duration + invulnerability_increment * static_cast<float>(mods.invulnerability);
	_armor_regen_delay =
		std::max(armor_regen_delay_min,
				 base_armor_regen_delay - armor_regen_delay_decrement * static_cast<float>(mods.armor_regen));
	_armor_regen_rate = base_armor_regen_rate + armor_regen_rate_increment * static_cast<float>(mods.armor_regen);
	_ammo_on_kill = ammo_on_kill_per_stack * mods.ammo_on_kill;

	_ricochet_stacks = mods.ricochet;
	_elite_damage_bonus = elite_hunter_bonus_per_stack * static_cast<float>(mods.elite_hunter);

	// 二段跳：层数就是空中可跳次数
	_air_jumps_max = mods.double_jump;
	_air_jumps = _air_jumps_max;

	// 僚机：数量变化时重建冷却表，并把相位错开
	_drone_count = mods.drone;
	if (static_cast<int>(_drone_timers.size()) != _drone_count) {
		_drone_timers.assign(static_cast<std::size_t>(_drone_count), 0.0f);
		for (int i = 0; i < _drone_count; ++i) {
			_drone_timers[static_cast<std::size_t>(i)] =
				drone_fire_interval * static_cast<float>(i) / static_cast<float>(_drone_count);
		}
	}

	// 护甲：上限提高时立刻补上差额，之后靠脱战回复慢慢填满
	int new_armor_max = armor_per_stack * mods.armor;
	if (new_armor_max > _armor_max) {
		_armor += static_cast<float>(new_armor_max - _armor_max);
	}
	_armor_max = new_armor_max;
	_armor = std::min(_armor, static_cast<float>(_armor_max));
}

void Player::on_enemy_killed(bool elite) {
	// 汲取：回了血才受层数影响，精英再额外多回一点
	if (_lifesteal_per_kill > 0) {
		const int heal = _lifesteal_per_kill + (elite ? elite_lifesteal_bonus : 0);
		_health = std::min(_max_health, _health + heal);
	}
	// 击杀补弹：直接填进弹匣，不产生余弹
	if (_ammo_on_kill > 0) {
		_ammo = std::min(_magazine_capacity, _ammo + _ammo_on_kill);
	}
}

void Player::draw(PIMAGE image) {
	bool blink = _invulnerable_timer > 0.0f && static_cast<int>(_invulnerable_timer * 12.0f) % 2 == 0;
	setfillcolor(blink ? rgb(255, 255, 255) : rgb(255, 0, 0), image);
	bar(static_cast<int>(_x - half_width()), static_cast<int>(_y - half_height()), static_cast<int>(_x + half_width()),
		static_cast<int>(_y + half_height()), image);

	// 僚机：绕着玩家转的小光点
	for (int i = 0; i < _drone_count; ++i) {
		const float orbit = _drone_angle + 2.0f * pi * static_cast<float>(i) / static_cast<float>(_drone_count);
		const float drone_x = _x + std::cos(orbit) * drone_orbit_radius;
		const float drone_y = _y + std::sin(orbit) * drone_orbit_radius;
		setfillcolor(rgb(130, 235, 255), image);
		fillcirclef(drone_x, drone_y, 3.0f, image);
		setlinecolor(rgb(40, 90, 120), image);
		circlef(drone_x, drone_y, 3.5f, image);
	}
}

void Player::update(float delta, Scene &scene) {
	float ax = 0.0f;
	if (_move_flags.test(MOVE_LEFT)) ax -= 1.0f;
	if (_move_flags.test(MOVE_RIGHT)) ax += 1.0f;

	if (ax != 0.0f) {
		_speed_x += ax * acceleration * delta;
	} else {
		float drop = acceleration * delta;
		if (_speed_x > 0.0f) {
			_speed_x = std::max(0.0f, _speed_x - drop);
		} else if (_speed_x < 0.0f) {
			_speed_x = std::min(0.0f, _speed_x + drop);
		}
	}
	_speed_x = std::clamp(_speed_x, -_speed_limit, _speed_limit);

	_speed_y += scene.gravity() * delta;
	if (_jump_requested) {
		if (_on_ground) {
			_speed_y = -_jump_velocity;
			_air_jumps = _air_jumps_max;
		} else if (_air_jumps > 0) {
			// 二段跳：空中再消耗一段
			_speed_y = -_jump_velocity;
			--_air_jumps;
		}
	}
	_jump_requested = false;
	_speed_y = std::min(_speed_y, fall_speed_max);

	float previous_y = _y;

	_x += _speed_x * delta;
	_y += _speed_y * delta;

	resolve_world(scene, previous_y, _drop_through);
	resolve_contacts(scene);

	if (_on_ground) {
		_air_jumps = _air_jumps_max;
	}

	if (_invulnerable_timer > 0.0f) {
		_invulnerable_timer -= delta;
	}

	// 护甲：脱战 _armor_regen_delay 秒后开始按 _armor_regen_rate 回复
	if (_armor_max > 0 && _armor < static_cast<float>(_armor_max)) {
		_armor_regen_timer += delta;
		if (_armor_regen_timer >= _armor_regen_delay) {
			_armor = std::min(static_cast<float>(_armor_max), _armor + _armor_regen_rate * delta);
		}
	}

	// 接触扣血：无敌帧内或已经死亡的敌人不结算
	scene.each<Enemy>([&](Enemy &enemy) {
		if (_invulnerable_timer > 0.0f || enemy.is_dead() || !overlaps(enemy)) {
			return;
		}

		// 荆棘：按层数概率免疫这次伤害
		if (_thorns_stacks > 0) {
			std::uniform_real_distribution<float> roll(0.0f, 1.0f);
			if (roll(_rng) < thorns_chance_per_stack * static_cast<float>(_thorns_stacks)) {
				_invulnerable_timer = _invulnerable_duration;
				return;
			}
		}

		// 护甲最多抵消 80% 伤害，剩下的仍然扣血；受击同时重置脱战计时
		_armor_regen_timer = 0.0f;
		int incoming = contact_damage;
		if (_armor > 0.0f) {
			int max_absorb = static_cast<int>(static_cast<float>(contact_damage) * armor_absorb_ratio);
			int absorbed = std::min(static_cast<int>(_armor), max_absorb);
			_armor -= static_cast<float>(absorbed);
			incoming -= absorbed;
		}

		_health -= incoming;
		_invulnerable_timer = _invulnerable_duration;

		if (_health <= 0) {
			_health = _max_health;
			_x = _spawn_x;
			_y = _spawn_y;
			_speed_x = 0.0f;
			_speed_y = 0.0f;
			// 重生时清理战斗状态：取消换弹、重置开火与护甲回复计时
			_reloading = false;
			_reload_timer = 0.0f;
			_fire_cooldown = 0.0f;
			_armor_regen_timer = 0.0f;
			// 死亡惩罚：掉一级经验等级
			scene.each<Progression>([&](Progression &progression) { progression.lose_level(); });
		}
	});

	float move_speed = std::sqrt(_speed_x * _speed_x + _speed_y * _speed_y);
	float target_ratio = std::clamp(move_speed / _speed_limit, 0.0f, 1.0f);
	_move_ratio += (target_ratio - _move_ratio) * (1.0f - std::exp(-spread_smooth_rate * delta));

	_fire_cooldown -= delta;
	if (_fire_cooldown < 0.0f) {
		_fire_cooldown = 0.0f;
	}

	if (_reloading) {
		_reload_timer -= delta;
		if (_reload_timer <= 0.0f) {
			_reload_timer = 0.0f;
			_reloading = false;
			_ammo = _magazine_capacity;
		}
	} else if (_firing) {
		if (_fire_cooldown <= 0.0f) {
			int need = _ammo_per_shot();
			if (_ammo >= need) {
				_fire(scene);
				_ammo -= need;
				_fire_cooldown = _shot_interval();
			} else {
				_start_reload();
			}
		}
	}

	_update_drones(delta, scene);
}

void Player::handle_keyboard(key_msg msg) {
	if (msg.key == key_A) {
		if (msg.msg == ege::key_msg_down) {
			_move_flags.set(MOVE_LEFT);
		} else if (msg.msg == ege::key_msg_up) {
			_move_flags.reset(MOVE_LEFT);
		}
	} else if (msg.key == key_D) {
		if (msg.msg == ege::key_msg_down) {
			_move_flags.set(MOVE_RIGHT);
		} else if (msg.msg == ege::key_msg_up) {
			_move_flags.reset(MOVE_RIGHT);
		}
	} else if (msg.key == key_W) {
		if (msg.msg == ege::key_msg_down) {
			if (!_jump_held) {
				_jump_requested = true;
				_jump_held = true;
			}
		} else if (msg.msg == ege::key_msg_up) {
			_jump_held = false;
		}
	} else if (msg.key == key_R) {
		if (msg.msg == ege::key_msg_down && !_reloading && _ammo < _magazine_capacity) {
			_start_reload();
		}
	} else if (msg.key == key_S) {
		if (msg.msg == ege::key_msg_down) {
			_drop_through = true;
		} else if (msg.msg == ege::key_msg_up) {
			_drop_through = false;
		}
	}
}

void Player::handle_mouse(mouse_msg msg) {
	_aim_x = static_cast<float>(msg.x);
	_aim_y = static_cast<float>(msg.y);

	if (msg.is_left()) {
		if (msg.is_down()) {
			_firing = true;
			_fire_cooldown = 0.0f;
		} else if (msg.is_up()) {
			_firing = false;
		}
	}
}

int Player::_ammo_per_shot() const { return _shotgun_stacks > 0 ? 1 + _shotgun_stacks : 1; }

float Player::_shot_interval() const {
	return _shotgun_stacks > 0 ? _fire_interval * shotgun_interval_scale : _fire_interval;
}

int Player::_roll_pellet_count() {
	// 弹丸数 = 消耗 * 1.1，小数部分按概率补 1 发
	float expected = static_cast<float>(_ammo_per_shot()) * shotgun_ammo_factor;
	int pellets = static_cast<int>(expected);
	float fractional = expected - static_cast<float>(pellets);
	if (fractional > 0.0f) {
		std::uniform_real_distribution<float> roll(0.0f, 1.0f);
		if (roll(_rng) < fractional) {
			++pellets;
		}
	}
	return std::max(1, pellets);
}

void Player::_fire(Scene &scene) {
	float dx = _aim_x - _x;
	float dy = _aim_y - _y;
	float len = std::sqrt(dx * dx + dy * dy);
	if (len < 1e-4f) {
		dx = 1.0f;
		dy = 0.0f;
		len = 1.0f;
	}
	dx /= len;
	dy /= len;

	// 正态散射：spread 为 3σ 包络，并夹取到 ±spread 以避免极端离群
	float spread = _current_spread();
	std::normal_distribution<float> dist(0.0f, spread / spread_sigma_scale);

	float base_angle = std::atan2(dy, dx);
	int pellets = _shotgun_stacks > 0 ? _roll_pellet_count() : 1;
	float cone = pellets > 1 ? shotgun_cone_deg * deg_to_rad * static_cast<float>(pellets - 1) : 0.0f;

	for (int i = 0; i < pellets; ++i) {
		// 弹丸在扇形里均匀排开，再各自叠加一点随机抖动
		float offset = 0.0f;
		if (pellets > 1) {
			float t = static_cast<float>(i) / static_cast<float>(pellets - 1) - 0.5f;
			offset = t * cone;
		}
		float jitter = std::clamp(dist(_rng), -spread, spread);
		float angle = base_angle + offset + jitter;
		fire_bullet(scene, _x, _y, angle);
	}
}

void Player::fire_bullet(Scene &scene, float x, float y, float angle) {
	BulletStats stats;
	stats.damage = _bullet_damage;
	stats.explosive = _explosive_stacks;
	stats.pierce = _pierce_stacks;
	stats.ricochet = _ricochet_stacks;
	stats.elite_bonus = _elite_damage_bonus;
	scene.add_spirit<Bullet>(x, y, std::cos(angle), std::sin(angle), bullet_speed, stats, _window_width,
							 _window_height);
}

void Player::_update_drones(float delta, Scene &scene) {
	if (_drone_count <= 0) {
		return;
	}

	_drone_angle += drone_orbit_speed * delta;
	if (_drone_angle > 2.0f * pi) {
		_drone_angle -= 2.0f * pi;
	}

	for (int i = 0; i < _drone_count; ++i) {
		float &timer = _drone_timers[static_cast<std::size_t>(i)];
		timer -= delta;
		if (timer > 0.0f) {
			continue;
		}
		timer = drone_fire_interval;

		const float orbit = _drone_angle + 2.0f * pi * static_cast<float>(i) / static_cast<float>(_drone_count);
		const float drone_x = _x + std::cos(orbit) * drone_orbit_radius;
		const float drone_y = _y + std::sin(orbit) * drone_orbit_radius;

		// 找射程内最近的敌人；没有目标就空转
		const Enemy *nearest = nullptr;
		float best = drone_range * drone_range;
		scene.each<Enemy>([&](Enemy &enemy) {
			if (enemy.is_dead()) {
				return;
			}
			const float dx = enemy.center_x() - drone_x;
			const float dy = enemy.center_y() - drone_y;
			const float distance_sq = dx * dx + dy * dy;
			if (distance_sq < best) {
				best = distance_sq;
				nearest = &enemy;
			}
		});
		if (nearest == nullptr) {
			continue;
		}

		const float dx = nearest->center_x() - drone_x;
		const float dy = nearest->center_y() - drone_y;
		fire_bullet(scene, drone_x, drone_y, std::atan2(dy, dx));
	}
}

float Player::_current_spread() const {
	return (spread_min_deg + (spread_max_deg - spread_min_deg) * _move_ratio) * deg_to_rad;
}

void Player::_start_reload() {
	_reloading = true;
	_reload_timer = _reload_duration;
}

int Player::ammo() const { return _ammo; }

int Player::magazine_size() const { return _magazine_capacity; }

bool Player::is_reloading() const { return _reloading; }

float Player::reload_progress() const { return _reloading ? 1.0f - _reload_timer / _reload_duration : 0.0f; }

int Player::health() const { return _health; }

int Player::max_health() const { return _max_health; }

bool Player::is_invulnerable() const { return _invulnerable_timer > 0.0f; }

float Player::pickup_radius() const { return _pickup_radius; }

float Player::experience_gain() const { return _experience_gain; }

float Player::armor() const { return _armor; }

int Player::max_armor() const { return _armor_max; }

float Player::fire_interval() const { return _fire_interval; }

int Player::bullet_damage() const { return _bullet_damage; }

float Player::speed_limit() const { return _speed_limit; }

float Player::reload_duration() const { return _reload_duration; }

int Player::lifesteal_per_kill() const { return _lifesteal_per_kill; }

float Player::jump_velocity() const { return _jump_velocity; }

float Player::invulnerable_duration() const { return _invulnerable_duration; }

float Player::armor_regen_delay() const { return _armor_regen_delay; }

float Player::armor_regen_rate() const { return _armor_regen_rate; }

int Player::ammo_on_kill() const { return _ammo_on_kill; }
