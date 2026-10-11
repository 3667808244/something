#include "spawner.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

#include "define.h"
#include "enemy.h"
#include "obstacle.h"
#include "platform.h"
#include "player.h"
#include "progression.h"
#include "random_seed.h"
#include "scene.h"
#include "spawn_effect.h"

namespace {

// 常显法阵的视觉参数
constexpr float node_ring_radius = 16.0f;
constexpr float node_charge_radius = 26.0f;
constexpr float node_inner_ratio = 0.55f;
constexpr float node_tick_len = 5.0f;
constexpr float node_core_radius = 2.0f;
constexpr float crowding_radius = 130.0f;
constexpr unsigned spawner_seed_salt = 0x5eed5eedu;

}  // namespace

Spawner::Spawner(std::vector<SpawnPointDef> points, int window_width, int window_height, const SpawnerConfig &config)
	: _config(config),
	  _window_width(window_width),
	  _window_height(window_height),
	  _rng(make_random_seed(spawner_seed_salt)) {
	_spawned_total = 0;
	_nodes.reserve(points.size());
	for (std::size_t i = 0; i < points.size(); ++i) {
		Node node;
		node.x = points[i].x;
		node.y = points[i].y;
		// 初始冷却逐个错开：否则开局几个点会同时到点，一次刷出一片
		node.cooldown = _config.initial_delay + static_cast<float>(i) * _config.initial_spacing;
		node.spawned = 0;
		node.blocked = false;
		_nodes.push_back(node);
	}
}

float Spawner::_elite_chance(int level) const {
	if (level < _config.elite_min_level) {
		return 0.0f;
	}
	const float growth = static_cast<float>(level - _config.elite_min_level) * _config.elite_chance_per_level;
	return std::clamp(_config.elite_base_chance + growth, 0.0f, _config.elite_chance_max);
}

bool Spawner::_roll_elite(int level) {
	std::uniform_real_distribution<float> roll(0.0f, 1.0f);
	return roll(_rng) < _elite_chance(level);
}

bool Spawner::_is_valid_spawn(float x, float y, Scene &scene) {
	float half = _config.spawn_size / 2.0f;
	if (x < half || x > static_cast<float>(_window_width) - half || y < half ||
		y > static_cast<float>(_window_height) - half) {
		return false;
	}

	bool valid = true;
	scene.each<Obstacle>([&](Obstacle &obstacle) {
		if (aabb_overlap(x, y, half, half, obstacle.center_x(), obstacle.center_y(), obstacle.half_width(),
						 obstacle.half_height())) {
			valid = false;
		}
	});
	scene.each<Platform>([&](Platform &platform) {
		if (aabb_overlap(x, y, half, half, platform.center_x(), platform.center_y(), platform.half_width(),
						 platform.half_height())) {
			valid = false;
		}
	});
	scene.each<Player>([&](Player &player) {
		float dx = player.center_x() - x;
		float dy = player.center_y() - y;
		if (dx * dx + dy * dy < _config.min_player_distance * _config.min_player_distance) {
			valid = false;
		}
	});
	return valid;
}

float Spawner::_crowding(float x, float y, const std::vector<std::pair<float, float>> &enemies) const {
	float score = 0.0f;
	for (const auto &enemy : enemies) {
		float dx = enemy.first - x;
		float dy = enemy.second - y;
		if (dx * dx + dy * dy <= crowding_radius * crowding_radius) {
			score += 1.0f;
		}
	}
	return score;
}

void Spawner::update(float delta, Scene &scene) {
	for (auto &node : _nodes) {
		if (node.cooldown > 0.0f) {
			node.cooldown = std::max(0.0f, node.cooldown - delta);
		}
	}

	std::vector<std::pair<float, float>> enemies;
	scene.each<Enemy>([&](Enemy &enemy) { enemies.emplace_back(enemy.center_x(), enemy.center_y()); });

	int player_level = 1;
	scene.each<Progression>([&](Progression &progression) { player_level = progression.level(); });
	_player_level = player_level;

	// 每帧都重新校验：冷却期和满员时也保持红/绿是当前帧的真实结果，不会停在旧值
	for (auto &node : _nodes) {
		node.blocked = !_is_valid_spawn(node.x, node.y, scene);
	}

	if (static_cast<int>(enemies.size()) >= _config.max_alive) {
		return;
	}

	// 就绪的点里优先选附近敌人最少的，避免敌人全挤在同一侧
	Node *best = nullptr;
	float best_score = 0.0f;
	for (auto &node : _nodes) {
		if (node.cooldown > 0.0f) {
			continue;
		}
		if (node.blocked) {
			node.cooldown = _config.retry_delay;
			continue;
		}
		float score = _crowding(node.x, node.y, enemies);
		if (best == nullptr || score < best_score) {
			best = &node;
			best_score = score;
		}
	}

	if (best == nullptr) {
		return;
	}

	int remaining = _config.max_alive - static_cast<int>(enemies.size());
	int count = std::min(_config.batch, remaining);
	for (int i = 0; i < count; ++i) {
		EnemyStats stats;
		if (_roll_elite(player_level)) {
			stats.elite = true;
			stats.max_health = enemy_base_health * _config.elite_health_multiplier;
			stats.exp_value = enemy_base_exp_value * _config.elite_exp_multiplier;
		}
		scene.add_spirit<Enemy>(best->x, best->y, _window_width, _window_height, stats);
		scene.add_spirit<SpawnEffect>(best->x, best->y);
		++best->spawned;
		++_spawned_total;
	}

	float upper = std::max(_config.interval_min, _config.interval_max);
	std::uniform_real_distribution<float> dist(_config.interval_min, upper);
	best->cooldown = dist(_rng);
}

