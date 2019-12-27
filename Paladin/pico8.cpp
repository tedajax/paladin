#include <algorithm>
#include <random>
#include <unordered_map>

#include <SDL2/SDL.h>

#include "pico8.h"
#include "renderer.h"

void print_fixed(const pico8::fixed16& f)
{
    printf("%0.5f\n", static_cast<float32>(f));
}

uint32* surface_pixel_addr(SDL_Surface* surface, uint64 x, uint64 y)
{
    return reinterpret_cast<uint32*>(surface->pixels) + y * surface->w + x;
}

uint32 get_surface_pixel(SDL_Surface* surface, uint64 x, uint64 y)
{
    return *surface_pixel_addr(surface, x, y);
}

void set_surface_pixel(SDL_Surface* surface, uint64 x, uint64 y, uint32 color)
{
    *surface_pixel_addr(surface, x, y) = color;
}

namespace pico8
{
    namespace literals
    {
        fixed16 operator"" _fx16(unsigned long long v)
        {
            return fixed16(static_cast<int>(v));
        }

        fixed16 operator"" _fx16(long double v)
        {
            return fixed16(static_cast<float32>(v));
        }
    }

    using namespace literals;

    struct
    {
        SDL_Surface* screen = nullptr;
        uint32* pixels = nullptr;
        int width = 0;
        int height = 0;
        fixed16 time = 0;
        byte memory[0x8000];
        fixed16 sleepTimer = 0;
        struct
        {
            pico8_callback initFn;
            pico8_callback updateFn;
            pico8_callback drawFn;
        } callbacks;
    } g_pico8;

    const uint32 k_screenCoordMask = 0x7F;

    const uint32 k_rawColors[16] = {
        0xFF000000,
        0xFF1D2B52,
        0XFF7E2553,
        0XFF008751,
        0xFFAB5236,
        0xFF5F5745,
        0xFFC2C3C7,
        0xFFFFF1E8,
        0xFFFF004D,
        0xFFFFA300,
        0xFFFFEC27,
        0xFF00E436,
        0xFF29ADFF,
        0xFF83769C,
        0xFFFF77A8,
        0xFFFFCCAA,
    };

    static std::unordered_map<uint32, uint8> s_picoColorsMap;

    static std::random_device rd;
    static std::mt19937 rgen(rd());
    static std::uniform_real_distribution<float32> rdist(0.f, 1.f);

    struct aarect { fixed16 x0, y0, x1, y1; };

    const aarect k_defaultClipRect = { 0, 0, 127, 127 };
    static aarect s_clipRect = k_defaultClipRect;

    const uint8 k_maskBoth = 0xFF;
    const uint8 k_maskLeft = 0x0F;
    const uint8 k_maskRight = 0xF0;

    constexpr inline uint8 pixel_mask(int x)
    {
        return ((x & 1) == 0) ? k_maskLeft : k_maskRight;
    }

    aarect make_aarect(const fixed16& x0, const fixed16& y0, const fixed16& x1, const fixed16& y1)
    {
        return aarect{
            min(x0, x1), min(y0, y1), max(x0, x1), max(y0, y1)
        };
    }

    bool aarect_intersect(const aarect& a, const aarect& b)
    {
        return a.x0 <= b.x1 &&
            a.x1 >= b.x0 &&
            a.y0 <= b.y1 &&
            a.y1 >= b.y0;
    }

    bool clip_point(int x, int y)
    {
        return x >= static_cast<int>(s_clipRect.x0) &&
            x <= static_cast<int>(s_clipRect.x1) &&
            y >= static_cast<int>(s_clipRect.y0) &&
            y <= static_cast<int>(s_clipRect.y1);
    }

    bool clip_rect(const aarect& source, aarect& dest)
    {
        if (!aarect_intersect(dest, s_clipRect))
        {
            return false;
        }
        else
        {
            dest.x0 = max(dest.x0, source.x0);
            dest.y0 = max(dest.y0, source.y0);
            dest.x1 = min(dest.x1, source.x1);
            dest.y1 = min(dest.y1, source.y1);
            return true;
        }

    }

    bool clip_rect(aarect& r)
    {
        return clip_rect(s_clipRect, r);
    }

    void clip()
    {
        s_clipRect = k_defaultClipRect;
    }

