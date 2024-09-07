#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <errno.h>

#define BUF_SIZE 65535

// Pseudo header needed for TCP checksum calculation
struct pseudo_header {
    u_int32_t source_address;
    u_int32_t dest_address;
    u_int8_t placeholder;
    u_int8_t protocol;
    u_int16_t tcp_length;
};

// TCP checksum calculation
unsigned short checksum(void *b, int len) {
    unsigned short *buf = (unsigned short *)b;
    unsigned int sum = 0;
    unsigned short result;

    while (len > 1) {
        sum += *buf++;
        len -= sizeof(unsigned short);
    }

    if (len == 1) {
        sum += *(unsigned char *)buf;
    }

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

// Function to create a raw socket
int create_raw_socket() {
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    return sockfd;
}

// Function to set socket options
void set_socket_options(int sockfd) {
    int option = 1;
    if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &option, sizeof(option)) < 0) {
        perror("Failed to set socket options");
        exit(EXIT_FAILURE);
    }
}

// Function to forward packets
void forward_packets(int local_sockfd, const std::string& remote_ip, int remote_port) {
    struct sockaddr_in local_addr, remote_addr;
    char buffer[BUF_SIZE];
    struct iphdr *iph = (struct iphdr *)buffer;
    struct tcphdr *tcph = (struct tcphdr *)(buffer + sizeof(struct iphdr));
    struct pseudo_header psh;

    // Set up local address
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = INADDR_ANY;
    local_addr.sin_port = htons(0); // Use any port

    // Bind local socket
    if (bind(local_sockfd, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Get the remote address
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons(remote_port);
    if (inet_pton(AF_INET, remote_ip.c_str(), &remote_addr.sin_addr) <= 0) {
        perror("Invalid remote IP address");
        exit(EXIT_FAILURE);
    }

    while (true) {
        socklen_t len = sizeof(local_addr);
        ssize_t packet_len = recvfrom(local_sockfd, buffer, BUF_SIZE, 0, (struct sockaddr *)&local_addr, &len);
        if (packet_len < 0) {
            perror("Receive failed");
            continue;
        }

        // Forward the packet to the remote server
        if (sendto(local_sockfd, buffer, packet_len, 0, (struct sockaddr *)&remote_addr, sizeof(remote_addr)) < 0) {
            perror("Send failed");
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <local_port> <remote_ip> <remote_port>" << std::endl;
        return EXIT_FAILURE;
    }

    int local_port = std::stoi(argv[1]);
    std::string remote_ip = argv[2];
    int remote_port = std::stoi(argv[3]);

    int sockfd = create_raw_socket();
    set_socket_options(sockfd);
    forward_packets(sockfd, remote_ip, remote_port);

    close(sockfd);
    return EXIT_SUCCESS;
}
// g++ -o raw_port_forwarder raw_port_forwarder.cpp
// sudo ./raw_port_forwarder 12345 192.168.1.100 80