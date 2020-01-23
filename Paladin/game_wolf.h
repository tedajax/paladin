#pragma once

#include "types.h"
#include "tdjx_game.h"

struct WolfContext
{
    struct Player
    {
        float32 x, y;
        float32 rot;
    };

    Player player = { 2, 4, 0 };
    SDL_Window* window = nullptr;
};

struct WolfGame : public tdjx::Game<WolfContext>
{
    WolfGame(SDL_Window* window);
    ~WolfGame();

    void update() override final;
    void render() override final;
};