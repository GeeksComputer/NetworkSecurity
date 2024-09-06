#!/bin/bash

# Check if the correct number of arguments is provided
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <hostname> <udp_port>"
    exit 1
fi

HOSTNAME=$1
UDP_PORT=$2

# Function to send a UDP packet
send_udp() {
    while true; do
        echo "Sending UDP packet to $HOSTNAME on port $UDP_PORT..."
        echo "This is a test message." | nc -u -w 3 $HOSTNAME $UDP_PORT
        sleep 1  # Optional: sleep for 1 second to avoid high CPU usage
    done
}

# Execute the function
send_udp

# chmod +x udp_flood.sh
# ./udp_flood.sh example.com 12345
