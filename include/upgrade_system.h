#pragma once

#include <array>
#include <cstddef>
#include <deque>
#include <random>
#include <vector>

#include "spirit.h"
#include "upgrade.h"

// 升级卡系统：跟随 Progression 的等级变化发卡（三选一）。
// 每一级的选择都记进 history，属性由 history 整体重算；
// 降级时先取消还没选的卡，否则回滚最近一次已选升级。
class UpgradeSystem : public Spirit {
	public:
		struct Choice {
				int level;
				UpgradeId id;
		};

	protected:
		static constexpr std::size_t offer_count = 3;

		std::vector<Choice> _history;
		std::deque<int> _pending_levels;  // 欠着的选卡，元素是触发它的等级
		std::array<UpgradeId, offer_count> _offered{};
		std::size_t _offered_size = 0;
		bool _choosing = false;
		int _hover = -1;
		int _pending_pick = -1;
		UpgradeModifiers _modifiers{};
		int _known_level = 1;
		bool _known_level_valid = false;
		std::deque<UpgradeId> _recent_picks[upgrade_tier_count];
		int _window_width, _window_height;
		std::mt19937 _rng;
		bool _show_history = false;
		bool _f3_held = false;

		UpgradeTier _tier_for_level(int level) const;
		int _stack_count(UpgradeId id) const;
		void _open_cards(int level);
		void _pick(std::size_t index, Scene &scene);
		void _recompute();
		void _apply(Scene &scene);
		void _card_rect(std::size_t index, int &left, int &top, int &width, int &height) const;
		void _draw_history(PIMAGE image);
		// 选卡时自己画一个光标箭头：EGE 隐藏的系统光标在窗口非前台时恢复不了
		void _draw_cursor(PIMAGE image);

	public:
		UpgradeSystem(int window_width, int window_height);

		bool is_choosing() const;
		const UpgradeModifiers &modifiers() const;
		const std::vector<Choice> &history() const;

		// 暂停期间由 App 调用：处理已经排队的选卡
		void modal_update(Scene &scene);
		// 覆盖层由 App 在 HUD 之后绘制，保证卡片压在最上层
		void draw_overlay(PIMAGE image);

		void draw(PIMAGE image) override;
		void update(float delta, Scene &scene) override;
		void handle_keyboard(key_msg msg) override;
		void handle_mouse(mouse_msg msg) override;
		bool is_invalid() const override;
};
