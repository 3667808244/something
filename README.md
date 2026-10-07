# something

一个用 [EGE](https://xege.org) 写的 2D 平台射击小游戏，用来练手 EGE + C++17。

## 玩法

- `A` / `D` 左右移动，`W` 跳跃，`S` 从单向平台上向下穿过
- 鼠标瞄准，按住左键射击
- `R` 换弹；子弹会被障碍物挡住，打中敌人会掉血
- 接触敌人会掉血并有短暂无敌时间

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
- `include/scene.h`：`Spirit` 容器，负责生命周期与更新/绘制分发
- 坐标系约定：所有 `Rect` 派生类（`Actor` / `Obstacle` / `Platform`）的 `_x/_y` 都是碰撞盒中心点
