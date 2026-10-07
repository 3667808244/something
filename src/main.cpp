#include "app.h"
#include "enemy.h"
#include "obstacle.h"
#include "platform.h"
#include "player.h"

int main() {
	constexpr int window_width = 640;
	constexpr int window_height = 480;

	App app(window_width, window_height, "ege window");
	// Obstacle / Platform 的坐标是碰撞盒中心点，与 Actor 保持同一套约定
	app.add_spirit<Obstacle>(120, 410, 160, 20);
	app.add_spirit<Obstacle>(300, 350, 120, 20);
	app.add_spirit<Obstacle>(530, 290, 140, 20);
	app.add_spirit<Obstacle>(320, 474, 640, 12);
	app.add_spirit<Platform>(140, 306, 120, 12);
	app.add_spirit<Platform>(380, 246, 160, 12);
	app.add_spirit<Platform>(535, 386, 130, 12);
	app.add_spirit<Enemy>(150, 200, window_width, window_height);
	app.add_spirit<Enemy>(560, 300, window_width, window_height);
	app.add_spirit<Enemy>(620, 80, window_width, window_height);
	// 出生点放在平台上方，避免落进 Platform(380, 246) 的碰撞盒里
	app.add_spirit<Player>(window_width, window_height, 320.0f, 180.0f);
	app.run();
	return 0;
}
