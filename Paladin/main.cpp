#include <vector>
#include <random>
#include <iostream>
#include <cstdlib>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <Box2D/Box2D.h>

#include <effolkronium/random.hpp>

#include "algebra.h"
#include "color.h"
#include "renderer.h"

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

#include "pico8.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

using Random = effolkronium::random_static;

void game_init();
void game_update(float32 dt);
void game_draw();

int main(int argc, char* argv[]) {
    Random::seed(1);

    SDL_Init(SDL_INIT_VIDEO);

    int screenScale = 8;
    int screenDimension = 128 * screenScale;

    SDL_Window* window = SDL_CreateWindow("Paladin", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenDimension, screenDimension, SDL_WINDOW_OPENGL);
    
    tdjx::render::init(window);
    pico8::init();

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
    uint64 frameTicks = 0;

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
        {
            ticks = SDL_GetPerformanceCounter();
            uint64 frequency = SDL_GetPerformanceFrequency();

            uint64 diff = ticks - lastTicks;
            diff *= (1000000000 / frequency);

            lastTicks = ticks;

            dt = (float32)diff / 1000000000.f;
            time += dt;

            ++framesInSecond;
            frameTicks += diff;
            if (frameTicks >= 1000000000) {
                frameTicks -= 1000000000;
                fps = framesInSecond;
                framesInSecond = 0;
            }
        }

        // PICO-8 LOOP
        {
            pico8::update(dt);
            game_update(dt);
            game_draw();
        }

        // UI
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();

        {
            ImGui::Begin("Debug");

            ImGui::Text("FPS: %d", fps);

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

    pico8::shutdown();

    tdjx::render::shutdown();

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

void game_init()
{

}

void game_update(float32 dt)
{
    pico8::update(dt);
}

void game_draw()
{
    pico8::cls(0);

    pico8::srand(1);
    for (int i = 0; i < 128; ++i)
    {
        int x = pico8::rnd(128);
        int y = pico8::rnd(128);
        pico8::pset(x, y, std::floor(pico8::rnd(7)) + 8);
    }

    int xx = std::cos(pico8::time()) * 32 + 64;
    int yy = std::sin(pico8::time()) * 32 + 64;

    pico8::line(64, 64, xx, yy, 8);

    pico8::flip();
}