    void clip(fixed16 x, fixed16 y, fixed16 w, fixed16 h)
    {
        aarect r = make_aarect(x, y, x + w - 1_fx16, y + h - 1_fx16);
        if (clip_rect(k_defaultClipRect, r))
        {
            s_clipRect = r;
        }
    }

    uint32 get_raw_color(fixed16 picoColor)
    {
        int index = (static_cast<int>(picoColor) & 0xF);
        return k_rawColors[index];
    }

    uint32 get_raw_color(uint8 picoColor)
    {
        return k_rawColors[picoColor & 0xF];
    }

    fixed16 get_pico_color(uint32 rawColor)
    {
        return s_picoColorsMap[rawColor];
    }

    inline bool valid_screen_coord(int x, int y)
    {
        return x >= 0 && y >= 0 && x < g_pico8.width && y < g_pico8.height;
    }

    // assumes all input is valid so it's faster
    uint8* get_pixel_addr(int x, int y)
    {
        return &g_pico8.memory[k_offsetScreenData + static_cast<uint64>(y) * 64ull + static_cast<uint64>(x / 2)];
    }

    uint8 _pget(int x, int y)
    {
        uint8 data = *get_pixel_addr(x, y);
        if ((x & 1) == 0)
        {
            return data & 0xF;
        }
        else
        {
            return data >> 4;
        }
    }

    void _pset(int x, int y, uint8 mask, uint8 picoColor)
    {
        picoColor |= (picoColor << 4);
        uint8* data = get_pixel_addr(x, y);
        *data = (*data & ~mask) | (picoColor & mask);
    }

    void srand(uint32 seed);

    void system_init(pico8_callback initFn, pico8_callback updateFn, pico8_callback drawFn)
    {
        g_pico8.callbacks = {
            initFn, updateFn, drawFn
        };

        g_pico8.width = 128;
        g_pico8.height = 128;
        g_pico8.screen = SDL_CreateRGBSurface(0, g_pico8.width, g_pico8.height,
            32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);

        if (g_pico8.screen == nullptr)
        {
            printf("SDL Error: %s\n", SDL_GetError());
            return;
        }

        g_pico8.pixels = reinterpret_cast<uint32*>(g_pico8.screen->pixels);

        std::memset(g_pico8.memory, 0, sizeof(g_pico8.memory));

        for (uint8 i = 0; i < 16; ++i)
        {
            s_picoColorsMap.insert_or_assign(k_rawColors[i], i);
        }

        srand(rgen.default_seed);

        if (g_pico8.callbacks.initFn)
        {
            g_pico8.callbacks.initFn();
        }
    }

    void system_shutdown()
    {
        SDL_FreeSurface(g_pico8.screen);

        g_pico8.width = 0;
        g_pico8.height = 0;
        g_pico8.screen = nullptr;
        g_pico8.pixels = nullptr;
    }

    void system_update(float32 dt)
    {
        if (g_pico8.sleepTimer > 0_fx16)
        {
            g_pico8.sleepTimer -= dt;
            return;
        }

        g_pico8.time += static_cast<fixed16>(dt);

        if (g_pico8.callbacks.updateFn)
        {
            g_pico8.callbacks.updateFn();
        }
    }

    void system_draw()
    {
        if (g_pico8.sleepTimer > 0_fx16)
        {
            return;
        }

        if (g_pico8.callbacks.drawFn)
        {
            g_pico8.callbacks.drawFn();
        }

        flip();
    }

    void sleep(fixed16 seconds)
    {
        g_pico8.sleepTimer += seconds;
    }

    void flip()
    {
        SDL_LockSurface(g_pico8.screen);

        for (uint64 y = 0; y < g_pico8.height; ++y)
        {
            for (uint64 x = 0; x < 64; ++x)
            {
                byte pixel = g_pico8.memory[k_offsetScreenData + y * 64ull + x];
                
                uint8 left = pixel & 0xF;
                uint8 right = pixel >> 4;

                uint32 leftColor = get_raw_color(left);
                uint32 rightColor = get_raw_color(right);

                set_surface_pixel(g_pico8.screen, x * 2 + 0, y, leftColor);
                set_surface_pixel(g_pico8.screen, x * 2 + 1, y, rightColor);
            }
        }

        tdjx::render::set_texture_data(g_pico8.pixels, g_pico8.width, g_pico8.height);
        SDL_UnlockSurface(g_pico8.screen);
    }

