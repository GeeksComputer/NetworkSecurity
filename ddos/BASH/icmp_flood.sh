#!/bin/bash

# Check if the correct number of arguments is provided
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <hostname> <ping_count>"
    exit 1
fi

HOSTNAME=$1
PING_COUNT=$2

# Function to send ICMP packets
send_icmp() {
    while true; do
        echo "Sending $PING_COUNT ICMP packets to $HOSTNAME..."
        ping -c $PING_COUNT $HOSTNAME
        sleep 1  # Optional: sleep for 1 second to avoid high CPU usage
    done
}

# Execute the function
send_icmp

# chmod +x icmp_flood.sh
# ./icmp_flood.sh example.com 4
