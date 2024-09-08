#include <iostream>
#include <cstdlib>  // For system()

void blockPort(int port) {
    std::string command = "sudo iptables -A INPUT -p tcp --dport " + std::to_string(port) + " -j DROP";
    if (system(command.c_str()) == 0) {
        std::cout << "Port " << port << " blocked successfully." << std::endl;
    } else {
        std::cerr << "Failed to block port " << port << std::endl;
    }
}

void openPort(int port) {
    std::string command = "sudo iptables -A INPUT -p tcp --dport " + std::to_string(port) + " -j ACCEPT";
    if (system(command.c_str()) == 0) {
        std::cout << "Port " << port << " opened successfully." << std::endl;
    } else {
        std::cerr << "Failed to open port " << port << std::endl;
    }
}

int main() {
    int portToBlock = 8080;
    int portToOpen = 8081;

    blockPort(portToBlock);
    openPort(portToOpen);

    return 0;
}
