#include "app.h"
#include "obstacle.h"
#include "platform.h"
#include "player.h"

int main() {
	App app(640, 480, "ege window");
	app.add_spirit<Obstacle>(40, 400, 160, 20);
	app.add_spirit<Obstacle>(240, 340, 120, 20);
	app.add_spirit<Obstacle>(460, 280, 140, 20);
	app.add_spirit<Obstacle>(0, 468, 640, 12);
	app.add_spirit<Platform>(80, 300, 120, 12);
	app.add_spirit<Platform>(300, 240, 160, 12);
	app.add_spirit<Platform>(470, 380, 130, 12);
	app.add_spirit<Player>(640, 480);
	app.run();
	return 0;
}
