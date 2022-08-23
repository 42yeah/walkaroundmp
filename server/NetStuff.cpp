//
// Created by Hao Zhou on 22/08/2022.
//

#include "NetStuff.h"

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
                if (current_net_stuff->check_and_die(client == current_net_stuff->connections.end(),
                                                 "Established connection not found in active connection list?")) {
                    return;
                }
                char name[512] = {0};
                info->m_info.m_addrRemote.ToString(name, sizeof(name), true);
                if (info->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally) {
                    std::cout << "[NetStuff] " << name << " is gone due to network difficulties: " << info->m_info.m_szEndDebug << std::endl;
                } else {
                    std::cout << "[NetStuff] " << name << " is gone: " << info->m_info.m_szEndDebug << std::endl;
                }
                // TODO: broadcast gone message to client?
                current_net_stuff->connections.erase(client, client + 1);
            } else {
                if (current_net_stuff->check_and_die(info->m_eOldState != k_ESteamNetworkingConnectionState_Connecting,
                                                     "Invalid connection state?")) {
                    return;
                } else {
                    std::cout << "[NetStuff] Some dude failed to connect." << std::endl;
                }
            }
            current_net_stuff->interface->CloseConnection(*client, 0, nullptr, false);
            break;
        }
        // TODO: other connection states
    }
}

NetStuff::NetStuff() : valid(false), err_reason() {
    listen_addr.Clear();
    listen_addr.m_port = default_port;
    SteamDatagramErrMsg err_msg;
    if (!GameNetworkingSockets_Init(nullptr, err_msg)) {
        valid = false;
        err_reason = err_msg;
        return;
    }
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
}

NetStuff::~NetStuff() {
    for (auto client : connections) {
        interface->CloseConnection(client, 0, nullptr, false);
    }
    interface->DestroyPollGroup(poll_group);
    interface->CloseListenSocket(listen_socket);
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
        std::cout << "[NetStuff] Received message: " << message << std::endl;
        incoming_message->Release();
        // TODO: handle message
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
