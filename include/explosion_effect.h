#pragma once

#include "spirit.h"

// 爆裂弹的 AOE 爆炸：一圈快速扩散的橙红色冲击环
class ExplosionEffect : public Spirit {
	protected:
		float _x, _y;
		float _radius;
		float _life;

	public:
		ExplosionEffect(float x, float y, float radius);

		void draw(PIMAGE image) override;
		void update(float delta, Scene &scene) override;
		bool is_invalid() const override;
};
