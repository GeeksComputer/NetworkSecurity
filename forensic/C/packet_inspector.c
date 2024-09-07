#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>

void print_ip_header(const unsigned char *buffer) {
    struct iphdr *iph = (struct iphdr *)buffer;
    struct sockaddr_in source, dest;

    source.sin_addr.s_addr = iph->saddr;
    dest.sin_addr.s_addr = iph->daddr;

    printf("IP Header\n");
    printf(" |-IP Version: %d\n", (unsigned int)iph->version);
    printf(" |-IP Header Length: %d bytes\n", (unsigned int)iph->ihl * 4);
    printf(" |-IP Source Address: %s\n", inet_ntoa(source.sin_addr));
    printf(" |-IP Destination Address: %s\n", inet_ntoa(dest.sin_addr));
}

void print_tcp_header(const unsigned char *buffer) {
    struct iphdr *iph = (struct iphdr *)buffer;
    struct tcphdr *tcph = (struct tcphdr *)(buffer + iph->ihl * 4);

    printf("TCP Header\n");
    printf(" |-Source Port: %u\n", ntohs(tcph->source));
    printf(" |-Destination Port: %u\n", ntohs(tcph->dest));
    printf(" |-Sequence Number: %u\n", ntohl(tcph->seq));
    printf(" |-Acknowledgment Number: %u\n", ntohl(tcph->ack_seq));
    printf(" |-TCP Header Length: %d bytes\n", (unsigned int)(tcph->doff * 4));
}

void print_udp_header(const unsigned char *buffer) {
    struct iphdr *iph = (struct iphdr *)buffer;
    struct udphdr *udph = (struct udphdr *)(buffer + iph->ihl * 4);

    printf("UDP Header\n");
    printf(" |-Source Port: %u\n", ntohs(udph->source));
    printf(" |-Destination Port: %u\n", ntohs(udph->dest));
    printf(" |-UDP Length: %u\n", ntohs(udph->len));
}

void print_icmp_header(const unsigned char *buffer) {
    struct iphdr *iph = (struct iphdr *)buffer;
    struct icmphdr *icmph = (struct icmphdr *)(buffer + iph->ihl * 4);

    printf("ICMP Header\n");
    printf(" |-Type: %d\n", (unsigned int)icmph->type);
    printf(" |-Code: %d\n", (unsigned int)icmph->code);
    printf(" |-Checksum: %u\n", ntohs(icmph->checksum));
}

int main() {
    int sockfd;
    unsigned char buffer[65536];
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

    printf("Packet Inspector Started\n");

    while (1) {
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
                printf("Unsupported protocol: %d\n", (unsigned int)iph->protocol);
                break;
        }

        printf("\n");
    }

    close(sockfd);
    return 0;
}