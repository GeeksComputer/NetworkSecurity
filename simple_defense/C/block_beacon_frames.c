#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/if_ether.h>
#include <linux/if_packet.h>

#define BUFFER_SIZE 2048

// Define the IEEE 802.11 Beacon Frame structure (simplified)
struct ieee80211_beacon_frame {
    uint8_t frame_control[2];
    uint8_t duration[2];
    uint8_t addr1[6];
    uint8_t addr2[6];
    uint8_t addr3[6];
    uint8_t sequence_control[2];
    uint8_t timestamp[8];
    uint8_t beacon_interval[2];
    uint8_t capability_info[2];
    // Following fields depend on the specific implementation
};

// Function to block frames of a specific type
void block_frames(const char *interface_name, int block_beacon) {
    int sockfd;
    struct ifreq ifr;
    struct sockaddr_ll sa;
    unsigned char buffer[BUFFER_SIZE];
    
    // Create a raw socket
    sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    // Specify the network interface to use
    strncpy(ifr.ifr_name, interface_name, IFNAMSIZ);
    if (ioctl(sockfd, SIOCGIFINDEX, &ifr) < 0) {
        perror("IOCTL failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    
    memset(&sa, 0, sizeof(sa));
    sa.sll_family = AF_PACKET;
    sa.sll_ifindex = ifr.ifr_ifindex;

    // Bind the socket to the specified interface
    if (bind(sockfd, (struct sockaddr*)&sa, sizeof(sa)) < 0) {
        perror("Socket bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Listening on interface %s\n", interface_name);

    // Loop to receive and process packets
    while (1) {
        ssize_t len = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, NULL, NULL);
        if (len < 0) {
            perror("Packet receive failed");
            break;
        }

        // Check if the packet is a beacon frame (simplified check)
        if (block_beacon) {
            struct ieee80211_beacon_frame *frame = (struct ieee80211_beacon_frame*)buffer;
            
            // Assuming Beacon frame is detected by specific Frame Control field (simplified)
            if ((frame->frame_control[0] & 0x0F) == 0x08) { // Check for Beacon frame
                printf("Beacon frame detected and blocked.\n");
                continue; // Drop the packet
            }
        }

        // If not a beacon frame or not blocking, you can process or forward it as needed
    }
    
    close(sockfd);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <interface> <block_beacon>\n", argv[0]);
        return 1;
    }

    const char *interface_name = argv[1];
    int block_beacon = atoi(argv[2]);

    block_frames(interface_name, block_beacon);
    return 0;
}
