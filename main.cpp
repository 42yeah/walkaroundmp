#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include "Game.h"

#ifndef TILESET_PREFIX
#define TILESET_PREFIX "."
#endif

int main() {
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        std::cerr << "SDL initialization failed?" << std::endl;
        return 1;
    }
    if (!IMG_Init(IMG_INIT_PNG)) {
        std::cerr << "SDL_image initialization failed?" << std::endl;
        return 2;
    }
    Game game;
    while (!game.quit && game.valid) {
        game.update();
        game.render();
    }
    if (!game.valid) {
        std::cerr << "Game is invalid: " << game.err_reason() << std::endl;
    }
    SDL_Quit();
    return 0;
}
