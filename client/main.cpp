#include <iostream>
#include <thread>
#include <SDL.h>
#include <SDL_image.h>
#include <steam/steamnetworkingsockets.h>
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
    SteamDatagramErrMsg err_msg;
    if (!GameNetworkingSockets_Init(nullptr, err_msg)) {
        std::cerr << "Failed to initialize GNS?" << std::endl;
        return 3;
    }
    Game game;
    std::thread user_input_parser_thread(&Game::parse_user_inputs, &game);
    user_input_parser_thread.detach();
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
