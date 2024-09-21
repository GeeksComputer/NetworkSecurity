import socket
import argparse

def scan_port(host, port):
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(1)
        s.connect((host, port))
    except (socket.timeout, socket.error):
        return False
    else:
        return True
    finally:
        s.close()

def main():
    parser = argparse.ArgumentParser(description="Port Scanner Using raw sockets")
    parser.add_argument("host", help="Host to Scan")
    parser.add_argument("start_port", type=int, help="Starting Port Number")
    parser.add_argument("end_port", type=int, help="Ending Port Number")
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