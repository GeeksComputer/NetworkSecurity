#include <iostream>
#include <cstring>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <linux/if_arp.h>  // For ARPHRD_ETHER

void changeMacAddress(const std::string& interface, const std::string& newMac) {
    int sockfd;
    struct ifreq ifr;

    // Create a raw socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
    }

    // Clear the ifreq structure
    memset(&ifr, 0, sizeof(ifr));
    
    // Set the interface name
    strncpy(ifr.ifr_name, interface.c_str(), IFNAMSIZ);

    // Convert MAC address from string to binary form
    if (sscanf(newMac.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
               &ifr.ifr_hwaddr.sa_data[0], &ifr.ifr_hwaddr.sa_data[1],
               &ifr.ifr_hwaddr.sa_data[2], &ifr.ifr_hwaddr.sa_data[3],
               &ifr.ifr_hwaddr.sa_data[4], &ifr.ifr_hwaddr.sa_data[5]) != 6) {
        std::cerr << "Invalid MAC address format" << std::endl;
        close(sockfd);
    }

    // Set the address family to ARPHRD_ETHER (Ethernet)
    ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;

    // Perform the ioctl call to change the MAC address
    if (ioctl(sockfd, SIOCSIFHWADDR, &ifr) < 0) {
        perror("ioctl");
        close(sockfd);
    }

    std::cout << "MAC address changed successfully" << std::endl;

    // Close the socket
    close(sockfd);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <interface> <new_mac>" << std::endl;
        return 1;
    }

    std::string interface = argv[1];
    std::string newMac = argv[2];

    // Change the MAC address
    changeMacAddress(interface, newMac);

    return 0;
}
// g++ -o macchanger macchanger.cpp
// sudo ./macchanger en0 02:42:ac:11:00:02