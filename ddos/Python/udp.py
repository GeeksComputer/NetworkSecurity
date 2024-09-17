import socket
import struct
import sys
import random
import time

def create_ip_header(src_ip, dest_ip):
    # IP Header fields
    version_ihl = (4 << 4) + 5  # IPv4 and header length of 5 (20 bytes)
    tos = 0
    total_length = 0  # kernel will fill the correct total length
    identification = random.randint(0, 65535)
    flags_frag_offset = 0
    ttl = 255
    protocol = socket.IPPROTO_UDP
    checksum = 0  # kernel will fill the correct checksum

    # Pack header fields into a byte string
    ip_header = struct.pack(
        '!BBHHHBBH4s4s',
        version_ihl, tos, total_length, identification,
        flags_frag_offset, ttl, protocol, checksum,
        socket.inet_aton(src_ip), socket.inet_aton(dest_ip)
    )
    return ip_header

def create_udp_header(src_port, dest_port, length):
    # UDP Header fields
    checksum = 0  # No checksum calculation for simplicity, but should be included in production

    # Pack header fields into a byte string
    udp_header = struct.pack(
        '!HHHH',
        src_port, dest_port, length, checksum
    )
    return udp_header

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
    src_port = 12345  # Random source port
    dest_port = 54321  # Destination port (can be any port)
    payload = b'Hello, UDP!'  # Payload data
    payload_length = len(payload)
    udp_length = 8 + payload_length  # UDP header length (8 bytes) + payload length

    for _ in range(int(number_of_packets)):
        # Create IP header and UDP header
        ip_header = create_ip_header(src_ip, destination_ip)
        udp_header = create_udp_header(src_port, dest_port, udp_length)

        # Combine IP header, UDP header, and payload to create the full packet
        packet = ip_header + udp_header + payload

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

if __name__ == '__main__':
    if len(sys.argv) != 3:
        print("Usage: python send_udp_raw.py <destination_ip> <number_of_packets>")
        sys.exit(1)

    dest_ip = sys.argv[1]
    num_packets = sys.argv[2]
    main(dest_ip, num_packets)

# sudo python3 send_udp_raw.py <destination_ip> <number_of_packets>
