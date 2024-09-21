import sys
from scapy.all import *

def send_beacon(ssid, iface):
    dot11 = Dot11(type=0, subtype=8, addr1='ff:ff:ff:ff:ff:ff', addr2='00:11:22:33:44:55', addr3='00:11:22:33:44:55')
    beacon = Dot11Beacon(cap='ESS+privacy')
    essid = Dot11Elt(ID='SSID', info=ssid, len=len(ssid))
    frame = RadioTap()/dot11/beacon/essid
    sendp(frame, iface=iface, inter=0.1, loop=1)

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python3 fake_beacon.py <SSID> <interface>")
        sys.exit(1)
    
    ssid = sys.argv[1]
    iface = sys.argv[2]
    send_beacon(ssid, iface)