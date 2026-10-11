#include "level_loader.h"

#include <fstream>
#include <initializer_list>
#include <string>

#include "nlohmann_json/nlohmann_json.hpp"

namespace {

using Json = nlohmann::json;

// 只允许已知字段：多出未知字段通常意味着拼写错误（例如 "widht"），直接报错
bool check_fields(const Json &record, std::initializer_list<const char *> allowed, int line, LevelLoadError &error) {
	for (auto it = record.begin(); it != record.end(); ++it) {
		bool known = false;
		for (const char *key : allowed) {
			if (it.key() == key) {
				known = true;
				break;
			}
		}
		if (!known) {
			error = { line, "未知字段 \"" + it.key() + "\"" };
			return false;
		}
	}
	return true;
}

bool read_number(const Json &record, const char *key, int line, float &value, LevelLoadError &error) {
	auto it = record.find(key);
	if (it == record.end()) {
		error = { line, std::string("缺少字段 \"") + key + "\"" };
		return false;
	}
	if (!it->is_number()) {
		error = { line, std::string("字段 \"") + key + "\" 必须是数字" };
		return false;
	}
	value = it->get<float>();
	return true;
}

bool read_size(const Json &record, int line, float &width, float &height, LevelLoadError &error) {
	if (!read_number(record, "width", line, width, error) || !read_number(record, "height", line, height, error)) {
		return false;
	}
	if (width <= 0.0f || height <= 0.0f) {
		error = { line, "width/height 必须为正数" };
		return false;
	}
	return true;
}

bool starts_with(const std::string &text, const char *prefix) {
	return text.compare(0, std::char_traits<char>::length(prefix), prefix) == 0;
}

}  // namespace

bool load_level_from_file(const std::string &path, LevelData &out, LevelLoadError &error) {
	std::ifstream input(path, std::ios::binary);
	if (!input) {
		error = { 0, "无法打开关卡文件" };
		return false;
	}

	out = LevelData{};
	bool has_player = false;
	int line_number = 0;
	std::string raw;

	while (std::getline(input, raw)) {
		++line_number;
		// 文件可能带 UTF-8 BOM：第一行开头的 BOM 会让解析直接失败，先去掉
		if (line_number == 1 && raw.size() >= 3 && static_cast<unsigned char>(raw[0]) == 0xEF &&
			static_cast<unsigned char>(raw[1]) == 0xBB && static_cast<unsigned char>(raw[2]) == 0xBF) {
			raw.erase(0, 3);
		}
		if (!raw.empty() && raw.back() == '\r') {
			raw.pop_back();
		}

		const std::size_t first = raw.find_first_not_of(" \t");
		if (first == std::string::npos) {
			continue;  // 空行
		}
		Json record;
		try {
			record = Json::parse(raw, nullptr, true, /*ignore_comments=*/true);
		} catch (const Json::parse_error &parse_error) {
			// 纯注释行（"// ..." 或 "/* ... */"）解析后没有值，按注释跳过
			const std::string trimmed = raw.substr(first);
			if (starts_with(trimmed, "//") || starts_with(trimmed, "/*")) {
				continue;
			}
			error = { line_number, std::string("JSON 解析失败: ") + parse_error.what() };
			return false;
		}

		if (!record.is_object()) {
			error = { line_number, "每行必须是一个 JSON 对象" };
			return false;
		}

		auto type_it = record.find("type");
		if (type_it == record.end() || !type_it->is_string()) {
			error = { line_number, "缺少字符串字段 \"type\"" };
			return false;
		}
		const std::string type = type_it->get<std::string>();

		if (type == "player") {
			if (has_player) {
				error = { line_number, "重复的 player 记录" };
				return false;
			}
			if (!check_fields(record, { "type", "x", "y" }, line_number, error)) {
				return false;
			}
			if (!read_number(record, "x", line_number, out.player_x, error) ||
				!read_number(record, "y", line_number, out.player_y, error)) {
				return false;
			}
			has_player = true;
		} else if (type == "obstacle" || type == "platform") {
			if (!check_fields(record, { "type", "x", "y", "width", "height" }, line_number, error)) {
				return false;
			}
			float x = 0.0f;
			float y = 0.0f;
			float width = 0.0f;
			float height = 0.0f;
			if (!read_number(record, "x", line_number, x, error) || !read_number(record, "y", line_number, y, error) ||
				!read_size(record, line_number, width, height, error)) {
				return false;
			}
			if (type == "obstacle") {
				out.obstacles.push_back({ x, y, width, height });
			} else {
				out.platforms.push_back({ x, y, width, height });
			}
		} else if (type == "spawn") {
			if (!check_fields(record, { "type", "x", "y" }, line_number, error)) {
				return false;
			}
			float x = 0.0f;
			float y = 0.0f;
			if (!read_number(record, "x", line_number, x, error) || !read_number(record, "y", line_number, y, error)) {
				return false;
			}
			out.spawn_points.push_back({ x, y });
		} else {
			error = { line_number, "未知的 type \"" + type + "\"" };
			return false;
		}
	}

	if (!has_player) {
		error = { 0, "缺少 player 记录（关卡必须有玩家出生点）" };
		return false;
	}
	return true;
}
