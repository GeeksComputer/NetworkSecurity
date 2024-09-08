#include <iostream>
#include <cstdlib>  // For system()

void blockAllPorts() {
    std::string commandInput = "sudo iptables -P INPUT DROP";
    std::string commandForward = "sudo iptables -P FORWARD DROP";
    std::string commandOutput = "sudo iptables -P OUTPUT DROP";
    
    if (system(commandInput.c_str()) == 0 &&
        system(commandForward.c_str()) == 0 &&
        system(commandOutput.c_str()) == 0) {
        std::cout << "All ports blocked successfully." << std::endl;
    } else {
        std::cerr << "Failed to block all ports." << std::endl;
    }
}

void openAllPorts() {
    std::string commandInput = "sudo iptables -P INPUT ACCEPT";
    std::string commandForward = "sudo iptables -P FORWARD ACCEPT";
    std::string commandOutput = "sudo iptables -P OUTPUT ACCEPT";
    
    if (system(commandInput.c_str()) == 0 &&
        system(commandForward.c_str()) == 0 &&
        system(commandOutput.c_str()) == 0) {
        std::cout << "All ports opened successfully." << std::endl;
    } else {
        std::cerr << "Failed to open all ports." << std::endl;
    }
}

int main() {
    // Block all ports
    blockAllPorts();
    
    // Open all ports
    // openAllPorts();
    
    return 0;
}
