#pragma once

#include <string>

#include "level.h"

// 关卡文件的加载错误：line 为 1 基行号；0 表示与具体行无关（例如打不开文件）
struct LevelLoadError {
	int line = 0;
	std::string message;
};

// 从 JSONL 关卡文件读取关卡数据：每行一个扁平 JSON 对象，按 "type" 区分
// player / obstacle / platform / spawn。注释只用 nlohmann/json 原生支持的 // 与
// /* */（整行注释也行）。
// 成功返回 true；失败返回 false 并把原因（尽量带行号）填进 error。
bool load_level_from_file(const std::string &path, LevelData &out, LevelLoadError &error);
