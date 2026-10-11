#pragma once

#include "actor.h"

class Player;

// 掉落物基类：负责通用的重力/落地、超时消失、靠近吸附与拾取判定。
// 拾取半径不在掉落物上，而是读 Player::pickup_radius()，由玩家属性统一决定。
// "捡到之后发生什么" 交给子类实现 apply()。
class Pickup : public Actor {
	protected:
		int _value;       // 拾取给多少，含义由子类解释（经验 / 血量 / 弹药）
		float _life;      // 剩余存活时间，归零后消失
		bool _collected;  // 已被拾取

		virtual void apply(Scene &scene, Player &player) = 0;

		// 快消失时的闪烁开关，子类 draw 里可以直接用
		bool blink_hidden() const;

	public:
		Pickup(float x, float y, float width, float height, int value, int window_width, int window_height);

		// 预留接口：目前无调用点
		int value() const;
		bool is_collected() const;

		// 掉落物不参与 Actor 之间的接触限速
		bool is_solid() const override;

		void update(float delta, Scene &scene) override;
		bool is_invalid() const override;
};
