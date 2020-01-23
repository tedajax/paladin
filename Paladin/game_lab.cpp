#include "game_lab.h"

#include "tdjx_gfx.h"
#include "perlin.h"

perlin_gen gen(0);

LabGame::LabGame(SDL_Window* window)
{
    tdjx::gfx::init_with_window(640, 360, window);
    tdjx::gfx::load_palette("assets/palettes/vga13h.png");
    tdjx::gfx::load_palette("assets/palettes/palette_64_00.png");
}

LabGame::~LabGame()
{
    tdjx::gfx::shutdown();
}

void LabGame::update()
{

}

void LabGame::render()
{
    tdjx::gfx::clear(0);

    int width, height;
    tdjx::gfx::query_screen_dimensions(width, height);
    int paletteSize;
    tdjx::gfx::query_palette_size(paletteSize);

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

    tdjx::gfx::flip();
}
