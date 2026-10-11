#include "experience_orb.h"

#include <cmath>
#include <cstdint>
#include <random>

#include "define.h"
#include "hit_effect.h"
#include "progression.h"
#include "random_seed.h"
#include "scene.h"

namespace {

constexpr float orb_size = 8.0f;
constexpr float orb_scatter_speed = 70.0f;
constexpr float orb_pop_speed_min = -170.0f;
constexpr float orb_pop_speed_max = -90.0f;
constexpr float orb_bob_amplitude = 2.0f;
constexpr float orb_bob_speed = 6.0f;
constexpr float orb_glow_radius = 7.0f;
constexpr float orb_core_radius = 2.0f;
constexpr unsigned orb_seed_salt = 0x9e3779b9u;

}  // namespace

ExperienceOrb::ExperienceOrb(float x, float y, int value, int window_width, int window_height)
	: Pickup(x, y, orb_size, orb_size, value, window_width, window_height) {
	// 混入出生点，避免同一帧掉落的球拿到同一串随机数
	std::mt19937 rng(make_random_seed(static_cast<std::uint64_t>(static_cast<unsigned>(x)) ^ orb_seed_salt));
	std::uniform_real_distribution<float> scatter(-orb_scatter_speed, orb_scatter_speed);
	std::uniform_real_distribution<float> pop(orb_pop_speed_min, orb_pop_speed_max);
	_speed_x = scatter(rng);
	_speed_y = pop(rng);
}

void ExperienceOrb::apply(Scene &scene, Player &player [[maybe_unused]]) {
	scene.each<Progression>([&](Progression &progression) { progression.add_experience(_value); });
	// 拾取反馈：复用已有的命中爆闪
	scene.add_spirit<HitEffect>(_x, _y);
}

void ExperienceOrb::draw(PIMAGE image) {
	if (blink_hidden()) {
		return;
	}

	float bob = std::sin(_life * orb_bob_speed) * orb_bob_amplitude;
	float cx = _x;
	float cy = _y + bob;

	setfillcolor(rgb(28, 86, 92), image);
	fillcirclef(cx, cy, orb_glow_radius, image);

	ege_point points[4] = {
		{ cx, cy - orb_size * 0.62f },
		{ cx + orb_size * 0.62f, cy },
		{ cx, cy + orb_size * 0.62f },
		{ cx - orb_size * 0.62f, cy },
	};
	setfillcolor(rgb(90, 245, 220), image);
	ege_fillpoly(4, points, image);

	setfillcolor(rgb(235, 255, 252), image);
	fillcirclef(cx - orb_size * 0.14f, cy - orb_size * 0.14f, orb_core_radius, image);
}
