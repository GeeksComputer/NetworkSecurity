import socket
import sys

def unblock_packet(protocol):
    try:
        # Create a raw socket
        s = socket.socket(socket.AF_INET, socket.SOCK_RAW, protocol)
        
        # Set the socket to promiscuous mode
        s.setsockopt(socket.IPPROTO_IP, socket.IP_HDRINCL, 1)
        
        print(f"Listening for {protocol} packets...")
        
        while True:
            # Receive packets
            packet, addr = s.recvfrom(65565)
            print(f"Received packet from {addr}: {packet[:100]}")  # Print first 100 bytes for brevity
            
    except Exception as e:
        print(f"Error: {e}")
    finally:
        s.close()

def main():
    if len(sys.argv) != 2:
        print("Usage: python unblock_packets.py <protocol>")
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
    unblock_packet(protocol)

if __name__ == "__main__":
    main()
