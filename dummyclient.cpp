//
// Created by Hao Zhou on 22/08/2022.
//

#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>
#include <iostream>
#include <thread>
#include "constants.h"

void on_steam_net_connection_status_changed(SteamNetConnectionStatusChangedCallback_t *info) {
    std::cout << "Something happened!" << std::endl;
}

int main() {
    SteamNetworkingIPAddr addr{};
    addr.Clear();
    char c_addr[512] = {0};
    std::sprintf(c_addr, "127.0.0.1:%d", 12345);
    if (!addr.ParseString(c_addr)) {
        std::cerr << "Failed to parse IP?" << std::endl;
        return 1;
    }
    SteamDatagramErrMsg err_msg;
    if (!GameNetworkingSockets_Init(nullptr, err_msg)) {
        std::cerr << "Failed to initialize GNS?" << std::endl;
        return 2;
    }
    ISteamNetworkingSockets *interface = SteamNetworkingSockets();
    char addr_raw[512] = {0};
    addr.ToString(addr_raw, sizeof(addr_raw), true);
    std::cout << "Connecting to " << addr_raw << std::endl;
    // set state change mechanisms

    // now make the connection
    SteamNetworkingConfigValue_t opt{};
    opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void *) on_steam_net_connection_status_changed);
    HSteamNetConnection conn = interface->ConnectByIPAddress(addr, 1, &opt);
    if (conn == k_HSteamNetConnection_Invalid) {
        std::cerr << "Failed to connect?" << std::endl;
        return 3;
    }
    std::cout << "Connected!" << std::endl;
    bool quit = false;
    bool first = true;
    while (!quit) {
        // poll incoming messages
        while (!quit) {
            ISteamNetworkingMessage *incoming_message = nullptr;
            int num_msgs = interface->ReceiveMessagesOnConnection(conn, &incoming_message, 1);
            if (num_msgs == 0) {
                break;
            }
            if (num_msgs < 0) {
                std::cerr << "Error while checking messages?" << std::endl;
                break;
            }
            std::string msg_str;
            msg_str.assign((const char *) incoming_message->m_pData, incoming_message->m_cbSize);
            std::cout << "We got: " << msg_str << std::endl;
            incoming_message->Release();
        }
        // send a hello message
        if (first) {
            first = false;
            interface->SendMessageToConnection(conn, "Hey man!", 8, k_nSteamNetworkingSend_Reliable, nullptr);
        }
        // check for state changes

        // sleep for a heartbeat
        std::this_thread::sleep_for(std::chrono::milliseconds (10));
    }
    return 0;
}