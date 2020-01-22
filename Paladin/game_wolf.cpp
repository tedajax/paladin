#include "game_wolf.h"

#include <SDL2/SDL.h>
#include <algorithm>
#include "tdjx_gfx.h"
#include "renderer.h"

struct Room
{
    int w, h;
    const char* data;
};

Room testRoom = {
    8, 8,
    "********"
    "*      *"
    "*      *"
    "*      *"
    "*      *"
    "*      *"
    "*      *"
    "********"
};

bool cast_ray(const Room& room, float32 ox, float32 oy, float32 rot, float32& distOut);

void WolfGame::init(void* userData)
{

}

void WolfGame::update()
{
    WolfContext::Player& player = context.player;
    float32 dt = time.delta;

    const uint8* keys = SDL_GetKeyboardState(nullptr);

    float32 dx = std::cos(player.rot);
    float32 dy = std::sin(player.rot);


    if (keys[SDL_SCANCODE_UP])
    {
        player.x += dx * dt * 4;
        player.y += dy * dt * 4;
    }
    if (keys[SDL_SCANCODE_DOWN])
    {
        player.x -= dx * dt * 4;
        player.y -= dy * dt * 4;
    }

    if (keys[SDL_SCANCODE_LEFT])
    {
        player.rot -= M_PI * dt;
    }
    if (keys[SDL_SCANCODE_RIGHT])
    {
        player.rot += M_PI * dt;
    }
}

void WolfGame::render()
{
    WolfContext::Player& player = context.player;

    int width, height;
    tdjx::gfx::query_screen_dimensions(width, height);

    tdjx::gfx::clear(0);

    Room room = testRoom;

    const int size = 16;

    {
        for (int x = 0; x < width; ++x)
        {
            float32 dist;

            float32 r = player.rot + (static_cast<float32>(x) / width)* M_PI / 4.f;

            if (cast_ray(room, player.x, player.y, r, dist))
            {
                float32 yy = 8.0f / dist * height / 2;
                //tdjx::gfx::line(x, k_height / 2 - yy, x, k_height / 2 + yy, 12);
            }
        }
    }

    for (int y = 0; y < room.h; ++y)
    {
        for (int x = 0; x < room.w; ++x)
        {
            char cell = room.data[x + y * room.w];
            if (cell == '*')
            {
                int sx = x * size;
                int sy = y * size;
                tdjx::gfx::rectangle_fill(sx, sy, sx + size - 1, sy + size - 1, 21);
            }
        }
    }

    auto draw_ray = [](float32 ox, float32 oy, float32 r, float32 d, int color)
    {
        tdjx::gfx::line(
            static_cast<int>(ox * 16),
            static_cast<int>(oy * 16),
            static_cast<int>(ox * 16 + d * 16 * std::cos(r)),
            static_cast<int>(oy * 16 + d * 16 * std::sin(r)),
            color
        );
    };

    {
        float32 dist = 0.0f;
        bool hit = cast_ray(room, player.x, player.y, player.rot, dist);

        dist = (hit) ? dist : 8;

        int hitX = static_cast<int>(player.x * size + std::cos(player.rot) * dist);
        int hitY = static_cast<int>(player.y * size + std::sin(player.rot) * dist);

        tdjx::gfx::circle(hitX, hitY, 4, 28);

        draw_ray(player.x, player.y, player.rot, 8, 8);
        draw_ray(player.x, player.y, player.rot - M_PI / 4.f, 8, 8);
        draw_ray(player.x, player.y, player.rot + M_PI / 4.f, 8, 8);
    }

    tdjx::render::set_intensity(tdjx::gfx::get_pixels());
}

template <typename T> int sgn(T val)
{
    return (static_cast<T>(0) < val) - (val < static_cast<T>(0));
}


bool cast_ray(const Room& room, float32 ox, float32 oy, float32 rot, float32& distOut)
{
    float32 dx = std::cos(rot);
    float32 dy = std::sin(rot);

    int cellX = static_cast<int>(ox);
    int cellY = static_cast<int>(oy);

    int stepX, outX, x = cellX;
    int stepY, outY, y = cellY;

    if (x < 0 || x >= room.w || y < 0 || y >= room.h)
    {
        return false;
    }

    stepX = sgn(dx);
    stepY = sgn(dy);

    float32 cbX = static_cast<float32>(x + ((dx > 0) ? 1 : 0));
    float32 cbY = static_cast<float32>(y + ((dy > 0) ? 1 : 0));

    outX = (dx > 0) ? room.w : -1;
    outY = (dy > 0) ? room.h : -1;

    float32 rxr, ryr;
    float32 tMaxX, tMaxY;
    float32 tDeltaX, tDeltaY;

    if (dx != 0)
    {
        rxr = 1.0f / dx;
        tMaxX = cbX - ox * rxr;
        tDeltaX = room.w * stepX * rxr;
    }
    else
    {
        tMaxX = std::numeric_limits<float32>::infinity();
    }

    if (dy != 0)
    {
        ryr = 1.0f / dy;
        tMaxY = cbY - oy * rxr;
        tDeltaY = room.h * stepY * ryr;
    }
    else
    {
        tMaxY = std::numeric_limits<float32>::infinity();
    }

    while (true)
    {
        char cell = room.data[x + y * room.w];
        if (cell == '*')
        {
            break;
        }

        if (tMaxX < tMaxY)
        {
            x = x + stepX;
            if (x == outX)
            {
                return false;
            }
            tMaxX += tDeltaX;
        }
        else
        {
            y = y + stepY;
            if (y == outY)
            {
                return false;
            }
            tMaxY += tDeltaY;
        }
    }

    distOut = std::min(tMaxX, tMaxY);

    return true;
}