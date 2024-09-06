#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <sys/socket.h>
#include <sys/types.h>

#define BLOCKED_IP "192.168.1.100"

int main() {
    int sockfd;
    char buffer[65536];
    struct sockaddr_in source;
    socklen_t saddr_len = sizeof(source);

    // Create a raw socket
    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (sockfd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    while (1) {
        // Receive packets
        ssize_t data_size = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&source, &saddr_len);
        if (data_size < 0) {
            perror("Packet receive failed");
            return 1;
        }

        struct iphdr *ip_header = (struct iphdr *)buffer;
        struct udphdr *udp_header = (struct udphdr *)(buffer + (ip_header->ihl * 4));

        // Check if the packet is from the blocked IP
        struct in_addr src_addr;
        src_addr.s_addr = ip_header->saddr;
        if (strcmp(inet_ntoa(src_addr), BLOCKED_IP) == 0) {
            printf("Blocked UDP packet from %s\n", inet_ntoa(src_addr));
            continue; // Drop the packet
        }

        // Process the packet (for demonstration, just print the source IP)
        printf("Received UDP packet from %s\n", inet_ntoa(src_addr));
    }

    close(sockfd);
    return 0;
}
