#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/if_ether.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>

#define BEACON_FRAME_LEN 128
#define WLAN_HEADER_LEN 24
#define ETH_HEADER_LEN 14

// IEEE 802.11 Beacon Frame Header (simplified)
struct wlan_hdr {
    uint8_t frame_control[2];
    uint8_t duration_id[2];
    uint8_t addr1[6];
    uint8_t addr2[6];
    uint8_t addr3[6];
    uint8_t sequence_ctrl[2];
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <interface>\n", argv[0]);
        return 1;
    }

    const char* interface = argv[1];

    while (1)
    {
        int sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
        if (sockfd < 0) {
            perror("Socket creation failed");
            return 1;
        }

        struct ifreq ifr;
        memset(&ifr, 0, sizeof(ifr));
        strncpy(ifr.ifr_name, interface, IFNAMSIZ - 1);
        if (ioctl(sockfd, SIOCGIFHWADDR, &ifr) < 0) {
            perror("ioctl failed");
            close(sockfd);
            return 1;
        }

        // Prepare the raw frame
        uint8_t frame[ETH_HEADER_LEN + WLAN_HEADER_LEN + BEACON_FRAME_LEN];
        memset(frame, 0, sizeof(frame));

        // Ethernet Header
        struct ethhdr* eth = (struct ethhdr*)frame;
        memset(eth->h_dest, 0xFF, 6); // Broadcast MAC address
        memcpy(eth->h_source, ifr.ifr_hwaddr.sa_data, 6);
        eth->h_proto = htons(ETH_P_ALL);

        // IEEE 802.11 Beacon Frame Header
        struct wlan_hdr* wlan = (struct wlan_hdr*)(frame + ETH_HEADER_LEN);
        wlan->frame_control[0] = 0x80; // Beacon frame (Type: Management, Subtype: Beacon)
        wlan->frame_control[1] = 0x00; // To DS: 0, From DS: 0
        memset(wlan->addr1, 0xFF, 6);   // Broadcast address for all clients
        memcpy(wlan->addr2, ifr.ifr_hwaddr.sa_data, 6); // Source MAC address
        memcpy(wlan->addr3, ifr.ifr_hwaddr.sa_data, 6); // BSSID

        // Send the frame
        struct sockaddr_ll sa;
        memset(&sa, 0, sizeof(sa));
        sa.sll_ifindex = if_nametoindex(interface);
        sa.sll_protocol = htons(ETH_P_ALL);

        if (sendto(sockfd, frame, sizeof(frame), 0, (struct sockaddr*)&sa, sizeof(sa)) < 0) {
            perror("Send failed");
            close(sockfd);
            return 1;
        }

        printf("Beacon frame sent successfully\n");

        close(sockfd);
    }
    
    return 0;
}
// gcc -o fake_beacon fake_beacon.c
// sudo ./fake_beacon <interface>
