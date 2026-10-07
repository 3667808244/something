#ifndef EGEXX_H_
#define EGEXX_H_ 1

#include <graphics.h>
#include <cstdarg>
#include <cstdio>
#include <stdexcept>
#include <string>

/*
EGEXX – 修复与补全的 EGE 25.11 C++11 封装
用法：仅包含本头文件即可使用全部功能。
*/

namespace egexx
{
    // ========================== Image 类 ==========================
    class Image
    {
    protected:
        PIMAGE _img; // 被管理的 PIMAGE，nullptr 表示屏幕窗口
        bool _owned; // 是否拥有该图像（true: 需要释放，false: 不释放）

    public:
        // 构造函数：接管已有图像，默认拥有所有权（窗口除外）
        explicit Image(PIMAGE img = nullptr, bool owned = true)
            : _img(img), _owned(owned && img != nullptr) {}

        // 禁用拷贝
        Image(const Image &) = delete;
        Image &operator=(const Image &) = delete;

        // 移动构造
        Image(Image &&other) noexcept
            : _img(other._img), _owned(other._owned)
        {
            other._img = nullptr;
            other._owned = false;
        }

        // 移动赋值
        Image &operator=(Image &&other) noexcept
        {
            if (this != &other)
            {
                // 释放当前资源（如果拥有）
                if (_owned && _img != nullptr)
                    delimage(_img);
                _img = other._img;
                _owned = other._owned;
                other._img = nullptr;
                other._owned = false;
            }
            return *this;
        }

        // 析构函数：释放拥有的图像资源
        ~Image()
        {
            if (_owned && _img != nullptr)
                delimage(_img);
        }

        // 获取原始指针（谨慎使用）
        PIMAGE get() const { return _img; }

        // 判断是否为窗口（不代表实际图像）
        bool isWindow() const { return _img == nullptr; }

        // 工厂：获取屏幕窗口（不拥有所有权）
        static Image get_win()
        {
            return Image(nullptr, false);
        }

        // 工厂：新建空白图像（1x1）
        static Image get_new()
        {
            return Image(newimage(), true);
        }

        // 工厂：新建指定大小图像
        static Image get_new(int w, int h)
        {
            return Image(newimage(w, h), true);
        }

        // 从文件加载图像（自动调整目标图像大小）
        static Image fromFile(const std::string &filename, int zoomW = 0, int zoomH = 0)
        {
            Image img = get_new();
            if (getimage(img._img, filename.c_str(), zoomW, zoomH) != 0)
                throw std::runtime_error("Failed to load image");
            return img;
        }
        static Image fromFile(const std::wstring &filename, int zoomW = 0, int zoomH = 0)
        {
            Image img = get_new();
            if (getimage(img._img, filename.c_str(), zoomW, zoomH) != 0)
                throw std::runtime_error("Failed to load image");
            return img;
        }

        // 保存图像
        bool save(const std::string &filename, bool withAlpha = false) const
        {
            return saveimage(_img, filename.c_str(), withAlpha) == 0;
        }
        bool save(const std::wstring &filename, bool withAlpha = false) const
        {
            return saveimage(_img, filename.c_str(), withAlpha) == 0;
        }
        bool savePNG(const std::string &filename, bool withAlpha = false) const
        {
            return savepng(_img, filename.c_str(), withAlpha) == 0;
        }
        bool saveBMP(const std::string &filename, bool withAlpha = false) const
        {
            return savebmp(_img, filename.c_str(), withAlpha) == 0;
        }

        // 调整大小
        bool resize(int w, int h) { return ege::resize(_img, w, h) == 0; }
        bool resize_f(int w, int h) { return ege::resize_f(_img, w, h) == 0; }

        // 尺寸查询
        int width() const { return getwidth(_img); }
        int height() const { return getheight(_img); }
        int getx() const { return ege::getx(_img); }
        int gety() const { return ege::gety(_img); }

        // 直接缓冲区访问
        color_t *buffer() { return getbuffer(_img); }
        const color_t *buffer() const { return getbuffer(_img); }

        // ========== 基本绘图 ==========
        void cleardevice() { ege::cleardevice(_img); }

        // 像素操作
        color_t getpixel(int x, int y) const { return ege::getpixel(x, y, _img); }
        void putpixel(int x, int y, color_t c) { ege::putpixel(x, y, c, _img); }
        void putpixel_f(int x, int y, color_t c) { ege::putpixel_f(x, y, c, _img); }
        void putpixel_withalpha(int x, int y, color_t c) { ege::putpixel_withalpha(x, y, c, _img); }

