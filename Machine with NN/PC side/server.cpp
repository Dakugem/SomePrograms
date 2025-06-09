#include "server.h"

#include "WinSock2.h"
#include "WS2tcpip.h"

#include <iostream>
#include <stdio.h>
#include <iomanip>

namespace Socket_IO
{
    Server::Server()
    {
        packet_size = 0;
        wsData = WSADATA();
        servSock = INVALID_SOCKET;
        client = Client();
    }
    int Server::SocketBind()
    {
        in_addr ip{0};
        sockaddr_in servInfo;

        if (inet_pton(AF_INET, SERVER_IP, &ip) <= 0)
        {
            std::cout << "Incorrect IP" << std::endl;
            return 1;
        }

        if (WSAStartup(MAKEWORD(2, 2), &wsData) != 0)
        {
            std::cout << "WinSock initialization FAILED. Error ";
            std::cout << WSAGetLastError() << std::endl;
            WSACleanup();
            return 1;
        }

        servSock = socket(AF_INET, SOCK_STREAM, 0);
        if (servSock == INVALID_SOCKET)
        {
            std::cout << "Server socket initialization FAILED. Error ";
            std::cout << WSAGetLastError() << std::endl;
            closesocket(servSock);
            WSACleanup();
            return 1;
        }

        ZeroMemory(&servInfo, sizeof(servInfo));
        servInfo.sin_family = AF_INET;
        servInfo.sin_addr = ip;
        servInfo.sin_port = htons(SERVER_PORT);
        if ((bind(servSock, (sockaddr *)&servInfo, sizeof(servInfo))) != 0)
        {
            std::cout << "Socket binging FAILED. Error ";
            std::cout << WSAGetLastError() << std::endl;
            closesocket(servSock);
            WSACleanup();
            return 1;
        }

        return 0;
    }
    int Server::Listening()
    {
        if (listen(servSock, SOMAXCONN) != 0)
        {
            std::cout << "Listening FAILED. Error ";
            std::cout << WSAGetLastError() << std::endl;
            closesocket(servSock);
            WSACleanup();
            return 1;
        }

        return 0;
    }
    bool Server::Setup(PCSTR server_ip, size_t server_port, size_t buff_size)
    {
        this->SERVER_IP = server_ip;
        this->SERVER_PORT = server_port;
        this->BUFF_SIZE = buff_size;

        if (SocketBind() || Listening())
            return false;

        return true;
    }
    int Server::ConnectToClient()
    {
        client = Client(servSock);

        if (client.isOkay == false)
        {
            closesocket(servSock);
            WSACleanup();
            return 1;
        }

        return 0;
    }
    int Server::recvMessage(std::vector<uint8_t> &msg)
    {
        if(msg.size() < BUFF_SIZE) msg.resize(BUFF_SIZE);
        packet_size = recv(client.clientSock, (char *)msg.data(), msg.size(), 0);

        if (packet_size == SOCKET_ERROR)
        {
            std::cout << "Can't receive message from Client. Error ";
            std::cout << WSAGetLastError() << std::endl;
            client.isOkay = false;
            //closesocket(servSock);
            //closesocket(client.clientSock);
            //WSACleanup();
            return 1;
        }

        msg;

        return 0;
    }
    int Server::sendMessage(const std::vector<uint8_t> msg)
    {
        if(msg.size() > BUFF_SIZE) std::cout << "MSG is truncated to BUFF_SIZE" << std::endl;
        packet_size = send(client.clientSock, (char *)msg.data(), (msg.size() > BUFF_SIZE)? BUFF_SIZE : msg.size(), 0);

        if (packet_size == SOCKET_ERROR)
        {
            std::cout << "Can't send message to Client. Error ";
            std::cout << WSAGetLastError() << std::endl;
            client.isOkay = false;
            //closesocket(servSock);
            //closesocket(client.clientSock);
            //WSACleanup();
            return 1;
        }

        return 0;
    }
    bool Server::isClientOkay()
    {
        return client.isOkay;
    }
    Server::~Server()
    {
        closesocket(servSock);
        WSACleanup();
    }

    Client::Client()
    {
        isOkay = false;
        clientSock = INVALID_SOCKET;
    }
    Client::Client(SOCKET servSock)
    {
        sockaddr_in clientInfo;
        int clientInfo_size = sizeof(clientInfo);
        isOkay = false;

        ZeroMemory(&clientInfo, sizeof(clientInfo));

        clientSock = accept(servSock, (sockaddr *)&clientInfo, &clientInfo_size);
        if (clientSock == INVALID_SOCKET)
        {
            std::cout << "Client accept FAILED. Error ";
            std::cout << WSAGetLastError() << std::endl;
            closesocket(clientSock);
            isOkay = false;
        }
        else
        {
            char clientIP[22];
            inet_ntop(AF_INET, &clientInfo.sin_addr, clientIP, INET_ADDRSTRLEN);
            std::cout << "Client connected with IP: " << clientIP << std::endl;
        }

        isOkay = true;
    }
    Client::~Client()
    {
    }
}