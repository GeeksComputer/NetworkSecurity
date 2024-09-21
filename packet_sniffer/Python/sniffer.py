import argparse
from scapy.all import sniff

def packet_callback(packet):
    print(packet.summary())

def main():
    parser = argparse.ArgumentParser(description="Packet sniffer using Scapy")
    parser.add_argument("-i", "--interface", type=str, required=True, help="Network interface to sniff on")
    parser.add_argument("-c", "--count", type=int, default=0, help="Number of packets to capture (0 for infinite)")
    args = parser.parse_args()

    print(f"Sniffing on interface {args.interface} for {args.count} packets")
    sniff(iface=args.interface, prn=packet_callback, count=args.count)

if __name__ == "__main__":
    main()