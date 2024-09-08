#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>

void error(const char* msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[1024];
    std::string command;
    ssize_t bytes_sent, bytes_received;

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("Socket creation failed");
    }

    // Set up server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(4444); // Port to connect to
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Server IP address

    // Connect to the server
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        error("Connection failed");
    }

    std::cout << "Connected to server.\n";

    // Communication loop
    while (true) {
        // Get command from user
        std::cout << "Enter command: ";
        std::getline(std::cin, command);

        // Exit the loop if the user types "exit"
        if (command == "exit") {
            break;
        }

        // Send command to the server
        bytes_sent = send(sockfd, command.c_str(), command.length(), 0);
        if (bytes_sent < 0) {
            error("Send failed");
        }

        // Receive and display response from the server
        memset(buffer, 0, sizeof(buffer));
        while ((bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0)) > 0) {
            std::cout << std::string(buffer, bytes_received);
            memset(buffer, 0, sizeof(buffer));
        }
        
        if (bytes_received < 0) {
            error("Receive failed");
        }
    }

    // Clean up
    close(sockfd);

    return 0;
}
