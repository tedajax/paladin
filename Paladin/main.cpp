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

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

using Random = effolkronium::random_static;

uint32 get_surface_pixel(SDL_Surface* surface, int x, int y);
void set_surface_pixel(SDL_Surface* surface, int x, int y, uint32 color);

namespace pico8
{
    namespace
    {
        struct
        {
            SDL_Surface* screen = nullptr;
            int width = 0;
            int height = 0;
        } g_pico8;

        uint32 get_true_color(uint8 picoColor);
        uint8 get_pico_color(uint32 trueColor);
    }

    void init();
    void shutdown();

    uint8 pget(int x, int y);

    void cls(uint8 c);
    void pset(int x, int y, uint8 c);
    void rect(int x, int y, int w, int h, uint8 c);
    void rectfill(int x, int y, int w, int h, uint8 c);
    void circ(int x, int y, int r, uint8 c);
    void circfill(int x, int y, int r, uint8 c);
}

int main(int argc, char* argv[]) {
    Random::seed(1);

    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    SDL_Window* window = SDL_CreateWindow("Paladin", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1920, 1080, SDL_WINDOW_OPENGL);
    
    tdjx::render::init(window);

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

    SDL_Surface* screen = SDL_CreateRGBSurface(0, k_width, k_height, 32, 0, 0, 0, 0);

    auto draw_rect = [screen](int x, int y, int w, int h, uint32 color)
    {
        SDL_Rect rect = { x, y, w, h };
        SDL_FillRect(screen, &rect, color);
    };

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

        // UPDATE
        {
            const SDL_Rect fullScreen = { 0, 0, k_width, k_height };
            SDL_FillRect(screen, &fullScreen, 0x00000000);

            float x = math::sin(180.f * time) * (k_width / 2.5f) + k_width / 2;
            draw_rect(static_cast<int>(x), k_height / 2, 10, 10, 0x0022cc90);
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

            SDL_LockSurface(screen);
            tdjx::render::set_image_data(static_cast<uint*>(screen->pixels), k_width, k_height);
            SDL_UnlockSurface(screen);

            tdjx::render::draw(dt);

            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            SDL_GL_SwapWindow(window);
        }
    }

    SDL_FreeSurface(screen);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    tdjx::render::shutdown();

    //SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

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
    //namespace
    //{
    //    struct
    //    {
    //        SDL_Surface* screen = nullptr;
    //        int width = 0;
    //        int height = 0;
    //    } g_pico8;
    //}

    namespace
    {
        const uint32 k_colors[16] = {
            0x00000000,
            0x00532B1D,
            0x0053257E,
            0x00518700,

            0x003652AB,
            0x0045575F,
            0x00C7C3C2,
            0x00E8F1FF,

            0x004D00FF,
            0x0000A3FF,
            0x0027ECFF,
            0x0036E400,

            0x00FFAD29,
            0x009C7683,
            0x00A877FF,
            0x00AACCFF,
        };

        uint32 get_true_color(uint8 picoColor)
        {
            int index = (picoColor & 0xF);
            return k_colors[index];
        }

        uint8 get_pico_color(uint32 trueColor)
        {
            for (int i = 0; i < 16; ++i)
            {
                if (k_colors[i] == trueColor)
                {
                    return i;
                }
            }

            return 0;
        }
    }

    void init()
    {
        g_pico8.width = 128;
        g_pico8.height = 128;
        g_pico8.screen = SDL_CreateRGBSurface(0, g_pico8.width, g_pico8.height,
            24, 0, 0, 0, 0);
    }

    void shutdown()
    {
        SDL_FreeSurface(g_pico8.screen);
    }

    uint8 pget(int x, int y)
    {
        return get_pico_color(get_surface_pixel(g_pico8.screen, x, y));
    }

    void cls(uint8 c)
    {
        SDL_FillRect(g_pico8.screen, nullptr, get_true_color(c));
    }

    void pset(int x, int y, uint8 c)
    {
        set_surface_pixel(g_pico8.screen, x, y, get_pico_color(c));
    }

    void rect(int x, int y, int w, int h, uint8 c);
    void rectfill(int x, int y, int w, int h, uint8 c);
    void circ(int x, int y, int r, uint8 c);
    void circfill(int x, int y, int r, uint8 c);
}