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

		// 线段 (x0,y0)->(x1,y1) 与 AABB 的首个交点参数 t∈[0,1]；不相交返回 -1
		static float segment_aabb_hit_t(float left, float top, float right, float bottom, float x0, float y0, float x1,
		                                float y1);

		virtual void update(float delta, Scene &scene);
		virtual void handal_keybord(key_msg msg);
		virtual void handal_mouse(mouse_msg msg);
		virtual bool is_invalid() const;
};
