#include <vector>
#include <random>
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

#include "pico8.h"
#include "tdjx_gfx.h"

#include "util.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

using Random = effolkronium::random_static;

void game_draw();

int g_stressNum = 10000;

int main(int argc, char* argv[])
{
    using namespace pico8::literals;

    Random::seed(1);

    SDL_Init(SDL_INIT_VIDEO);

    const int kWindowWidth = 1440;
    const int kWindowHeight = 1080;

    SDL_Window* window = SDL_CreateWindow("Paladin", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, kWindowWidth, kWindowHeight, SDL_WINDOW_OPENGL);

    // ImGui initialization
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    auto render_init = [window]()
    {
        tdjx::render::init(window, 320, 240);

        ImGui_ImplSDL2_InitForOpenGL(window, tdjx::render::glContext());
        ImGui_ImplOpenGL3_Init(tdjx::render::glslVersion());

        // Setup palette
        uint32 paletteSize = 0;
        {
            int w, h, bpp;
            uint8* data = stbi_load("assets/palettes/rosy42.png", &w, &h, &bpp, 4);

            uint32 size = w * h;
            paletteSize = tdjx::util::next_pow2(size);

            uint8* paddedData = new uint8[paletteSize * 4];

            std::memset(paddedData, 0, paletteSize * 4);
            std::memcpy(paddedData, data, w * h * 4);

            if (data != nullptr)
            {
                tdjx::render::set_palette(paddedData, paletteSize);
            }

            //delete paddedData;
            stbi_image_free(data);
        }

        ImGui::StyleColorsDark();

        return paletteSize;
    };

    auto render_shutdown = []()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        tdjx::render::shutdown();
    };

    int colors = render_init();
    tdjx::gfx::init(320, 240, 64);

    int fps = 0;
    uint64 ticks = SDL_GetPerformanceCounter();
    uint64 lastTicks = ticks;
    uint64 frame_ns = 0;

    int framesInSecond = 0;
    float32 dt = 0.f;
    float32 time = 0.f;

    const int k_width = 320;
    const int k_height = 180;

    bool isRunning = true;
    bool shouldReloadShaders = false;

    auto app_quit = [&isRunning]() { isRunning = false; };
    auto app_reload_shaders = [&shouldReloadShaders]() { shouldReloadShaders = true; };

    const std::unordered_map<SDL_Scancode, std::function<void(void)>> kDebugCommands = {
        { SDL_SCANCODE_ESCAPE, app_quit },
        { SDL_SCANCODE_F2, app_reload_shaders },
        { SDL_SCANCODE_F3, tdjx::render::prev_mode },
        { SDL_SCANCODE_F4, tdjx::render::next_mode },
    };

    while (isRunning)
    {
        if (shouldReloadShaders)
        {
            shouldReloadShaders = false;
            render_shutdown();
            render_init();
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

        tdjx::gfx::clear(static_cast<int>(time) % 16);

        float32 tt = time * 8;

        srand(1);
        for (int i = 0; i < 64; ++i)
        {
            int x = i % 8 * 40 + 20;
            int y = i / 8 * 30 + 15;
            int r = 20;
            tdjx::gfx::circle_fill(x, y, r + std::sin(tt + i / 10.f * M_PI * 2.f) * (r * 0.5f), i % 42);
        }

        for (int i = 0; i < 32; ++i)
        {
            int x = rand() % 320;
            int y = rand() % 240;
            tdjx::gfx::rectangle(x, y, x + 15, y + 15, 8 + i % 7);
        }

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

    tdjx::gfx::shutdown();

    render_shutdown();
    ImGui::DestroyContext();

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

void game_draw()
{
    using namespace pico8;

    cls(0);
    if (false)
    {
        pico8::srand(1);

        for (int i = 0; i < 16; ++i)
        {
            int x = i % 4 * 16;
            int y = (i / 4) * 16;
            rectfill(x, y, 16, 16, i);
        }

        for (int i = 0; i < 128; ++i)
        {
            int x = rnd(128);
            int y = rnd(128);
            pset(x, y, static_cast<int>(rnd(7)) + 8);
        }

        fixed16 xx = cos(time() * 0.125_fx16) * 32_fx16 + 64_fx16;
        fixed16 yy = sin(time() * 0.125_fx16) * 32_fx16 + 64_fx16;
        line(64_fx16, 64_fx16, xx, yy, 8_fx16);

        line(0_fx16, 0_fx16, 2_fx16, 0_fx16, 11);
        line(0_fx16, 1_fx16, 3_fx16, 1_fx16, 10);
        line(1_fx16, 2_fx16, 2_fx16, 2_fx16, 9);
        line(1_fx16, 3_fx16, 3_fx16, 3_fx16, 8);

        rect(50_fx16, 10_fx16, 40_fx16, 30_fx16, 12);

        rectfill(20_fx16, 40_fx16, 8_fx16, 56_fx16, 3);

        line(16, -20, 12, 300, 2);

        circ(10, 25, 0, 8);
        circ(10, 35, 1, 9);
        circ(10, 45, 2, 10);
        circ(70, 25, sin(time()) * 8_fx16 + 9_fx16, 7);
        circfill(90, 89, cos(time() * 0.125_fx16) * 24_fx16 + 25_fx16, 14);

        for (int i = 0; i < 100; ++i)
        {
            poke(0x6000 + static_cast<int>(rnd(0x2000)), 0x55);
        }
    }
    else
    {
        for (int i = 0; i < g_stressNum; ++i)
        {
            int l = 8_fx16 + rnd(7);
            int r = 8_fx16 + rnd(7);

            poke(0x6000 + static_cast<int>(rnd(0x2000)), (l << 4) + r);

            //pset(rnd(128), rnd(128), 8_fx16 + rnd(7));
        }
    }
}