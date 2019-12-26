#include <algorithm>
#include <random>

#include <SDL2/SDL.h>

#include "pico8.h"
#include "renderer.h"


uint32 get_surface_pixel(SDL_Surface* surface, int x, int y)
{
    if (surface->pixels == nullptr)
    {
        return 0;
    }

    if (x < 0 || y < 0 || x >= surface->w || y >= surface->h)
    {
        return 0;
    }

    int bpp = surface->format->BytesPerPixel;
    uint8* pixel = reinterpret_cast<uint8*>(surface->pixels) + y * surface->pitch + x * bpp;

    switch (bpp)
    {
    case 1: return *pixel;
    case 2: return static_cast<uint16>(*pixel);
    case 3: return pixel[0] | (pixel[1] << 8) | (pixel[2] << 16);
    case 4: return static_cast<uint32>(*pixel);
    default: return 0;
    }
}

void set_surface_pixel(SDL_Surface* surface, int x, int y, uint32 color)
{
    if (surface->pixels == nullptr)
    {
        return;
    }

    int bpp = surface->format->BytesPerPixel;
    uint8* pixel = reinterpret_cast<uint8*>(surface->pixels) + y * surface->pitch + x * bpp;

    switch (bpp)
    {
    case 1: *pixel = color; break;
    case 2: *reinterpret_cast<uint16*>(pixel) = color; break;
    case 3:
        pixel[0] = color;
        pixel[1] = (color >> 8);
        pixel[2] = (color >> 16);
        break;
    case 4: *reinterpret_cast<uint32*>(pixel) = color; break;
    default: return;
    }
}

namespace pico8
{
    struct
    {
        SDL_Surface* screen = nullptr;
        uint32* pixels = nullptr;
        int width = 0;
        int height = 0;
        float32 time = 0;
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

    static std::random_device rd;
    static std::mt19937 rgen(rd());
    static std::uniform_real_distribution<float32> rdist(0.f, 1.f);

    uint32 get_raw_color(uint8 picoColor)
    {
        int index = (picoColor & 0xF);
        return k_rawColors[index];
    }

    uint8 get_pico_color(uint32 rawColor)
    {
        for (int i = 0; i < 16; ++i)
        {
            if (k_rawColors[i] == rawColor)
            {
                return i;
            }
        }

        return 0;
    }

    inline bool valid_screen_coord(int x, int y)
    {
        return x >= 0 && y >= 0 && x < g_pico8.width && y < g_pico8.height;
    }

    // assumes all input is valid so it's faster
    uint32* get_screen_addr(int x, int y)
    {
        return g_pico8.pixels + y * g_pico8.width + x;
    }

    uint32 get_screen_raw_pixel(int x, int y)
    {
        return *get_screen_addr(x, y);
    }

    void set_screen_raw_pixel(int x, int y, uint32 rawColor)
    {
        *get_screen_addr(x, y) = rawColor;
    }

    void init()
    {
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

        srand(rgen.default_seed);
    }

    void shutdown()
    {
        SDL_FreeSurface(g_pico8.screen);

        g_pico8.width = 0;
        g_pico8.height = 0;
        g_pico8.screen = nullptr;
        g_pico8.pixels = nullptr;
    }

    void update(float32 dt)
    {
        g_pico8.time += dt;
    }

    void flip()
    {
        SDL_LockSurface(g_pico8.screen);
        tdjx::render::set_texture_data(g_pico8.pixels, g_pico8.width, g_pico8.height);
        SDL_UnlockSurface(g_pico8.screen);
    }

    void srand(uint32 seed)
    {
        rgen.seed(seed);
    }

    float32 rnd(float32 r)
    {
        return rdist(rgen) * r;
    }

    uint8 pget(int x, int y)
    {
        return get_pico_color(get_surface_pixel(g_pico8.screen, x, y));
    }

    float32 time()
    {
        return g_pico8.time;
    }

    void cls(uint8 c)
    {
        SDL_FillRect(g_pico8.screen, nullptr, get_raw_color(c));
    }

    void pset(int x, int y, uint8 c)
    {
        set_screen_raw_pixel(x, y, get_raw_color(c));
    }

    void hline(int y, int left, int right, uint8 c)
    {
        uint32* leftAddr = get_screen_addr(left, y);
        uint32* rightAddr = get_screen_addr(right, y) + 1;

        std::fill(leftAddr, rightAddr, get_raw_color(c));
    }

    void vline(int x, int top, int bottom, uint8 c)
    {
        uint32 color = get_raw_color(c);
        for (int y = top; y < bottom; ++y)
        {
            set_screen_raw_pixel(x, y, color);
        }
    }

    void line(int x0, int y0, int x1, int y1, uint8 c)
    {
        int dx = std::abs(x1 - x0);
        int dy = std::abs(y1 - y0);

        
        if (dy == 0)
        {
            if (x1 < x0) std::swap(x0, x1);
            hline(y0, x0, x1, c);
            return;
        }
        else if (dx == 0)
        {
            if (y1 < y0) std::swap(y0, y1);
            vline(x0, y0, y1, c);
            return;
        }

        int sx = (x0 < x1) ? 1 : -1;
        int sy = (y0 < y1) ? 1 : -1;
        int err = (dx > dy ? dx : -dy) / 2, e2;

        uint32 color = get_raw_color(c);

        while (true)
        {
            set_screen_raw_pixel(x0, y0, color);
            if (x0 == x1 && y0 == y1) break;
            e2 = err;
            if (e2 > -dx) { err -= dy; x0 += sx; }
            if (e2 < dy) { err += dx; y0 += sy; }
        }
    }

    //void rect(int x, int y, int w, int h, uint8 c);
    //void rectfill(int x, int y, int w, int h, uint8 c);
    //void circ(int x, int y, int r, uint8 c);
    //void circfill(int x, int y, int r, uint8 c);
}