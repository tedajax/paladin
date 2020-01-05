#include "tdjx_gfx.h"

#include <SDL2/SDL.h>
#include <cstdio>
#include <cmath>
#include <algorithm>

#include <stb/stb_image.h>

namespace tdjx
{
    namespace gfx
    {
        struct
        {
            uint8* pixels = nullptr;
            int width = 0;
            int height = 0;
            int size = 0;
            Rect clipArea = Rect{ 0, 0, 0, 0 };
            int paletteSize = 0;
            int paletteMask = 0;
            int paletteScalar = 1;
        } g_gfx;

        uint8* pixel_xy(int x, int y)
        {
            return &g_gfx.pixels[y * g_gfx.width + x];
        }

        bool gfx_clip_rect(Rect& r)
        {
            return rect::clip_rect(g_gfx.clipArea, r);
        }

        bool gfx_clip_line(int& x0, int& y0, int& x1, int& y1)
        {
            return rect::clip_line(g_gfx.clipArea, x0, y0, x1, y1);
        }

        bool gfx_point_visible(int x0, int y0)
        {
            return rect::contains_point(g_gfx.clipArea, x0, y0);
        }

        void scanline(int y, int x0, int x1, int color)
        {
            std::fill(pixel_xy(x0, y), pixel_xy(x1, y) + 1, color);
        }

        void init(int width, int height, int colors)
        {
            g_gfx.width = width;
            g_gfx.height = height;
            g_gfx.clipArea = { 0, 0, g_gfx.width - 1, g_gfx.height - 1 };
            g_gfx.size = g_gfx.width * g_gfx.height;
            g_gfx.paletteSize = colors;
            g_gfx.paletteMask = (1 << colors) - 1;
            g_gfx.paletteScalar = 256 / g_gfx.paletteSize;

            g_gfx.pixels = new uint8[g_gfx.size];
        }

        void shutdown()
        {
            delete g_gfx.pixels;
        }

        void mask_color(int& color)
        {
            color = (color & g_gfx.paletteMask) * g_gfx.paletteScalar;
        }

        void clear(int color)
        {
            mask_color(color);

            for (int y = g_gfx.clipArea.y0; y <= g_gfx.clipArea.y1; ++y)
            {
                scanline(y, g_gfx.clipArea.x0, g_gfx.clipArea.x1, color);
            }
        }

        void point(int x, int y, int color)
        {
            if (rect::contains_point(g_gfx.clipArea, x, y))
            {
                color &= g_gfx.paletteMask;
                *pixel_xy(x, y) = color;
            }
        }

        void line(int x0, int y0, int x1, int y1, int color)
        {
            if (!rect::clip_line(g_gfx.clipArea, x0, y0, x1, y1))
            {
                return;
            }

            int dx = std::abs(x1 - x0);
            int dy = std::abs(y1 - y0);

            mask_color(color);

            if (dy == 0)
            {
                if (x1 < x0)
                {
                    std::swap(x0, x1);
                }
                scanline(y0, x0, x1, color);
                return;
            }
            else if (dx == 0)
            {
                for (int y = y0; y <= y1; ++y)
                {
                    *pixel_xy(x0, y) = color;
                }
                return;
            }

            int sx = (x0 < x1) ? 1 : -1;
            int sy = (y0 < y1) ? 1 : -1;
            int err = (dx > dy ? dx : -dy) / 2, e2;

            while (true)
            {
                *pixel_xy(x0, y0) = color;
                if (x0 == x1 && y0 == y1) break;
                e2 = err;
                if (e2 > -dx) { err -= dy; x0 += sx; }
                if (e2 < dy) { err += dx; y0 += sy; }
            }
        }

        void circle(int x0, int y0, int radius, int color)
        {
            Rect bounds = Rect{ x0 - radius, y0 - radius, x0 + radius, y0 + radius };
            if (!rect::clip_rect(g_gfx.clipArea, bounds))
            {
                return;
            }

            mask_color(color);

            auto putpixel = [](int x, int y, uint8 c)
            {
                if (gfx_point_visible(x, y))
                {
                    *pixel_xy(x, y) = c;
                }
            };

            {
                int x = radius;
                int y = 0;
                int err = 0;

                while (x >= y)
                {
                    putpixel(x0 + x, y0 + y, color);
                    putpixel(x0 + y, y0 + x, color);
                    putpixel(x0 + x, y0 - y, color);
                    putpixel(x0 + y, y0 - x, color);
                    putpixel(x0 - x, y0 + y, color);
                    putpixel(x0 - y, y0 + x, color);
                    putpixel(x0 - x, y0 - y, color);
                    putpixel(x0 - y, y0 - x, color);

                    if (err <= 0)
                    {
                        ++y;
                        err += 2 * y + 1;
                    }

                    if (err > 0)
                    {
                        --x;
                        err -= 2 * x + 1;
                    }
                }
            }
        }

