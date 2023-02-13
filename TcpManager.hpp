//
// Created by Florian Damiot on 13/02/2023.
//

#pragma once

#include "./NewNetworkManager.hpp"
#include "./EventRegistry.hpp"

#include <string>
#include <unordered_map>
#include <mutex>
#include <arpa/inet.h>
#include <unistd.h>
#include <future>
#include <utility>

class TcpManager {
    public:

        /**
         * @brief Construct a new Tcp Manager object.
         * @param host The host address to bind the socket to.
         * @param port The port to bind the socket to.
         */
        TcpManager(std::string host,
                   const unsigned int &port) :
                   _host(std::move(host)), _port(port) {

        }

        ~TcpManager()
        {
            if (started) {
                std::cout << "Stopping TCP server..." << std::endl;
                close(_serverSocket);
                std::cout << "Stopped TCP server" << std::endl;
            }
        }

        void start() {
            if (started)
                throw std::runtime_error("TcpManager already started");
            started = true;
            std::cout << "Starting TCP server on " << _host << ":" << _port << std::endl;
            _serverSocket = socket(AF_INET, SOCK_STREAM, 0);
            if (_serverSocket == -1) {
                throw std::runtime_error("Failed to create socket for TCP server");
            }
            std::cout << "Created socket for TCP server" << std::endl;
            sockaddr_in sockaddr{};
            sockaddr.sin_family = AF_INET;
            sockaddr.sin_addr.s_addr = _host == "localhost" ? INADDR_ANY : inet_addr(_host.c_str());
            sockaddr.sin_port = htons(_port);
            if (bind(_serverSocket, (struct sockaddr *) &sockaddr, sizeof(sockaddr)) < 0) {
                throw std::runtime_error("Failed to bind socket for TCP server, port already in use");
            }
            std::cout << "Bound socket for TCP server" << std::endl;
            if (listen(_serverSocket, 3) < 0) {
                throw std::runtime_error("Failed to listen on socket for TCP server");
            }
            std::cout << "Listening on socket for TCP server" << std::endl;
            std::thread t([this] { startAcceptHandler(_serverSocket); });
            t.detach();
        }

        template<typename EventType>
        ssize_t sendEvent(int socket, const int eventId, const EventType event)
        {
            std::vector<std::byte> data = _eventRegistry.serializeData(event);
            ssize_t result = send(socket, reinterpret_cast<const char*>(data.data()), data.size(), 0);
            if (result == -1) {
                std::cerr << "Error: Failed to send message to client" << std::endl;
            }
            std::cout << "Sent message to client with " << result << " bytes" << std::endl;
            return (result);
        }

        template<class EventType>
        void broadcastExcept(int socketExcept, unsigned int eventId, EventType &event)
        {
            std::vector<std::byte> data = _eventRegistry.serializeData(event);
            // get list of all key of _connections
            std::vector<int> keys;
            for (auto const& [key, val] : _connections) {
                if (key != socketExcept)
                    sendEvent(key, eventId, event);
            }
        }

        template<typename EvenType>
        void broadcast(unsigned int eventId, EvenType event)
        {
            std::vector<std::byte> data = _eventRegistry.serializeData(event);
            // get list of all key of _connections
            std::vector<int> keys;
            for (auto const& [key, val] : _connections) {
                sendEvent(key, eventId, event);
            }
        }

    // For private methods only
    private:

        void startAcceptHandler(const int &serverSocket) {
            std::cout << "Starting accept thread" << std::endl;
            while (true) {
                sockaddr_in client_address{};
                socklen_t client_address_size = sizeof(client_address);

                int client_socket = accept(serverSocket, reinterpret_cast<sockaddr *>(&client_address), &client_address_size);
                if (client_socket == -1) {
                    std::cerr << "Error: Failed to accept incoming connection" << std::endl;
                    continue;
                }

                std::cout << "Accepted connection from " << inet_ntoa(client_address.sin_addr) << ":" << ntohs(client_address.sin_port) << std::endl;

                // Add the new connection to the connections map
                std::unique_lock<std::mutex> lock(_connectionsMutex);
                std::string uuid = generateRandomUuid();
                std::pair<std::string, sockaddr_in> clientInfo(uuid, client_address);
                _connections[client_socket] = clientInfo;
                lock.unlock();

                // Start a new thread to handle incoming messages on this connection
                std::thread t([this, client_socket] { handleIncomingMessagesHandler(client_socket); });
                t.detach();
            }
        }

        void handleIncomingMessagesHandler(const int &socket) {
            std::pair<std::string, sockaddr_in> clientInfo = _connections[socket];
            // get the client ip address
            std::string clientIp = inet_ntoa(clientInfo.second.sin_addr);
            // get the client port
            int clientPort = ntohs(clientInfo.second.sin_port);
            std::cout << "Starting message handler thread for " << clientIp << ":" << clientPort << std::endl;
            while (true) {
                char buffer[BUFFER_SIZE] = {0};
                ssize_t recv_result = recv(socket, buffer, BUFFER_SIZE, 0);
                if (recv_result <= 0) {
                    break;
                }
                std::cout << "Received " << recv_result << " bytes from " << clientIp << ":" << clientPort << std::endl;
            }

            // Remove the disconnected connection from the connections map
            std::unique_lock<std::mutex> lock(_connectionsMutex);
            _connections.erase(socket);
            lock.unlock();

            close(socket);
        }

    // For private variables only
    private:
        std::string _host;
        unsigned int _port;

        bool started = false;
        int _serverSocket = -1;
        std::unordered_map<int, std::pair<std::string, sockaddr_in>> _connections{};
        std::mutex _connectionsMutex;
        EventRegistry _eventRegistry;
};
