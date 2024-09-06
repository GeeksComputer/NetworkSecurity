#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>

// Calculate checksum for IP and transport layer protocols
unsigned short checksum(void *b, int len) { 
    unsigned short *buf = (unsigned short *)b;
    unsigned int sum = 0;
    unsigned short result;

    while (len > 1) {
        sum += *buf++;
        len -= 2;
    }

    if (len == 1) {
        sum += *(unsigned char *)buf;
    }

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

void send_tcp_packet(const char *dest_ip, int dest_port) {
    while (1)
    {
        int sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
        if (sock < 0) {
            perror("TCP socket creation failed");
            return;
        }

        struct sockaddr_in dest_addr;
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(dest_port);
        inet_pton(AF_INET, dest_ip, &dest_addr.sin_addr);

        char packet[4096];
        memset(packet, 0, sizeof(packet));

        struct iphdr *iph = (struct iphdr *)packet;
        struct tcphdr *tcph = (struct tcphdr *)(packet + sizeof(struct iphdr));

        iph->ihl = 5;
        iph->version = 4;
        iph->tos = 0;
        iph->tot_len = sizeof(struct iphdr) + sizeof(struct tcphdr);
        iph->id = htonl(54321);
        iph->frag_off = 0;
        iph->ttl = 255;
        iph->protocol = IPPROTO_TCP;
        iph->check = 0; // Leave checksum 0 now, filled later by pseudo header
        iph->saddr = inet_addr("192.168.1.1");  // Source IP address (modify as needed)
        iph->daddr = dest_addr.sin_addr.s_addr;

        // TCP header
        tcph->source = htons(12345);  // Source port
        tcph->dest = htons(dest_port);
        tcph->seq = 0;
        tcph->ack_seq = 0;
        tcph->doff = 5;  // TCP header size
        tcph->fin = 0;
        tcph->syn = 1;  // SYN flag
        tcph->rst = 0;
        tcph->psh = 0;
        tcph->ack = 0;
        tcph->urg = 0;
        tcph->window = htons(5840);  // Maximum allowed window size
        tcph->check = 0;  // Leave checksum 0 now, filled later by pseudo header
        tcph->urg_ptr = 0;

        // IP checksum
        iph->check = checksum((unsigned short *)packet, iph->tot_len);

        // Send packet
        printf("TCP packet send successfully\n");

        if (sendto(sock, packet, iph->tot_len, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
            perror("TCP packet send failed");
        }

        close(sock);
    }
}

void send_udp_packet(const char *dest_ip, int dest_port) {
    while (1)
    {
        int sock = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
        if (sock < 0) {
            perror("UDP socket creation failed");
            return;
        }

        struct sockaddr_in dest_addr;
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(dest_port);
        inet_pton(AF_INET, dest_ip, &dest_addr.sin_addr);

        char packet[4096];
        memset(packet, 0, sizeof(packet));

        struct iphdr *iph = (struct iphdr *)packet;
        struct udphdr *udph = (struct udphdr *)(packet + sizeof(struct iphdr));

        iph->ihl = 5;
        iph->version = 4;
        iph->tos = 0;
        iph->tot_len = sizeof(struct iphdr) + sizeof(struct udphdr);
        iph->id = htonl(54321);
        iph->frag_off = 0;
        iph->ttl = 255;
        iph->protocol = IPPROTO_UDP;
        iph->check = 0; // Leave checksum 0 now, filled later by pseudo header
        iph->saddr = inet_addr("192.168.1.1");  // Source IP address (modify as needed)
        iph->daddr = dest_addr.sin_addr.s_addr;

        // UDP header
        udph->source = htons(12345);
        udph->dest = htons(dest_port);
        udph->len = htons(sizeof(struct udphdr));
        udph->check = 0;  // Leave checksum 0 now, filled later by pseudo header

        // IP checksum
        iph->check = checksum((unsigned short *)packet, iph->tot_len);

        // Send packet
        printf("UDP packet send successfully\n");

        if (sendto(sock, packet, iph->tot_len, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
            perror("UDP packet send failed");
        }

        close(sock);
    }
}

void send_icmp_packet(const char *dest_ip) {
    while (1)
    {
        int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
        if (sock < 0) {
            perror("ICMP socket creation failed");
            return;
        }

        struct sockaddr_in dest_addr;
        dest_addr.sin_family = AF_INET;
        inet_pton(AF_INET, dest_ip, &dest_addr.sin_addr);

        char packet[64];
        memset(packet, 0, sizeof(packet));

        struct icmphdr *icmp = (struct icmphdr *)packet;

        icmp->type = ICMP_ECHO;
        icmp->code = 0;
        icmp->checksum = 0;
        icmp->un.echo.id = htons(0x1234);  // Identifier
        icmp->un.echo.sequence = htons(0x0001);  // Sequence number

        icmp->checksum = checksum((unsigned short *)packet, sizeof(packet));

        // Send packet
        printf("ICMP packet send successfully\n");

        if (sendto(sock, packet, sizeof(packet), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
            perror("ICMP packet send failed");
        }

        close(sock);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <dest_ip> <dest_port> <protocol>\n", argv[0]);
        fprintf(stderr, "protocol: tcp, udp, icmp\n");
        return 1;
    }

    const char *dest_ip = argv[1];
    int dest_port = atoi(argv[2]);
    const char *protocol = argv[3];

    if (strcmp(protocol, "tcp") == 0) {
        send_tcp_packet(dest_ip, dest_port);
    } else if (strcmp(protocol, "udp") == 0) {
        send_udp_packet(dest_ip, dest_port);
    } else if (strcmp(protocol, "icmp") == 0) {
        send_icmp_packet(dest_ip);
    } else {
        fprintf(stderr, "Unknown protocol: %s\n", protocol);
        return 1;
    }

    return 0;
}
