#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>  // Include this header for sockaddr_ll
#include <unistd.h>
#include <arpa/inet.h>

#define BEACON_FRAME_SIZE 128

// Structure to define the beacon frame
struct BeaconFrame {
    struct ethhdr eth_header;
    unsigned char beacon_frame[BEACON_FRAME_SIZE];
};

void fake_beacon(const char *iface) {
    while (true)
    {
        int sockfd;
        struct ifreq ifr;
        struct sockaddr_ll sa;
        BeaconFrame beacon_frame;

        // Initialize the raw socket
        if ((sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0) {
            perror("socket");
        }

        // Get the index of the interface
        memset(&ifr, 0, sizeof(ifr));
        strncpy(ifr.ifr_name, iface, IFNAMSIZ);
        if (ioctl(sockfd, SIOCGIFINDEX, &ifr) < 0) {
            perror("ioctl");
            close(sockfd);
        }

        // Fill the sockaddr_ll structure
        memset(&sa, 0, sizeof(sa));
        sa.sll_ifindex = ifr.ifr_ifindex;
        sa.sll_protocol = htons(ETH_P_ALL);

        // Fill in the Ethernet header
        memset(&beacon_frame, 0, sizeof(beacon_frame));
        memset(beacon_frame.eth_header.h_dest, 0xff, ETH_ALEN); // Broadcast MAC
        memset(beacon_frame.eth_header.h_source, 0x00, ETH_ALEN); // Source MAC
        beacon_frame.eth_header.h_proto = htons(ETH_P_ALL);

        // Fill in the Beacon frame (simplified for example purposes)
        beacon_frame.beacon_frame[0] = 0x80; // Beacon Frame
        // Add other necessary fields here for a valid Beacon frame

        // Send the Beacon frame
        if (sendto(sockfd, &beacon_frame, sizeof(beacon_frame), 0, (struct sockaddr*)&sa, sizeof(sa)) < 0) {
            perror("sendto");
            close(sockfd);
        }

        std::cout << "Beacon frame sent successfully!" << std::endl;

        close(sockfd);
    }
}

int main() {
    char *iface;
    std::cout << "Enter Interface Name >> "; std::cin >> iface;
    fake_beacon(iface);
    return 0;
}