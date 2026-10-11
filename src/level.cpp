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
