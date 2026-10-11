#pragma once

#include <string>

#include "spirit.h"

// 经验/等级系统：场景级组件，累计经验并按曲线升级。
// 数值通过 getter 暴露给 HUD（App::_draw_hud），升级时自己画一行提示。
class Progression : public Spirit {
	protected:
		int _experience;          // 当前等级内已积累
		int _level;
		int _experience_to_next;  // 升到下一级所需
		float _level_up_flash;    // 升级提示的剩余显示时间
		float _level_down_flash;  // 掉级提示的剩余显示时间
		int _window_width;

		// 按当前等级刷新 _experience_to_next
		void _refresh_requirement();

	public:
		explicit Progression(int window_width);

		void add_experience(int amount);
		// 死亡惩罚：掉一级（最低 1 级），当前等级内的经验清零
		void lose_level();

		int level() const;
		int experience() const;
		int experience_to_next() const;
		float progress() const;

		void draw(PIMAGE image) override;
		// 等级提示的覆盖层：由 App 在所有实体之后绘制，避免被新生成的敌人/子弹盖住
		void draw_overlay(PIMAGE image);
		void update(float delta, Scene &scene) override;
		bool is_invalid() const override;
};
