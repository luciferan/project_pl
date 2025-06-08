using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Sockets;
using System.Reflection.Metadata.Ecma335;
using System.Text;
using System.Threading.Tasks;
using ZstdSharp.Unsafe;

namespace Server
{
    public class ProtocolCS
    {
        public const Int32 HEARTBEAT = 1;
        public const Int32 ECHO = 2;
    }

    public class PacketHead
    {
        public UInt32 CheckHead { get; set; }
        public Int32 PacketLength { get; set; }

        public PacketHead(byte[] Buffer) {
            int offset = 0;
            CheckHead = BitConverter.ToUInt32(Buffer, offset); offset += sizeof(UInt32);
            PacketLength = BitConverter.ToInt32(Buffer, offset); offset += sizeof(Int32);
        }

        public byte[] Serialize() {
            int offset = 0;
            byte[] Buffer = new byte[8];
            Array.Copy(BitConverter.GetBytes(CheckHead), 0, Buffer, offset, sizeof(UInt32)); offset += sizeof(UInt32);
            Array.Copy(BitConverter.GetBytes(PacketLength), 0, Buffer, offset, sizeof(UInt32)); offset += sizeof(UInt32);

            return Buffer;
        }
        public int Serialize(byte[] Buffer, int offset = 0) {
            CheckHead = BitConverter.ToUInt32(Buffer, offset); offset += sizeof(UInt32);
            PacketLength = BitConverter.ToInt32(Buffer, offset); offset += sizeof(Int32);
            return GetSize();
        }

        public int GetSize() {
            return sizeof(UInt32) * 2;
        }
    }

    public class PacketBody
    {
        public UInt32 PacketType { get; set; }
        public byte[] PacketData { get; set; } = Array.Empty<byte>();

        public PacketBody(byte[] Buffer, int Length, int offset = 0) {
            PacketData = new byte[Length - sizeof(UInt32)];
            PacketType = BitConverter.ToUInt32(Buffer, offset); offset += sizeof(UInt32);
            Array.Copy(Buffer, offset, PacketData, 0, Length - sizeof(UInt32));
        }

        public byte[] Serialize() {
            Int32 offset = 0;
            byte[] Buffer = new byte[sizeof(UInt32) + PacketData.Length];
            Array.Copy(BitConverter.GetBytes(PacketType), Buffer, sizeof(UInt32)); offset += sizeof(UInt32);
            Array.Copy(PacketData, 0, Buffer, offset, PacketData.Length);

            return Buffer;
        }

        public int Serialize(byte[] Buffer, int Length, int offset = 0) {
            PacketData = new byte[Length - sizeof(UInt32)];
            PacketType = BitConverter.ToUInt32(Buffer, offset); offset += sizeof(UInt32);
            Array.Copy(Buffer, offset, PacketData, 0, Length - offset);
            return GetSize();
        }

        public int GetSize() {
            return sizeof(UInt32) + PacketData.Length;
        }
    }

    public class PacketTail
    {
        public UInt32 CheckTail { get; set; }
        public UInt32 PacketTime { get; set; }

        public PacketTail(byte[] Buffer) {
            int offset = 0;
            CheckTail = BitConverter.ToUInt32(Buffer, offset); offset += sizeof(UInt32);
            PacketTime = BitConverter.ToUInt32(Buffer, offset); offset += sizeof(UInt32);
        }
        public PacketTail(byte[] Buffer, int Length) {
            int offset = Length - (sizeof(UInt32) * 2);
            CheckTail = BitConverter.ToUInt32(Buffer, offset); offset += sizeof(UInt32);
            PacketTime = BitConverter.ToUInt32(Buffer, offset); offset += sizeof(UInt32);
        }

        public byte[] Serialize() {
            int offset = 0;
            byte[] Buffer = new byte[sizeof(UInt32) * 2];
            Array.Copy(BitConverter.GetBytes(CheckTail), 0, Buffer, offset, sizeof(UInt32)); offset += sizeof(UInt32);
            Array.Copy(BitConverter.GetBytes(PacketTime), 0, Buffer, offset, sizeof(UInt32)); offset += sizeof(UInt32);
            return Buffer;
        }

        public int Serialize(byte[] Buffer) {
            int offset = 0;
            CheckTail = BitConverter.ToUInt32(Buffer, offset); offset += sizeof(UInt32);
            PacketTime = BitConverter.ToUInt32(Buffer, offset); offset += sizeof(UInt32);
            return GetSize();
        }

        public int GetSize() {
            return sizeof(UInt32) * 2;
        }
    }

    public class Packet
    {
        public PacketHead Head { get; set; }
        public PacketBody Body { get; set; }
        public PacketTail Tail { get; set; }

        public Packet(byte[] Buffer, int Length) {
            Head = new PacketHead(Buffer);
            Tail = new PacketTail(Buffer, Length);
            Body = new PacketBody(Buffer, Head.PacketLength - Head.GetSize() - Tail.GetSize(), Head.GetSize());
        }

        public byte[] Serialize() {
            byte[] Buffer = new byte[GetSize()];

            Int32 offset = 0;
            Head.Serialize().CopyTo(Buffer, offset); offset += Head.GetSize();
            Body.Serialize().CopyTo(Buffer, offset); offset += Body.GetSize();
            Tail.Serialize().CopyTo(Buffer, offset); offset += Tail.GetSize();

            return Buffer;
        }

        public void Serialize(byte[] Buffer) {
            Head = new PacketHead(Buffer);
            Tail = new PacketTail(Buffer, Head.PacketLength);
            Body = new PacketBody(Buffer, Head.GetSize(), Head.PacketLength - Head.GetSize() - Tail.GetSize());
        }

        public int GetSize() {
            return Head.PacketLength;
        }

        public UInt32 GetPacketType() {
            return Body.PacketType;
        }
    }

    //
    public class PacketCS_Echo
    {
        public UInt32 PacketType { get; set; }
        public Int32 EchoMsgLength { get; set; }
        public byte[] EchoMsg { get; set; } = Array.Empty<byte>();

        public PacketCS_Echo() {
        }
        public PacketCS_Echo(PacketBody body) {
            PacketType = body.PacketType;
            Int32 offset = 0;
            EchoMsgLength = BitConverter.ToInt32(body.PacketData, offset); offset += sizeof(Int32);
            EchoMsg = new byte[EchoMsgLength];
            Array.Copy(body.PacketData, offset, EchoMsg, 0, EchoMsgLength);
        }

        public byte[] Serialize() {
            int offset = 0;
            byte[] Buffer = new byte[sizeof(UInt32) + sizeof(Int32) + EchoMsgLength];
            Array.Copy(BitConverter.GetBytes(PacketType), 0, Buffer, offset, sizeof(UInt32)); offset += sizeof(UInt32);
            Array.Copy(BitConverter.GetBytes(EchoMsgLength), 0, Buffer, offset, sizeof(Int32)); offset += sizeof(Int32);
            Array.Copy(EchoMsg, 0, Buffer, offset, EchoMsgLength);
            return Buffer;
        }

        public int Serialize(byte[] Buffer, int Length, int offset) {
            PacketType = BitConverter.ToUInt32(Buffer, offset); offset += sizeof(UInt32);
            EchoMsgLength = BitConverter.ToInt32(Buffer, offset); offset += sizeof(Int32);
            EchoMsg = new byte[EchoMsgLength];
            Array.Copy(Buffer, offset, EchoMsg, 0, EchoMsgLength);
            return GetSize();
        }

        public int GetSize() {
            return sizeof(UInt32) + sizeof(Int64) + sizeof(Int32) * EchoMsgLength;
        }

    }
}
