#include <iostream>
#include <cstdlib> // For std::system

int main() {
    std::cout << "Clearing DNS cache..." << std::endl;
    
    // Execute the command to clear the DNS cache
    int result = std::system("ipconfig /flushdns");
    
    if (result == 0) {
        std::cout << "DNS cache cleared successfully." << std::endl;
    } else {
        std::cout << "Failed to clear DNS cache." << std::endl;
    }

    return 0;
}
