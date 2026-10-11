#pragma once

#include "spirit.h"

// 坐标系约定：_x/_y 是矩形中心点，left()/top()/right()/bottom() 由中心点与宽高推导。
// Obstacle / Platform / Actor 全部沿用这一套语义，不再有"左上角 vs 中心点"的分歧。
class Rect : public Spirit {
	protected:
		float _x, _y;
		float _width, _height;

	public:
		Rect(float center_x, float center_y, float width, float height);

		float left() const;
		float top() const;
		float right() const;
		float bottom() const;
		float center_x() const;
		float center_y() const;
		float half_width() const;
		float half_height() const;

		// 线段 (x0,y0)->(x1,y1) 与 AABB 的首个交点参数 t∈[0,1]；不相交返回 -1
		static float segment_aabb_hit_t(float left, float top, float right, float bottom, float x0, float y0, float x1,
										float y1);

		// 本矩形的 AABB 与线段 (x0,y0)->(x1,y1) 的首个交点参数；不相交返回 -1。
		// Actor / Obstacle 共用这一份实现，不再各自包一层。
		float segment_hit_t(float x0, float y0, float x1, float y1) const;

		void update(float delta, Scene &scene) override;
		bool is_invalid() const override;
};

// 两个以中心点定义的 AABB 是否重叠（Actor::overlaps 与 Spawner 的生成校验共用）
bool aabb_overlap(float ax, float ay, float ahalf_w, float ahalf_h, float bx, float by, float bhalf_w, float bhalf_h);
