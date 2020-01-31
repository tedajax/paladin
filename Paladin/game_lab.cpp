#include "game_lab.h"

#include "tdjx_gfx.h"
#include "perlin.h"

#include <SDL2/SDL.h>

using tdjx::math::Rect;

perlin_gen gen(0);
int width, height, paletteSize;
float32 aspectRatio;
tdjx::gfx::Canvas mandelbrot_canvas;

std::optional<Rect<int>> g_select;

LabGame::LabGame(SDL_Window* window)
{
    gen = perlin_gen(SDL_GetTicks());

    tdjx::gfx::init_with_window(320, 160, window);
    //tdjx::gfx::load_palette("assets/palettes/win16_16.png");
    tdjx::gfx::load_palette("assets/palettes/palette_64_00.png");
    //tdjx::gfx::load_palette("assets/palettes/arne32.png");

    tdjx::gfx::query_screen_dimensions(width, height);
    tdjx::gfx::query_palette_size(paletteSize);
    aspectRatio = static_cast<float32>(width) / height;

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

void render_perlin(const tdjx::GameTime& time, float32 scale, float32 colorScale, int baseColor);
bool try_mandelbrot_iterations(float32 real, float32 imaginary, int max, int& iterations);

void render_mandelbrot(const Rect<float32>& view);

Rect<float32> g_lastView = {};
Rect<float32> g_view = { -2, -2, 2, 2 };

void LabGame::render()
{
    tdjx::gfx::clear(0);
    context.scale = (tdjx::math::sin(time.elapsed * 5.0f) + 1) * 16.0f + 1.0f;
    render_perlin(time, context.scale, context.colorScalar, context.baseColor);
}

void LabGame::on_mouse_down(int x, int y, int button)
{
    if (button == SDL_BUTTON_LEFT)
    {
        g_select = tdjx::math::Rect<int>{
            x, y, x, y
        };
    }
}

void LabGame::on_mouse_up(int x, int y, int button)
{
    if (button == SDL_BUTTON_LEFT)
    {
        if (g_select.has_value())
        {
            g_select->x1 = x;
            g_select->y1 = y;

            Rect<int> selection = *g_select;
            //tdjx::math::rect::constrain_to_aspect_ratio(selection, 2.0f);

            // convert selection box screen coordinates to normalized [-1..1] space
            float32 hw = width / 2.0f, hh = height / 2.0f;

            Rect<float32> normRect = {
                selection.x0 / hw - 1,
                selection.y0 / hh - 1,
                selection.x1 / hw - 1,
                selection.y1 / hh - 1,
            };

            printf("%0.2f %0.2f %0.2f %0.2f\n", normRect.x0, normRect.y0, normRect.x1, normRect.y1);

            g_select.reset();
        }
    }
}

void LabGame::on_mouse_move(int x, int y, int dx, int dy)
{
    if (g_select.has_value())
    {
        g_select->x1 = x;
        g_select->y1 = y;
    }
}

void render_perlin(const tdjx::GameTime& time, float32 scale, float32 colorScale, int baseColor)
{
    float32 ar = static_cast<float32>(width) / height;

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            float32 nx = static_cast<float32>(x) / width * scale * ar - scale;
            float32 ny = static_cast<float32>(y) / height * scale - scale / 2 ;

            float32 z = time.elapsed / 8.0f;
            float32 v = gen.noise(nx, ny, z);

            int iv = static_cast<int>(v * colorScale);

            tdjx::gfx::point(x, y, iv + baseColor);
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
                tdjx::gfx::point(x, y, iterations % 12 + 24);
            }
        }
    }
}