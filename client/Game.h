//
// Created by Hao Zhou on 22/08/2022.
//

#ifndef WALKAROUNDMP_GAME_H
#define WALKAROUNDMP_GAME_H

#include <SDL.h>
#include <iostream>
#include "World.h"
#include "MPSession.h"

class Game {
public:
    Game();
    // no copy constructors please
    Game(const Game &) = delete;
    // no move constructors either
    Game(Game &&) = delete;
    ~Game() noexcept;
    void update();
    void render();

    mutable bool valid;
    bool quit;
    const std::string &err_reason() const {
        return error;
    }
    void sprite(int id, int x, int y, int w, int h) const;
    void parse_user_inputs();

private:
    bool test_and_fail(bool cond, const std::string &message) const;

    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *tileset;
    mutable std::string error;
    World world;
    Thing *player;
    int previous_tick;
    float delta_time;
    MPSession mpsession;
    struct {
        bool moved{false};
    } flags;

    friend class MPSession;
};

#endif //WALKAROUNDMP_GAME_H
