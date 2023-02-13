#include <iostream>
#include <time.h>
#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "./NewNetworkManager.hpp"

void sendStringUdp(const std::string& message) {
    // Create the socket
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == -1) {
        // handle error
        return;
    }

    // Set up the destination address
    sockaddr_in destination_address{};
    destination_address.sin_family = AF_INET;
    destination_address.sin_port = htons(4243);
    if (inet_pton(AF_INET, "127.0.0.1", &destination_address.sin_addr) != 1) {
        // handle error
        close(sock);
        return;
    }

    // Send the message
    int result = sendto(sock, message.c_str(), message.length(), 0, reinterpret_cast<sockaddr*>(&destination_address), sizeof(destination_address));
    if (result == -1) {
        // handle error
    }

    // Clean up
    close(sock);
}

int main() {

    NewNetworkManager networkManager("127.0.0.1", 4242, 4243);
    networkManager.start();

    for (int i = 0; i < 100; i++) {
        std::cout << "Sleeping for 10 seconds..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
        sendStringUdp("Hello world");
        std::string message = "Hello world";
        networkManager.getTcpManager().broadcast(1, message);
        std::cout << "End of main" << std::endl;
    }

    // send message to localhost 4243 (UDP)



    return 0;
}
