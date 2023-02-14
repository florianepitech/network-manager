
#include <string>

class NetworkManagerServer {

    public:
        NetworkManagerServer(std::string host, unsigned int port);

    public:
        template<typename EvenType>
        broadcast(const EvenType &event);

        template<typename EvenType>
        sendTo(const std::string &uuid, const EvenType &event);

        template<typename EvenType>
        broadcastExcept(const std::string &uuid, const EvenType &event);

    private:
        void dataReceived(const unsigned int packetId, const std::vector<std::byte> &data);
};

class NetworkManagerClient {
    public:
        NetworkManagerClient(std::string host, unsigned int port);

    public:
        template<typename EvenType>
        send(const EvenType &event);

    private:
        void dataReceived(const unsigned int packetId, const std::vector<std::byte> &data);
};
