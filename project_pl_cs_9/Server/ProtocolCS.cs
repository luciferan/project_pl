using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;

namespace Server
{
    public class ProtocolCS
    {
        public const Int32 HEARTBEAT = 1;
        public const Int32 ECHO = 2;
    }

    public interface IPacketBase
    {
        byte[] GetSerialize();
        int GetSize();
    }

    public class PacketHeader : IPacketBase
    {
        public Int32 PacketType { get; set; }
        public Int32 PacketSize { get; set; }

        public PacketHeader(byte[] buffer) {
            PacketType = BitConverter.ToInt32(buffer, 0);
            PacketSize = BitConverter.ToInt32(buffer, 4);
        }

        public byte[] GetSerialize() {
            byte[] buffer = new byte[8];
            Array.Copy(BitConverter.GetBytes(PacketType), 0, buffer, 0, 4);
            Array.Copy(BitConverter.GetBytes(PacketSize), 0, buffer, 4, 4);

            return buffer;
        }

        public int GetSize() {
            return sizeof(Int32) * 2;
        }

        public void GetSerialize(byte[] packetBuffer) {
            PacketType = BitConverter.ToInt32(packetBuffer, 0);
            PacketSize = BitConverter.ToInt32(packetBuffer, 4);
        }
    }

    public class PacketBody : IPacketBase
    {
        public byte[] PacketData;

        public PacketBody(byte[] buffer) {
            PacketData = new byte[buffer.Length];
            buffer.CopyTo(PacketData, 0);
        }
        public PacketBody(byte[] buffer, Int32 offset, Int32 packetSize) {
            PacketData = new byte[packetSize];
            Array.Copy(buffer, offset, PacketData, 0, packetSize);
        }

        public byte[] GetSerialize() {
            return PacketData;
        }

        public int GetSize() {
            return PacketData.Length;
        }

        public void GetSerialize(byte[] packetBuffer, Int32 offset, Int32 bufferSize) {
            Array.Copy(PacketData, 0, packetBuffer, offset, bufferSize - offset);
        }
    }

    public class Packet : IPacketBase
    {
        PacketHeader packetHeader;
        PacketBody packetBody;

        public Packet(byte[] buffer) {
            packetHeader = new PacketHeader(buffer);
            packetBody = new PacketBody(buffer, packetHeader.GetSize(), packetHeader.PacketSize);
        }
        public byte[] GetSerialize() {
            byte[] buffer = new byte[GetSize()];
            packetHeader.GetSerialize().CopyTo(buffer, 0);
            packetBody.GetSerialize().CopyTo(buffer, packetHeader.GetSize());

            return buffer;
        }

        public int GetSize() {
            return packetHeader.GetSize() + packetBody.GetSize();
        }

        public Int32 GetPacketType() {
            return packetHeader.PacketType;
        }

        public byte[] GetPacketData() {
            return packetBody.PacketData;
        }

        public void GetSerialize(byte[] packetBuffer) {
            packetHeader.GetSerialize(packetBuffer);
            packetBody.GetSerialize(packetBuffer, 8, packetBuffer.Length);
        }
    }
}
