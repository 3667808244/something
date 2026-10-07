#pragma once

#include "spirit.h"

class Rect : public Spirit {
	protected:
		float _x, _y;
		float _width, _height;

	public:
		Rect(float x, float y, float width, float height);

		float left() const;
		float top() const;
		float right() const;
		float bottom() const;
		float center_x() const;
		float center_y() const;
		float half_width() const;
		float half_height() const;

		bool intersects(float cx, float cy, float half_w, float half_h) const;

		virtual void update(float delta, Scene &scene);
		virtual void handal_keybord(key_msg msg);
		virtual void handal_mouse(mouse_msg msg);
		virtual bool is_invalid() const;
};
