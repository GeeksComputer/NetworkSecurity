#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <unistd.h>

// Function to print IP header information
void print_ip_header(const char *buffer) {
    struct iphdr *iph = (struct iphdr *)buffer;
    struct sockaddr_in source, dest;

    source.sin_addr.s_addr = iph->saddr;
    dest.sin_addr.s_addr = iph->daddr;

    std::cout << "IP Header" << std::endl;
    std::cout << " |-IP Version: " << (unsigned int)iph->version << std::endl;
    std::cout << " |-IP Header Length: " << (unsigned int)iph->ihl * 4 << " bytes" << std::endl;
    std::cout << " |-IP Source Address: " << inet_ntoa(source.sin_addr) << std::endl;
    std::cout << " |-IP Destination Address: " << inet_ntoa(dest.sin_addr) << std::endl;
}

// Function to print TCP header information
void print_tcp_header(const char *buffer) {
    struct iphdr *iph = (struct iphdr *)buffer;
    struct tcphdr *tcph = (struct tcphdr *)(buffer + iph->ihl * 4);

    std::cout << "TCP Header" << std::endl;
    std::cout << " |-Source Port: " << ntohs(tcph->source) << std::endl;
    std::cout << " |-Destination Port: " << ntohs(tcph->dest) << std::endl;
    std::cout << " |-Sequence Number: " << ntohl(tcph->seq) << std::endl;
    std::cout << " |-Acknowledgment Number: " << ntohl(tcph->ack_seq) << std::endl;
    std::cout << " |-TCP Header Length: " << (unsigned int)(tcph->doff * 4) << " bytes" << std::endl;
}

// Function to print UDP header information
void print_udp_header(const char *buffer) {
    struct iphdr *iph = (struct iphdr *)buffer;
    struct udphdr *udph = (struct udphdr *)(buffer + iph->ihl * 4);

    std::cout << "UDP Header" << std::endl;
    std::cout << " |-Source Port: " << ntohs(udph->source) << std::endl;
    std::cout << " |-Destination Port: " << ntohs(udph->dest) << std::endl;
    std::cout << " |-UDP Length: " << ntohs(udph->len) << std::endl;
}

// Function to print ICMP header information
void print_icmp_header(const char *buffer) {
    struct iphdr *iph = (struct iphdr *)buffer;
    struct icmphdr *icmph = (struct icmphdr *)(buffer + iph->ihl * 4);

    std::cout << "ICMP Header" << std::endl;
    std::cout << " |-Type: " << (unsigned int)icmph->type << std::endl;
    std::cout << " |-Code: " << (unsigned int)icmph->code << std::endl;
    std::cout << " |-Checksum: " << ntohs(icmph->checksum) << std::endl;
}

int main() {
    int sockfd;
    char buffer[65536];
    struct sockaddr_in saddr;
    socklen_t saddr_len = sizeof(saddr);

    // Create a raw socket
    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP); // Use IPPROTO_IP for all protocols
    if (sockfd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    // Bind the socket to an interface (optional)
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sockfd, (struct sockaddr *)&saddr, sizeof(saddr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        return 1;
    }

    std::cout << "Packet Inspector Started" << std::endl;

    while (true) {
        // Receive a packet
        ssize_t data_size = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&saddr, &saddr_len);
        if (data_size < 0) {
            perror("Recvfrom failed");
            close(sockfd);
            return 1;
        }

        // Print IP header
        print_ip_header(buffer);

        // Determine the protocol and print the appropriate header
        struct iphdr *iph = (struct iphdr *)buffer;
        switch (iph->protocol) {
            case IPPROTO_TCP:
                print_tcp_header(buffer);
                break;
            case IPPROTO_UDP:
                print_udp_header(buffer);
                break;
            case IPPROTO_ICMP:
                print_icmp_header(buffer);
                break;
            default:
                std::cout << "Unsupported protocol: " << (unsigned int)iph->protocol << std::endl;
                break;
        }

        std::cout << std::endl;
    }

    close(sockfd);
    return 0;
}