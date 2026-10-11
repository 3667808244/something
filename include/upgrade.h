#pragma once

#include <cstddef>
#include <vector>

#include "ege/graphics.h"

// 升级分为普通（每级）与高级（每 5 级）两档，分开抽卡。
enum class UpgradeTier { Common, Advanced };

enum class UpgradeId {
	// 普通：无硬上限，卡片上写多少就加多少
	FireRate,
	Magazine,
	MaxHealth,
	MoveSpeed,
	BulletDamage,
	Armor,
	ReloadSpeed,
	// 高级：每 5 级三选一，最多 3 层
	Explosive,
	Lifesteal,
	Magnet,
	Pierce,
	Thorns,
	ExperienceGain,
	Shotgun,
};

struct UpgradeDef {
	UpgradeId id;
	UpgradeTier tier;
	const wchar_t *name;
	const wchar_t *line1;  // 卡片说明第一行
	const wchar_t *line2;  // 卡片说明第二行
	int max_stacks;        // 0 表示无上限
};

// 各升级当前叠了几层；Player 从这里换算自己的实际数值
struct UpgradeModifiers {
	int fire_rate = 0;
	int magazine = 0;
	int max_health = 0;
	int move_speed = 0;
	int bullet_damage = 0;
	int armor = 0;
	int reload_speed = 0;
	int explosive = 0;
	int lifesteal = 0;
	int magnet = 0;
	int pierce = 0;
	int thorns = 0;
	int experience_gain = 0;
	int shotgun = 0;
};

constexpr std::size_t upgrade_tier_count = 2;

const UpgradeDef &upgrade_def(UpgradeId id);
const std::vector<UpgradeId> &upgrade_pool(UpgradeTier tier);
void upgrade_add_stack(UpgradeModifiers &mods, UpgradeId id);

// 读取某个升级当前叠了几层（暂停界面的「升级层数」用）
int upgrade_stack_count(const UpgradeModifiers &mods, UpgradeId id);

// 绘制一张升级卡（普通=蓝灰，高级=金色），index 用于显示 [1]/[2]/[3]
void upgrade_draw_card(PIMAGE image, const UpgradeDef &def, int index, int stacks, int left, int top, int width,
					   int height, bool highlighted);