    void srand(uint32 seed)
    {
        rgen.seed(seed);
    }

    void srand(fixed16 seed)
    {
        srand(seed.raw());
    }

    fixed16 rnd(fixed16 r)
    {
        return static_cast<fixed16>(rdist(rgen)) * r;
    }

    fixed16 pget(fixed16 x, fixed16 y)
    {
        return _pget(x, y);
    }

    fixed16 time()
    {
        return g_pico8.time;
    }

    void cls(uint8 c)
    {
        std::fill(&g_pico8.memory[k_offsetScreenData], &g_pico8.memory[k_offsetScreenData] + k_screenSize, c);
    }

    void pset(fixed16 x, fixed16 y, fixed16 c)
    {
        _pset(x, y, pixel_mask(static_cast<int>(x)), static_cast<uint8>(c));
    }

    void _hline(int y, int left, int right, fixed16 c)
    {
        uint8* leftAddr = get_pixel_addr(left, y);
        uint8* rightAddr = get_pixel_addr(right, y);
        
        if ((left & 1) != 0)
        {
            _pset(left, y, k_maskRight, c);
            leftAddr += 1;
        }
        if ((right & 1) == 0)
        {
            _pset(right, y, k_maskLeft, c);
            rightAddr -= 1;
        }

        uint8 cc = static_cast<uint8>(c);
        cc = (cc << 4) | cc;

        std::fill(leftAddr, rightAddr + 1, cc);
    }

    void _vline(int x, int top, int bottom, fixed16 c)
    {
        uint32 color = static_cast<uint8>(c);
        uint8 mask = pixel_mask(x);
        for (int y = top; y < bottom; ++y)
        {
            _pset(x, y, mask, color);
        }
    }

