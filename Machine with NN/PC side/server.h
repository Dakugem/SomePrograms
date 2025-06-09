#pragma once

#include "WinSock2.h"
#include "WS2tcpip.h"

#include <vector>
#include <cstdint>

namespace Socket_IO{
    class Client {
public:
    bool isOkay;
    SOCKET clientSock;
    Client();
    Client(SOCKET servSock);
    ~Client();
};
class Server {
    PCSTR SERVER_IP;
    size_t SERVER_PORT;
    size_t BUFF_SIZE;
    WSADATA wsData;
    SOCKET servSock;
    Client client;
    short packet_size;

    int SocketBind();
    int Listening();
public:
    Server();
    bool Setup(PCSTR server_ip = "127.0.0.1", size_t server_port = 65000, size_t buff_size = 64);
    int ConnectToClient();
    int recvMessage(std::vector<uint8_t> &msg);
    int sendMessage(const std::vector<uint8_t> msg);

    bool isClientOkay();
    ~Server();
};
}