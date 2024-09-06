#include <iostream>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>

#define TIMEOUT 1 // Timeout in seconds

bool isPortOpen(const std::string &host, int port) {
    int sockfd;
    struct sockaddr_in servaddr;
    struct timeval tv;
    fd_set fdset;
    int flags;

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return false;
    }

    // Set socket timeout
    tv.tv_sec = TIMEOUT;
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    // Set up the sockaddr_in structure
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = inet_addr(host.c_str());

    // Attempt to connect
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        close(sockfd);
        return false;
    }

    // If connection is successful, port is open
    close(sockfd);
    return true;
}

int main() {
    std::string host;
    int startPort, endPort;

    std::cout << "Enter the IP address or hostname: ";
    std::cin >> host;
    std::cout << "Enter the start port: ";
    std::cin >> startPort;
    std::cout << "Enter the end port: ";
    std::cin >> endPort;

    if (startPort < 1 || endPort > 65535 || startPort > endPort) {
        std::cerr << "Invalid port range" << std::endl;
        return 1;
    }

    std::cout << "Scanning ports from " << startPort << " to " << endPort << " on " << host << "...\n";

    for (int port = startPort; port <= endPort; ++port) {
        if (isPortOpen(host, port)) {
            std::cout << "Port " << port << " is open\n";
        }
    }

    return 0;
}
// g++ -o portscanner portscanner.cpp
// ./portscanner