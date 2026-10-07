#pragma once

#include "spirit.h"

class HitEffect : public Spirit {
	protected:
		float _x, _y;
		float _life;

	public:
		HitEffect(float x, float y);

		virtual void draw(PIMAGE image);
		virtual void update(float delta, Scene &scene);
		virtual void handal_keybord(key_msg msg);
		virtual void handal_mouse(mouse_msg msg);
		virtual bool is_invalid() const;
};
