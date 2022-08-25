//
// Created by Hao Zhou on 22/08/2022.
//

#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>
#include <iostream>
#include <thread>
#include "World.h"
#include "constants.h"

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
            if (dummy_netstuff.conn != k_HSteamNetConnection_Invalid) {
                dummy_netstuff.interface->CloseConnection(dummy_netstuff.conn, 0, nullptr, false);
                dummy_netstuff.conn = k_HSteamNetConnection_Invalid;
                dummy_netstuff.alive = false;
            }
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

void handle_user_input() {
    std::vector<std::string> tokens;
    tokens.reserve(max_server_command_token_count);
    while (dummy_netstuff.alive) {
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
        if (!dummy_netstuff.alive) {
            break;
        }
        if (tokens[0] == "quit") {
            dummy_netstuff.alive = false;
            dummy_netstuff.interface->CloseConnection(dummy_netstuff.conn, 0, nullptr, false);
            dummy_netstuff.conn = k_HSteamNetConnection_Invalid;
        }
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
    char msg_header[2];
    std::thread user_input_thread(handle_user_input);
    user_input_thread.detach();
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
            if (incoming_message->m_cbSize < 2) {
                std::cerr << "Incorrect message length?" << std::endl;
                continue;
            }
            std::memcpy(msg_header, msg_str.c_str(), 2);
            if (std::strcmp(msg_header, "MS") == 0) {
                std::cout << "Received message: " << msg_str.substr(2) << std::endl;
            } else if (std::strcmp(msg_header, "WR") == 0) {
                std::cout << "Received world." << std::endl;
                int things_size;
                const char *data = msg_str.c_str() + 2;
                std::memcpy(&things_size, data, sizeof(int));
                data += sizeof(int);
                std::cout << "Things size: " << things_size << std::endl;
                for (int i = 0; i < things_size; i++) {
                    Thing new_thing;
                    std::memcpy(&new_thing, data, sizeof(Thing));
                    data += sizeof(Thing);
                    std::cout << "Thing: " << new_thing.sprite_id << " " << new_thing.id << " " <<
                        new_thing.x << " " << new_thing.y << " " << new_thing.w << " " << new_thing.h << std::endl;
                }
                // TODO: visualize world in some sense
            } else {
                std::cout << "Unknown message header?: ";
                std::cout.write(msg_header, 2);
                std::cout << msg_str.substr(2) << std::endl;
            }
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
