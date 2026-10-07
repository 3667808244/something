#pragma once

#include "spirit.h"

class HitEffect : public Spirit {
	protected:
		float _x, _y;
		float _life;

	public:
		HitEffect(float x, float y);

		void draw(PIMAGE image) override;
		void update(float delta, Scene &scene) override;
		bool is_invalid() const override;
};
