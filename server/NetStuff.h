//
// Created by Hao Zhou on 22/08/2022.
//

#ifndef WALKAROUNDMP_NETSTUFF_H
#define WALKAROUNDMP_NETSTUFF_H

#include <steam/steamnetworkingsockets.h>
#include <iostream>
#include <vector>
#include "../shared/constants.h"

class NetStuff {
public:
    NetStuff();
    ~NetStuff();
    void poll_messages();

    mutable bool valid;
    mutable std::string err_reason;

private:
    static void on_steam_net_connection_status_changed(SteamNetConnectionStatusChangedCallback_t *info);
    bool check_and_die(bool cond, const std::string &message) const;

    ISteamNetworkingSockets *interface;
    std::vector<HSteamNetConnection> connections;
    HSteamNetPollGroup poll_group;
    SteamNetworkingIPAddr listen_addr;
    HSteamListenSocket listen_socket;
};

#endif //WALKAROUNDMP_NETSTUFF_H
