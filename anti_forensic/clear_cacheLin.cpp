#include <iostream>
#include <cstdlib> // For std::system

int main() {
    std::cout << "Clearing DNS cache..." << std::endl;
    
    // Try to clear cache for systemd-resolved
    if (std::system("sudo systemctl restart systemd-resolved") == 0) {
        std::cout << "DNS cache cleared successfully using systemd-resolved." << std::endl;
    } else {
        // If systemd-resolved is not used, try dnsmasq
        if (std::system("sudo systemctl restart dnsmasq") == 0) {
            std::cout << "DNS cache cleared successfully using dnsmasq." << std::endl;
        } else {
            std::cout << "Failed to clear DNS cache using known services." << std::endl;
        }
    }

    return 0;
}
