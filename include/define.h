#pragma once

#include "ege/graphics.h"

// 用 inline 函数代替宏，避免 rgb/rgba 这类常见标识符污染全局命名空间
inline color_t rgb(int r, int g, int b) { return EGERGB(r, g, b); }

inline color_t rgba(int r, int g, int b, int a) { return EGERGBA(r, g, b, a); }
