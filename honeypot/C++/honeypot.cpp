#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <cstdlib>
#include <netinet/in.h>

#define PORT 80 // Change the port according to your choice 

int main(int argc, char *argv[])
{
    int server_fd, new_socket;
    long valread;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    const char *honypot = "127.0.0.1";

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("In socket");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    memset(address.sin_zero, '\0', sizeof(address.sin_zero));

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("In bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0)
    {
        perror("In listen");
        exit(EXIT_FAILURE);
    }
    
    std::cout << "\n [+] Starting Honeypot \n\n";

    while (true)
    {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0)
        {
            perror("In accept");
            exit(EXIT_FAILURE);
        }

        char buffer[30000] = {0};
        valread = read(new_socket, buffer, sizeof(buffer) - 1);
        std::cout << buffer;
        std::cout << "\n======================================\n\n\n";
        write(new_socket, honypot, strlen(honypot));
        close(new_socket);
    }

    return 0;
}
// g++ -o honeypot honeypot.cpp
