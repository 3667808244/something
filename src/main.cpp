#include "app.h"
#include "level.h"
#include "player.h"
#include "progression.h"
#include "spawner.h"

namespace {

LevelData make_level() {
	LevelData level;
	// Obstacle / Platform 的坐标是碰撞盒中心点，与 Actor 保持同一套约定
	level.obstacles = {
		{120.0f, 410.0f, 160.0f, 20.0f},
		{300.0f, 350.0f, 120.0f, 20.0f},
		{530.0f, 290.0f, 140.0f, 20.0f},
		{320.0f, 474.0f, 640.0f, 12.0f},
	};
	level.platforms = {
		{140.0f, 306.0f, 120.0f, 12.0f},
		{380.0f, 246.0f, 160.0f, 12.0f},
		{535.0f, 386.0f, 130.0f, 12.0f},
	};
	// 刷怪点显式列出：顶部两角、正上方，以及两块平台上空
	level.spawn_points = {
		{60.0f, 110.0f},
		{580.0f, 110.0f},
		{320.0f, 70.0f},
		{100.0f, 250.0f},
		{540.0f, 175.0f},
	};
	level.player_x = 320.0f;
	level.player_y = 180.0f;
	return level;
}

}  // namespace

int main() {
	constexpr int window_width = 640;
	constexpr int window_height = 480;

	LevelData level = make_level();

	App app(window_width, window_height, "ege window");
	// 绘制顺序 = 插入顺序：几何 -> 刷怪点 -> 玩家。
	// 刷怪点标记因此落在几何之上、玩家和敌人之下，不会挡住实体。
	build_level(app.root_scene(), level);
	app.add_spirit<Spawner>(level.spawn_points, window_width, window_height, SpawnerConfig{});
	app.add_spirit<Player>(window_width, window_height, level.player_x, level.player_y);
	// 经验/等级系统；放在最后，升级提示文字盖在其它实体之上
	app.add_spirit<Progression>(window_width);
	app.run();
	return 0;
}
