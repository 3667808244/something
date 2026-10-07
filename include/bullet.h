#pragma once

#include "spirit.h"

class Bullet : public Spirit {
	protected:
		int _window_width, _window_height;
		float _x, _y, _vel_x, _vel_y;
		bool _hit;

	public:
		Bullet(float x, float y, float dir_x, float dir_y, float speed, int window_width, int window_height);

		void draw(PIMAGE image) override;
		void update(float delta, Scene &scene) override;
		bool is_invalid() const override;
};
