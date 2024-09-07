#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstring>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/if_arp.h>
#include <arpa/inet.h>
#include <unistd.h>

bool getMacAddress(const std::string& ipAddress, std::string& macAddress) {
    int sockfd;
    struct arpreq arpReq;
    struct sockaddr_in* sin;

    // Create a socket for ARP requests
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return false;
    }

    // Clear and set up ARP request structure
    memset(&arpReq, 0, sizeof(arpReq));
    sin = (struct sockaddr_in*)&arpReq.arp_pa;
    sin->sin_family = AF_INET;
    if (inet_pton(AF_INET, ipAddress.c_str(), &sin->sin_addr) <= 0) {
        perror("inet_pton");
        close(sockfd);
        return false;
    }

    // Perform the ARP ioctl to get the MAC address
    if (ioctl(sockfd, SIOCGARP, &arpReq) < 0) {
        perror("ioctl");
        close(sockfd);
        return false;
    }

    // Extract MAC address from the ARP response
    unsigned char* mac = (unsigned char*)arpReq.arp_ha.sa_data;
    std::ostringstream macStream;
    macStream << std::hex << std::setfill('0');
    for (int i = 0; i < 6; ++i) {
        if (i != 0) macStream << ":";
        macStream << std::setw(2) << static_cast<int>(mac[i]);
    }
    macAddress = macStream.str();

    close(sockfd);
    return true;
}

int main() {
    std::string ipAddress;
    std::string macAddress;

    std::cout << "Enter IP Address Target >> "; std::cin >> ipAddress;

    if (getMacAddress(ipAddress, macAddress)) {
        std::cout << "MAC Address for IP " << ipAddress << " is " << macAddress << std::endl;
    } else {
        std::cerr << "Failed to get MAC Address for IP " << ipAddress << std::endl;
    }

    return 0;
}
// g++ -o arp_lookup arp_lookup.cpp