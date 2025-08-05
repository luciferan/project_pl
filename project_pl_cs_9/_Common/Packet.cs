using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection.Metadata.Ecma335;
using System.Security.Cryptography.X509Certificates;
using System.Text;
using System.Threading.Tasks;

namespace PL_Common
{
    public abstract class PacketBase
    {
        Int32 packetType = 0;
        public Int32 PacketType { get { return packetType; } set { packetType = value; } }

        public PacketBase(Int32 packetType) {
            this.packetType = packetType;
        }
        public void SerializeType(Serializer ser) {
            ser.Value(ref packetType);
        }
        public void SerializeBody(Serializer ser) {
            Serialize(ser);
        }
        public abstract void Serialize(Serializer ser);
        public int GetTypeSize() { return sizeof(Int32); }
        public abstract int GetSize();
    }

    public class PacketHead
    {
        UInt32 checkHead = 0x00000000;
        Int32 packetLength = 0;
        public UInt32 CheckHead { get { return checkHead; } set { checkHead = value; } }
        public Int32 PacketLength { get { return packetLength; } set { packetLength = value; } }

        public void Serialize(Serializer ser) {
            ser.Value(ref checkHead);
            ser.Value(ref packetLength);
        }
        public int GetSize() {
            return sizeof(UInt32) + sizeof(Int32);
        }
        public static int Length { get { return sizeof(UInt32) + sizeof(Int32); } }
    }

    public class PacketBody
    {
        Int32 packetType = 0;
        byte[] packetData = Array.Empty<byte>();
        public Int32 PacketType { get { return packetType; } set { packetType = value; } }
        public byte[] PacketData { get { return packetData; } set { PacketData = value; } }

        public PacketBody() { }
        public void Serialize(Serializer ser, int bodyLength) {
            packetData = new byte[bodyLength - sizeof(Int32)];
            ser.Value(ref packetType);
            ser.Value(ref packetData, packetData.Length);
        }
        public int GetSize() {
            return sizeof(UInt32) + packetData.Length;
        }
    }

    public class PacketTail
    {
        UInt32 checkTail = 0x10000000;
        UInt32 packetSeq = 0;
        UInt32 packetTime = 0;
        public UInt32 CheckTail { get { return checkTail; } set { checkTail = value; } }
        public UInt32 PacketSeq { get { return packetSeq; } set { packetSeq = value; } }
        public UInt32 PacketTime { get { return packetTime; } set { packetTime = value; } }

        public void Serialize(Serializer ser) {
            ser.Value(ref checkTail);
            ser.Value(ref packetSeq);
            ser.Value(ref packetTime);
        }
        public int GetSize() {
            return sizeof(UInt32) * 3;
        }
        public static int Length { get { return sizeof(UInt32) * 3; } }
    }

    public class Packet
    {
        public PacketHead Head { get; } = new();
        public PacketBody Body { get; } = new();
        public PacketTail Tail { get; } = new();

        public Packet(byte[] Buffer, int length) : this(new Serializer(Buffer, length)) {
        }
        public Packet(Serializer ser) {
            Head.Serialize(ser);
            Body.Serialize(ser, Head.PacketLength - Head.GetSize() - Tail.GetSize());
            Tail.Serialize(ser);
        }
    }
}
