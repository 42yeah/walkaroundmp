//
// Created by Hao Zhou on 22/08/2022.
//

#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>
#include <iostream>
#include <thread>
#include "../shared/constants.h"

struct {
    ISteamNetworkingSockets *interface{nullptr};
    HSteamNetConnection conn;
    bool alive{true};
} dummy_netstuff;

void on_steam_net_connection_status_changed(SteamNetConnectionStatusChangedCallback_t *info) {
    switch (info->m_info.m_eState) {
        case k_ESteamNetworkingConnectionState_None:
            break;

        case k_ESteamNetworkingConnectionState_ClosedByPeer:
        case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
        {
            if (info->m_eOldState == k_ESteamNetworkingConnectionState_Connecting) {
                std::cerr << "Cannot connect to remote host?: " << info->m_info.m_szEndDebug << std::endl;
            } else if (info->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally) {
                std::cerr << "Lost contact with host?: " << info->m_info.m_szEndDebug << std::endl;
            } else {
                std::cerr << "Kicked by host?: " << info->m_info.m_szEndDebug << std::endl;
            }
            dummy_netstuff.interface->CloseConnection(dummy_netstuff.conn, 0, nullptr, false);
            dummy_netstuff.conn = k_HSteamNetConnection_Invalid;
            dummy_netstuff.alive = false;
            break;
        }

        case k_ESteamNetworkingConnectionState_Connecting:
            break;

        case k_ESteamNetworkingConnectionState_Connected:
        {
            std::cout << "Connection established." << std::endl;
            break;
        }

        default:
            break;
    }
}

int main() {
    SteamNetworkingIPAddr addr{};
    addr.Clear();
    char c_addr[512] = {0};
    std::sprintf(c_addr, "127.0.0.1:%d", default_port);
    if (!addr.ParseString(c_addr)) {
        std::cerr << "Failed to parse IP?" << std::endl;
        return 1;
    }
    SteamDatagramErrMsg err_msg;
    if (!GameNetworkingSockets_Init(nullptr, err_msg)) {
        std::cerr << "Failed to initialize GNS?" << std::endl;
        return 2;
    }
    dummy_netstuff.interface = SteamNetworkingSockets();
    char addr_raw[512] = {0};
    addr.ToString(addr_raw, sizeof(addr_raw), true);
    std::cout << "Connecting to " << addr_raw << std::endl;
    // now make the connection
    SteamNetworkingConfigValue_t opt{};
    opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void *) on_steam_net_connection_status_changed);
    dummy_netstuff.conn = dummy_netstuff.interface->ConnectByIPAddress(addr, 1, &opt);
    if (dummy_netstuff.conn == k_HSteamNetConnection_Invalid) {
        std::cerr << "Failed to connect?" << std::endl;
        return 3;
    }
    bool first = true;
    while (dummy_netstuff.alive) {
        // poll incoming messages
        while (dummy_netstuff.alive) {
            ISteamNetworkingMessage *incoming_message = nullptr;
            int num_msgs = dummy_netstuff.interface->ReceiveMessagesOnConnection(dummy_netstuff.conn,
                                                                                 &incoming_message, 1);
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
            dummy_netstuff.interface->SendMessageToConnection(dummy_netstuff.conn, "Hey man!", 8, k_nSteamNetworkingSend_Reliable, nullptr);
        }
        // check for state changes
        dummy_netstuff.interface->RunCallbacks();
        // sleep for a heartbeat
        std::this_thread::sleep_for(std::chrono::milliseconds (10));
    }
    return 0;
}
