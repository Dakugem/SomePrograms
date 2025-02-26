#include <iostream>
#include <stdio.h>
#include <vector>
#include <iomanip>

#include "WinSock2.h"
#include "WS2tcpip.h"

//#pragma comment(lib, "Ws2_32.lib")

using namespace std;

class Client {
    int BUFF_SIZE = 1024;
public:
    bool isOkay;
    vector<char> clientBuff;
    SOCKET clientSock;
    Client();
    Client(SOCKET servSock);
    ~Client();
};
class Server {
    PCSTR SERVER_IP = "127.0.0.1";
    const int   SERVER_PORT = 1234;
    const short BUFF_SIZE = 1024;
    vector<char> servBuff;
    WSADATA wsData;
    SOCKET servSock;
    Client client;
    short packet_size;
public:
    Server();
    int SocketBind();
    int Listening();
    int ConnectToClient();
    int recvMessage();
    int sendMessage();
    ~Server();
};

int main() {
    Server server = Server();
    if (server.SocketBind()) {
        system("pause");
        return 1;
    }
    if (server.Listening()) {
        system("pause");
        return 1;
    }
    if (server.ConnectToClient()) {
        system("pause");
        return 1;
    }
    
    while (true) {
        if (server.recvMessage()) {
            system("pause");
            return 1;
        }
        if (server.sendMessage()) {
            system("pause");
            return 1;
        }
    }
}

Server::Server() {
    packet_size = 0;
    wsData = WSADATA();
    servSock = INVALID_SOCKET;
    servBuff = vector<char>(BUFF_SIZE);
}
int Server::SocketBind() {
    in_addr ip{ 0 };
    sockaddr_in servInfo;

    if (inet_pton(AF_INET, SERVER_IP, &ip) <= 0) {
        cout << "Incorrect IP" << endl;
        return 1;
    }

    if (WSAStartup(MAKEWORD(2, 2), &wsData) != 0) {
        cout << "WinSock initialization FAILED. Error ";
        cout << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

    servSock = socket(AF_INET, SOCK_STREAM, 0);
    if (servSock == INVALID_SOCKET) {
        cout << "Server socket initialization FAILED. Error ";
        cout << WSAGetLastError() << endl;
        closesocket(servSock);
        WSACleanup();
        return 1;
    }

    ZeroMemory(&servInfo, sizeof(servInfo));
    servInfo.sin_family = AF_INET;
    servInfo.sin_addr = ip;
    servInfo.sin_port = htons(SERVER_PORT);
    if ((bind(servSock, (sockaddr*)&servInfo, sizeof(servInfo))) != 0) {
        cout << "Socket binging FAILED. Error ";
        cout << WSAGetLastError() << endl;
        closesocket(servSock);
        WSACleanup();
        return 1;
    }

    return 0;
}
int Server::Listening() {
    if (listen(servSock, SOMAXCONN) != 0) {
        cout << "Listening FAILED. Error ";
        cout << WSAGetLastError() << endl;
        closesocket(servSock);
        WSACleanup();
        return 1;
    }

    return 0;
}
int Server::ConnectToClient() {
    client = Client(servSock);

    if (client.isOkay == false) {
        closesocket(servSock);
        WSACleanup();
        return 1;
    }

    return 0;
}
int Server::recvMessage() {
    packet_size = recv(client.clientSock, servBuff.data(), servBuff.size(), 0);

    if (packet_size == SOCKET_ERROR) {
        cout << "Can't receive message from Client. Error ";
        cout << WSAGetLastError() << endl;
        closesocket(servSock);
        closesocket(client.clientSock);
        WSACleanup();
        return 1;
    }

    if ((servBuff[0] == 'e' || servBuff[0] == 'E') && servBuff[1] == 'x' && servBuff[2] == 'i' && servBuff[3] == 't') {
        cout << "Connection closed by Client" << endl;
        shutdown(client.clientSock, SD_BOTH);
        closesocket(servSock);
        closesocket(client.clientSock);
        WSACleanup();
        return 1;
    }

    cout << setw(16) << "Client message: " << servBuff.data() << endl;

    return 0;
}
int Server::sendMessage() {
    cout << setw(16) << "Your message: ";
    fgets(client.clientBuff.data(), client.clientBuff.size(), stdin);

    packet_size = send(client.clientSock, client.clientBuff.data(), client.clientBuff.size(), 0);

    if (packet_size == SOCKET_ERROR) {
        cout << "Can't send message to Client. Error ";
        cout << WSAGetLastError() << endl;
        closesocket(servSock);
        closesocket(client.clientSock);
        WSACleanup();
        return 1;
    }

    if ((client.clientBuff[0] == 'e' || client.clientBuff[0] == 'E') && client.clientBuff[1] == 'x' && client.clientBuff[2] == 'i' && client.clientBuff[3] == 't') {
        cout << "Connection closed" << endl;
        shutdown(client.clientSock, SD_BOTH);
        closesocket(servSock);
        closesocket(client.clientSock);
        WSACleanup();
        return 1;
    }

    return 0;
}
Server::~Server() {
    closesocket(servSock);
    WSACleanup();
    servBuff.clear();
}

Client::Client() {
    isOkay = true;
    clientSock = INVALID_SOCKET;
}
Client::Client(SOCKET servSock) {
    clientBuff = vector<char>(BUFF_SIZE);
    sockaddr_in clientInfo;
    int clientInfo_size = sizeof(clientInfo);
    isOkay = true;

    ZeroMemory(&clientInfo, sizeof(clientInfo));

    clientSock = accept(servSock, (sockaddr*)&clientInfo, &clientInfo_size);
    if (clientSock == INVALID_SOCKET) {
        cout << "Client accept FAILED. Error ";
        cout << WSAGetLastError() << endl;
        closesocket(clientSock);
        isOkay = false;
    }
    else {
        char clientIP[22];
        inet_ntop(AF_INET, &clientInfo.sin_addr, clientIP, INET_ADDRSTRLEN);
        cout << "Client connected with IP: " << clientIP << endl;
    }
}
Client::~Client() {
    clientBuff.clear();
}