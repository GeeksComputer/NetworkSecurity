import socket
import struct
import sys

def block_packet(protocol):
    try:
        # Create a raw socket
        s = socket.socket(socket.AF_INET, socket.SOCK_RAW, protocol)
        
        # Set the socket to promiscuous mode
        s.setsockopt(socket.IPPROTO_IP, socket.IP_HDRINCL, 1)
        
        while True:
            # Receive packets
            packet = s.recvfrom(65565)
            print(f"Blocked packet: {packet[0]}")
            
            # Optionally, you can implement logic to drop or ignore the packet here
            
    except Exception as e:
        print(f"Error: {e}")
    finally:
        s.close()

def main():
    if len(sys.argv) != 2:
        print("Usage: python block_packets.py <protocol>")
        print("Protocol options: 'tcp', 'udp', 'icmp'")
        sys.exit(1)
    
    protocol_str = sys.argv[1].lower()
    
    protocol_map = {
        'tcp': socket.IPPROTO_TCP,
        'udp': socket.IPPROTO_UDP,
        'icmp': socket.IPPROTO_ICMP
    }
    
    if protocol_str not in protocol_map:
        print("Invalid protocol. Use 'tcp', 'udp', or 'icmp'.")
        sys.exit(1)
    
    protocol = protocol_map[protocol_str]
    block_packet(protocol)

if __name__ == "__main__":
    main()