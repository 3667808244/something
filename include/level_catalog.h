#pragma once

#include <cstddef>
#include <string>
#include <vector>

// 可选地图：path 为空表示内置地图
struct LevelOption {
	std::string path;
	std::wstring label;
};

// 扫描 exe 目录、当前目录及各自的 assets/ 子目录下的 *.jsonl（按绝对路径去重、按文件名排序），
// 末尾追加一项「内置地图」。返回值至少含一项。
std::vector<LevelOption> scan_level_options(int argc, char **argv);

// argv[1] 指向的文件在列表里就用它，否则用第一项
std::size_t default_level_index(const std::vector<LevelOption> &options, int argc, char **argv);
