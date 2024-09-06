#include <stdio.h>
#include <pcap.h>

// Callback function for packet capture
void packet_handler(u_char *user_data, const struct pcap_pkthdr *pkthdr, const u_char *packet) {
    printf("Packet Length: %d bytes\n", pkthdr->len);

    printf("Packet Data (first 16 bytes): ");
    for (int i = 0; i < (pkthdr->len > 16 ? 16 : pkthdr->len); i++) {
        printf("%02x ", packet[i]);
    }
    printf("\n\n");
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <interface>\n", argv[0]);
        return 1;
    }

    // Open the network interface for packet capture
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *handle = pcap_open_live(argv[1], BUFSIZ, 1, 1000, errbuf);

    if (handle == NULL) {
        fprintf(stderr, "Could not open device %s: %s\n", argv[1], errbuf);
        return 1;
    }

    // Start packet capture
    if (pcap_loop(handle, 0, packet_handler, NULL) < 0) {
        fprintf(stderr, "pcap_loop() failed: %s\n", pcap_geterr(handle));
        pcap_close(handle);
        return 1;
    }

    // Close the capture handle
    pcap_close(handle);
    return 0;
}

// gcc -o network_monitor network_monitor.c -lpcap
// sudo ./network_monitor <interface>