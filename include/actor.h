#pragma once

#include "rect.h"

// 约定：Actor 及其子类的 _x/_y 是碰撞盒中心点，而 Obstacle/Platform 的 _x/_y 是左上角。
// 这里重写几何访问器，让 left()/top()/right()/bottom()/center_x()/center_y() 对两者都返回
// 碰撞盒的真实边缘与中心，避免再出现画的位置和碰撞盒错位的问题。
class Actor : public Rect {
	protected:
		int _window_width, _window_height;
		float _speed_x, _speed_y;
		bool _on_ground;

	public:
		Actor(float x, float y, float width, float height, int window_width, int window_height);

		float left() const;
		float top() const;
		float right() const;
		float bottom() const;
		float center_x() const;
		float center_y() const;

		float speed_x() const;
		float speed_y() const;
		bool on_ground() const;

		bool overlaps(const Actor &other) const;
		float segment_hit_t(float x0, float y0, float x1, float y1) const;

		// 与其它 Actor 接触时把自己的速度强制压到上限以内
		void resolve_contacts(Scene &scene);

		bool resolve_world(Scene &scene, float previous_y, bool drop_through);
};
