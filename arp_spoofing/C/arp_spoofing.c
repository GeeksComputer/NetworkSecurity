#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <net/if.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <netinet/if_ether.h>
#include <net/if_arp.h>

// ARP header structure
struct arp_hdr {
    uint16_t hardware_type;    // Hardware type (e.g., Ethernet)
    uint16_t protocol_type;    // Protocol type (e.g., IPv4)
    uint8_t hardware_len;      // Hardware address length (e.g., 6 for MAC)
    uint8_t protocol_len;      // Protocol address length (e.g., 4 for IPv4)
    uint16_t operation;        // Operation (1 = request, 2 = reply)
    uint8_t sender_mac[6];     // Sender MAC address
    uint8_t sender_ip[4];      // Sender IP address
    uint8_t target_mac[6];     // Target MAC address (zeroed out in request)
    uint8_t target_ip[4];      // Target IP address
};

// ARP packet construction
void fill_arp_header(struct arp_hdr *arp, const uint8_t *sender_mac, const uint8_t *sender_ip, const uint8_t *target_ip) {
    arp->hardware_type = htons(ARPHRD_ETHER); // Ethernet
    arp->protocol_type = htons(ETH_P_IP);     // IPv4
    arp->hardware_len = 6;                    // MAC address length
    arp->protocol_len = 4;                    // IP address length
    arp->operation = htons(ARPOP_REQUEST);    // ARP Request
    memcpy(arp->sender_mac, sender_mac, 6);
    memcpy(arp->sender_ip, sender_ip, 4);
    memset(arp->target_mac, 0, 6);            // Target MAC address is zeroed in a request
    memcpy(arp->target_ip, target_ip, 4);
}

void spoofing() {
    while (1)
    {
        int sock;
        struct sockaddr_ll sa;
        struct arp_hdr arp;
        uint8_t sender_mac[6] = {0x00, 0x0c, 0x29, 0x3e, 0x4f, 0x5d}; // Replace with your MAC address
        uint8_t sender_ip[4] = {127, 0, 0, 2}; // Replace with your IP address
        uint8_t target_ip[4] = {127, 0, 0, 1};  // Replace with the target IP address
        uint8_t packet[sizeof(struct ether_header) + sizeof(struct arp_hdr)];

        // Create a raw socket
        sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
        if (sock < 0) {
            perror("Socket creation failed");
        }

        // Zero out the packet buffer
        memset(packet, 0, sizeof(packet));

        // Fill ARP header
        fill_arp_header((struct arp_hdr *)(packet + sizeof(struct ether_header)), sender_mac, sender_ip, target_ip);

        // Fill Ethernet header
        struct ether_header *eth = (struct ether_header *)packet;
        memset(eth->ether_dhost, 0xff, 6); // Broadcast address
        memcpy(eth->ether_shost, sender_mac, 6); // Sender's MAC address
        eth->ether_type = htons(ETH_P_ARP);

        // Set up sockaddr_ll
        memset(&sa, 0, sizeof(sa));
        sa.sll_ifindex = if_nametoindex("lo"); // Replace with your network interface name
        sa.sll_family = AF_PACKET;
        sa.sll_protocol = htons(ETH_P_ARP);

        // Send the packet
        if (sendto(sock, packet, sizeof(packet), 0, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
            perror("Send failed");
            close(sock);
        }

        printf("ARP Request sent successfully\n");

        // Close the socket
        close(sock);
    }
}

int main() {
    spoofing();
    
    return 0;
}
// gcc -o send_raw_arp send_raw_arp.c