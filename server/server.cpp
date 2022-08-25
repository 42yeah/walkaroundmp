//
// Created by Hao Zhou on 22/08/2022.
//

#include <thread>
#include <chrono>
#include <iostream>
#include <map>
#include <steam/steamnetworkingsockets.h>
#include "World.h"
#include "constants.h"
#include "NetStuff.h"

World server_world;
NetStuff netstuff(&server_world);
bool running = false;

void process_inputs() {
    std::vector<std::string> tokens;
    tokens.reserve(max_server_command_token_count);
    while (netstuff.valid && running) {
        std::string input;
        std::getline(std::cin, input);
        tokens.clear();
        int prev_pos = 0, pos = 0;
        while ((pos = input.find(" ", pos)) != std::string::npos) {
            std::string token = input.substr(prev_pos, pos - prev_pos);
            tokens.push_back(token);
            prev_pos = ++pos;
        }
        tokens.push_back(input.substr(prev_pos));
        if (!netstuff.valid || !running) {
            break;
        }
        if (tokens[0] == "quit") {
            running = false;
        } else if (tokens[0] == "say") {
            std::string msg = "";
            for (int i = 1; i < tokens.size(); i++) {
                msg += tokens[i] + " ";
            }
            // remove last space
            msg = msg.substr(0, msg.size() - 1);
            netstuff.broadcast_message(msg);
        } else if (tokens[0] == "send") {
            if (tokens[1] == "world") {
                if (netstuff.connections.size() > 0) {
                    netstuff.send_world(netstuff.connections[0]);
                } else {
                    std::cerr << "No established connections?" << std::endl;
                }
            } else {
                std::cerr << "Send what?" << std::endl;
            }
        } else {
            std::cerr << "?" << std::endl;
        }
    }
}

int main() {
    SteamDatagramErrMsg err_msg;
    if (!GameNetworkingSockets_Init(nullptr, err_msg)) {
        std::cerr << "Cannot initialize GameNetworkingSockets?: " << err_msg << std::endl;
        return 1;
    }
    // tasks:
    // 1. sync worlds to connected clients
    // 2. receive and process client's input
    // 3. update server's world
    // 4. send server's world to connected clients
    // 5. repeat
    running = true;
    std::map<HSteamNetConnection, int> connection_id;
    netstuff.on_message = [&](NetStuffEvent event) {
        switch (event.type) {
            case NetStuffEventType_None:
                break;

            case NetStuffEventType_Connect:
            {
                Thing &thing = server_world.add_thing(Thing(84, -1, 0.0f, 0.0f));
                netstuff.send_message(event.connection, "Hi!");
                connection_id[event.connection] = thing.id;
                std::cout << "Client ID " << thing.id << " has connected: connection " << event.connection << std::endl;
                char assignment[2 + sizeof(int)] = "AS";
                std::memcpy(assignment + 2, &thing.id, sizeof(int));
                netstuff.send_message(event.connection, assignment, 2 + sizeof(int));
                // since there is someone new, sync the world to everyone (except the new guy)
                for (auto &connection : netstuff.connections) {
                    if (connection != event.connection) {
                        netstuff.send_world(connection);
                    }
                }
                break;
            }

            case NetStuffEventType_Disconnect:
            {
                int thing_id = connection_id[event.connection];
                auto it = std::find_if(server_world.get_things().begin(), server_world.get_things().end(), [&](const Thing &thing) {
                    return thing.id == thing_id;
                });
                std::cout << "Client ID " << event.connection << "-" << (it->id) << " has disconnected." << std::endl;
                if (it != server_world.get_things().end()) {
                    server_world.get_things().erase(it);
                } else {
                    std::cerr << "Client is not inside world?" << event.connection << "-" << connection_id[event.connection] << std::endl;
                }
                connection_id.erase(event.connection);
                // since there is someone gone, sync the world to everyone
                for (auto &connection : netstuff.connections) {
                    netstuff.send_world(connection);
                }
                break;
            }

            case NetStuffEventType_Message:
            {
                Thing *thing = server_world.get_thing_by_id(connection_id[event.connection]);
                const char *msg = event.data.c_str();
                if (!thing) {
                    std::cerr << "Client is not inside world?: " << event.connection << "-" << connection_id[event.connection] << std::endl;
                    break;
                }
                if (std::strncmp(msg, "MS", 2) == 0) {
                    netstuff.broadcast_message(event.data.substr(2));
                    std::cout << "[" << event.connection << "-" << connection_id[event.connection] << "] " << event.data.substr(2) << std::endl;
                } else if (std::strncmp(msg, "WR", 2) == 0) {
                    netstuff.send_world(event.connection);
                } else if (std::strncmp(msg, "UP", 2) == 0) {
                    const Thing &new_thing = *((Thing *) (msg + 2));
                    Thing *t = server_world.get_thing_by_id(new_thing.id);
                    if (t == nullptr) {
                        std::cerr << "Thing sent from " << event.connection << "-" << connection_id[event.connection] <<  " does not exist in server world?" << std::endl;
                        break;
                    }
                    t->copy_from(new_thing);
                    netstuff.send_thing(*t, event.connection);
                }
                break;
            }
        }
    };
    netstuff.start();
    std::thread input_thread(process_inputs);
    input_thread.detach();
    while (netstuff.valid && running) {
        const auto time_begin = std::chrono::high_resolution_clock::now();
        auto things = server_world.get_things();
        for (Thing &thing : things) {
            // update things
        }
        netstuff.poll_messages();
        const auto elapsed = std::chrono::high_resolution_clock::now() - time_begin;
        const auto elapsed_millis = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
        std::this_thread::sleep_for(std::chrono::milliseconds(tick_rate - elapsed_millis));
    }
    if (!netstuff.valid) {
        std::cerr << "Server invalid? Reason: " << netstuff.err_reason << std::endl;
        return 1;
    }
    return 0;
}
