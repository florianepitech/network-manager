//
// Created by Florian Damiot on 14/02/2023.
//

#pragma once

#define BUFFER_SIZE 4096
#define MSG_CONFIRM 0

#include <arpa/inet.h>
#include <unistd.h>
#include <vector>
#include <thread>

struct NetClient {
    std::string uuid;

    int socket;
    std::string ip;
    unsigned int port;
    sockaddr_in address;

    std::thread receiveThread;

    bool operator==(const NetClient &other) const {
        return uuid == other.uuid;
    }

};

struct NetPacket {
    int packetId;
    std::vector<std::byte> data;
};
