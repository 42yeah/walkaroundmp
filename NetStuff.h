//
// Created by Hao Zhou on 22/08/2022.
//

#ifndef WALKAROUNDMP_NETSTUFF_H
#define WALKAROUNDMP_NETSTUFF_H

#include <steam/steamnetworkingsockets.h>
#include <iostream>
#include <vector>
#include "constants.h"

class NetStuff {
public:
    NetStuff();
    ~NetStuff();
    void poll_messages();

    bool valid;
    std::string err_reason;

private:
    ISteamNetworkingSockets *interface;
    std::vector<HSteamNetConnection> connections;
    HSteamNetPollGroup poll_group;
    SteamNetworkingIPAddr listen_addr;
    HSteamListenSocket listen_socket;
};

#endif //WALKAROUNDMP_NETSTUFF_H
