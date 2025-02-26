#include <iostream>
#include <stdio.h>
#include <iomanip>
#include <vector>

#include "WinSock2.h"
#include "WS2tcpip.h"

//#pragma comment(lib, "ws2_32")

using namespace std;

class Client {
    PCSTR SERVER_IP = "127.0.0.1";
    const int SERVER_PORT = 1234;
    const short BUFF_SIZE = 1024;
    vector<char> servBuff;
    vector<char> clientBuff;
    WSADATA wsData;
    SOCKET clientSock;
    short packet_size;

public:
    Client();
    int ConnectToServer();
    int sendMessage();
    int recvMessage();
    ~Client();
};

int main() {
    Client client = Client();
    if (client.ConnectToServer()) {
        system("pause");
        return 1;
    }

    while (true) {
        if (client.sendMessage()) {
            system("pause");
            return 1;
        }
        if (client.recvMessage()) {
            system("pause");
            return 1;
        }
    }
}

Client::Client() {
    packet_size = 0;
    wsData = WSADATA();
    clientSock = INVALID_SOCKET;
    servBuff = vector<char>(BUFF_SIZE);
    clientBuff = vector<char>(BUFF_SIZE);
}
int Client::ConnectToServer() {
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

    clientSock = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSock == INVALID_SOCKET) {
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
    if (connect(clientSock, (sockaddr*)&servInfo, sizeof(servInfo)) != 0) {
        cout << "Connection FAILED. Error ";
        cout << WSAGetLastError() << endl;
        closesocket(clientSock);
        WSACleanup();
        return 1;
    }

    return 0;
}
int Client::sendMessage() {
    cout << setw(16) << "Your message: ";
    fgets(clientBuff.data(), clientBuff.size(), stdin);

    packet_size = send(clientSock, clientBuff.data(), clientBuff.size(), 0);

    if (packet_size == SOCKET_ERROR) {
        cout << "Can't send message to Server. Error ";
        cout << WSAGetLastError() << endl;
        closesocket(clientSock);
        WSACleanup();
        return 1;
    }

    if ((clientBuff[0] == 'e' || clientBuff[0] == 'E') && clientBuff[1] == 'x' && clientBuff[2] == 'i' && clientBuff[3] == 't') {
        cout << "Connection closed" << endl;
        shutdown(clientSock, SD_BOTH);
        closesocket(clientSock);
        WSACleanup();
        return 1;
    }

    return 0;
}
int Client::recvMessage() {
    packet_size = recv(clientSock, servBuff.data(), servBuff.size(), 0);

    if (packet_size == SOCKET_ERROR) {
        cout << "Can't receive message from Server. Error ";
        cout << WSAGetLastError() << endl;
        closesocket(clientSock);
        WSACleanup();
        return 1;
    }

    if ((servBuff[0] == 'e' || servBuff[0] == 'E') && servBuff[1] == 'x' && servBuff[2] == 'i' && servBuff[3] == 't') {
        cout << "Connection closed by Server" << endl;
        shutdown(clientSock, SD_BOTH);
        closesocket(clientSock);
        WSACleanup();
        return 1;
    }
    cout << setw(16) << "Server message: " << servBuff.data() << endl;

    return 0;
}
Client::~Client() {
    closesocket(clientSock);
    WSACleanup();
    servBuff.clear();
    clientBuff.clear();
}

