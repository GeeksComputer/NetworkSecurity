import socket
import struct
import sys

def parse_ip_header(data):
    # Parse the IP header from the data
    ip_header = struct.unpack('!BBHHHBBH4s4s', data[:20])
    version_ihl = ip_header[0]
    version = version_ihl >> 4
    ihl = version_ihl & 0xF
    tos, total_length, identification, flags_frag_offset, ttl, protocol, checksum, src_ip, dest_ip = ip_header

    return {
        'version': version,
        'ihl': ihl,
        'tos': tos,
        'total_length': total_length,
        'identification': identification,
        'flags_frag_offset': flags_frag_offset,
        'ttl': ttl,
        'protocol': protocol,
        'checksum': checksum,
        'src_ip': socket.inet_ntoa(src_ip),
        'dest_ip': socket.inet_ntoa(dest_ip)
    }

def parse_tcp_header(data):
    # Parse the TCP header from the data (if present)
    if len(data) < 20:
        return None
    tcp_header = struct.unpack('!HHLLBBHHH', data[:20])
    src_port, dest_port, seq_num, ack_num, data_offset_res, flags, window_size, checksum, urgent_pointer = tcp_header

    # Extract data offset (4 bits) and reserved (4 bits) from data_offset_res
    data_offset = (data_offset_res >> 4) * 4
    return {
        'src_port': src_port,
        'dest_port': dest_port,
        'seq_num': seq_num,
        'ack_num': ack_num,
        'data_offset': data_offset,
        'flags': flags,
        'window_size': window_size,
        'checksum': checksum,
        'urgent_pointer': urgent_pointer
    }

def parse_udp_header(data):
    # Parse the UDP header from the data (if present)
    if len(data) < 8:
        return None
    udp_header = struct.unpack('!HHHH', data[:8])
    src_port, dest_port, length, checksum = udp_header

    return {
        'src_port': src_port,
        'dest_port': dest_port,
        'length': length,
        'checksum': checksum
    }

def packet_inspector(interface):
    # Create a raw socket to listen on the specified interface
    try:
        s = socket.socket(socket.AF_PACKET, socket.SOCK_RAW, socket.ntohs(0x0003))
        s.bind((interface, 0))
        print(f"Listening on {interface}...")
    except PermissionError:
        print("Permission denied. You need to run this script with administrative privileges.")
        sys.exit(1)
    except Exception as e:
        print(f"Error creating socket: {e}")
        sys.exit(1)

    while True:
        try:
            # Receive a packet
            packet = s.recvfrom(65565)[0]

            # Parse and print IP header
            ip_header = parse_ip_header(packet)
            print(f"\nIP Header:")
            print(f"  Version: {ip_header['version']}")
            print(f"  IHL: {ip_header['ihl']}")
            print(f"  TOS: {ip_header['tos']}")
            print(f"  Total Length: {ip_header['total_length']}")
            print(f"  Identification: {ip_header['identification']}")
            print(f"  Flags/Fragment Offset: {ip_header['flags_frag_offset']}")
            print(f"  TTL: {ip_header['ttl']}")
            print(f"  Protocol: {ip_header['protocol']}")
            print(f"  Checksum: {ip_header['checksum']}")
            print(f"  Source IP: {ip_header['src_ip']}")
            print(f"  Destination IP: {ip_header['dest_ip']}")

            # Extract the transport layer data
            ip_header_length = ip_header['ihl'] * 4
            transport_data = packet[ip_header_length:]

            # Determine if the packet is TCP or UDP
            protocol = ip_header['protocol']
            if protocol == socket.IPPROTO_TCP:
                # Parse and print TCP header
                tcp_header = parse_tcp_header(transport_data)
                if tcp_header:
                    print(f"TCP Header:")
                    print(f"  Source Port: {tcp_header['src_port']}")
                    print(f"  Destination Port: {tcp_header['dest_port']}")
                    print(f"  Sequence Number: {tcp_header['seq_num']}")
                    print(f"  Acknowledgment Number: {tcp_header['ack_num']}")
                    print(f"  Data Offset: {tcp_header['data_offset']}")
                    print(f"  Flags: {tcp_header['flags']}")
                    print(f"  Window Size: {tcp_header['window_size']}")
                    print(f"  Checksum: {tcp_header['checksum']}")
                    print(f"  Urgent Pointer: {tcp_header['urgent_pointer']}")
                else:
                    print("TCP header is too short to unpack.")
                    
            elif protocol == socket.IPPROTO_UDP:
                # Parse and print UDP header
                udp_header = parse_udp_header(transport_data)
                if udp_header:
                    print(f"UDP Header:")
                    print(f"  Source Port: {udp_header['src_port']}")
                    print(f"  Destination Port: {udp_header['dest_port']}")
                    print(f"  Length: {udp_header['length']}")
                    print(f"  Checksum: {udp_header['checksum']}")
                else:
                    print("UDP header is too short to unpack.")

        except KeyboardInterrupt:
            print("Stopping packet inspection.")
            break
        except Exception as e:
            print(f"Error processing packet: {e}")

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("Usage: python packet_inspector.py <interface>")
        sys.exit(1)

    interface = sys.argv[1]
    packet_inspector(interface)