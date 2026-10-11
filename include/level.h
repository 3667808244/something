#pragma once

#include <vector>

class Scene;

// 关卡里的静态几何：坐标都是碰撞盒中心点，与 Rect 的约定一致
struct ObstacleDef {
	float x, y, width, height;
};

struct PlatformDef {
	float x, y, width, height;
};

// 刷怪点：敌人从这些点生成。位置显式列在这里，Spawner 也用它来画常显的法阵标记
struct SpawnPointDef {
	float x, y;
};

struct LevelData {
	std::vector<ObstacleDef> obstacles;
	std::vector<PlatformDef> platforms;
	std::vector<SpawnPointDef> spawn_points;
	float player_x = 0.0f;
	float player_y = 0.0f;
};

// 只把静态几何（障碍物 + 单向平台）加进场景。玩家与刷怪点由调用方按顺序添加：
// Spawner 插在几何之后、玩家之前，刷怪点标记就会画在几何之上、实体之下。
void build_level(Scene &scene, const LevelData &level);
