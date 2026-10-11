#pragma once

#include "rect.h"

class Actor : public Rect {
	protected:
		int _window_width, _window_height;
		float _speed_x, _speed_y;
		bool _on_ground;

	public:
		// center_x/center_y 是碰撞盒中心点，与 Rect 的约定一致
		Actor(float center_x, float center_y, float width, float height, int window_width, int window_height);

		// 预留接口：目前无调用点
		float speed_x() const;
		float speed_y() const;
		bool on_ground() const;

		bool overlaps(const Actor &other) const;
		// 是否参与 Actor 之间的接触限速：掉落物覆写为 false，避免拖慢玩家、干扰吸附
		virtual bool is_solid() const;

		// 与其它 Actor 接触时压低自己的水平速度
		void resolve_contacts(Scene &scene);

		bool resolve_world(Scene &scene, float previous_y, bool drop_through);
};
