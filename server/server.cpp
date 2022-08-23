//
// Created by Hao Zhou on 22/08/2022.
//

#include <steam/steamnetworkingsockets.h>
#include <thread>
#include <chrono>
#include <iostream>
#include "../shared/World.h"
#include "NetStuff.h"

int main() {
    World server_world;
    // tasks:
    // 1. sync worlds to connected clients
    // 2. receive and process client's input
    // 3. update server's world
    // 4. send server's world to connected clients
    // 5. repeat
    int tps = 60;
    long long tick_rate = 1000 / tps;
    bool running = true;
    NetStuff netstuff;
    while (running) {
        const auto time_begin = std::chrono::high_resolution_clock::now();
        auto things = server_world.get_things();
        for (Thing &thing : things) {
            // update things
        }
        const auto elapsed = std::chrono::high_resolution_clock::now() - time_begin;
        const auto elapsed_millis = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
        std::this_thread::sleep_for(std::chrono::milliseconds(tick_rate - elapsed_millis));
    }
    return 0;
}
