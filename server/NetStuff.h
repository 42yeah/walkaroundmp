//
// Created by Hao Zhou on 22/08/2022.
//

#ifndef WALKAROUNDMP_NETSTUFF_H
#define WALKAROUNDMP_NETSTUFF_H

#include <steam/steamnetworkingsockets.h>
#include <iostream>
#include <vector>
#include <functional>
#include "World.h"
#include "constants.h"

enum NetStuffEventType {
    NetStuffEventType_None,
    NetStuffEventType_Connect,
    NetStuffEventType_Disconnect,
    NetStuffEventType_Message
};

struct NetStuffEvent {
    NetStuffEventType type;
    HSteamNetConnection connection;
    std::string data;
};

class NetStuff {
public:
    NetStuff(World *world);
    ~NetStuff();
    void start();
    void poll_messages();
    void broadcast_message(const std::string &msg) const;
    void send_message(HSteamNetConnection conn, const std::string &msg) const;
    void send_message(HSteamNetConnection conn, char *data, int len) const;
    void send_world(HSteamNetConnection conn) const;
    void send_thing(const Thing &thing, HSteamNetConnection exception) const;

    mutable bool valid;
    mutable std::string err_reason;
    std::vector<HSteamNetConnection> connections;
    World *world; // bound world for syncing
    std::function<void(const NetStuffEvent)> on_message = [](auto) {
        std::cerr << "No NetStuffEvent handler registered" << std::endl;
    };

private:
    static void on_steam_net_connection_status_changed(SteamNetConnectionStatusChangedCallback_t *info);
    bool check_and_die(bool cond, const std::string &message) const;

    ISteamNetworkingSockets *interface;
    HSteamNetPollGroup poll_group;
    SteamNetworkingIPAddr listen_addr;
    HSteamListenSocket listen_socket;
};

#endif //WALKAROUNDMP_NETSTUFF_H
