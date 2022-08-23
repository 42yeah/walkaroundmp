//
// Created by Hao Zhou on 22/08/2022.
//

#include "Game.h"
#include <iostream>
#include <SDL_image.h>
#include "constants.h"

Game::Game() : window(nullptr), renderer(nullptr), tileset(nullptr), valid(false), quit(false), previous_tick(0), delta_time(0.0f) {
    window = SDL_CreateWindow("Walkaround MP", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        error = "Failed to create window?";
        return;
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr) {
        error = "Failed to create renderer?";
        SDL_DestroyWindow(window);
        return;
    }
    std::string tileset_path = std::string(TILESET_PREFIX) + "/Tilemap/tilemap_packed.png";
    SDL_Surface *tileset_surface = IMG_Load(tileset_path.c_str());
    if (!tileset_surface) {
        error = "Failed to load tileset?";
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        return;
    }
    tileset = SDL_CreateTextureFromSurface(renderer, tileset_surface);
    SDL_FreeSurface(tileset_surface);
    valid = true;
    // update world to include player himself
    world.add_thing(Thing(84, 23.0f, 45.0f));
    world.add_thing(Thing(85, 35.0f, 100.0f));
    player = world.get_things().begin();
    // time related initialization
    previous_tick = SDL_GetTicks();
}

Game::~Game() noexcept {
    SDL_DestroyTexture(tileset);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}

void Game::update() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            quit = true;
        }
    }
    int current_tick = SDL_GetTicks();
    delta_time = (current_tick - previous_tick) / 1000.0f;
    previous_tick = current_tick;
    // SDL check keydown status
    const Uint8 *keystate = SDL_GetKeyboardState(nullptr);
    // update world according to events
    if (keystate[SDL_SCANCODE_W]) {
        player->y -= player->speed * delta_time;
    }
    if (keystate[SDL_SCANCODE_A]) {
        player->x -= player->speed * delta_time;
    }
    if (keystate[SDL_SCANCODE_S]) {
        player->y += player->speed * delta_time;
    }
    if (keystate[SDL_SCANCODE_D]) {
        player->x += player->speed * delta_time;
    }
}

void Game::render() {
    SDL_RenderClear(renderer);
    int winw, winh;
    SDL_GetWindowSize(window, &winw, &winh);
    for (const Thing &thing : world.get_things()) {
        // translate the world to player's view
        float px = player->x + player->w / 2.0f, py = player->y + player->h / 2.0f;
        sprite(thing.id, thing.x - px + winw / 2, thing.y - py + winh / 2, thing.w, thing.h);
    }
    SDL_RenderPresent(renderer);
}

void Game::sprite(int id, int x, int y, int w, int h) const {
    if (test_and_fail(id >= 0 && id < 132, "Invalid tile id?")) {
        return;
    }
    int tilex = id % 12, tiley = id / 12;
    SDL_Rect srcrect = {tilex * tile_size, tiley * tile_size, tile_size, tile_size};
    SDL_Rect dstrect = {x, y, w, h};
    SDL_RenderCopy(renderer, tileset, &srcrect, &dstrect);
}

bool Game::test_and_fail(bool cond, const std::string &message) const {
    if (!cond) {
        error = message;
        valid = false;
    }
    return !cond;
}
