#include "ui.h"

#include "define.h"

namespace {

constexpr int button_corner = 8;
constexpr float button_border = 1.5f;
constexpr float button_border_hover = 2.5f;
const wchar_t *ui_font = L"Microsoft YaHei";

// 压暗用的缓存全屏黑图；跟随窗口尺寸重建（进程退出时随图形系统一起释放）
PIMAGE dim_image = nullptr;
int dim_width = 0;
int dim_height = 0;

}  // namespace

bool UiButton::contains(int x, int y) const {
	return x >= left && x < left + width && y >= top && y < top + height;
}

void ui_draw_dim(PIMAGE image, int width, int height, int alpha) {
	if (alpha <= 0 || width <= 0 || height <= 0) {
		return;
	}
	// setfillcolor / bar 不吃 ege_setalpha（那样等于盖一块不透明黑布），
	// 用一张缓存的全屏纯黑图做 alpha 混合，世界才会透出来。
	if (dim_image == nullptr || dim_width != width || dim_height != height) {
		if (dim_image != nullptr) {
			delimage(dim_image);
			dim_image = nullptr;
		}
		dim_image = newimage(width, height);
		if (dim_image != nullptr) {
			dim_width = width;
			dim_height = height;
			setfillcolor(rgb(0, 0, 0), dim_image);
			bar(0, 0, width, height, dim_image);
		}
	}
	if (dim_image == nullptr) {
		return;
	}
	putimage_alphablend(image, dim_image, 0, 0, static_cast<unsigned char>(alpha));
}

void ui_draw_panel(PIMAGE image, int left, int top, int width, int height) {
	setfillcolor(rgb(12, 16, 24), image);
	fillroundrect(left, top, left + width, top + height, 16, image);
	setlinecolor(rgb(72, 86, 110), image);
	setlinewidth(1.5f, image);
	roundrect(left, top, left + width, top + height, 16, image);
	setlinewidth(1.0f, image);
}

void ui_draw_button(PIMAGE image, const UiButton &button, bool highlighted) {
	const int right = button.left + button.width;
	const int bottom = button.top + button.height;
	const color_t text_color = highlighted ? rgb(240, 248, 255) : rgb(198, 210, 228);

	setfillcolor(highlighted ? rgb(40, 50, 70) : rgb(24, 30, 42), image);
	fillroundrect(button.left, button.top, right, bottom, button_corner, image);
	setlinecolor(highlighted ? rgb(130, 190, 255) : rgb(90, 120, 160), image);
	setlinewidth(highlighted ? button_border_hover : button_border, image);
	roundrect(button.left, button.top, right, bottom, button_corner, image);
	setlinewidth(1.0f, image);

	const int font_height = 16;
	const int text_y = button.top + (button.height - font_height) / 2 - 2;
	ui_draw_text_center(image, button.left + button.width / 2, text_y, font_height, ui_font, text_color,
												button.label.c_str());
}

void ui_draw_cursor(PIMAGE image) {
	int mouse_x = 0;
	int mouse_y = 0;
	mousepos(&mouse_x, &mouse_y);

	static const int arrow[7][2] = {
		{ 0, 0 }, { 0, 17 }, { 4, 13 }, { 8, 21 }, { 11, 19 }, { 7, 11 }, { 12, 11 },
	};

	ege_point points[7];
	for (int i = 0; i < 7; ++i) {
		points[i].x = static_cast<float>(mouse_x + arrow[i][0]);
		points[i].y = static_cast<float>(mouse_y + arrow[i][1]);
	}

	setfillcolor(rgb(255, 255, 255), image);
	ege_fillpoly(7, points, image);

	setlinecolor(rgb(20, 22, 28), image);
	for (int i = 0; i < 7; ++i) {
		const int j = (i + 1) % 7;
		line(mouse_x + arrow[i][0], mouse_y + arrow[i][1], mouse_x + arrow[j][0], mouse_y + arrow[j][1], image);
	}
}

void ui_draw_text_center(PIMAGE image, int center_x, int y, int font_height, const wchar_t *font, color_t color,
			const wchar_t *text) {
	setfont(font_height, 0, font, image);
	setbkmode(TRANSPARENT, image);
	settextcolor(color, image);
	const int width = textwidth(text, image);
	outtextxy(center_x - width / 2, y, text, image);
}

std::wstring ui_utf8_to_wide(const std::string &text) {
	if (text.empty()) {
		return std::wstring();
	}
	const int size = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), static_cast<int>(text.size()), nullptr, 0);
	if (size <= 0) {
		return std::wstring();
	}
	std::wstring wide(static_cast<std::size_t>(size), L'\0');
	MultiByteToWideChar(CP_UTF8, 0, text.c_str(), static_cast<int>(text.size()), &wide[0], size);
	return wide;
}
