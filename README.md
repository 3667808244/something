# something

一个用 [EGE](https://xege.org) 写的 2D 平台射击小游戏，用来练手 EGE + C++17。

## 玩法

- `A` / `D` 左右移动，`W` 跳跃，`S` 从单向平台上向下穿过
- 鼠标瞄准，按住左键射击
- `R` 换弹；子弹会被障碍物挡住，打中敌人会掉血
- 接触敌人会掉血并有短暂无敌时间
- `F1` 显示/隐藏刷怪点，`F2` 叠加调试信息（编号、坐标、冷却），`F3` 显示升级记录
- 升级选卡时用鼠标点击或 `1` / `2` / `3` 选择

## 刷怪

敌人由 `Spawner` 自动生成，不再手工摆放在 `main.cpp` 里：

- 刷怪点显式列在 `LevelData::spawn_points`（见 `src/main.cpp` 的 `make_level()`），
  并以常显的法阵标记画在场景中，位置在几何之上、玩家和敌人之下。
- 每个点独立冷却：**灰蓝扇形**表示剩余冷却，**橙色脉冲放大**表示即将生成，
  **绿色**表示就绪。生成瞬间会有一圈青色爆闪。
- 生成前会校验：在窗口内、不压障碍物/平台、离玩家至少 `min_player_distance`；
  校验不过的点显示为**红色**。
- 场上敌人数量上限由 `SpawnerConfig::max_alive` 控制，参数都在 `include/spawner.h`。

## 掉落与经验

- 敌人死亡时掉落一个**经验球**（`ExperienceOrb`）。基类是 `Pickup`，负责重力/落地、
  超时消失、靠近吸附与拾取判定；"捡到之后发生什么" 由子类实现 `apply()`。
- **拾取半径由玩家决定**：`Player::pickup_radius()`，掉落物自己不定义范围；
  吸附范围取该半径的 3 倍（`src/pickup.cpp` 的 `pickup_magnet_scale`）。
- 经验球存活 15s，最后 3s 闪烁；被拾取后给场景里的 `Progression` 加经验。
- `Progression` 是场景级组件：1→2 级需 20 点，之后每级 +15；升级时屏幕上方闪一行
  `LEVEL UP  Lv n`。
- HUD 第三行是经验条，右侧显示 `Lv n`。
- **死亡惩罚**：玩家被敌人打死后（血量归零重生）扣掉一级经验等级，最低 1 级，
  当前等级内的经验清零；掉级时屏幕上方闪一行红色的 `LEVEL DOWN  Lv n`。

## 升级卡

- 每升一级弹出 3 张**普通**升级卡三选一；每 5 级弹出 3 张**高级**升级卡，同样三选一。
  选卡期间游戏暂停，输入只给升级卡界面，不会顺手开枪；此时会画一个鼠标箭头方便点击卡片。
- 普通升级（无硬上限，**线性**，卡片上写多少就加多少）：
  射速 `每层 +10%` / 弹匣容量 `每层 +4 发` / 生命值 `每层 +15` / 移速 `每层 +8%` /
  子弹伤害 `每层 +1` / 护甲提升 `每层 +20 护甲` / 换弹速度 `每层 +12%`。
  生命值升级时当前血量按同一比例回复，向上取整。
- 护甲：独立的第二条蓝色条（在血条下方）。受击**先扣护甲**，但护甲最多抵消 **80%** 伤害，
  剩下的仍然扣血；脱战 3.5 秒后以 4/秒 回复。升级时上限提高会立刻补上差额。
- 高级升级（每种最多 3 层）：爆裂弹（命中概率 AOE）/ 汲取（击杀回血）/ 磁力场（拾取半径）/
  穿透 / 荆棘（受伤概率免疫）/ 求知欲（经验加成）/ 霰弹。
- 霰弹：每次射击消耗多枚弹药（每层 +1 发），弹丸数 = 消耗 × 1.1，**小数部分按概率补 1 发**，
  每发保持原伤害、在扇形内均匀散开；装备后射击间隔固定 ×1.15。
- 抽卡排除：普通池排除最近 1 次选中的升级，高级池排除最近 2 次，两档分开记。
- 每一级的选择都记进 `UpgradeSystem::history`，属性由这份记录整体重算；
  **被敌人打死掉级时回滚最近一次选择**（还没选的卡则直接取消）。
- HUD：弹药 pip、血条 + `当前/上限`、护甲条 + `甲 当前/上限`（有护甲才显示）、经验条 + `Lv n`。
- 参数与升级表见 `include/upgrade.h`、`include/upgrade_system.h`。

## 构建

使用 `ezmk`（见 `ezmk.toml`）：

```sh
ezmk build                    # 默认 debug profile
ezmk build --profile release
ezmk run
ezmk project cc               # 生成 compile_commands.json 供 clangd 使用
```

> `release` profile 通过 `[link.profile.release]` 加了 `-mwindows`（MSVC 为
> `/SUBSYSTEM:WINDOWS`），产物是 Windows 子系统、双击不弹命令行窗口；
> `debug` 仍是控制台子系统，`F3` 的调试输出只在 debug 下看得到。

## 结构

- `include/`、`src/`：游戏本体
- `include/ege/`、`lib/`：EGE 图形库的头文件与静态库
- `include/level.h`：关卡数据（几何 + 刷怪点），`build_level()` 只负责静态几何
- `include/spawner.h`：自动刷怪；`include/spawn_effect.h`：生成爆闪
- `include/pickup.h`：掉落物基类；`include/experience_orb.h`：经验球
- `include/progression.h`：经验/等级系统
- `include/upgrade.h` / `include/upgrade_system.h`：升级目录、卡片绘制、三选一发卡与记录回滚
- `include/explosion_effect.h`：爆裂弹的 AOE 爆炸特效
- `include/scene.h`：`Spirit` 容器，负责生命周期与更新/绘制分发
- 坐标系约定：所有 `Rect` 派生类（`Actor` / `Obstacle` / `Platform`）的 `_x/_y` 都是碰撞盒中心点
