//
// Created by Florian Damiot on 13/02/2023.
//

#pragma once

#define BUFFER_SIZE 4096

#include <string>
#include "./uuid.hpp"
#include "./TcpManager.hpp"
#include "./UdpManager.hpp"

class NewNetworkManager {
    public:
        NewNetworkManager(const std::string &host,
                          const unsigned int portTcp,
                          const unsigned int portUdp) : _host(host), _portTcp(portTcp), _portUdp(portUdp)
        {

        };
        ~NewNetworkManager() = default;

        void start() {
            std::cout << "Starting network manager..." << std::endl;
            _tcpManager = std::make_shared<TcpManager>(_host, _portTcp);
            _udpManager = std::make_shared<UdpManager>(_host, _portUdp, _tcpManager);
            getTcpManager().start();
            getUdpManager().start();
            std::cout << "Network manager started for UDP and TCP mode" << std::endl;
        }

        // Getters

        TcpManager &getTcpManager() {
            if (_tcpManager == nullptr)
                throw std::runtime_error("TcpManager is not initialized, did you call start() ?");
            return *_tcpManager;
        }

        UdpManager &getUdpManager() {
            if (_udpManager == nullptr)
                throw std::runtime_error("UdpManager is not initialized, did you call start() ?");
            return *_udpManager;
        }

    private:
        // Constructor variables
        std::string _host;
        unsigned int _portTcp;
        unsigned int _portUdp;
        // Variables
        std::shared_ptr<TcpManager> _tcpManager = nullptr;
        std::shared_ptr<UdpManager> _udpManager = nullptr;
};
