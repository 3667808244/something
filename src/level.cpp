#include "level.h"

#include "obstacle.h"
#include "platform.h"
#include "scene.h"

void build_level(Scene &scene, const LevelData &level) {
	for (const auto &def : level.obstacles) {
		scene.add_spirit<Obstacle>(def.x, def.y, def.width, def.height);
	}
	for (const auto &def : level.platforms) {
		scene.add_spirit<Platform>(def.x, def.y, def.width, def.height);
	}
}

LevelData builtin_level() {
	LevelData level;
	// Obstacle / Platform 的坐标是碰撞盒中心点，与 Actor 保持同一套约定
	level.obstacles = {
		{ 120.0f, 410.0f, 160.0f, 20.0f },
		{ 300.0f, 350.0f, 120.0f, 20.0f },
		{ 530.0f, 290.0f, 140.0f, 20.0f },
		{ 320.0f, 474.0f, 640.0f, 12.0f },
	};
	level.platforms = {
		{ 140.0f, 306.0f, 120.0f, 12.0f },
		{ 380.0f, 246.0f, 160.0f, 12.0f },
		{ 535.0f, 386.0f, 130.0f, 12.0f },
	};
	// 刷怪点显式列出：顶部两角、正上方，以及两块平台上空
	level.spawn_points = {
		{ 60.0f, 110.0f },
		{ 580.0f, 110.0f },
		{ 320.0f, 70.0f },
		{ 100.0f, 250.0f },
		{ 540.0f, 175.0f },
	};
	level.player_x = 320.0f;
	level.player_y = 180.0f;
	return level;
}
