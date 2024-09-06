#include <pcap.h>
#include <iostream>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/icmp6.h>
#include <netinet/igmp.h>

// Callback function to process packets
void packet_handler(u_char *user_data, const struct pcap_pkthdr *pkthdr, const u_char *packet) {
    // Print packet timestamp and length
    std::cout << "Packet length: " << pkthdr->len << std::endl;

    // Extract IP header
    struct ip *ip_header = (struct ip *)(packet + 14); // 14 bytes for Ethernet header
    std::cout << "Source IP: " << inet_ntoa(ip_header->ip_src) << std::endl;
    std::cout << "Destination IP: " << inet_ntoa(ip_header->ip_dst) << std::endl;

    // Check protocol type
    switch (ip_header->ip_p) {
        case IPPROTO_TCP:
            std::cout << "Protocol: TCP" << std::endl;
            break;
        case IPPROTO_UDP:
            std::cout << "Protocol: UDP" << std::endl;
            break;
        case IPPROTO_ICMP:
            std::cout << "Protocol: ICMP" << std::endl;
            break;
        case IPPROTO_IGMP:
            std::cout << "Protocol: IGMP" << std::endl;
            break;
        default:
            std::cout << "Protocol: Other" << std::endl;
            break;
    }
}

int main() {
    // Device to capture packets from (empty string will choose the default)
    const char *dev = nullptr;
    char errbuf[PCAP_ERRBUF_SIZE];

    // Find a default device to capture from
    dev = pcap_lookupdev(errbuf);
    if (dev == nullptr) {
        std::cerr << "Error finding default device: " << errbuf << std::endl;
        return 1;
    }
    std::cout << "Using device: " << dev << std::endl;

    // Open the capture device
    pcap_t *handle = pcap_open_live(dev, BUFSIZ, 1, 1000, errbuf);
    if (handle == nullptr) {
        std::cerr << "Error opening capture device: " << errbuf << std::endl;
        return 1;
    }

    // Start capturing packets
    if (pcap_loop(handle, 0, packet_handler, nullptr) < 0) {
        std::cerr << "Error starting packet capture: " << pcap_geterr(handle) << std::endl;
        return 1;
    }

    // Close the capture device
    pcap_close(handle);
    return 0;
}
// g++ -o sniffer sniffer.cpp -lpcap