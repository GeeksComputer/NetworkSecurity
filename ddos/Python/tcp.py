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
    protocol = socket.IPPROTO_TCP
    checksum = 0  # kernel will fill the correct checksum

    # Pack header fields into a byte string
    ip_header = struct.pack(
        '!BBHHHBBH4s4s',
        version_ihl, tos, total_length, identification,
        flags_frag_offset, ttl, protocol, checksum,
        socket.inet_aton(src_ip), socket.inet_aton(dest_ip)
    )
    return ip_header

def create_tcp_header(src_ip, dest_ip, src_port, dest_port, seq_num, ack_num, flags, window_size):
    # TCP Header fields
    data_offset_res = (5 << 4)  # TCP header length of 20 bytes
    tcp_flags = flags
    urgent_pointer = 0

    # Pack header fields into a byte string
    tcp_header = struct.pack(
        '!HHLLBBHHH',
        src_port, dest_port, seq_num, ack_num,
        data_offset_res, tcp_flags, window_size,
        0, urgent_pointer
    )
    return tcp_header

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
    dest_port = 80  # Destination port (e.g., HTTP)

    for _ in range(int(number_of_packets)):
        # Create IP and TCP headers
        ip_header = create_ip_header(src_ip, destination_ip)
        tcp_header = create_tcp_header(src_ip, destination_ip, src_port, dest_port, 0, 0, 0x02, 5840)  # SYN flag

        # Calculate IP checksum
        ip_checksum = calculate_checksum(ip_header)
        ip_header = struct.pack(
            '!BBHHHBBH4s4s',
            ip_header[0], ip_header[1], ip_header[2],
            ip_header[3], ip_header[4], ip_header[5],
            ip_header[6], ip_checksum,
            ip_header[8:12], ip_header[12:]
        )
        
        # Create raw packet
        packet = ip_header + tcp_header

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
        print("Usage: python send_tcp_raw.py <destination_ip> <number_of_packets>")
        sys.exit(1)

    dest_ip = sys.argv[1]
    num_packets = sys.argv[2]
    main(dest_ip, num_packets)

# sudo python3 send_tcp_raw.py 192.168.0.1 10
