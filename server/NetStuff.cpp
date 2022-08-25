//
// Created by Hao Zhou on 22/08/2022.
//

#include "NetStuff.h"
#include <sstream>

NetStuff *current_net_stuff = nullptr;
void NetStuff::on_steam_net_connection_status_changed(SteamNetConnectionStatusChangedCallback_t *info) {
    if (!current_net_stuff) {
        std::cerr << "Current NetStuff is null?" << std::endl;
        return;
    }
    switch (info->m_info.m_eState) {
        case k_ESteamNetworkingConnectionState_None:
            break;

        case k_ESteamNetworkingConnectionState_ClosedByPeer:
        case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
        {
            if (info->m_eOldState == k_ESteamNetworkingConnectionState_Connected) {
                auto client = std::find(current_net_stuff->connections.begin(), current_net_stuff->connections.end(), info->m_hConn);
                if (current_net_stuff->check_and_die(client != current_net_stuff->connections.end(),
                                                 "Established connection not found in active connection list?")) {
                    return;
                }
                if (info->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally) {
                    std::cout << "[NetStuff] " << info->m_info.m_szConnectionDescription << " is gone due to network difficulties: " << info->m_info.m_szEndDebug << std::endl;
                } else {
                    std::cout << "[NetStuff] " << info->m_info.m_szConnectionDescription << " is gone: " << info->m_info.m_szEndDebug << std::endl;
                }
                current_net_stuff->connections.erase(client, client + 1);
                NetStuffEvent event;
                event.type = NetStuffEventType_Disconnect;
                event.connection = info->m_hConn;
                current_net_stuff->on_message(event);
            } else {
                if (current_net_stuff->check_and_die(info->m_eOldState != k_ESteamNetworkingConnectionState_Connecting,
                                                     "Invalid connection state?")) {
                    return;
                } else {
                    std::cout << "[NetStuff] Some dude failed to connect." << std::endl;
                }
            }
            current_net_stuff->interface->CloseConnection(info->m_hConn, 0, nullptr, false);
            break;
        }

        case k_ESteamNetworkingConnectionState_Connecting:
        {
            if (current_net_stuff->check_and_die(std::find(current_net_stuff->connections.begin(), current_net_stuff->connections.end(), info->m_hConn) == current_net_stuff->connections.end(),
                                                 "Connecting connection found in active connection list?")) {
                return;
            }
            std::cout << "[NetStuff] Connection request from: " << info->m_info.m_szConnectionDescription << std::endl;
            if (current_net_stuff->interface->AcceptConnection(info->m_hConn) != k_EResultOK) {
                current_net_stuff->interface->CloseConnection(info->m_hConn, 0, nullptr, false);
                std::cerr << "Failed to accept connection?" << std::endl;
                break;
            }
            if (!current_net_stuff->interface->SetConnectionPollGroup(info->m_hConn, current_net_stuff->poll_group)) {
                current_net_stuff->interface->CloseConnection(info->m_hConn, 0, nullptr, false);
                std::cerr << "Failed to set connection poll group?" << std::endl;
                break;
            }
            // TODO: notify clients?
            current_net_stuff->connections.push_back(info->m_hConn);
            NetStuffEvent event;
            event.type = NetStuffEventType_Connect;
            event.connection = info->m_hConn;
            current_net_stuff->on_message(event);
            break;
        }

        case k_ESteamNetworkingConnectionState_Connected:
            break;

        default:
            break;
    }
}

NetStuff::NetStuff(World *world) : valid(false), err_reason(), world(world) {
}

NetStuff::~NetStuff() {
    for (auto client : connections) {
        interface->CloseConnection(client, 0, nullptr, false);
    }
    if (valid) {
        interface->DestroyPollGroup(poll_group);
        interface->CloseListenSocket(listen_socket);
    }
}

void NetStuff::poll_messages() {
    while (valid) {
        ISteamNetworkingMessage *incoming_message = nullptr;
        int num_messages = interface->ReceiveMessagesOnPollGroup(poll_group, &incoming_message, 1);
        if (num_messages == 0) {
            break;
        }
        if (num_messages < 0) {
            valid = false;
            err_reason = "Error checking incoming messages?";
            return;
        }
        if (num_messages != 1 || !incoming_message) {
            valid = false;
            err_reason = "Unexpected number of incoming messages?";
            return;
        }
        std::string message;
        message.assign((const char *) incoming_message->m_pData, incoming_message->m_cbSize);
        NetStuffEvent event;
        event.type = NetStuffEventType_Message;
        event.connection = incoming_message->m_conn;
        event.data = std::move(message);
        incoming_message->Release();
        on_message(event);
    }
    current_net_stuff = this;
    interface->RunCallbacks();
}

bool NetStuff::check_and_die(bool cond, const std::string &message) const {
    if (!cond) {
        valid = false;
        err_reason = message;
    }
    return !cond;
}

void NetStuff::start() {
    listen_addr.Clear();
    listen_addr.m_port = default_port;
    interface = SteamNetworkingSockets();
    SteamNetworkingConfigValue_t opt{};
    opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void *) on_steam_net_connection_status_changed);
    listen_socket = interface->CreateListenSocketIP(listen_addr, 1, &opt);
    if (listen_socket == k_HSteamListenSocket_Invalid) {
        valid = false;
        err_reason = "Failed to create listen socket?";
        return;
    }
    poll_group = interface->CreatePollGroup();
    if (poll_group == k_HSteamNetPollGroup_Invalid) {
        valid = false;
        err_reason = "Invalid poll group?";
        return;
    }
    std::cout << "[NetStuff] Created listen socket at " << listen_addr.m_port << std::endl;
    valid = true;
}

// server message identification
// (2 BYTE HEADER) (OTHER STUFFS)
// example: MShello world!
void NetStuff::broadcast_message(const std::string &message) const {
    std::stringstream ss;
    ss.write("MS", 2);
    ss.write(message.c_str(), message.size());
    const std::string send = ss.str();
    for (const auto conn : connections) {
        interface->SendMessageToConnection(conn, send.c_str(), send.size(), k_nSteamNetworkingSend_ReliableNoNagle, nullptr);
    }
}

void NetStuff::send_message(HSteamNetConnection conn, const std::string &message) const {
    std::stringstream ss;
    ss.write("MS", 2);
    ss.write(message.c_str(), message.size());
    const std::string send = ss.str();
    interface->SendMessageToConnection(conn, send.c_str(), send.size(), k_nSteamNetworkingSend_ReliableNoNagle, nullptr);
}

void NetStuff::send_world(HSteamNetConnection conn) const {
    if (check_and_die(world, "No world?")) {
        return;
    }
    std::stringstream ss;
    ss.write("WR", 2);
    ss << world->serialize()->rdbuf();
    const std::string send = ss.str();
    interface->SendMessageToConnection(conn, send.c_str(), send.size(), k_nSteamNetworkingSend_ReliableNoNagle, nullptr);
}

void NetStuff::send_message(HSteamNetConnection conn, char *data, int len) const {
    interface->SendMessageToConnection(conn, data, len, k_nSteamNetworkingSend_ReliableNoNagle, nullptr);
}

void NetStuff::send_thing(const Thing &thing, HSteamNetConnection exception) const {
    char data[2 + sizeof(Thing)] = "UP";
    std::memcpy(data + 2, &thing, sizeof(Thing));
    for (auto conn : connections) {
        if (conn == exception) {
            continue;
        }
        interface->SendMessageToConnection(conn, data, sizeof(data), k_nSteamNetworkingSend_ReliableNoNagle, nullptr);
    }
}
