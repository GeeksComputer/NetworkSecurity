#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/if_ether.h>
#include <linux/if_packet.h>
#include <netinet/ether.h>

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
void block_frames(const std::string& interface_name, bool block_beacon) {
    int sockfd;
    struct ifreq ifr;
    struct sockaddr_ll sa;
    unsigned char buffer[BUFFER_SIZE];
    
    // Create a raw socket
    sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sockfd < 0) {
        perror("Socket creation failed");
        return;
    }
    
    // Specify the network interface to use
    strncpy(ifr.ifr_name, interface_name.c_str(), IFNAMSIZ);
    if (ioctl(sockfd, SIOCGIFINDEX, &ifr) < 0) {
        perror("IOCTL failed");
        close(sockfd);
        return;
    }
    
    memset(&sa, 0, sizeof(sa));
    sa.sll_family = AF_PACKET;
    sa.sll_ifindex = ifr.ifr_ifindex;

    // Bind the socket to the specified interface
    if (bind(sockfd, (struct sockaddr*)&sa, sizeof(sa)) < 0) {
        perror("Socket bind failed");
        close(sockfd);
        return;
    }

    std::cout << "Listening on interface " << interface_name << std::endl;

    // Loop to receive and process packets
    while (true) {
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
                std::cout << "Beacon frame detected and blocked." << std::endl;
                continue; // Drop the packet
            }
        }

        // If not a beacon frame or not blocking, you can process or forward it as needed
    }
    
    close(sockfd);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <interface> <block_beacon>" << std::endl;
        return 1;
    }

    std::string interface_name = argv[1];
    bool block_beacon = std::stoi(argv[2]) != 0;

    block_frames(interface_name, block_beacon);
    return 0;
}
// g++ -o block_frames block_frames.cpp
// sudo ./block_frames wlan0 1
// block beacon frames (use 1 to block, 0 to ignore).