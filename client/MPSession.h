//
// Created by Hao Zhou on 25/08/2022.
//

#ifndef WALKAROUNDMP_MPSESSION_H
#define WALKAROUNDMP_MPSESSION_H

#include <steam/steamnetworkingsockets.h>
#include "World.h"

class Game;

// MPSession has full authority to manipulate every and all game states.
class MPSession {
public:
    MPSession(Game *game);
    ~MPSession();
    void connect(SteamNetworkingIPAddr addr);
    void notify_server(const Thing &thing) const;
    void poll_messages();
    void send_message(const std::string msg) const;

    bool alive;

private:
    static void on_steam_net_connection_status_changed(SteamNetConnectionStatusChangedCallback_t *info);
    void connection_status_changed(SteamNetConnectionStatusChangedCallback_t *info);

    ISteamNetworkingSockets *interface;
    HSteamNetConnection connection;
    Game *game;
    int assigned_id;
};

#endif //WALKAROUNDMP_MPSESSION_H
