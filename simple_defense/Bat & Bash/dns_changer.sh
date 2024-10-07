#!/bin/bash

# Function to display the current DNS settings
function show_current_dns() {
    echo "Current DNS servers:"
    cat /etc/resolv.conf | grep 'nameserver'
    echo
}

# Function to change the DNS servers
function change_dns() {
    # Create a backup of the current resolv.conf
    sudo cp /etc/resolv.conf /etc/resolv.conf.backup
    echo "Backup of current DNS settings created at /etc/resolv.conf.backup"
    
    # Write new DNS settings to resolv.conf
    echo -e "" > /etc/resolv.conf # Clear the current settings
    for dns in "$@"; do
        echo "nameserver $dns" | sudo tee -a /etc/resolv.conf > /dev/null
    done
    
    echo "DNS servers changed to:"
    cat /etc/resolv.conf | grep 'nameserver'
}

# Main script
echo "DNS Changer Script"
show_current_dns

# Prompt user for new DNS servers
read -p "Enter the DNS servers you want to use (space-separated): " -a dns_servers

# Change the DNS servers
change_dns "${dns_servers[@]}"