        // 线段/形状
        void line(int x1, int y1, int x2, int y2) { ege::line(x1, y1, x2, y2, _img); }
        void line_f(float x1, float y1, float x2, float y2) { ege::line_f(x1, y1, x2, y2, _img); }
        void lineto(int x, int y) { ege::lineto(x, y, _img); }
        void lineto_f(float x, float y) { ege::lineto_f(x, y, _img); }
        void linerel(int dx, int dy) { ege::linerel(dx, dy, _img); }
        void rectangle(int left, int top, int right, int bottom) { ege::rectangle(left, top, right, bottom, _img); }
        void fillrect(int left, int top, int right, int bottom) { ege::fillrect(left, top, right, bottom, _img); }
        void solidrect(int left, int top, int right, int bottom) { ege::solidrect(left, top, right, bottom, _img); }
        void circle(int x, int y, int r) { ege::circle(x, y, r, _img); }
        void fillcircle(int x, int y, int r) { ege::fillcircle(x, y, r, _img); }
        void solidcircle(int x, int y, int r) { ege::solidcircle(x, y, r, _img); }
        void ellipse(int x, int y, int sa, int ea, int xr, int yr) { ege::ellipse(x, y, sa, ea, xr, yr, _img); }
        void fillellipse(int x, int y, int xr, int yr) { ege::fillellipse(x, y, xr, yr, _img); }
        void pie(int x, int y, int sa, int ea, int xr, int yr) { ege::pie(x, y, sa, ea, xr, yr, _img); }
        void fillpie(int x, int y, int sa, int ea, int xr, int yr) { ege::fillpie(x, y, sa, ea, xr, yr, _img); }
        void polygon(int num, const int *pts) { ege::polygon(num, pts, _img); }
        void fillpoly(int num, const int *pts) { ege::fillpoly(num, pts, _img); }
        void drawbezier(int num, const int *pts) { ege::drawbezier(num, pts, _img); }
        void bar(int x1, int y1, int x2, int y2) { ege::bar(x1, y1, x2, y2, _img); }

        // ========== 文本 ==========
        void outtext(const std::string &s) { ege::outtext(s.c_str(), _img); }
        void outtext(const std::wstring &s) { ege::outtext(s.c_str(), _img); }
        void outtextxy(int x, int y, const std::string &s) { ege::outtextxy(x, y, s.c_str(), _img); }
        void outtextxy(int x, int y, const std::wstring &s) { ege::outtextxy(x, y, s.c_str(), _img); }
        void xyprintf(int x, int y, const char *fmt, ...)
        {
            va_list ap;
            va_start(ap, fmt);
            char buf[2048];
            vsnprintf(buf, sizeof(buf), fmt, ap);
            va_end(ap);
            ege::outtextxy(x, y, buf, _img);
        }
        void xyprintf(int x, int y, const wchar_t *fmt, ...)
        {
            va_list ap;
            va_start(ap, fmt);
            wchar_t buf[2048];
            vswprintf(buf, sizeof(buf) / sizeof(wchar_t), fmt, ap);
            va_end(ap);
            ege::outtextxy(x, y, buf, _img);
        }
        int textwidth(const std::string &s) const { return ege::textwidth(s.c_str(), _img); }
        int textheight(const std::string &s) const { return ege::textheight(s.c_str(), _img); }
        int textwidth(const std::wstring &s) const { return ege::textwidth(s.c_str(), _img); }
        int textheight(const std::wstring &s) const { return ege::textheight(s.c_str(), _img); }

        // 字体与对齐
        void setfont(int h, int w, const std::string &face) { ege::setfont(h, w, face.c_str(), _img); }
        void setfont(const LOGFONTW *lf) { ege::setfont(lf, _img); }
        void settextjustify(int horiz, int vert) { ege::settextjustify(horiz, vert, _img); }
        void settextcolor(color_t c) { ege::settextcolor(c, _img); }
        void setfontbkcolor(color_t c) { ege::setfontbkcolor(c, _img); }

        // ========== 视口 ==========
        void getviewport(int *l, int *t, int *r, int *b, int *clip = nullptr) const
        {
            ege::getviewport(l, t, r, b, clip, _img);
        }
        void setviewport(int l, int t, int r, int b, int clip = 1)
        {
            ege::setviewport(l, t, r, b, clip, _img);
        }
        void clearviewport() { ege::clearviewport(_img); }

