#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/if_ether.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <netpacket/packet.h>

// Define ARP packet structure
struct ARPHeader {
    uint16_t hw_type;       // Hardware type (1 for Ethernet)
    uint16_t proto_type;    // Protocol type (0x0800 for IPv4)
    uint8_t hw_size;        // Hardware address length (6 for MAC)
    uint8_t proto_size;     // Protocol address length (4 for IPv4)
    uint16_t op;            // Operation (1 for request, 2 for reply)
    uint8_t sender_mac[6]; // Sender MAC address
    uint8_t sender_ip[4];  // Sender IP address
    uint8_t target_mac[6]; // Target MAC address (all zeros for request)
    uint8_t target_ip[4];  // Target IP address
};

// Function to construct and send ARP request
void send_arp_request(const std::string& iface_name, const std::string& target_ip_str) {
    while (true)
    {
        int sockfd;
        struct ifreq ifr;
        struct sockaddr_ll sa;
        struct ARPHeader arp_req;

        // Create raw socket
        if ((sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP))) < 0) {
            perror("socket");
            exit(EXIT_FAILURE);
        }

        // Get interface index
        memset(&ifr, 0, sizeof(ifr));
        strncpy(ifr.ifr_name, iface_name.c_str(), IFNAMSIZ - 1);
        if (ioctl(sockfd, SIOCGIFINDEX, &ifr) < 0) {
            perror("ioctl");
            close(sockfd);
            exit(EXIT_FAILURE);
        }

        // Fill sockaddr_ll structure
        memset(&sa, 0, sizeof(sa));
        sa.sll_family = AF_PACKET;
        sa.sll_ifindex = ifr.ifr_ifindex;
        sa.sll_protocol = htons(ETH_P_ARP);

        // Fill ARP request header
        memset(&arp_req, 0, sizeof(arp_req));
        arp_req.hw_type = htons(1);           // Ethernet
        arp_req.proto_type = htons(ETH_P_IP); // IPv4
        arp_req.hw_size = 6;                  // MAC address size
        arp_req.proto_size = 4;               // IP address size
        arp_req.op = htons(1);                // ARP request

        // Get source MAC address
        if (ioctl(sockfd, SIOCGIFHWADDR, &ifr) < 0) {
            perror("ioctl");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
        memcpy(arp_req.sender_mac, ifr.ifr_hwaddr.sa_data, 6);

        // Get source IP address
        struct sockaddr_in sa_in;
        memset(&sa_in, 0, sizeof(sa_in));
        sa_in.sin_family = AF_INET;
        if (ioctl(sockfd, SIOCGIFADDR, &ifr) < 0) {
            perror("ioctl");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
        sa_in = *((struct sockaddr_in*)&ifr.ifr_addr);
        memcpy(arp_req.sender_ip, &sa_in.sin_addr, 4);

        // Set target MAC address to 00:00:00:00:00:00 (unknown)
        memset(arp_req.target_mac, 0, 6);

        // Convert target IP address from string to binary
        struct in_addr target_ip;
        if (inet_pton(AF_INET, target_ip_str.c_str(), &target_ip) != 1) {
            perror("inet_pton");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
        memcpy(arp_req.target_ip, &target_ip.s_addr, 4);

        // Construct Ethernet frame
        uint8_t packet[ETH_FRAME_LEN];
        struct ethhdr* eth = (struct ethhdr*)packet;
        memset(eth->h_dest, 0xff, 6); // Broadcast address
        memset(eth->h_source, 0, 6);  // Placeholder source address
        eth->h_proto = htons(ETH_P_ARP);

        // Copy ARP header into packet
        memcpy(packet + sizeof(struct ethhdr), &arp_req, sizeof(arp_req));

        // Send packet
        if (sendto(sockfd, packet, sizeof(struct ethhdr) + sizeof(arp_req), 0, (struct sockaddr*)&sa, sizeof(sa)) < 0) {
            perror("sendto");
            close(sockfd);
            exit(EXIT_FAILURE);
        }

        std::cout << "ARP request sent successfully" << std::endl;

        close(sockfd);
    }
}

int main() {
    std::string iface_name;
    std::string target_ip;

    std::cout << "Enter Your Interface Name >> "; std::cin >> iface_name;
    std::cout << "Enter Target IP Target >> "; std::cin >> target_ip;

    send_arp_request(iface_name, target_ip);

    return 0;
}
// g++ -o arp_request arp_request.cpp