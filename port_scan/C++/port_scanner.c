#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>

#define TIMEOUT 1 // Timeout in seconds

// Function to check if a port is open
int is_port_open(const char *host, int port) {
    int sockfd;
    struct sockaddr_in servaddr;
    struct timeval tv;
    fd_set fdset;
    int flags;

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 0;
    }

    // Set socket timeout
    tv.tv_sec = TIMEOUT;
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));

    // Set up the sockaddr_in structure
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = inet_addr(host);

    // Attempt to connect
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        close(sockfd);
        return 0;
    }

    // If connection is successful, port is open
    close(sockfd);
    return 1;
}

int main() {
    char host[256];
    int start_port, end_port;

    printf("Enter the IP address or hostname: ");
    scanf("%s", host);
    printf("Enter the start port: ");
    scanf("%d", &start_port);
    printf("Enter the end port: ");
    scanf("%d", &end_port);

    if (start_port < 1 || end_port > 65535 || start_port > end_port) {
        fprintf(stderr, "Invalid port range\n");
        return 1;
    }

    printf("Scanning ports from %d to %d on %s...\n", start_port, end_port, host);

    for (int port = start_port; port <= end_port; ++port) {
        if (is_port_open(host, port)) {
            printf("Port %d is open\n", port);
        }
    }

    return 0;
}
// gcc -o portscanner portscanner.c
// ./portscanner