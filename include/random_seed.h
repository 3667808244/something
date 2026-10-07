#pragma once

#include <chrono>
#include <cstdint>
#include <random>

// 高精度时钟 + random_device 生成种子；salt 用来区分同一批创建的实体，
// 避免它们因为时钟分辨率相同而拿到同一颗种子。
inline std::mt19937::result_type make_random_seed(std::uint64_t salt = 0) {
	const auto clock_seed =
	    static_cast<std::mt19937::result_type>(std::chrono::steady_clock::now().time_since_epoch().count());
	const auto device_seed = static_cast<std::mt19937::result_type>(std::random_device{}());
	return device_seed ^ clock_seed ^ static_cast<std::mt19937::result_type>(salt);
}
