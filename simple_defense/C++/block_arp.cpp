#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/if_ether.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFER_SIZE 65536

void process_packet(unsigned char* buffer, int size) {
    struct ethhdr* eth = (struct ethhdr*)buffer;
    struct ether_arp* arp = (struct ether_arp*)(buffer + sizeof(struct ethhdr));

    // Example condition to block ARP packets from a specific IP address
    if (arp->arp_spa[0] == 192 && arp->arp_spa[1] == 168 && arp->arp_spa[2] == 1 && arp->arp_spa[3] == 1) {
        std::cout << "Blocked ARP packet from 192.168.1.1" << std::endl;
        return;
    }

    // Process the packet (e.g., log it)
    std::cout << "Received ARP packet from " 
              << (int)arp->arp_spa[0] << "." 
              << (int)arp->arp_spa[1] << "." 
              << (int)arp->arp_spa[2] << "." 
              << (int)arp->arp_spa[3] << std::endl;
}

int main() {
    int sock_raw;
    struct sockaddr saddr;
    unsigned char* buffer = new unsigned char[BUFFER_SIZE];

    sock_raw = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
    if (sock_raw < 0) {
        std::cerr << "Socket creation failed" << std::endl;
        return 1;
    }

    while (true) {
        socklen_t saddr_size = sizeof(saddr);
        int data_size = recvfrom(sock_raw, buffer, BUFFER_SIZE, 0, &saddr, &saddr_size);
        if (data_size < 0) {
            std::cerr << "Failed to receive packets" << std::endl;
            return 1;
        }
        process_packet(buffer, data_size);
    }

    close(sock_raw);
    delete[] buffer;
    return 0;
}
