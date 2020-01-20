#include <vector>
#include <random>
#include <functional>
#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <cinttypes>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <Box2D/Box2D.h>

#include <effolkronium/random.hpp>

#include "algebra.h"
#include "renderer.h"

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

#include "tdjx_gfx.h"

#include "util.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#include <glm/glm.hpp>

using Random = effolkronium::random_static;

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

int main(int argc, char* argv[])
{
    Random::seed(1);

    SDL_Init(SDL_INIT_VIDEO);

    //const int kWindowWidth = 1440;
    //const int kWindowHeight = 1080;
    const int kWindowWidth = 640;
    const int kWindowHeight = 480;

    SDL_Window* window = SDL_CreateWindow("Paladin", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, kWindowWidth, kWindowHeight, SDL_WINDOW_OPENGL);

    //{
    //    int w, h, n;
    //    uint8* data = stbi_load("assets/sheet_00.png", &w, &h, &n, 4);
    //    printf("w: %d, h: %d, n: %d\n", w, h, n);
    //}
    
    const int k_width = 320;
    const int k_height = 240;

    tdjx::gfx::init_with_window(k_width, k_height, window);
    auto image = tdjx::gfx::load_image("assets/shooter.png");

    // ImGui initialization
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForOpenGL(window, tdjx::render::glContext());
    ImGui_ImplOpenGL3_Init(tdjx::render::glslVersion());

    int fps = 0;
    uint64 ticks = SDL_GetPerformanceCounter();
    uint64 lastTicks = ticks;
    uint64 frame_ns = 0;

    int framesInSecond = 0;
    float32 dt = 0.f;
    float32 time = 0.f;


    bool isRunning = true;
    bool shouldReloadShaders = false;

    auto app_quit = [&isRunning]() { isRunning = false; };
    auto app_reload_shaders = [&shouldReloadShaders]() { shouldReloadShaders = true; };

    const std::unordered_map<SDL_Scancode, std::function<void(void)>> kDebugCommands = {
        { SDL_SCANCODE_ESCAPE, app_quit },
        { SDL_SCANCODE_F2, app_reload_shaders },
        { SDL_SCANCODE_F3, tdjx::render::prev_mode },
        { SDL_SCANCODE_F4, tdjx::render::next_mode },
        { SDL_SCANCODE_P, []() { __debugbreak(); } },
    };

    const float32 size = 0.5f;
    glm::vec3 vertices[8] = {
        { size, size, size },
        { size, size, -size },
        { size, -size, size },
        { size, -size, -size },
        { -size, size, size },
        { -size, size, -size },
        { -size, -size, size },
        { -size, -size, -size }
    };

    const int kTriangles = 8;
    int indices[kTriangles * 3] = {
        0, 3, 2,
        0, 1, 3,
        4, 7, 6,
        4, 5, 7,
        0, 6, 2,
        0, 4, 6,
        1, 7, 3,
        1, 5, 7
    };

    struct Player
    {
        float32 x, y;
        float32 rot;
    };

    Player player = { 2, 4, 0 };

    while (isRunning)
    {
        if (shouldReloadShaders)
        {
            shouldReloadShaders = false;
            tdjx::render::reload_shaders();
        }

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            switch (event.type)
            {
            case SDL_KEYDOWN:
            {
                auto search = kDebugCommands.find(event.key.keysym.scancode);
                if (search != kDebugCommands.end())
                {
                    search->second();
                }
                break;
            }
            case SDL_QUIT:
                app_quit();
                break;
            case SDL_WINDOWEVENT:
                switch (event.window.event)
                {
                case SDL_WINDOWEVENT_RESIZED:
                    tdjx::render::on_resize();
                    break;
                default: break;
                }
                break;
            }
        }

        // TIME
        uint64 frequency = SDL_GetPerformanceFrequency();
        uint64 delta_ns = 0;
        {
            const uint64 k_nano = 1'000'000'000;

            lastTicks = ticks;
            ticks = SDL_GetPerformanceCounter();

            uint64 diff = ticks - lastTicks;

            delta_ns = diff * k_nano / frequency;

            dt = static_cast<float32>(static_cast<float64>(delta_ns) / static_cast<float64>(k_nano));
            time += dt;

            ++framesInSecond;
            frame_ns += delta_ns;
            if (frame_ns >= k_nano)
            {
                frame_ns -= k_nano;
                fps = framesInSecond;
                framesInSecond = 0;
            }
        }

        // UI
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();

        {
            ImGui::Begin("Debug");

            ImGui::Text("FPS: %d", fps);
            ImGui::Text("Delta MS: %0.2fms", static_cast<float64>(delta_ns) / 1000000);

            if (ImGui::Button("Reload Shaders"))
            {
                app_reload_shaders();
            }

            if (ImGui::Button(tdjx::render::get_mode_name()))
            {
                tdjx::render::next_mode();
            }

            ImGui::End();
        }

        {
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

        tdjx::gfx::clear(0);

        Room room = testRoom;

        const int size = 16;

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
            draw_ray(player.x, player.y, player.rot, 8, 8);
            draw_ray(player.x, player.y, player.rot - M_PI / 4.f, 8, 8);
            draw_ray(player.x, player.y, player.rot + M_PI / 4.f, 8, 8);
        }

        {
            for (int x = 0; x < k_width; ++x)
            {
                float32 dist;

                float32 r = player.rot + (static_cast<float32>(x) / k_width) * M_PI / 4.f;

                if (cast_ray(room, player.x, player.y, r, dist))
                {
                    float32 yy = 8.0f / dist * k_height / 2;
                    tdjx::gfx::line(x, k_height / 2 - yy, x, k_height / 2 + yy, 12);
                }
            }
        }

        ////tdjx::gfx::blit(image, 50, 50);
        //float32 tt = time * tdjx::math::TAU / 6;

        //int x = 160;
        //int y = 120;

        //const float32 aspectRatio = 4.f / 3.f;

        //glm::mat4 rotation = glm::rotate(glm::mat4(1), time, glm::vec3(0, 1, 0));
        //glm::mat4 translate = glm::translate(glm::mat4(1), glm::vec3(0, 0, 2));

        //auto transform_vert = [&](glm::vec3& vert)
        //{
        //    vert = translate * rotation * glm::vec4(vert, 1);
        //    return vert;
        //};

        //struct Point { int x, y; };
        //auto world_to_screen = [](const glm::vec3& world)
        //{
        //    return Point{
        //        static_cast<int>(world.x / world.z * 160) + 160,
        //        static_cast<int>(world.y / world.z * 120) + 120
        //    };
        //};

        //for (int i = 0; i < kTriangles * 3; i += 3)
        //{
        //    int i0 = indices[i];
        //    int i1 = indices[i + 1];
        //    int i2 = indices[i + 2];

        //    glm::vec3 vert0 = vertices[i0];
        //    glm::vec3 vert1 = vertices[i1];
        //    glm::vec3 vert2 = vertices[i2];

        //    transform_vert(vert0);
        //    transform_vert(vert1);
        //    transform_vert(vert2);

        //    Point a = world_to_screen(vert0);
        //    Point b = world_to_screen(vert1);
        //    Point c = world_to_screen(vert2);

        //    tdjx::gfx::line(a.x, a.y, b.x, b.y, 8);
        //    tdjx::gfx::line(b.x, b.y, c.x, c.y, 8);
        //    tdjx::gfx::line(a.x, a.y, c.x, c.y, 8);

        //    tdjx::gfx::triangle(a.x, a.x, b.x, b.y, c.x, c.y, 14);
        //}

        //for (int i = 0; i < 8; ++i)
        //{
        //    glm::vec3 vert = vertices[i];

        //    glm::vec3 world = translate * rotation * glm::vec4(vert, 1);

        //    int sx = static_cast<int>(world.x / world.z * 160) + 160;
        //    int sy = static_cast<int>(world.y / world.z * 120) + 120;

        //    tdjx::gfx::point(sx, sy, 12);
        //}

        ////tdjx::gfx::line(x, y, x + std::cos(tt) * 50, y + std::sin(tt) * 50, 8);

        ////tdjx::gfx::triangle(60, 20, 300, 20, 180, 140, 8);
        ////tdjx::gfx::triangle(160, 20, 100, 220, 190, 220, 16);
        ////tdjx::gfx::triangle(x - 30, y + 40, x + 10, y - 33, x + std::cos(tt) * 75, y + std::sin(tt) * 75, 29);

        ////for (int i = 0; i < 256; ++i)
        ////{
        ////    int x = i % 16 * 20;
        ////    int y = i / 16 * 15;
        ////    tdjx::gfx::rectangle_fill(x, y, x + 19, y + 14, i);
        ////}

        ////srand(1);
        ////for (int i = 0; i < 64; ++i)
        ////{
        ////    int x = i % 8 * 40 + 20;
        ////    int y = i / 8 * 30 + 15;
        ////    int r = 20;
        ////    tdjx::gfx::circle_fill(x, y, r + std::sin(tt + i / 10.f * M_PI * 2.f) * (r * 0.5f), i);
        ////}

        ////for (int i = 0; i < 32; ++i)
        ////{
        ////    int x = rand() % 320;
        ////    int y = rand() % 240;
        ////    tdjx::gfx::rectangle(x, y, x + 15, y + 15, 8 + i % 7);
        ////}
        
        tdjx::render::set_intensity(tdjx::gfx::get_pixels());

        // RENDER
        {
            ImGui::Render();
            SDL_GL_MakeCurrent(window, tdjx::render::glContext());

            tdjx::render::draw();

            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            SDL_GL_SwapWindow(window);
        }
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    tdjx::gfx::shutdown();

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
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
    
    float32 cbX = static_cast<float32>(x + (dx > 0) ? 1 : 0);
    float32 cbY = static_cast<float32>(y + (dy > 0) ? 1 : 0);

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