#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/igmp.h>
#include <arpa/inet.h>
#include <unistd.h>

#define IGMP_MEMBERSHIP_REPORT 0x16
#define MULTICAST_GROUP "127.0.0.1"

struct igmp_header {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    struct in_addr group_address;
};

uint16_t checksum(void *buf, int len) {
    uint16_t *data = (uint16_t *)buf;
    uint32_t sum = 0;
    uint16_t result;

    for (int i = 0; i < len / 2; i++) {
        sum += data[i];
    }
    
    if (len % 2) {
        sum += ((uint8_t *)buf)[len - 1];
    }
    
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;

    return result;
}

int main() {
    int sockfd;
    struct sockaddr_in dest_addr;
    struct igmp_header igmp_pkt;
    struct iphdr ip_hdr;

    while (true) {
        // Create a raw socket
        sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_IGMP);
        if (sockfd < 0) {
            perror("Socket creation failed");
            return 1;
        }

        // Construct the IGMP packet
        memset(&igmp_pkt, 0, sizeof(igmp_pkt));
        igmp_pkt.type = IGMP_MEMBERSHIP_REPORT;
        igmp_pkt.code = 0;
        igmp_pkt.checksum = 0; // Will calculate later
        inet_pton(AF_INET, MULTICAST_GROUP, &(igmp_pkt.group_address));

        // Calculate checksum
        igmp_pkt.checksum = checksum(&igmp_pkt, sizeof(igmp_pkt));

        // Set destination address
        memset(&dest_addr, 0, sizeof(dest_addr));
        dest_addr.sin_family = AF_INET;
        inet_pton(AF_INET, MULTICAST_GROUP, &(dest_addr.sin_addr));

        // Send the IGMP packet
        if (sendto(sockfd, &igmp_pkt, sizeof(igmp_pkt), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
            perror("Send failed");
            close(sockfd);
            return 1;
        }

        std::cout << "IGMP packet sent successfully" << std::endl;

        close(sockfd);

    }

    
    return 0;
}
