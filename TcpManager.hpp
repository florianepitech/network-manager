//
// Created by Florian Damiot on 13/02/2023.
//

#pragma once

#include "./NewNetworkManager.hpp"
#include "./EventRegistry.hpp"

#include <iostream>
#include <string>
#include <unordered_map>
#include <mutex>
#include <arpa/inet.h>
#include <unistd.h>
#include <future>
#include <utility>
#include <memory>
#include <vector>

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
            std::cout << "Creating TCP manager on " << _host << ":" << _port << std::endl;
        }

        ~TcpManager()
        {
            std::cout << "Destroying TCP manager" << std::endl;
        }

        void start() {
            if (_started)
                throw std::runtime_error("TcpManager already started");
            _started = true;

            std::cout << "Starting TCP server on " << _host << ":" << _port << "..." << std::endl;
            _serverSocket = socket(AF_INET, SOCK_STREAM, 0);
            if (_serverSocket == -1)
                throw std::runtime_error("Failed to create socket for TCP server");
            std::cout << "Created socket for TCP server" << std::endl;
            sockaddr_in sockaddr{};
            sockaddr.sin_family = AF_INET;
            sockaddr.sin_addr.s_addr = _host == "localhost" ? INADDR_ANY : inet_addr(_host.c_str());
            sockaddr.sin_port = htons(_port);
            if (bind(_serverSocket, (struct sockaddr *) &sockaddr, sizeof(sockaddr)) < 0)
                throw std::runtime_error("Failed to bind socket for TCP server, port already in use");
            std::cout << "Bound socket for TCP server" << std::endl;
            if (listen(_serverSocket, 3) < 0)
                throw std::runtime_error("Failed to listen on socket for TCP server");
            std::cout << "Listening on socket for TCP server with socket" << _serverSocket << "..." << std::endl;
            startAcceptConnectionAsync();
            sleep(100);
            std::cout << "All TCP server started" << std::endl;
        }

        void stop() {
            if (!_started)
                throw std::runtime_error("TcpManager is not started");
            // lock with threadLock
            std::lock_guard<std::mutex> lock(_threadsMutex);
            // set started to false
            _started = false;
            // close server socket
            close(_serverSocket);
            // clear connections
            _clients.clear();
            // release lock
            _threadsMutex.unlock();
        }

        // template<typename EventType>
        // ssize_t sendEvent(int socket, const int eventId, const EventType event)
        // {
        //     std::vector<std::byte> data = _eventRegistry.serializeData(event);
        //     ssize_t result = send(socket, reinterpret_cast<const char*>(data.data()), data.size(), 0);
        //     if (result == -1) {
        //         std::cerr << "Error: Failed to send message to client" << std::endl;
        //     }
        //     std::cout << "Sent message to client with " << result << " bytes" << std::endl;
        //     return (result);
        // }

        // template<class EventType>
        // void broadcastExcept(int socketExcept, unsigned int eventId, EventType &event)
        // {
        //     std::vector<std::byte> data = _eventRegistry.serializeData(event);
        //     // get list of all key of _connections
        //     std::vector<int> keys;
        //     for (auto const& [key, val] : _connections) {
        //         if (key != socketExcept)
        //             sendEvent(key, eventId, event);
        //     }
        // }

        // template<typename EvenType>
        // void broadcast(unsigned int eventId, EvenType event)
        // {
        //     std::vector<std::byte> data = _eventRegistry.serializeData(event);
        //     // get list of all key of _connections
        //     std::vector<int> keys;
        //     for (auto const& [key, val] : _connections) {
        //         sendEvent(key, eventId, event);
        //     }
        // }

        // Set the handler

        void setOnClientConnectEvent(const std::function<void(const NetClient&)> &onClientConnectEvent)
        {
            _onConnectHandler = std::make_shared<std::function<void(const NetClient&)>>(onClientConnectEvent);
        }

        void setOnClientDisconnectEvent(const std::function<void(const NetClient&)> &onClientDisconnectEvent)
        {
            _onDisconnectHandler = std::make_shared<std::function<void(const NetClient&)>>(onClientDisconnectEvent);
        }

    // For private methods only
    private:

        void startAcceptConnectionAsync() {

            std::thread t([] {
                std::cout << "Starting accept thread" << std::endl;
                while (true) {
                    std::cout << "Waiting for incoming connection..." << std::endl;
                }
                //while (true) {
                //    sockaddr_in client_address{};
                //    socklen_t client_address_size = sizeof(client_address);
                //    int client_socket = accept(servSock, reinterpret_cast<sockaddr *>(&client_address), &client_address_size);
                //    if (client_socket == -1) {
                //        std::cerr << "Error: Failed to accept incoming connection" << std::endl;
                //        continue;
                //    }
                //    std::cout << "Accepted connection from " << inet_ntoa(client_address.sin_addr) << ":" << ntohs(client_address.sin_port) << std::endl;
                //    // Start read thread
                //    std::thread t([&client_socket, this] { handleIncomingMessagesHandler(client_socket); });
                //}
            });
            std::cout << "Started accept thread with thread id " << t.get_id() << std::endl;
        }

        void handleIncomingMessagesHandler(const int sock) const {
            std::cout << "Starting message handler thread for " << sock << std::endl;
            while (true) {
                char buffer[BUFFER_SIZE] = {0};
                const ssize_t recv_result = recv(sock, buffer, BUFFER_SIZE, 0);
                if (recv_result <= 0)
                    break;
                std::cout << "Received " << recv_result << " bytes from " << sock << std::endl;
            }

            // Remove the disconnected connection from the connections map
            //onDisconnectClient(netClient);
        }

        /**
         * This method is called when the server recives a new client.
         * This method add the client to the list of clients and call the onConnectClient method.
         * @param client
         */
        void onConnectClient(const NetClient &client)
        {
            std::unique_lock<std::mutex> lock(_clientsMutex);
            //_clients.push_back(client);
            lock.unlock();
            if (_onConnectHandler)
                (*_onConnectHandler)(client);
        }

        /**
         * This method is called when the server disconnect a client.
         * This method remove the client from the list of clients and call the onDisconnectClient method.
         * @param client
         */
        void onDisconnectClient(const NetClient &client)
        {
            std::unique_lock<std::mutex> lock(_clientsMutex);
            auto erased = _clients.erase(std::remove(_clients.begin(), _clients.end(), client), _clients.end());
            if (erased == _clients.end())
                std::cerr << "Error: Failed to remove client from list" << std::endl;
            int result = close(client.socket);
            if (result == -1)
                std::cerr << "Error: Failed to close socket" << std::endl;
            lock.unlock();
            if (_onDisconnectHandler)
                (*_onDisconnectHandler)(client);
        }

    // For private variables only
    private:
        // Constructor parameters
        std::string _host;
        unsigned int _port;

        // Internal state
        bool _started = false;
        int _serverSocket = -1;

        std::vector<NetClient> _clients{};
        std::mutex _clientsMutex;

        std::mutex _threadsMutex;

        // Event from Game
        EventRegistry _eventRegistry;

        // Event from network
        std::shared_ptr<std::function<void(const NetClient&)>> _onConnectHandler = nullptr;
        std::shared_ptr<std::function<void(const NetClient&)>> _onDisconnectHandler = nullptr;

};
