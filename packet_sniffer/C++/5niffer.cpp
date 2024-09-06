#include <iostream>
#include <pcap.h>
#include <netinet/ether.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>  // Updated header for ICMP
#include <arpa/inet.h>
#include <net/if_arp.h>

// Function to print IP header
void print_ip_header(const u_char *packet) {
    struct ip *iph = (struct ip *)(packet + sizeof(struct ether_header));

    std::cout << "IP Header Information:" << std::endl;
    std::cout << "   |-IP Version : " << (unsigned int)iph->ip_v << std::endl;
    std::cout << "   |-IP Header Length : " << (unsigned int)(iph->ip_hl * 4) << " bytes" << std::endl;
    std::cout << "   |-Type Of Service : " << (unsigned int)iph->ip_tos << std::endl;
    std::cout << "   |-Total Length : " << ntohs(iph->ip_len) << " bytes" << std::endl;
    std::cout << "   |-Identification : " << ntohs(iph->ip_id) << std::endl;
    std::cout << "   |-Time To Live : " << (unsigned int)iph->ip_ttl << std::endl;
    std::cout << "   |-Protocol : " << (unsigned int)iph->ip_p << std::endl;
    std::cout << "   |-Checksum : " << ntohs(iph->ip_sum) << std::endl;
    std::cout << "   |-Source IP : " << inet_ntoa(iph->ip_src) << std::endl;
    std::cout << "   |-Destination IP : " << inet_ntoa(iph->ip_dst) << std::endl;
}

// Function to print TCP header
void print_tcp_header(const u_char *packet) {
    struct ip *iph = (struct ip *)(packet + sizeof(struct ether_header));
    struct tcphdr *tcph = (struct tcphdr *)(packet + sizeof(struct ether_header) + iph->ip_hl * 4);

    std::cout << "TCP Header Information:" << std::endl;
    std::cout << "   |-Source Port : " << ntohs(tcph->th_sport) << std::endl;
    std::cout << "   |-Destination Port : " << ntohs(tcph->th_dport) << std::endl;
    std::cout << "   |-Sequence Number : " << ntohl(tcph->th_seq) << std::endl;
    std::cout << "   |-Acknowledgement Number : " << ntohl(tcph->th_ack) << std::endl;
    std::cout << "   |-TCP Header Length : " << (unsigned int)(tcph->th_off * 4) << " bytes" << std::endl;
    std::cout << "   |-Flags : " << std::endl;
    std::cout << "       |-URG : " << ((unsigned int)tcph->th_flags & TH_URG ? 1 : 0) << std::endl;
    std::cout << "       |-ACK : " << ((unsigned int)tcph->th_flags & TH_ACK ? 1 : 0) << std::endl;
    std::cout << "       |-PSH : " << ((unsigned int)tcph->th_flags & TH_PUSH ? 1 : 0) << std::endl;
    std::cout << "       |-RST : " << ((unsigned int)tcph->th_flags & TH_RST ? 1 : 0) << std::endl;
    std::cout << "       |-SYN : " << ((unsigned int)tcph->th_flags & TH_SYN ? 1 : 0) << std::endl;
    std::cout << "       |-FIN : " << ((unsigned int)tcph->th_flags & TH_FIN ? 1 : 0) << std::endl;
    std::cout << "   |-Window Size : " << ntohs(tcph->th_win) << std::endl;
    std::cout << "   |-Checksum : " << ntohs(tcph->th_sum) << std::endl;
    std::cout << "   |-Urgent Pointer : " << ntohs(tcph->th_urp) << std::endl;
}

// Function to print UDP header
void print_udp_header(const u_char *packet) {
    struct ip *iph = (struct ip *)(packet + sizeof(struct ether_header));
    struct udphdr *udph = (struct udphdr *)(packet + sizeof(struct ether_header) + iph->ip_hl * 4);

    std::cout << "UDP Header Information:" << std::endl;
    std::cout << "   |-Source Port : " << ntohs(udph->uh_sport) << std::endl;
    std::cout << "   |-Destination Port : " << ntohs(udph->uh_dport) << std::endl;
    std::cout << "   |-UDP Length : " << ntohs(udph->uh_ulen) << std::endl;
    std::cout << "   |-UDP Checksum : " << ntohs(udph->uh_sum) << std::endl;
}

// Function to print ICMP header
void print_icmp_header(const u_char *packet) {
    struct ip *iph = (struct ip *)(packet + sizeof(struct ether_header));
    struct icmphdr *icmph = (struct icmphdr *)(packet + sizeof(struct ether_header) + iph->ip_hl * 4);

    std::cout << "ICMP Header Information:" << std::endl;
    std::cout << "   |-Type : " << (unsigned int)icmph->type << std::endl;
    std::cout << "   |-Code : " << (unsigned int)icmph->code << std::endl;
    std::cout << "   |-Checksum : " << ntohs(icmph->checksum) << std::endl;
}

// Function to print ARP header
void print_arp_header(const u_char *packet) {
    struct ether_header *eth = (struct ether_header *)packet;
    struct arphdr *arph = (struct arphdr *)(packet + sizeof(struct ether_header));

    std::cout << "ARP Header Information:" << std::endl;
    std::cout << "   |-Hardware Type : " << ntohs(arph->ar_hrd) << std::endl;
    std::cout << "   |-Protocol Type : " << ntohs(arph->ar_pro) << std::endl;
    std::cout << "   |-Hardware Address Length : " << (unsigned int)arph->ar_hln << std::endl;
    std::cout << "   |-Protocol Address Length : " << (unsigned int)arph->ar_pln << std::endl;
    std::cout << "   |-Operation : " << ntohs(arph->ar_op) << std::endl;
}

void packet_handler(u_char *user_data, const struct pcap_pkthdr *pkthdr, const u_char *packet) {
    struct ether_header *eth = (struct ether_header *)packet;

    // Check for ARP
    if (ntohs(eth->ether_type) == ETHERTYPE_ARP) {
        print_arp_header(packet);
    }
    // Check for IP
    else if (ntohs(eth->ether_type) == ETHERTYPE_IP) {
        print_ip_header(packet);

        struct ip *iph = (struct ip *)(packet + sizeof(struct ether_header));

        // Check for UDP
        if (iph->ip_p == IPPROTO_UDP) {
            print_udp_header(packet);
        }
        // Check for TCP
        else if (iph->ip_p == IPPROTO_TCP) {
            print_tcp_header(packet);
        }
        // Check for ICMP
        else if (iph->ip_p == IPPROTO_ICMP) {
            print_icmp_header(packet);
        }
    }
}

int main() {
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *handle;

    // Open the capture interface
    handle = pcap_open_live("wlp3s0", BUFSIZ, 1, 1000, errbuf);
    if (handle == NULL) {
        std::cerr << "Error opening device: " << errbuf << std::endl;
        return 1;
    }

    std::cout << "Packet Sniffer Started..." << std::endl;

    // Start packet capture
    if (pcap_loop(handle, 0, packet_handler, NULL) < 0) {
        std::cerr << "Error in pcap_loop: " << pcap_geterr(handle) << std::endl;
        return 1;
    }

    pcap_close(handle);
    return 0;
}
// g++ -o sniffer sniffer.cpp -lpcap
