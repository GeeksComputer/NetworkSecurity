#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFER_SIZE 65536

void process_packet(unsigned char* buffer, int size) {
    struct iphdr* iph = (struct iphdr*)buffer;
    struct udphdr* udph = (struct udphdr*)(buffer + iph->ihl * 4);

    // Example condition to block packets from a specific IP address
    if (iph->saddr == inet_addr("192.168.1.1")) {
        std::cout << "Blocked UDP packet from 192.168.1.1" << std::endl;
        return;
    }

    // Process the packet (e.g., log it)
    std::cout << "Received UDP packet from " << inet_ntoa(*(in_addr*)&iph->saddr) << std::endl;
}

int main() {
    int sock_raw;
    struct sockaddr saddr;
    unsigned char* buffer = new unsigned char[BUFFER_SIZE];

    sock_raw = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
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
