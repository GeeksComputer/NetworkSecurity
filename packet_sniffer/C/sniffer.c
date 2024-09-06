#include <stdio.h>
#include <pcap.h>

// Callback function to handle each captured packet
void packet_handler(u_char *user_data, const struct pcap_pkthdr *header, const u_char *packet) {
    printf("Received packet with length: %d\n", header->len);
    
    // Print the first 16 bytes of the packet (for demonstration)
    printf("Packet data (first 16 bytes): ");
    for (int i = 0; i < 16; i++) {
        printf("%02x ", packet[i]);
    }
    printf("\n");
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <network_interface>\n", argv[0]);
        return 1;
    }

    char errbuf[PCAP_ERRBUF_SIZE];
    const char *device = argv[1];

    // Open the network interface for packet capture
    pcap_t *handle = pcap_open_live(device, BUFSIZ, 1, 1000, errbuf);
    if (handle == NULL) {
        fprintf(stderr, "Error opening device %s: %s\n", device, errbuf);
        return 1;
    }

    // Start capturing packets and use packet_handler as the callback
    if (pcap_loop(handle, 0, packet_handler, NULL) < 0) {
        fprintf(stderr, "Error capturing packets: %s\n", pcap_geterr(handle));
        pcap_close(handle);
        return 1;
    }

    pcap_close(handle);
    return 0;
}
// gcc -o sniffer sniffer.c -lpcap
// sudo ./sniffer eth0
