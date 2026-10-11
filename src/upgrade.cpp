#include "upgrade.h"

#include <cmath>
#include <cwchar>
#include <vector>

#include "define.h"

namespace {

constexpr int card_corner = 10;
constexpr int card_title_height = 17;
constexpr int card_body_height = 13;
constexpr int card_hint_height = 14;
constexpr float card_border_width = 1.5f;
constexpr float card_border_width_hover = 3.0f;

const wchar_t *ui_font = L"Microsoft YaHei";

constexpr UpgradeDef defs[] = {
	{ UpgradeId::FireRate, UpgradeTier::Common, L"射速", L"射击间隔缩短", L"每层 +10% 射速", 0 },
	{ UpgradeId::Magazine, UpgradeTier::Common, L"弹匣容量", L"弹匣容量提升", L"每层 +4 发", 0 },
	{ UpgradeId::MaxHealth, UpgradeTier::Common, L"生命值", L"最大生命提升", L"每层 +15", 0 },
	{ UpgradeId::MoveSpeed, UpgradeTier::Common, L"移速", L"移动速度提升", L"每层 +8% 移速", 0 },
	{ UpgradeId::BulletDamage, UpgradeTier::Common, L"子弹伤害", L"子弹伤害提升", L"每层 +1", 0 },
	{ UpgradeId::Armor, UpgradeTier::Common, L"护甲提升", L"受击先扣护甲", L"每层 +20 护甲", 0 },
	{ UpgradeId::ReloadSpeed, UpgradeTier::Common, L"换弹速度", L"换弹时间缩短", L"每层 +12% 换弹速度", 0 },
	{ UpgradeId::JumpPower, UpgradeTier::Common, L"弹跳力", L"跳得更高", L"每层 +8% 跳跃力", 0 },
	{ UpgradeId::Invulnerability, UpgradeTier::Common, L"无敌延长", L"受击后无敌更久", L"每层 +0.12 秒", 0 },
	{ UpgradeId::ArmorRegen, UpgradeTier::Common, L"护甲回复", L"脱战后护甲回复更快", L"延迟 -0.3s 速度 +1/s", 0 },
	{ UpgradeId::AmmoOnKill, UpgradeTier::Common, L"击杀补弹", L"击杀敌人补充弹药", L"每层 +2 发", 0 },
	{ UpgradeId::Explosive, UpgradeTier::Advanced, L"爆裂弹", L"命中敌人时概率造成", L"小范围 AOE 伤害", 3 },
	{ UpgradeId::Lifesteal, UpgradeTier::Advanced, L"汲取", L"击杀敌人回复生命", L"每层 +3", 3 },
	{ UpgradeId::Magnet, UpgradeTier::Advanced, L"磁力场", L"拾取半径扩大", L"每层 +40%", 3 },
	{ UpgradeId::Pierce, UpgradeTier::Advanced, L"穿透", L"子弹可多穿透敌人", L"每层 +1 个", 3 },
	{ UpgradeId::Thorns, UpgradeTier::Advanced, L"荆棘", L"受到伤害时概率免疫", L"每层 +20%", 3 },
	{ UpgradeId::ExperienceGain, UpgradeTier::Advanced, L"求知欲", L"获得经验提升", L"每层 +25%", 3 },
	{ UpgradeId::Shotgun, UpgradeTier::Advanced, L"霰弹", L"一次消耗多枚弹药", L"弹丸为消耗的 1.1 倍", 3 },
	{ UpgradeId::DoubleJump, UpgradeTier::Advanced, L"二段跳", L"空中可以再跳", L"每层 +1 段跳跃", 3 },
	{ UpgradeId::Ricochet, UpgradeTier::Advanced, L"跳弹", L"子弹打到墙会反弹", L"每层 +1 次反弹", 3 },
	{ UpgradeId::EliteHunter, UpgradeTier::Advanced, L"精英猎手", L"对精英敌人伤害提高", L"每层 +50%", 3 },
	{ UpgradeId::Drone, UpgradeTier::Advanced, L"僚机", L"环绕你自动攻击", L"每层 +1 架", 3 },
};

constexpr UpgradeId common_pool[] = {
	UpgradeId::FireRate,	 UpgradeId::Magazine,	 UpgradeId::MaxHealth,	  UpgradeId::MoveSpeed,
	UpgradeId::BulletDamage, UpgradeId::Armor,		 UpgradeId::ReloadSpeed,  UpgradeId::JumpPower,
	UpgradeId::Invulnerability, UpgradeId::ArmorRegen, UpgradeId::AmmoOnKill,
};

constexpr UpgradeId advanced_pool[] = {
	UpgradeId::Explosive, UpgradeId::Lifesteal,		 UpgradeId::Magnet,		UpgradeId::Pierce,
	UpgradeId::Thorns,	  UpgradeId::ExperienceGain, UpgradeId::Shotgun,	UpgradeId::DoubleJump,
	UpgradeId::Ricochet,  UpgradeId::EliteHunter,	 UpgradeId::Drone,
};

}  // namespace

