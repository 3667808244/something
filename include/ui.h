#pragma once

#include <string>

#include "ege/graphics.h"

// 菜单与暂停界面共用的小控件和绘制工具。
// 配色/圆角/字体都沿用升级卡那一套，保证整个 UI 观感一致。

struct UiButton {
	int id = -1;
	std::wstring label;
	int left = 0;
	int top = 0;
	int width = 0;
	int height = 0;

	bool contains(int x, int y) const;
};

// 全屏压暗（0~255，0 表示不画）
void ui_draw_dim(PIMAGE image, int width, int height, int alpha);

// 居中圆角面板
void ui_draw_panel(PIMAGE image, int left, int top, int width, int height);

// 按钮：highlighted 时用高亮边框和文字
void ui_draw_button(PIMAGE image, const UiButton &button, bool highlighted);

// 自绘鼠标箭头（窗口调用过 showmouse(0)，菜单需要自己画）
void ui_draw_cursor(PIMAGE image);

// 以 center_x 为中心的居中文字
void ui_draw_text_center(PIMAGE image, int center_x, int y, int font_height, const wchar_t *font, color_t color,
			const wchar_t *text);

// UTF-8（关卡加载错误信息用）转宽字符
std::wstring ui_utf8_to_wide(const std::string &text);
