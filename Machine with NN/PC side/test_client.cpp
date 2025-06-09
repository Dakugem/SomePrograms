#include <iostream>
#include <stdio.h>
#include <iomanip>
#include <vector>
#include <chrono>

#include "WinSock2.h"
#include "WS2tcpip.h"

// #pragma comment(lib, "ws2_32")

using namespace std;
using namespace std::chrono;

void writeVector(vector<uint8_t> vec)
{
    for (uint8_t el : vec)
    {
        cout << +el << " ";
    }
}

class Client
{
    PCSTR SERVER_IP = "127.0.0.1";
    const int SERVER_PORT = 65000;
    const short BUFF_SIZE = 64;
    WSADATA wsData;
    SOCKET clientSock;
    short packet_size;

public:
    Client();
    int ConnectToServer();
    int sendMessage(const std::vector<uint8_t> msg);
    int recvMessage(std::vector<uint8_t> &msg);
    ~Client();
};

int main()
{
    vector<uint8_t> msg_to, msg_from;
    Client client = Client();
    if (client.ConnectToServer())
    {
        system("pause");
        return 1;
    }

    msg_to.push_back(0);

    int64_t duration;
    int16_t AccelGyro[6];
    while (true)
    {
        if (client.recvMessage(msg_from))
        {
            system("pause");
            return 1;
        }
        
        duration = 0;
        for (size_t i = 0; i < 8; i++)
        {
            duration |= (int64_t)msg_from.at(i) << (8 * (7 - i));
        }
        cout << setw(16) << "Server message: ";
        cout << "delta = " << setw(6) << duration << " ";
        cout << "AG: ";
        for (size_t i = 0; i < 6; i++)
        {
            AccelGyro[i] = (msg_from[2 * i + 8] << 8) | msg_from[2 * i + 1 + 8];
            double temp = (i < 3) ? (double)(AccelGyro[i] * 3.14 / 180.) : (double)(AccelGyro[i] / 16384.);
            cout << setw(10) << AccelGyro[i] << " ";
        }
        cout << endl;

        if(msg_from.at(20) == 0xFF){
            cout << +msg_from.at(20);
            return 0;
        }

        ++msg_to[0];
        if (client.sendMessage(msg_to))
        {
            system("pause");
            return 1;
        }
    }
}

Client::Client()
{
    packet_size = 0;
    wsData = WSADATA();
    clientSock = INVALID_SOCKET;
}

int Client::ConnectToServer()
{
    in_addr ip{0};
    sockaddr_in servInfo;

    if (inet_pton(AF_INET, SERVER_IP, &ip) <= 0)
    {
        cout << "Incorrect IP" << endl;
        return 1;
    }

    if (WSAStartup(MAKEWORD(2, 2), &wsData) != 0)
    {
        cout << "WinSock initialization FAILED. Error ";
        cout << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

    clientSock = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSock == INVALID_SOCKET)
    {
        cout << "Client socket initialization FAILED. Error ";
        cout << WSAGetLastError() << endl;
        closesocket(clientSock);
        WSACleanup();
        return 1;
    }

    ZeroMemory(&servInfo, sizeof(servInfo));
    servInfo.sin_family = AF_INET;
    servInfo.sin_addr = ip;
    servInfo.sin_port = htons(SERVER_PORT);
    if (connect(clientSock, (sockaddr *)&servInfo, sizeof(servInfo)) != 0)
    {
        cout << "Connection FAILED. Error ";
        cout << WSAGetLastError() << endl;
        closesocket(clientSock);
        WSACleanup();
        return 1;
    }

    return 0;
}

int Client::sendMessage(const std::vector<uint8_t> msg)
{
    if (msg.size() > BUFF_SIZE)
        std::cout << "MSG is truncated to BUFF_SIZE" << std::endl;
    packet_size = send(clientSock, (char *)msg.data(), (msg.size() > BUFF_SIZE) ? BUFF_SIZE : msg.size(), 0);

    if (packet_size == SOCKET_ERROR)
    {
        cout << "Can't send message to Server. Error ";
        cout << WSAGetLastError() << endl;
        closesocket(clientSock);
        WSACleanup();
        return 1;
    }

    return 0;
}

int Client::recvMessage(std::vector<uint8_t> &msg)
{
    //msg.clear();
    if (msg.size() < BUFF_SIZE)
        msg.resize(BUFF_SIZE);
    packet_size = recv(clientSock, (char *)msg.data(), msg.size(), 0);

    if (packet_size == SOCKET_ERROR)
    {
        cout << "Can't receive message from Server. Error ";
        cout << WSAGetLastError() << endl;
        closesocket(clientSock);
        WSACleanup();
        return 1;
    }

    // cout << setw(16) << "Server message: ";
    // writeVector(msg);
    // cout << endl;

    return 0;
}

Client::~Client()
{
    closesocket(clientSock);
    WSACleanup();
}
