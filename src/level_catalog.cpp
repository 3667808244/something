#include "level_catalog.h"

#include <algorithm>
#include <filesystem>
#include <system_error>

namespace {

namespace fs = std::filesystem;

const wchar_t *builtin_label = L"内置地图";

fs::path executable_dir(int argc, char **argv) {
	if (argc <= 0 || argv == nullptr || argv[0] == nullptr || argv[0][0] == '\0') {
		return {};
	}
	std::error_code ec;
	const fs::path exe = fs::absolute(argv[0], ec);
	if (ec) {
		return {};
	}
	return exe.parent_path();
}

void collect_from(const fs::path &dir, std::vector<fs::path> &out) {
	if (dir.empty()) {
		return;
	}
	std::error_code ec;
	if (!fs::is_directory(dir, ec)) {
		return;
	}
	for (fs::directory_iterator it(dir, ec), end; !ec && it != end; it.increment(ec)) {
		std::error_code file_error;
		if (!it->is_regular_file(file_error)) {
			continue;
		}
		if (it->path().extension() == ".jsonl") {
			out.push_back(it->path());
		}
	}
}

}  // namespace

std::vector<LevelOption> scan_level_options(int argc, char **argv) {
	std::vector<fs::path> files;
	const fs::path exe_dir = executable_dir(argc, argv);
	collect_from(exe_dir, files);
	collect_from(exe_dir / "assets", files);

	std::error_code ec;
	const fs::path cwd = fs::current_path(ec);
	if (!ec) {
		collect_from(cwd, files);
		collect_from(cwd / "assets", files);
	}

	std::vector<fs::path> unique;
	for (const fs::path &file : files) {
		std::error_code canonical_error;
		const fs::path canonical = fs::weakly_canonical(file, canonical_error);
		const fs::path key = canonical_error ? file : canonical;
		if (std::find(unique.begin(), unique.end(), key) == unique.end()) {
			unique.push_back(key);
		}
	}
	std::sort(unique.begin(), unique.end(), [](const fs::path &a, const fs::path &b) {
		return a.filename().wstring() < b.filename().wstring();
	});

	std::vector<LevelOption> options;
	options.reserve(unique.size() + 1);
	for (const fs::path &file : unique) {
		options.push_back({ file.string(), file.stem().wstring() });
	}
	options.push_back({ std::string(), builtin_label });
	return options;
}

std::size_t default_level_index(const std::vector<LevelOption> &options, int argc, char **argv) {
	if (argc > 1 && argv[1] != nullptr && argv[1][0] != '\0') {
		std::error_code ec;
		const fs::path given = fs::weakly_canonical(fs::absolute(argv[1], ec), ec);
		if (!ec) {
			for (std::size_t i = 0; i < options.size(); ++i) {
				if (options[i].path.empty()) {
					continue;
				}
				std::error_code option_error;
				const fs::path option = fs::weakly_canonical(options[i].path, option_error);
				if (!option_error && option == given) {
					return i;
				}
			}
		}
	}
	return 0;
}
