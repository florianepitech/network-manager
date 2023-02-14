#include <string>
#include "./NewNetworkManager.hpp"

int main() {

    // set random seed
    TcpManager tcpManager("127.0.0.1", 4242);
    tcpManager.start();

    // creating std::thread to print "Hello World every seconds"

    // sleep 5 seconds
    sleep(100);

    std::cout << "Stopping TCP server..." << std::endl;
    //tcpManager.stop();

    return 0;
}