void Spawner::draw(PIMAGE image) {
	if (!_config.visible) {
		return;
	}

	for (std::size_t i = 0; i < _nodes.size(); ++i) {
		const Node &node = _nodes[i];
		float cx = node.x;
		float cy = node.y;
		float radius = node_ring_radius;
		color_t ring_color = rgb(90, 110, 130);
		bool cooling = false;
		float remain = 0.0f;

		if (node.blocked) {
			// 被玩家贴住或被几何压住，本帧刷不出来
			ring_color = rgb(215, 70, 70);
		} else if (node.cooldown <= 0.0f) {
			// 就绪，等一个空位就能刷
			ring_color = rgb(70, 225, 185);
		} else if (node.cooldown <= _config.charge_time) {
			// 预警：环随进度放大，亮暗交替
			float progress = 1.0f - node.cooldown / std::max(_config.charge_time, 1e-4f);
			radius = node_ring_radius + (node_charge_radius - node_ring_radius) * progress;
			ring_color = (static_cast<int>(node.cooldown * 20.0f) % 2 == 0) ? rgb(255, 232, 90) : rgb(255, 150, 40);
		} else {
			// 冷却：把剩余时间换算成扇形占比
			cooling = true;
			remain = (node.cooldown - _config.charge_time) /
					 std::max(_config.interval_max - _config.charge_time, 1e-4f);
			remain = std::clamp(remain, 0.0f, 1.0f);
		}

		// 底盘
		setfillcolor(rgb(22, 28, 36), image);
		fillcirclef(cx, cy, radius - 2.0f, image);

		// 剩余冷却扇形：从顶部逆时针铺开，冷却越少扇形越小，一眼能读出进度
		if (cooling) {
			setfillcolor(rgb(70, 92, 116), image);
			solidpief(cx, cy, 90.0f, 90.0f + 360.0f * remain, radius - 3.0f, radius - 3.0f, image);
		}

		// 外环 + 内环
		setlinecolor(ring_color, image);
		circlef(cx, cy, radius, image);
		circlef(cx, cy, radius * node_inner_ratio, image);

		// 四向刻度 + 中心核
		float inner = radius * node_inner_ratio;
		ege_line(cx - radius - node_tick_len, cy, cx - inner, cy, image);
		ege_line(cx + inner, cy, cx + radius + node_tick_len, cy, image);
		ege_line(cx, cy - radius - node_tick_len, cx, cy - inner, image);
		ege_line(cx, cy + inner, cx, cy + radius + node_tick_len, image);

		setfillcolor(ring_color, image);
		fillcirclef(cx, cy, node_core_radius, image);
	}

	if (!_config.debug_info) {
		return;
	}

	setfont(12, 0, "Consolas", image);
	setbkmode(TRANSPARENT, image);
	setlinecolor(rgb(230, 240, 255), image);
	{
		const int percent = static_cast<int>(_elite_chance(_player_level) * 100.0f + 0.5f);
		const std::string elite_label =
			"Lv " + std::to_string(_player_level) + "  elite " + std::to_string(percent) + "%";
		ege_outtextxy(12.0f, 4.0f, elite_label.c_str(), image);
	}
	for (std::size_t i = 0; i < _nodes.size(); ++i) {
		const Node &node = _nodes[i];
		std::string cd = std::to_string(node.cooldown).substr(0, 4);
		std::string label = "S" + std::to_string(i + 1) + " (" + std::to_string(static_cast<int>(node.x)) + "," +
							std::to_string(static_cast<int>(node.y)) + ") cd=" + cd;
		ege_outtextxy(node.x + node_ring_radius + 6.0f, node.y - 7.0f, label.c_str(), image);
	}
}

void Spawner::handle_keyboard(key_msg msg) {
	if (msg.key == key_f1) {
		if (msg.msg == ege::key_msg_down) {
			if (!_f1_held) {
				_config.visible = !_config.visible;
				_f1_held = true;
			}
		} else if (msg.msg == ege::key_msg_up) {
			_f1_held = false;
		}
	} else if (msg.key == key_f2) {
		if (msg.msg == ege::key_msg_down) {
			if (!_f2_held) {
				_config.debug_info = !_config.debug_info;
				_f2_held = true;
			}
		} else if (msg.msg == ege::key_msg_up) {
			_f2_held = false;
		}
	}
}

bool Spawner::is_invalid() const { return false; }

std::size_t Spawner::spawned_total() const { return _spawned_total; }