const UpgradeDef &upgrade_def(UpgradeId id) {
	for (const auto &def : defs) {
		if (def.id == id) {
			return def;
		}
	}
	return defs[0];
}

const std::vector<UpgradeId> &upgrade_pool(UpgradeTier tier) {
	static const std::vector<UpgradeId> common(std::begin(common_pool), std::end(common_pool));
	static const std::vector<UpgradeId> advanced(std::begin(advanced_pool), std::end(advanced_pool));
	return tier == UpgradeTier::Advanced ? advanced : common;
}

void upgrade_add_stack(UpgradeModifiers &mods, UpgradeId id) {
	switch (id) {
		case UpgradeId::FireRate: ++mods.fire_rate; break;
		case UpgradeId::Magazine: ++mods.magazine; break;
		case UpgradeId::MaxHealth: ++mods.max_health; break;
		case UpgradeId::MoveSpeed: ++mods.move_speed; break;
		case UpgradeId::BulletDamage: ++mods.bullet_damage; break;
		case UpgradeId::Armor: ++mods.armor; break;
		case UpgradeId::ReloadSpeed: ++mods.reload_speed; break;
		case UpgradeId::JumpPower: ++mods.jump_power; break;
		case UpgradeId::Invulnerability: ++mods.invulnerability; break;
		case UpgradeId::ArmorRegen: ++mods.armor_regen; break;
		case UpgradeId::AmmoOnKill: ++mods.ammo_on_kill; break;
		case UpgradeId::Explosive: ++mods.explosive; break;
		case UpgradeId::Lifesteal: ++mods.lifesteal; break;
		case UpgradeId::Magnet: ++mods.magnet; break;
		case UpgradeId::Pierce: ++mods.pierce; break;
		case UpgradeId::Thorns: ++mods.thorns; break;
		case UpgradeId::ExperienceGain: ++mods.experience_gain; break;
		case UpgradeId::Shotgun: ++mods.shotgun; break;
		case UpgradeId::DoubleJump: ++mods.double_jump; break;
		case UpgradeId::Ricochet: ++mods.ricochet; break;
		case UpgradeId::EliteHunter: ++mods.elite_hunter; break;
		case UpgradeId::Drone: ++mods.drone; break;
	}
}

