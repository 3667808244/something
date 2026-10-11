#pragma once

#include <cstddef>
#include <random>
#include <utility>
#include <vector>

#include "level.h"
#include "spirit.h"

// 刷怪参数：这里的默认值就是当前采用的强度
struct SpawnerConfig {
	int max_alive = 6;                    // 场上敌人数量上限
	int batch = 1;                        // 单次生成事件刷几只
	float interval_min = 2.0f;            // 一个点刷完后的最短冷却
	float interval_max = 4.0f;            // 一个点刷完后的最长冷却
	float initial_delay = 1.0f;           // 第一个点的初始延迟
	float initial_spacing = 1.4f;         // 各点初始延迟的错开间距，避免开局同时刷一片
	float retry_delay = 0.3f;             // 校验不通过后的重试间隔
	float charge_time = 0.6f;             // 生成前的预警时长
	float min_player_distance = 160.0f;   // 离玩家的最小生成距离
	float spawn_size = 20.0f;             // 生成敌人的碰撞盒边长，与 Enemy 一致
	bool visible = true;                  // F1 切换刷怪点显隐
	bool debug_info = false;              // F2 切换编号/坐标/冷却
};

class Spawner : public Spirit {
	protected:
		// 每个刷怪点各自冷却，所以可视化时每个点能独立显示自己的状态
		struct Node {
			float x, y;
			float cooldown;  // >charge_time 冷却；<=charge_time 预警；<=0 就绪
			int spawned;
			bool blocked;    // 最近一次校验是否被玩家或几何堵住
		};

		std::vector<Node> _nodes;
		SpawnerConfig _config;
		int _window_width, _window_height;
		std::mt19937 _rng;
		std::size_t _spawned_total;
		bool _f1_held = false;
		bool _f2_held = false;

		// 生成点是否可用：在窗口内、不压障碍物/平台、离玩家足够远
		bool _is_valid_spawn(float x, float y, Scene &scene);
		// 统计某个点附近已有多少敌人，用来优先选空旷的点
		float _crowding(float x, float y, const std::vector<std::pair<float, float>> &enemies) const;

	public:
		Spawner(std::vector<SpawnPointDef> points, int window_width, int window_height,
				const SpawnerConfig &config = SpawnerConfig{});

		void draw(PIMAGE image) override;
		void update(float delta, Scene &scene) override;
		void handle_keyboard(key_msg msg) override;
		bool is_invalid() const override;

		std::size_t spawned_total() const;
};
