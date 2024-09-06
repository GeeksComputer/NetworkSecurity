#!/bin/bash

# Check if the right number of arguments is provided
if [ "$#" -ne 4 ]; then
    echo "Usage: $0 <hostname> <tcp_port> <udp_port> <ping_count>"
    exit 1
fi

HOSTNAME=$1
TCP_PORT=$2
UDP_PORT=$3
PING_COUNT=$4

# Function to send a TCP packet
send_tcp() {
    while true; do
        echo "Sending TCP packet to $HOSTNAME on port $TCP_PORT..."
        echo "This is a test message." | nc -w 3 $HOSTNAME $TCP_PORT
        sleep 1  # Optional: sleep for 1 second to avoid high CPU usage
    done
    
}

# Function to send a UDP packet
send_udp() {
    while true; do
        echo "Sending UDP packet to $HOSTNAME on port $UDP_PORT..."
        echo "This is a test message." | nc -u -w 3 $HOSTNAME $UDP_PORT
        sleep 1  # Optional: sleep for 1 second to avoid high CPU usage
    done
}

# Function to send an ICMP packet
send_icmp() {
    while true; do
        echo "Sending ICMP packet to $HOSTNAME..."
        ping -c $PING_COUNT $HOSTNAME
        sleep 1  # Optional: sleep for 1 second to avoid high CPU usage
    done
}

# Execute functions
send_tcp
send_udp
send_icmp

# chmod +x flood.sh
# ./flood.sh example.com 12345 54321 4