int upgrade_stack_count(const UpgradeModifiers &mods, UpgradeId id) {
	switch (id) {
		case UpgradeId::FireRate: return mods.fire_rate;
		case UpgradeId::Magazine: return mods.magazine;
		case UpgradeId::MaxHealth: return mods.max_health;
		case UpgradeId::MoveSpeed: return mods.move_speed;
		case UpgradeId::BulletDamage: return mods.bullet_damage;
		case UpgradeId::Armor: return mods.armor;
		case UpgradeId::ReloadSpeed: return mods.reload_speed;
		case UpgradeId::JumpPower: return mods.jump_power;
		case UpgradeId::Invulnerability: return mods.invulnerability;
		case UpgradeId::ArmorRegen: return mods.armor_regen;
		case UpgradeId::AmmoOnKill: return mods.ammo_on_kill;
		case UpgradeId::Explosive: return mods.explosive;
		case UpgradeId::Lifesteal: return mods.lifesteal;
		case UpgradeId::Magnet: return mods.magnet;
		case UpgradeId::Pierce: return mods.pierce;
		case UpgradeId::Thorns: return mods.thorns;
		case UpgradeId::ExperienceGain: return mods.experience_gain;
		case UpgradeId::Shotgun: return mods.shotgun;
		case UpgradeId::DoubleJump: return mods.double_jump;
		case UpgradeId::Ricochet: return mods.ricochet;
		case UpgradeId::EliteHunter: return mods.elite_hunter;
		case UpgradeId::Drone: return mods.drone;
	}
	return 0;
}

void upgrade_draw_card(PIMAGE image, const UpgradeDef &def, int index, int stacks, int left, int top, int width,
					   int height, bool highlighted) {
	int right = left + width;
	int bottom = top + height;
	bool advanced = def.tier == UpgradeTier::Advanced;

	color_t border = advanced ? rgb(235, 180, 70) : rgb(110, 165, 225);
	color_t background = highlighted ? rgb(40, 50, 70) : rgb(24, 30, 42);
	if (advanced && highlighted) {
		background = rgb(58, 46, 28);
	}

	setfillcolor(background, image);
	fillroundrect(left, top, right, bottom, card_corner, image);
	setlinecolor(border, image);
	setlinewidth(highlighted ? card_border_width_hover : card_border_width, image);
	roundrect(left, top, right, bottom, card_corner, image);
	setlinewidth(1.0f, image);

	setbkmode(TRANSPARENT, image);

	// 标题
	setfont(card_title_height, 0, ui_font, image);
	settextcolor(advanced ? rgb(255, 224, 150) : rgb(225, 238, 255), image);
	int name_width = textwidth(def.name, image);
	outtextxy(left + (width - name_width) / 2, top + 14, def.name, image);

	// 档位标签
	setfont(card_body_height - 2, 0, ui_font, image);
	settextcolor(border, image);
	const wchar_t *tag = advanced ? L"高级" : L"普通";
	int tag_width = textwidth(tag, image);
	outtextxy(left + (width - tag_width) / 2, top + 38, tag, image);

	// 说明两行
	setfont(card_body_height, 0, ui_font, image);
	settextcolor(rgb(205, 214, 226), image);
	int line1_width = textwidth(def.line1, image);
	outtextxy(left + (width - line1_width) / 2, top + 76, def.line1, image);
	int line2_width = textwidth(def.line2, image);
	outtextxy(left + (width - line2_width) / 2, top + 96, def.line2, image);

	// 已叠加层数
	wchar_t stack_text[48];
	if (stacks > 0) {
		std::swprintf(stack_text, 48, L"已叠加 %d 层", stacks);
	} else {
		std::swprintf(stack_text, 48, L"尚未选择");
	}
	setfont(card_body_height, 0, ui_font, image);
	settextcolor(stacks > 0 ? rgb(150, 230, 190) : rgb(140, 150, 166), image);
	int stack_width = textwidth(stack_text, image);
	outtextxy(left + (width - stack_width) / 2, bottom - 44, stack_text, image);

	// 按键提示
	wchar_t hint[16];
	std::swprintf(hint, 16, L"[%d]", index + 1);
	setfont(card_hint_height, 0, ui_font, image);
	settextcolor(highlighted ? rgb(255, 255, 255) : rgb(170, 180, 196), image);
	int hint_width = textwidth(hint, image);
	outtextxy(left + (width - hint_width) / 2, bottom - 26, hint, image);
}
