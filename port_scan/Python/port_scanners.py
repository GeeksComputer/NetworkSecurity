import argparse
from scapy.all import *

def scan_port(host, port):
    ip = IP(dst=host)
    tcp = TCP(dport=port, flags="S")
    pkt = ip/tcp
    resp = sr1(pkt, timeout=1, verbose=0)
    if resp is None:
        return False
    if resp.haslayer(TCP):
        if resp.getlayer(TCP).flags == 0x12:
            return True
        elif resp.getlayer(TCP).flags == 0x14:
            return False
    return False

def main():
    parser = argparse.ArgumentParser(description="Port Scanner using Scapy")
    parser.add_argument("host", help="Host to scan")
    parser.add_argument("start_port", type=int, help="Starting port number")
    parser.add_argument("end_port", type=int, help="Ending port number")
    args = parser.parse_args()

    host = args.host
    start_port = args.start_port
    end_port = args.end_port

    print(f"Scanning {host} from port {start_port} to {end_port}...")

    for port in range(start_port, end_port + 1):
        if scan_port(host, port):
            print(f"Port {port} is open")
        else:
            print(f"Port {port} is closed")

if __name__ == "__main__":
    main()