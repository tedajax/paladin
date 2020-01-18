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

using Random = effolkronium::random_static;

int main(int argc, char* argv[])
{
    Random::seed(1);

    SDL_Init(SDL_INIT_VIDEO);

    const int kWindowWidth = 1440;
    const int kWindowHeight = 1080;

    SDL_Window* window = SDL_CreateWindow("Paladin", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, kWindowWidth, kWindowHeight, SDL_WINDOW_OPENGL);

    //{
    //    int w, h, n;
    //    uint8* data = stbi_load("assets/sheet_00.png", &w, &h, &n, 4);
    //    printf("w: %d, h: %d, n: %d\n", w, h, n);
    //}

    tdjx::gfx::init_with_window(320, 240, window);
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

        tdjx::gfx::clear(0);

        //tdjx::gfx::blit(image, 50, 50);
        float32 tt = time * tdjx::math::TAU / 6;

        int x = 160;
        int y = 120;

        //tdjx::gfx::line(x, y, x + std::cos(tt) * 50, y + std::sin(tt) * 50, 8);

        tdjx::gfx::triangle(60, 20, 300, 20, 180, 140, 8);
        tdjx::gfx::triangle(160, 20, 100, 220, 190, 220, 16);
        tdjx::gfx::triangle(x - 30, y + 40, x + 10, y - 33, x + std::cos(tt) * 50, y + std::sin(tt) * 50, 29);

        //for (int i = 0; i < 256; ++i)
        //{
        //    int x = i % 16 * 20;
        //    int y = i / 16 * 15;
        //    tdjx::gfx::rectangle_fill(x, y, x + 19, y + 14, i);
        //}

        //srand(1);
        //for (int i = 0; i < 64; ++i)
        //{
        //    int x = i % 8 * 40 + 20;
        //    int y = i / 8 * 30 + 15;
        //    int r = 20;
        //    tdjx::gfx::circle_fill(x, y, r + std::sin(tt + i / 10.f * M_PI * 2.f) * (r * 0.5f), i);
        //}

        //for (int i = 0; i < 32; ++i)
        //{
        //    int x = rand() % 320;
        //    int y = rand() % 240;
        //    tdjx::gfx::rectangle(x, y, x + 15, y + 15, 8 + i % 7);
        //}
        
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
