#include <filesystem>
#include <iostream>
#include <string>
#include <system_error>
#include <vector>

#include "app.h"
#include "level.h"
#include "level_loader.h"
#include "player.h"
#include "progression.h"
#include "spawner.h"
#include "upgrade_system.h"

namespace {

constexpr int window_width = 640;
constexpr int window_height = 480;
constexpr const char *level_file_name = "level1.jsonl";

// 内置兜底地图：找不到关卡文件时使用，保证游戏一定能启动
LevelData builtin_level() {
	LevelData level;
	// Obstacle / Platform 的坐标是碰撞盒中心点，与 Actor 保持同一套约定
	level.obstacles = {
		{ 120.0f, 410.0f, 160.0f, 20.0f },
		{ 300.0f, 350.0f, 120.0f, 20.0f },
		{ 530.0f, 290.0f, 140.0f, 20.0f },
		{ 320.0f, 474.0f, 640.0f, 12.0f },
	};
	level.platforms = {
		{ 140.0f, 306.0f, 120.0f, 12.0f },
		{ 380.0f, 246.0f, 160.0f, 12.0f },
		{ 535.0f, 386.0f, 130.0f, 12.0f },
	};
	// 刷怪点显式列出：顶部两角、正上方，以及两块平台上空
	level.spawn_points = {
		{ 60.0f, 110.0f },
		{ 580.0f, 110.0f },
		{ 320.0f, 70.0f },
		{ 100.0f, 250.0f },
		{ 540.0f, 175.0f },
	};
	level.player_x = 320.0f;
	level.player_y = 180.0f;
	return level;
}

// 关卡查找顺序：命令行参数 -> exe 目录 -> 当前目录 -> 上述两者的 assets/ 子目录
std::filesystem::path resolve_level_path(int argc, char **argv) {
	std::vector<std::filesystem::path> candidates;

	if (argc > 1 && argv[1] != nullptr && argv[1][0] != '\0') {
		candidates.emplace_back(argv[1]);
	}

	std::filesystem::path exe_dir;
	if (argc > 0 && argv[0] != nullptr && argv[0][0] != '\0') {
		std::error_code ec;
		const std::filesystem::path exe = std::filesystem::absolute(argv[0], ec);
		if (!ec) {
			exe_dir = exe.parent_path();
		}
	}
	if (!exe_dir.empty()) {
		candidates.push_back(exe_dir / level_file_name);
		candidates.push_back(exe_dir / ".." / "assets" / level_file_name);
	}

	std::error_code ec;
	const std::filesystem::path cwd = std::filesystem::current_path(ec);
	if (!ec) {
		candidates.push_back(cwd / level_file_name);
		candidates.push_back(cwd / "assets" / level_file_name);
	}

	for (const std::filesystem::path &candidate : candidates) {
		std::error_code file_error;
		if (std::filesystem::is_regular_file(candidate, file_error)) {
			return candidate;
		}
	}
	return {};
}

}  // namespace

int main(int argc, char **argv) {
#ifdef _WIN32
	// 控制台默认代码页通常是 GBK，中文提示会显示成乱码；切到 UTF-8
	SetConsoleOutputCP(CP_UTF8);
#endif
	LevelData level;
	const std::filesystem::path level_path = resolve_level_path(argc, argv);
	if (level_path.empty()) {
		std::cerr << "[level] 找不到 " << level_file_name << "，使用内置地图" << std::endl;
		level = builtin_level();
	} else {
		LevelLoadError error;
		if (!load_level_from_file(level_path.string(), level, error)) {
			if (error.line > 0) {
				std::cerr << level_path.string() << ":" << error.line << ": " << error.message << std::endl;
			} else {
				std::cerr << level_path.string() << ": " << error.message << std::endl;
			}
			return 1;
		}
	}

	App app(window_width, window_height, "ege window");
	// 绘制顺序 = 插入顺序：几何 -> 刷怪点 -> 玩家。
	// 刷怪点标记因此落在几何之上、玩家和敌人之下，不会挡住实体。
	build_level(app.root_scene(), level);
	app.add_spirit<Spawner>(level.spawn_points, window_width, window_height, SpawnerConfig{});
	app.add_spirit<Player>(window_width, window_height, level.player_x, level.player_y);
	// 经验/等级系统；放在最后，升级提示文字盖在其它实体之上
	app.add_spirit<Progression>(window_width);
	// 升级卡系统：跟随等级发卡；覆盖层由 App 在 HUD 之后绘制
	app.add_spirit<UpgradeSystem>(window_width, window_height);
	app.run();
	return 0;
}