        // ========== 样式属性 ==========
        void setlinewidth(float w) { ege::setlinewidth(w, _img); }
        void setlinestyle(int style, unsigned short pattern = 0, int thick = 1)
        {
            ege::setlinestyle(style, pattern, thick, _img);
        }
        void setlinecap(line_cap_type cap) { ege::setlinecap(cap, _img); }
        void setlinecaps(line_cap_type start, line_cap_type end) { ege::setlinecap(start, end, _img); }
        void setlinejoin(line_join_type join) { ege::setlinejoin(join, _img); }
        void setlinejoin(line_join_type join, float miter) { ege::setlinejoin(join, miter, _img); }
        void setfillstyle(int pattern, color_t color) { ege::setfillstyle(pattern, color, _img); }
        void setwritemode(int mode) { ege::setwritemode(mode, _img); }
        void setcolor(color_t c) { ege::setcolor(c, _img); }
        void setlinecolor(color_t c) { ege::setlinecolor(c, _img); }
        void setfillcolor(color_t c) { ege::setfillcolor(c, _img); }
        void setbkcolor(color_t c) { ege::setbkcolor(c, _img); }

        color_t getcolor() const { return ege::getcolor(_img); }
        color_t getlinecolor() const { return ege::getlinecolor(_img); }
        color_t getfillcolor() const { return ege::getfillcolor(_img); }
        color_t getbkcolor() const { return ege::getbkcolor(_img); }
        color_t gettextcolor() const { return ege::gettextcolor(_img); }

        // ========== 图像绘制 (putimage) ==========
        void putimage(int x, int y, const Image &src, DWORD rop = SRCCOPY)
        {
            ege::putimage(_img, x, y, src._img, rop);
        }
        void putimage(int x, int y, int w, int h, const Image &src, int sx, int sy, DWORD rop = SRCCOPY)
        {
            ege::putimage(_img, x, y, w, h, src._img, sx, sy, rop);
        }
        void stretchput(int x, int y, int w, int h, const Image &src, int sx, int sy, int sw, int sh, DWORD rop = SRCCOPY)
        {
            ege::putimage(_img, x, y, w, h, src._img, sx, sy, sw, sh, rop);
        }

        // 透明色绘制
        bool put_transparent(int x, int y, const Image &src, color_t transparentColor,
                             int sx = 0, int sy = 0, int sw = 0, int sh = 0)
        {
            return ege::putimage_transparent(_img, src._img, x, y, transparentColor, sx, sy, sw, sh) == 0;
        }

        // Alpha 混合绘制（整体透明度）
        bool alphablend(int x, int y, const Image &src, unsigned char alpha,
                        color_type ct = COLORTYPE_PRGB32)
        {
            return ege::putimage_alphablend(_img, src._img, x, y, alpha, ct) == 0;
        }
        bool alphablend(int x, int y, int w, int h, const Image &src, int sx, int sy,
                        unsigned char alpha, bool smooth = false)
        {
            return ege::putimage_alphablend(_img, src._img, x, y, w, h, alpha, sx, sy, src.width(), src.height(), smooth) == 0;
        }

        // 使用图像自身 Alpha 通道
        bool draw_withalpha(int x, int y, const Image &src, int sx = 0, int sy = 0, int sw = 0, int sh = 0)
        {
            return ege::putimage_withalpha(_img, src._img, x, y, sx, sy, sw, sh) == 0;
        }
    };

    // ========================== 随机数设备（Meyers 单例，线程安全） ==========================
    class RandomDevice
    {
    public:
        static RandomDevice &get()
        {
#ifdef EGEXX_RD_USING_STDLIB_H_
            // 使用标准库 rand()
            static RandomDevice instance([]
                                         {
                srand(static_cast<unsigned>(time(nullptr)));
                return true; }());
#else
            // 使用 EGE 随机函数
            static RandomDevice instance([]
                                         {
                randomize();
                return true; }());
#endif
            return instance;
        }

        double randomf()
        {
#ifdef EGEXX_RD_USING_STDLIB_H_
            return static_cast<double>(rand()) / RAND_MAX;
#else
            return ege::randomf();
#endif
        }

        unsigned int random(unsigned int n = 0)
        {
#ifdef EGEXX_RD_USING_STDLIB_H_
            return n ? rand() % n : rand();
#else
            return ege::random(n);
#endif
        }

    private:
        RandomDevice(bool) {} // 仅用于初始化种子
        RandomDevice(const RandomDevice &) = delete;
        RandomDevice &operator=(const RandomDevice &) = delete;
    };

} // namespace egexx

#endif // EGEXX_H_