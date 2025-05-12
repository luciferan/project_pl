using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Server
{
    public class ProtocolCS
    {
        public const uint HEARTBEAT = 1;
        public const uint ECHO = 2;
    }

    public interface IPacketBase
    {
        byte[] GetSerialize();
        int GetSize();
    }

    public class PacketHeader : IPacketBase
    {
        public UInt32 PacketType;
        public UInt32 PacketSize;

        public PacketHeader(byte[] buffer) {
            PacketType = BitConverter.ToUInt32(buffer, 0);
            PacketSize = BitConverter.ToUInt32(buffer, 4);
        }

        public byte[] GetSerialize() {
            byte[] buffer = new byte[8];
            Array.Copy(BitConverter.GetBytes(PacketType), 0, buffer, 0, 4);
            Array.Copy(BitConverter.GetBytes(PacketSize), 0, buffer, 4, 4);

            return buffer;
        }

        public int GetSize() {
            return 8;
        }
    }

    public class PacketBody : IPacketBase {
        public byte[] PacketData;
        
        public PacketBody(byte[] buffer) {
            PacketData = new byte[buffer.Length];
            buffer.CopyTo(PacketData, 0);
        }
        public PacketBody(byte[] buffer, int offset) {
            PacketData = new byte[buffer.Length-offset];
            Array.Copy(buffer, offset, PacketData, 0, buffer.Length - offset);
        }

        public byte[] GetSerialize() {
            return PacketData;
        }
        public int GetSize() {
            return PacketData.Length;
        }
    }

    public class Packet : IPacketBase {
        PacketHeader packetHeader;
        PacketBody packetBody;

        public Packet(byte[] buffer) {
            packetHeader = new PacketHeader(buffer);
            packetBody = new PacketBody(buffer, packetHeader.GetSize());
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

        public UInt32 GetPacketType() {
            return packetHeader.PacketType;
        }


    }

}
