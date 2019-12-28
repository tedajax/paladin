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

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

using Random = effolkronium::random_static;

void game_draw();

int main(int argc, char* argv[]) {
    using namespace pico8::literals;
    
    Random::seed(1);

    SDL_Init(SDL_INIT_VIDEO);

    int screenScale = 8;
    int screenDimension = 128 * screenScale;

    SDL_Window* window = SDL_CreateWindow("Paladin", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenDimension, screenDimension, SDL_WINDOW_OPENGL);
    
   
    tdjx::render::init(window);
    pico8::system_init([]() {},
        []() {},
        game_draw);

    // ImGui initialization
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui_ImplSDL2_InitForOpenGL(window, tdjx::render::glContext());
    ImGui_ImplOpenGL3_Init(tdjx::render::glslVersion());

    ImGui::StyleColorsDark();

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
    while (isRunning) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            switch (event.type) {
            case SDL_KEYDOWN:
                if (event.key.keysym.scancode != SDL_SCANCODE_ESCAPE) {
                    break;
                }
            case SDL_QUIT:
                isRunning = false;
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
            if (frame_ns >= k_nano) {
                frame_ns -= k_nano;
                fps = framesInSecond;
                framesInSecond = 0;
            }
        }

        // PICO-8 LOOP
        {
            pico8::system_update(dt);
            pico8::system_draw();
        }

        // UI
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();

        {
            ImGui::Begin("Debug");

            ImGui::Text("FPS: %d", fps);

            ImGui::Text("Time: %f", time);
            ImGui::Text("Delta NS: %" PRIu64, delta_ns);
            ImGui::Text("Delta: %f", dt);


            ImGui::End();
        }


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

    pico8::system_shutdown();

    tdjx::render::shutdown();

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

void game_draw()
{
    using namespace pico8;

    cls(0);

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