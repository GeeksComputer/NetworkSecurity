#!/bin/bash

# Check if the correct number of arguments is provided
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <hostname> <tcp_port>"
    exit 1
fi

HOSTNAME=$1
TCP_PORT=$2

# Function to send a TCP packet
send_tcp() {
    while true; do
        echo "Sending TCP packet to $HOSTNAME on port $TCP_PORT..."
        echo "This is a test message." | nc -w 3 $HOSTNAME $TCP_PORT
        sleep 1  # Optional: sleep for 1 second to avoid high CPU usage
    done
}

# Execute the function
send_tcp

# chmod +x tcp_flood.sh
# ./tcp_flood.sh example.com 12345