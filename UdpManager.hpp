//
// Created by Florian Damiot on 13/02/2023.
//

#pragma once

#include "./TcpManager.hpp"
#include <iostream>
#include <utility>
#include <vector>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <chrono>
#include <ctime>
#include <algorithm>
#include <functional>

class UdpManager {
    public:
        UdpManager(std::string host,
                   const unsigned int port,
                   const std::shared_ptr<TcpManager> &tcpManager)
                  : _host(std::move(host)), _port(port), _tcpManager(tcpManager) {

        }
        ~UdpManager() = default;

        void start() {

            int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            if (sock < 0) {
                throw std::runtime_error("Error: Failed to create socket for UDP server");
            }

            sockaddr_in server_address{};
            server_address.sin_family = AF_INET;
            server_address.sin_addr.s_addr = (_host == "localhost") ? INADDR_ANY : inet_addr(_host.c_str());
            server_address.sin_port = htons(_port);

            if (bind(sock, reinterpret_cast<sockaddr *>(&server_address), sizeof(server_address)) < 0) {
                close(sock);
                throw std::runtime_error("Error: Failed to bind socket for UDP server, port already in use");
            }

            std::cout << "UDP server started on port " << _port << std::endl;

            std::thread t([this, &sock] { startReceive(sock); });
            t.detach();
        }

    private:
        void startReceive(const int &socket) {
            std::cout << "Start receiving UDP packets" << std::endl;
            while (true) {
                sockaddr_in client_address{};
                socklen_t client_address_size = sizeof(client_address);

                std::vector<std::byte> buffer(1024);
                int result = recvfrom(socket, reinterpret_cast<char*>(buffer.data()), buffer.size(), 0, reinterpret_cast<sockaddr*>(&client_address), &client_address_size);
                if (result == -1) {
                    // handle error
                    continue;
                }

                std::cout << "Received event from UDP " << inet_ntoa(client_address.sin_addr) << ":" << ntohs(client_address.sin_port) << ": ";
                for (int i = 0; i < result; i++) {
                    std::cout << std::hex << static_cast<int>(buffer[i]) << " ";
                }

            }
        }

        template<typename EventType>
        void send(unsigned int eventId, const EventType &event) {
            std::vector<std::byte> dataBytes = _eventRegistry.serializeData(event);

        }

    private:
        std::shared_ptr<TcpManager> _tcpManager;
        std::string _host;
        unsigned int _port;
        EventRegistry _eventRegistry;
};
