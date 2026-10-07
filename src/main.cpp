#include "app.h"
#include "enemy.h"
#include "obstacle.h"
#include "platform.h"
#include "player.h"

int main() {
	constexpr int window_width = 640;
	constexpr int window_height = 480;

	App app(window_width, window_height, "ege window");
	app.add_spirit<Obstacle>(40, 400, 160, 20);
	app.add_spirit<Obstacle>(240, 340, 120, 20);
	app.add_spirit<Obstacle>(460, 280, 140, 20);
	app.add_spirit<Obstacle>(0, 468, 640, 12);
	app.add_spirit<Platform>(80, 300, 120, 12);
	app.add_spirit<Platform>(300, 240, 160, 12);
	app.add_spirit<Platform>(470, 380, 130, 12);
	app.add_spirit<Enemy>(150, 200, window_width, window_height);
	app.add_spirit<Enemy>(560, 300, window_width, window_height);
	app.add_spirit<Enemy>(620, 80, window_width, window_height);
	app.add_spirit<Player>(window_width, window_height);
	app.run();
	return 0;
}
