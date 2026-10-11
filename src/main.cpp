#include <cstddef>
#include <utility>
#include <vector>

#include "app.h"
#include "level_catalog.h"

int main(int argc, char **argv) {
#ifdef _WIN32
	// 控制台默认代码页通常是 GBK，中文提示会显示成乱码；切到 UTF-8
	SetConsoleOutputCP(CP_UTF8);
#endif

	constexpr int window_width = 640;
	constexpr int window_height = 480;

	// 扫描可选地图（exe 目录 / 当前目录及各自的 assets/），至少含一项「内置地图」
	std::vector<LevelOption> levels = scan_level_options(argc, argv);
	const std::size_t default_index = default_level_index(levels, argc, argv);

	App app(window_width, window_height, "ege window");
	app.set_level_options(std::move(levels), default_index);
	app.run();
	return 0;
}
