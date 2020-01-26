#include "game_lab.h"

#include "tdjx_gfx.h"
#include "perlin.h"

#include <SDL2/SDL.h>

using tdjx::math::Rect;

perlin_gen gen(0);
int width, height, paletteSize;
tdjx::gfx::Canvas mandelbrot_canvas;

std::optional<Rect<int>> g_select;

LabGame::LabGame(SDL_Window* window)
{
    tdjx::gfx::init_with_window(1280, 640, window);
    //tdjx::gfx::load_palette("assets/palettes/vga13h.png");
    //tdjx::gfx::load_palette("assets/palettes/palette_64_00.png");

    tdjx::gfx::query_screen_dimensions(width, height);
    tdjx::gfx::query_palette_size(paletteSize);

    mandelbrot_canvas = tdjx::gfx::canvas::create_blank_from_screen();

    g_select.reset();
}

LabGame::~LabGame()
{
    tdjx::gfx::shutdown();
}

void LabGame::update()
{
    
}

void render_perlin(const tdjx::GameTime& time);
bool try_mandelbrot_iterations(float32 real, float32 imaginary, int max, int& iterations);

void render_mandelbrot(const Rect<float32>& view);

Rect<float32> g_lastView = {};
Rect<float32> g_view = { -2, -2, 2, 2 };

void LabGame::render()
{
    int width, height;
    tdjx::gfx::query_screen_dimensions(width, height);
    int paletteSize;
    tdjx::gfx::query_palette_size(paletteSize);

    tdjx::gfx::clear(0);
    render_perlin(time);
    //if (g_view != g_lastView)
    //{
    //    tdjx::gfx::set_canvas(mandelbrot_canvas);
    //    tdjx::gfx::clear(0);
    //    render_mandelbrot(g_view);
    //    g_lastView = g_view;
    //    tdjx::gfx::set_canvas();
    //}
    //tdjx::gfx::draw_canvas_to_screen(mandelbrot_canvas);

    //if (g_select.has_value())
    //{
    //    Rect<int> selection = *g_select;
    //    tdjx::math::rect::constrain_to_aspect_ratio(selection, 2.0f);
    //    tdjx::gfx::rectangle(selection, 8);
    //}

    //return;
    //const Rect<int> a = {
    //    width / 2 - 20, height / 2 - 10,
    //    width / 2 + 20 , height / 2 + 10
    //};

    //const Rect<int> b = {
    //    5, 5, width - 5, height - 5
    //};

    //const int k = 0;
    //for (int i = 0; i < k; ++i)
    //{
    //    Rect<int> r = tdjx::math::rect::lerp(a, b, (tdjx::math::sin((time.elapsed + static_cast<float32>(i) / k) * 90.0f) + 1.0f) / 2.0f);
    //    int color = 27 + i % 4;
    //    tdjx::gfx::rectangle(r, color);
    //    for (int i = 1; i < 0; ++i)
    //    {
    //        tdjx::gfx::rectangle(r.x0 + i, r.y0 + i, r.x1 - i, r.y1 - i, color);
    //    }
    //}
}

void LabGame::on_mouse_down(int x, int y, int button)
{
    if (button == SDL_BUTTON_LEFT)
    {
        g_select = tdjx::math::Rect<int>{
            x / 2, y / 2, x / 2, y / 2
        };
    }
}

void LabGame::on_mouse_up(int x, int y, int button)
{
    if (button == SDL_BUTTON_LEFT)
    {
        if (g_select.has_value())
        {
            Rect<int> selection = *g_select;
            tdjx::math::rect::constrain_to_aspect_ratio(selection, 2.0f);

            // convert selection box screen coordinates to normalized [-1..1] space
            float32 hw = width / 2.0f, hh = height / 2.0f;

            Rect<float32> normRect = {
                selection.x0 / hw - 1,
                selection.y0 / hh - 1,
                selection.x1 / hw - 1,
                selection.y1 / hh - 1,
            };

            printf("%0.2f %0.2f %0.2f %0.2f\n", normRect.x0, normRect.y0, normRect.x1, normRect.y1);

            g_view.x0 *= normRect.x0;
            g_view.y0 *= normRect.y0;
            g_view.x1 *= normRect.x1;
            g_view.y1 *= normRect.y1;

            g_select.reset();
        }
    }
}

void LabGame::on_mouse_move(int x, int y, int dx, int dy)
{
    if (g_select.has_value())
    {
        g_select->x1 = x / 2;
        g_select->y1 = y / 2;
    }
}

void render_perlin(const tdjx::GameTime& time)
{
    float32 ar = static_cast<float32>(width) / height;

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            float32 nx = static_cast<float32>(x) / width * 4 * ar;
            float32 ny = static_cast<float32>(y) / height * 4;

            float32 z = std::sin(time.elapsed) + std::cos(time.elapsed * 1.27f) + time.elapsed / 8.f;
            z = time.elapsed / 4.0f;
            float32 v = gen.noise(nx, ny, z);

            int iv = static_cast<int>(v * paletteSize / 1.5f);

            tdjx::gfx::point(x, y, iv);
        }
    }
}

bool try_mandelbrot_iterations(float32 real, float32 imaginary, int max, int& iterations)
{
    struct complex { float32 r, i; };
    complex c = { real, imaginary };

    auto magSqr = [](const complex& c)
    {
        return c.r * c.r + c.i * c.i;
    };

    for (int i = 0; i < max; ++i)
    {
        float32 tempReal = c.r;
        c.r = (c.r * c.r) - (c.i * c.i);
        c.i *= 2 * tempReal;
        c.r += real;
        c.i += imaginary;

        if (magSqr(c) >= 16)
        {
            iterations = i;
            return true;
        }
    }

    return false;
}

void render_mandelbrot(const Rect<float32>& view)
{
    float32 ar = static_cast<float32>(width) / height;

    float32 x0, y0, x1, y1;
    tdjx::math::rect::unpack(view, x0, y0, x1, y1);

    for (int y = 0; y < height; ++y)
    {
        float32 my = tdjx::math::lerp(y0, y1, static_cast<float32>(y) / height);
        for (int x = 0; x < width; ++x)
        {
            float32 mx = tdjx::math::lerp(x0, x1, static_cast<float32>(x) / width) * ar;

            int iterations;
            if (try_mandelbrot_iterations(mx, my, 200, iterations))
            {
                tdjx::gfx::point(x, y, iterations % 12 + 4);
            }
        }
    }
}