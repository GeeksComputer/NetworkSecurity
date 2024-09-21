import argparse
from scapy.all import IP, TCP, UDP, ICMP, send

def send_packets(packet_type, dst_ip, count):
    packet = None
    if packet_type == 'TCP':
        packet = IP(dst=dst_ip) / TCP()
    elif packet_type == 'UDP':
        packet = IP(dst=dst_ip) / UDP()
    elif packet_type == 'ICMP':
        packet = IP(dst=dst_ip)/ICMP()
    else:
        print("Unsupproted packet type. Use TCP, UDP, or ICMP.")
        return
    
    send(packet, count=count)
    print(f"sent {count} {packet_type} packets to {dst_ip}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Send TCP, UDP, or ICMP packets using Scapy.")
    parser.add_argument("packet_type", choices=["TCP", "UDP", "ICMP"], help="Type of packet to send")
    parser.add_argument("dst_ip", help="Destination IP Address")
    parser.add_argument("count", type=int, help="Number of packets to send")

    args = parser.parse_args()
    send_packets(args.packet_type, args.dst_ip, args.count)