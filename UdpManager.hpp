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
#include <sys/socket.h>

class UdpManager {
    public:
        UdpManager(std::string host,
                   const unsigned int port,
                   const std::shared_ptr<TcpManager> &tcpManager)
                  : _host(std::move(host)), _port(port), _tcpManager(tcpManager) {

        }
        ~UdpManager() = default;

        void start() {

            int sockfd;
            struct sockaddr_in servaddr, cliaddr;

            // Creating socket file descriptor
            if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
                perror("socket creation failed");
                exit(EXIT_FAILURE);
            }

            memset(&servaddr, 0, sizeof(servaddr));
            memset(&cliaddr, 0, sizeof(cliaddr));

            // Filling server information
            servaddr.sin_family    = AF_INET; // IPv4
            servaddr.sin_addr.s_addr = INADDR_ANY;
            servaddr.sin_port = htons(_port);

            // Bind the socket with the server address
            if ( bind(sockfd, (const struct sockaddr *)&servaddr,
                      sizeof(servaddr)) < 0 )
            {
                perror("bind failed");
                exit(EXIT_FAILURE);
            }

            std::thread t([this, &cliaddr, &sockfd] { startReceive(cliaddr, sockfd); });
            t.detach();
        }

    private:
        void startReceive(sockaddr_in cliaddr, const int &sockfd) {
            std::cout << "Start receiving UDP packets" << std::endl;
            while (true) {
                socklen_t len;
                int n;

                len = sizeof(cliaddr);  //len is value/result

                const char *hello = "Hello from server";
                char buffer[BUFFER_SIZE];
                n = recvfrom(sockfd, (char *)buffer, BUFFER_SIZE, MSG_WAITALL, ( struct sockaddr *) &cliaddr, &len);
                buffer[n] = '\0';
                printf("Client : %s\n", buffer);
                sendto(sockfd, (const char *)hello, strlen(hello), MSG_CONFIRM, (const struct sockaddr *) &cliaddr,
                       len);
                std::cout<<"Hello message sent."<<std::endl;
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