    bool clip_line(const aarect& r, int& x0, int& y0, int& x1, int& y1)
    {
        struct point { int x, y; };

        const int xmin = static_cast<int>(r.x0);
        const int xmax = static_cast<int>(r.x1);
        const int ymin = static_cast<int>(r.y0);
        const int ymax = static_cast<int>(r.y1);

        enum side
        {
            k_none = 0,
            k_left = 1, k_right = 2, k_top = 4, k_bottom = 8
        };

        auto calcRegionCode = [&](const point& p) -> byte
        {
            return static_cast<byte>(
                ((p.x < xmin) ? k_left : k_none) +
                ((p.x > xmax) ? k_right : k_none) +
                ((p.y < ymin) ? k_top : k_none) +
                ((p.y > ymax) ? k_bottom : k_none));
        };

        point a = { x0, y0 };
        point b = { x1, y1 };

        while (true)
        {
            byte code1 = calcRegionCode(a);
            byte code2 = calcRegionCode(b);

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
                    byte outCode = code1;
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

    void hline(fixed16 yfx, fixed16 x0fx, fixed16 x1fx, fixed16 c)
    {
        int x0 = static_cast<int>(x0fx);
        int y0 = static_cast<int>(yfx);
        int x1 = static_cast<int>(x1fx);
        int y1 = static_cast<int>(yfx);

        if (!clip_line(s_clipRect, x0, y0, x1, y1))
        {
            return;
        }

        _hline(y0, x0, x1, c);
    }

    void vline(fixed16 xfx, fixed16 y0fx, fixed16 y1fx, fixed16 c)
    {
        int x0 = static_cast<int>(xfx);
        int y0 = static_cast<int>(y0fx);
        int x1 = static_cast<int>(xfx);
        int y1 = static_cast<int>(y1fx);

        if (!clip_line(s_clipRect, x0, y0, x1, y1))
        {
            return;
        }

        _vline(x0, y0, y1, c);
    }

    void line(fixed16 x0fx, fixed16 y0fx, fixed16 x1fx, fixed16 y1fx, fixed16 c)
    {
        int x0 = static_cast<int>(x0fx);
        int y0 = static_cast<int>(y0fx);
        int x1 = static_cast<int>(x1fx);
        int y1 = static_cast<int>(y1fx);

        if (!clip_line(s_clipRect, x0, y0, x1, y1))
        {
            return;
        }

        int dx = std::abs(x1 - x0);
        int dy = std::abs(y1 - y0);
        
        if (dy == 0)
        {
            if (x1 < x0)
            {
                std::swap(x0, x1);
            }
            _hline(y0, x0, x1, c);
            return;
        }
        else if (dx == 0)
        {
            if (y1 < y0)
            {
                std::swap(y0, y1);
            }
            _vline(x0, y0, y1, c);
            return;
        }

        int sx = (x0 < x1) ? 1 : -1;
        int sy = (y0 < y1) ? 1 : -1;
        int err = (dx > dy ? dx : -dy) / 2, e2;

        while (true)
        {
            _pset(x0, y0, pixel_mask(x0), c);
            if (x0 == x1 && y0 == y1) break;
            e2 = err;
            if (e2 > -dx) { err -= dy; x0 += sx; }
            if (e2 < dy) { err += dx; y0 += sy; }
        }
    }

    void rect(fixed16 x, fixed16 y, fixed16 w, fixed16 h, fixed16 c)
    {
        aarect r = { x, y, x + w - 1_fx16, y + h - 1_fx16 };
        
        if (!clip_rect(r))
        {
            return;
        }

        line(r.x0, r.y0, r.x1, r.y0, c);
        line(r.x0, r.y1, r.x1, r.y1, c);

        line(r.x0, r.y0, r.x0, r.y1, c);
        line(r.x1, r.y0, r.x1, r.y1, c);
    }

    void rectfill(fixed16 x, fixed16 y, fixed16 w, fixed16 h, fixed16 c)
    {
        aarect r = { x, y, x + w - 1_fx16, y + h - 1_fx16 };
        
        if (!clip_rect(r))
        {
            return;
        }

        int left = static_cast<int>(r.x0);
        int right = static_cast<int>(r.x1);

        if (right - left >= 0)
        {
            for (int yy = static_cast<int>(r.y0); yy < static_cast<int>(r.y1); ++yy)
            {
                _hline(yy, left, right, c);
            }
        }
    }

    void circ(fixed16 x, fixed16 y, fixed16 r, fixed16 c)
    {
        aarect bounds = make_aarect(x - r, y - r, x + r, y + r);
        if (!clip_rect(bounds))
        {
            return;
        }

        int x0 = static_cast<int>(x);
        int y0 = static_cast<int>(y);

        auto putpixel = [](int x, int y, uint8 c) {
            if (clip_point(x, y)) {
                _pset(x, y, pixel_mask(x), c);
            }
        };

        auto plot8 = [x0, y0, putpixel](int dx, int dy, uint8 c)
        {
            putpixel(x0 + dx, y0 + dy, c);
            putpixel(x0 + dy, y0 + dx, c);
            putpixel(x0 + dx, y0 - dy, c);
            putpixel(x0 + dy, y0 - dx, c);
            putpixel(x0 - dx, y0 + dy, c);
            putpixel(x0 - dy, y0 + dx, c);
            putpixel(x0 - dx, y0 - dy, c);
            putpixel(x0 - dy, y0 - dx, c);
        };

        {
            int x = static_cast<int>(r);
            int y = 0;
            int err = 0;

            while (x >= y)
            {
                plot8(x, y, c);

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

    void circfill(fixed16 x, fixed16 y, fixed16 r, fixed16 c)
    {
        aarect bounds = make_aarect(x - r, y - r, x + r, y + r);
        if (!clip_rect(bounds))
        {
            return;
        }

        int x0 = static_cast<int>(x);
        int y0 = static_cast<int>(y);

        auto scanline4 = [x0, y0](int dx, int dy, uint8 c)
        {
            hline(y0 - dy, x0 - dx, x0 + dx, c);
            hline(y0 - dx, x0 - dy, x0 + dy, c);
            hline(y0 + dy, x0 - dx, x0 + dx, c);
            hline(y0 + dx, x0 - dy, x0 + dy, c);
        };

        {
            int x = static_cast<int>(r);
            int y = 0;
            int err = 0;

            while (x >= y)
            {
                scanline4(x, y, c);

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

    //void rect(int x, int y, int w, int h, uint8 c);
    //void rectfill(int x, int y, int w, int h, uint8 c);
    //void circ(int x, int y, int r, uint8 c);
    //void circfill(int x, int y, int r, uint8 c);

}