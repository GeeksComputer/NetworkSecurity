#include <iostream>
#include <pcap.h>

// Callback function to handle each captured packet
void packetHandler(u_char *userData, const struct pcap_pkthdr *header, const u_char *packet) {
    std::cout << "Received packet with length: " << header->len << std::endl;
    // For demonstration, we'll print the first 16 bytes of the packet
    std::cout << "Packet data (first 16 bytes): ";
    for (int i = 0; i < 16; i++) {
        std::cout << std::hex << (int)packet[i] << " ";
    }
    std::cout << std::endl;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <network_interface>" << std::endl;
        return 1;
    }

    char errbuf[PCAP_ERRBUF_SIZE];
    const char *device = argv[1];

    // Open the network interface for packet capture
    pcap_t *handle = pcap_open_live(device, BUFSIZ, 1, 1000, errbuf);
    if (handle == nullptr) {
        std::cerr << "Error opening device: " << errbuf << std::endl;
        return 1;
    }

    // Start capturing packets and use packetHandler as the callback
    if (pcap_loop(handle, 0, packetHandler, nullptr) < 0) {
        std::cerr << "Error capturing packets: " << pcap_geterr(handle) << std::endl;
        pcap_close(handle);
        return 1;
    }

    pcap_close(handle);
    return 0;
}
// g++ -o sniffer sniffer.cpp -lpcap
// sudo ./sniffer wlp3s0