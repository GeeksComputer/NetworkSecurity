#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>

#define HTTP_PORT 80
#define BUFFER_SIZE 1024

void error(const char *msg) {
    perror(msg);
    exit(1);
}

void send_http_request(const char *hostname, const char *ip_address) {
    while (1)
    {
        int sockfd;
        struct sockaddr_in server_addr;
        struct hostent *server;
        char buffer[BUFFER_SIZE];
        char request[BUFFER_SIZE];
        int n;

        // Create a socket
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            error("ERROR opening socket");
        }

        // Resolve hostname to IP address if necessary
        if (ip_address == NULL) {
            server = gethostbyname(hostname);
            if (server == NULL) {
                fprintf(stderr, "ERROR, no such host\n");
                exit(0);
            }

            // Set up the server address structure
            memset(&server_addr, 0, sizeof(server_addr));
            server_addr.sin_family = AF_INET;
            memcpy(&server_addr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);
            server_addr.sin_port = htons(HTTP_PORT);
        } else {
            // Use the provided IP address
            memset(&server_addr, 0, sizeof(server_addr));
            server_addr.sin_family = AF_INET;
            inet_pton(AF_INET, ip_address, &server_addr.sin_addr);
            server_addr.sin_port = htons(HTTP_PORT);
        }

        // Connect to the server
        if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            error("ERROR connecting");
        }

        // Prepare the HTTP GET request
        snprintf(request, sizeof(request),
                "GET / HTTP/1.1\r\n"
                "Host: %s\r\n"
                "Connection: close\r\n"
                "\r\n", hostname);

        // Send the request
        if (send(sockfd, request, strlen(request), 0) < 0) {
            error("ERROR writing to socket");
        }

        // Receive and print the response
        while ((n = recv(sockfd, buffer, sizeof(buffer) - 1, 0)) > 0) {
            buffer[n] = '\0'; // Null-terminate the buffer
            printf("%s", buffer);
        }

        if (n < 0) {
            error("ERROR reading from socket");
        }

        close(sockfd);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <hostname> <IP address (or 'NULL' for domain lookup)>\n", argv[0]);
        exit(1);
    }

    const char *hostname = argv[1];
    const char *ip_address = strcmp(argv[2], "NULL") == 0 ? NULL : argv[2];

    send_http_request(hostname, ip_address);

    return 0;
}
// gcc -o http_request http_request.c