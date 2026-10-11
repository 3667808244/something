#pragma once

#include "pickup.h"

// 经验球：敌人死亡时掉落，被玩家拾取后给 Progression 加经验
class ExperienceOrb : public Pickup {
	protected:
		void apply(Scene &scene, Player &player) override;

	public:
		ExperienceOrb(float x, float y, int value, int window_width, int window_height);

		void draw(PIMAGE image) override;
};