        void circle_fill(int x0, int y0, int radius, int color)
        {
            Rect bounds = Rect{ x0 - radius, y0 - radius, x0 + radius, y0 + radius };
            if (!rect::clip_rect(g_gfx.clipArea, bounds))
            {
                return;
            }

            mask_color(color);

            auto clipped_scanline = [=](int y, int x0, int x1, int color)
            {
                if (y >= g_gfx.clipArea.y0 && y <= g_gfx.clipArea.y1)
                {
                    scanline(y, std::max(g_gfx.clipArea.x0, x0), std::min(g_gfx.clipArea.x1, x1), color);
                }
            };

            {
                int x = radius;
                int y = 0;
                int err = 0;

                while (x >= y)
                {
                    clipped_scanline(y0 - y, x0 - x, x0 + x, color);
                    clipped_scanline(y0 - x, x0 - y, x0 + y, color);
                    clipped_scanline(y0 + y, x0 - x, x0 + x, color);
                    clipped_scanline(y0 + x, x0 - y, x0 + y, color);

                    if (err <= 0)
                    {
                        ++y;
                        err += 2 * y + 1;
                    }

                    if (err > 0)
                    {
                        --x;
                        err -= 2 * x + 1;
                    }
                }
            }
        }

        void rectangle(Rect r, int color)
        {
            if (!rect::clip_rect(g_gfx.clipArea, r))
            {
                return;
            }

            line(r.x0, r.y0, r.x1, r.y0, color);
            line(r.x0, r.y1, r.x1, r.y1, color);
            line(r.x0, r.y0, r.x0, r.y1, color);
            line(r.x1, r.y0, r.x1, r.y1, color);
        }

        void rectangle(int x0, int y0, int x1, int y1, int color)
        {
            rectangle(Rect{ x0, y0, x1, y1 }, color);
        }

        void rectangle_fill(Rect r, int color)
        {
            if (!rect::clip_rect(g_gfx.clipArea, r))
            {
                return;
            }

            mask_color(color);

            for (int y = r.y0; y <= r.y1; ++y)
            {
                scanline(y, r.x0, r.x1, color);
            }
        }

        void rectangle_fill(int x0, int y0, int x1, int y1, int color)
        {
            rectangle_fill(Rect{ x0, y0, x1, y1 }, color);
        }

        void* get_context()
        {
            return &g_gfx;
        }

        uint8* get_pixels()
        {
            return g_gfx.pixels;
        }

        namespace rect
        {
            inline int width(const Rect& self)
            {
                return std::abs(self.x0 - self.x1);
            }

            inline int height(const Rect& self)
            {
                return std::abs(self.y0 - self.y1);
            }

            inline bool contains_point(const Rect& self, int x, int y)
            {
                return x >= self.x0 && y >= self.y0 && x <= self.x1 && y <= self.y1;
            }

            inline bool intersect(const Rect& a, const Rect& b)
            {
                return a.x0 <= b.x1 &&
                    a.x1 >= b.x0 &&
                    a.y0 <= b.y1 &&
                    a.y1 >= b.y0;
            }

            bool clip_rect(const Rect& source, Rect& dest)
            {
                dest.x0 = std::max(dest.x0, source.x0);
                dest.y0 = std::max(dest.y0, source.y0);
                dest.x1 = std::min(dest.x1, source.x1);
                dest.y1 = std::min(dest.y1, source.y1);

                if (dest.x0 > dest.x1 || dest.y0 > dest.y1)
                {
                    return false;
                }

                return true;
            }

            bool clip_line(const Rect& r, int& x0, int& y0, int& x1, int& y1)
            {
                struct point { int x, y; };

                const int xmin = r.x0;
                const int xmax = r.x1;
                const int ymin = r.y0;
                const int ymax = r.y1;

                enum side
                {
                    k_none = 0,
                    k_left = 1, k_right = 2, k_top = 4, k_bottom = 8
                };

                auto calcRegionCode = [=](const point& p)
                {
                    return static_cast<int>(
                        ((p.x < xmin) ? k_left : k_none) +
                        ((p.x > xmax) ? k_right : k_none) +
                        ((p.y < ymin) ? k_top : k_none) +
                        ((p.y > ymax) ? k_bottom : k_none));
                };

                point a = { x0, y0 };
                point b = { x1, y1 };

                while (true)
                {
                    int code1 = calcRegionCode(a);
                    int code2 = calcRegionCode(b);

                    if (code1 == 0 && code2 == 0)
                    {
                        // completely inside
                        goto _is_visible;
                    }
                    else
                    {
                        if ((code1 & code2) != 0)
                        {
                            // completely outside
                            return false;
                        }
                        else
                        {
                            // pick a point outside of the rectangle
                            point* outside = &a;
                            int outCode = code1;
                            if (code1 == 0)
                            {
                                outside = &b;
                                outCode = code2;
                            }

                            if (outCode & k_top)
                            {
                                outside->x = a.x + (b.x - a.x) * (ymin - a.y) / (b.y - a.y);
                                outside->y = ymin;
                            }
                            else if (outCode & k_bottom)
                            {
                                outside->x = a.x + (b.x - a.x) * (ymax - a.y) / (b.y - a.y);
                                outside->y = ymax;
                            }
                            else if (outCode & k_right)
                            {
                                outside->y = a.y + (b.y - a.y) * (xmax - a.x) / (b.x - a.x);
                                outside->x = xmax;
                            }
                            else if (outCode & k_left)
                            {
                                outside->y = a.y + (b.y - a.y) * (xmin - a.x) / (b.x - a.x);
                                outside->x = xmin;
                            }
                        }
                    }
                }

            _is_visible:
                {
                    x0 = a.x;
                    y0 = a.y;
                    x1 = b.x;
                    y1 = b.y;
                    return true;
                }
            }
        }
    }
}