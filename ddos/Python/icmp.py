import socket
import struct
import sys
import time
import random

def create_ip_header(src_ip, dest_ip):
    # IP Header fields
    version_ihl = (4 << 4) + 5  # IPv4 and header length of 5 (20 bytes)
    tos = 0
    total_length = 0  # kernel will fill the correct total length
    identification = random.randint(0, 65535)
    flags_frag_offset = 0
    ttl = 255
    protocol = socket.IPPROTO_ICMP
    checksum = 0  # kernel will fill the correct checksum

    # Pack header fields into a byte string
    ip_header = struct.pack(
        '!BBHHHBBH4s4s',
        version_ihl, tos, total_length, identification,
        flags_frag_offset, ttl, protocol, checksum,
        socket.inet_aton(src_ip), socket.inet_aton(dest_ip)
    )
    return ip_header

def create_icmp_header(type, code, checksum, identifier, sequence_number):
    # ICMP Header fields
    header = struct.pack(
        '!BBHHH',
        type, code, checksum, identifier, sequence_number
    )
    return header

def calculate_checksum(data):
    # Calculate checksum of the data
    if len(data) % 2 != 0:
        data += b'\x00'
    checksum = 0
    for i in range(0, len(data), 2):
        word = (data[i] << 8) + data[i + 1]
        checksum += word
        checksum = (checksum & 0xFFFF) + (checksum >> 16)
    return ~checksum & 0xFFFF

def main(destination_ip, number_of_packets):
    src_ip = '192.168.1.1'  # Replace with your own IP address
    identifier = random.randint(0, 65535)
    sequence_number = 1
    payload = b'Hello, ICMP!'  # Payload data

    for _ in range(int(number_of_packets)):
        # Create IP header and ICMP header
        ip_header = create_ip_header(src_ip, destination_ip)
        icmp_header = create_icmp_header(8, 0, 0, identifier, sequence_number)  # Type 8 is Echo Request, Code 0
        
        # Combine IP header and ICMP header
        packet = ip_header + icmp_header + payload

        # Calculate the checksum for the ICMP header
        icmp_checksum = calculate_checksum(icmp_header + payload)
        icmp_header = create_icmp_header(8, 0, icmp_checksum, identifier, sequence_number)

        # Combine IP header and ICMP header again with the correct checksum
        packet = ip_header + icmp_header + payload

        # Send packet using raw socket
        try:
            with socket.socket(socket.AF_INET, socket.SOCK_RAW, socket.IPPROTO_RAW) as s:
                s.sendto(packet, (destination_ip, 0))
                print(f'Packet sent to {destination_ip}')
        except PermissionError:
            print("Permission denied. You need to run this script with administrative privileges.")
            sys.exit(1)
        except Exception as e:
            print(f"Error sending packet: {e}")
            sys.exit(1)

        # time.sleep(1)  # Delay between packets
        sequence_number += 1

if __name__ == '__main__':
    if len(sys.argv) != 3:
        print("Usage: python send_icmp_raw.py <destination_ip> <number_of_packets>")
        sys.exit(1)

    dest_ip = sys.argv[1]
    num_packets = sys.argv[2]
    main(dest_ip, num_packets)
