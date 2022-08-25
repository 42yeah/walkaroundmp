//
// Created by Hao Zhou on 25/08/2022.
//

#include "MPSession.h"
#include <iostream>
#include "Game.h"

MPSession *current_mpsession = nullptr;
void MPSession::on_steam_net_connection_status_changed(SteamNetConnectionStatusChangedCallback_t *info) {
    if (!current_mpsession) {
        std::cerr << "Current MPSession is empty?" << std::endl;
        return;
    }
    current_mpsession->connection_status_changed(info);
}

MPSession::MPSession(Game *game) : alive(false), interface(nullptr), connection(k_HSteamNetConnection_Invalid), game(game), assigned_id(-1) {
    interface = SteamNetworkingSockets();
    if (game == nullptr) {
        std::cerr << "Warning: game is null. This might crash later." << std::endl;
    }
}

MPSession::~MPSession() {
    if (connection != k_HSteamNetConnection_Invalid) {
        interface->CloseConnection(connection, 0, nullptr, false);
        connection = k_HSteamNetConnection_Invalid;
    }
}

void MPSession::connect(SteamNetworkingIPAddr addr) {
    SteamNetworkingConfigValue_t opt{};
    opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void *) MPSession::on_steam_net_connection_status_changed);
    connection = interface->ConnectByIPAddress(addr, 1, &opt);
    std::cout << "Making connection..." << std::endl;
}

void MPSession::connection_status_changed(SteamNetConnectionStatusChangedCallback_t *info) {
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
            if (connection != k_HSteamNetConnection_Invalid) {
                interface->CloseConnection(connection, 0, nullptr, false);
                connection = k_HSteamNetConnection_Invalid;
                alive = false;
            }
            break;
        }

        case k_ESteamNetworkingConnectionState_Connecting:
            break;

        case k_ESteamNetworkingConnectionState_Connected:
        {
            std::cout << "Connection established." << std::endl;
            alive = true;
            break;
        }

        default:
            break;
    }
}

void MPSession::poll_messages() {
    while (alive) {
        ISteamNetworkingMessage *incoming_message = nullptr;
        int num_messages = interface->ReceiveMessagesOnConnection(connection, &incoming_message, 1);
        if (num_messages == 0) {
            break;
        }
        if (num_messages < 0) {
            std::cerr << "Negative amount of messages?" << std::endl;
            break;
        }
        std::string msg_str;
        msg_str.assign((char *) incoming_message->m_pData, incoming_message->m_cbSize);
        if (incoming_message->m_cbSize < 2) {
            std::cerr << "Incorrect message length?" << std::endl;
            break;
        }
        const char *header = msg_str.c_str();
        if (std::strncmp(header, "MS", 2) == 0) {
            std::cout << "[Server] " << msg_str.substr(2) << std::endl;
        } else if (std::strncmp(header, "WR", 2) == 0) {
            int num_things = 0;
            const char *data_body = msg_str.c_str() + 2;
            std::memcpy(&num_things, data_body, sizeof(int));
            game->world = World();
            for (int i = 0; i < num_things; i++) {
                const char *thing_data = data_body + sizeof(int) + i * sizeof(Thing);
                Thing thing;
                std::memcpy(&thing, thing_data, sizeof(Thing));
                Thing &modded_thing = game->world.add_thing(thing);
                modded_thing.id = thing.id;
            }
            if (assigned_id == -1) {
                std::cerr << "No assigned ID?" << std::endl;
                alive = false;
                break;
            }
            Thing *player_thing = game->world.get_thing_by_id(assigned_id);
            if (player_thing == nullptr) {
                std::cerr << "Cannot find player thing?" << std::endl;
                alive = false;
                break;
            }
            game->player = game->world.get_thing_by_id(assigned_id);
        } else if (std::strncmp(header, "AS", 2) == 0) {
            int id;
            std::memcpy(&id, msg_str.c_str() + 2, sizeof(int));
            std::cout << "Server has assigned us with an ID of " << id << std::endl;
            // fetch the world
            std::string cmd = "WR";
            interface->SendMessageToConnection(connection, cmd.c_str(), cmd.size(), k_nSteamNetworkingSend_ReliableNoNagle, nullptr);
            assigned_id = id;
        } else if (std::strncmp(header, "UP", 2) == 0) {
            // update on things
            Thing new_thing;
            std::memcpy(&new_thing, msg_str.c_str() + 2, sizeof(Thing));
            Thing *t = game->world.get_thing_by_id(new_thing.id);
            if (t == nullptr) {
                std::cerr << "Null thing?" << std::endl;
                alive = false;
                break;
            }
            t->copy_from(new_thing);
        } else {
            std::cerr << "Unknown message received?: " << msg_str << std::endl;
        }
    }
    current_mpsession = this;
    interface->RunCallbacks();
}

void MPSession::notify_server(const Thing &thing) const {
    char data[2 + sizeof(Thing)] = "UP";
    std::memcpy(data + 2, &thing, sizeof(Thing));
    interface->SendMessageToConnection(connection, data, sizeof(data), k_nSteamNetworkingSend_ReliableNoNagle, nullptr);
}

void MPSession::send_message(const std::string str) const {
    std::string cmd = "MS";
    cmd += str;
    interface->SendMessageToConnection(connection, cmd.c_str(), cmd.size(), k_nSteamNetworkingSend_ReliableNoNagle, nullptr);
}
