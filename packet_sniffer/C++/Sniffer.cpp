#include <pcap.h>
#include <iostream>

void packet_handler(u_char *user_data, const struct pcap_pkthdr *pkthdr, const u_char *packet) {
    // Print packet length
    std::cout << "Packet Length: " << pkthdr->len << " bytes" << std::endl;

    // Print first 16 bytes of packet data in hexadecimal
    std::cout << "Packet Data: ";
    for (int i = 0; i < std::min(pkthdr->len, (uint32_t)16); ++i) {
        std::cout << std::hex << (int)packet[i] << " ";
    }
    std::cout << std::dec << std::endl;
}

int main() {
    // Open the capture interface
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *handle = pcap_open_live("wlp3s0", BUFSIZ, 1, 1000, errbuf);

    if (handle == nullptr) {
        std::cerr << "Could not open device: " << errbuf << std::endl;
        return 1;
    }

    // Start packet capture
    if (pcap_loop(handle, 0, packet_handler, nullptr) < 0) {
        std::cerr << "pcap_loop() failed: " << pcap_geterr(handle) << std::endl;
        pcap_close(handle);
        return 1;
    }

    // Close the capture handle
    pcap_close(handle);
    return 0;
}

// g++ -o network_monitor network_monitor.c -lpcap
