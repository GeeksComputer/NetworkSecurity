#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <sys/socket.h>
#include <sys/types.h>

#define BLOCKED_IP "192.168.1.100"

int main() {
    int sockfd;
    char buffer[65536];
    struct sockaddr_ll source;
    socklen_t saddr_len = sizeof(source);

    // Create a raw socket
    sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
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

        struct ether_header *eth_header = (struct ether_header *)buffer;
        struct ether_arp *arp_header = (struct ether_arp *)(buffer + sizeof(struct ether_header));

        // Check if the packet is from the blocked IP
        struct in_addr src_addr;
        memcpy(&src_addr, arp_header->arp_spa, sizeof(struct in_addr));
        if (strcmp(inet_ntoa(src_addr), BLOCKED_IP) == 0) {
            printf("Blocked ARP packet from %s\n", inet_ntoa(src_addr));
            continue; // Drop the packet
        }

        // Process the packet (for demonstration, just print the source IP)
        printf("Received ARP packet from %s\n", inet_ntoa(src_addr));
    }

    close(sockfd);
    return 0;
}
// gcc -o block_arp block_arp.c
// sudo ./block_arp