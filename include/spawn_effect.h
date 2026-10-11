#pragma once

#include "spirit.h"

// 刷怪点的生成爆闪：一圈迅速扩散的青色冲击环
class SpawnEffect : public Spirit {
	protected:
		float _x, _y;
		float _life;

	public:
		SpawnEffect(float x, float y);

		void draw(PIMAGE image) override;
		void update(float delta, Scene &scene) override;
		bool is_invalid() const override;
};
