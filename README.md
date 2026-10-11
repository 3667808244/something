# something

一个用 [EGE](https://xege.org) 写的 2D 平台射击小游戏，用来练手 EGE + C++17。

## 玩法

- `A` / `D` 左右移动，`W` 跳跃，`S` 从单向平台上向下穿过
- 鼠标瞄准，按住左键射击
- `R` 换弹；子弹会被障碍物挡住，打中敌人会掉血
- 接触敌人会掉血并有短暂无敌时间
- `F1` 显示/隐藏刷怪点，`F2` 叠加调试信息（编号、坐标、冷却）

## 刷怪

敌人由 `Spawner` 自动生成，不再手工摆放在 `main.cpp` 里：

- 刷怪点显式列在 `LevelData::spawn_points`（见 `src/main.cpp` 的 `make_level()`），
  并以常显的法阵标记画在场景中，位置在几何之上、玩家和敌人之下。
- 每个点独立冷却：**灰蓝扇形**表示剩余冷却，**橙色脉冲放大**表示即将生成，
  **绿色**表示就绪。生成瞬间会有一圈青色爆闪。
- 生成前会校验：在窗口内、不压障碍物/平台、离玩家至少 `min_player_distance`；
  校验不过的点显示为**红色**。
- 场上敌人数量上限由 `SpawnerConfig::max_alive` 控制，参数都在 `include/spawner.h`。

## 构建

使用 `ezmk`（见 `ezmk.toml`）：

```sh
ezmk build                    # 默认 debug profile
ezmk build --profile release
ezmk run
ezmk project cc               # 生成 compile_commands.json 供 clangd 使用
```

## 结构

- `include/`、`src/`：游戏本体
- `include/ege/`、`lib/`：EGE 图形库的头文件与静态库
- `include/level.h`：关卡数据（几何 + 刷怪点），`build_level()` 只负责静态几何
- `include/spawner.h`：自动刷怪；`include/spawn_effect.h`：生成爆闪
- `include/scene.h`：`Spirit` 容器，负责生命周期与更新/绘制分发
- 坐标系约定：所有 `Rect` 派生类（`Actor` / `Obstacle` / `Platform`）的 `_x/_y` 都是碰撞盒中心点
