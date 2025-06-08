using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Network
{
    public class PacketBase
    {
        public UInt32 PacketType = (UInt32)PacketTypeCS.Invalid;
        public PacketBase(UInt32 packetType) {
            PacketType = packetType;
        }
    }

    public class Serializer
    {
        static readonly int MaxPacketBufferSize = 1024 * 10;

        bool Write = true;
        int offset = 0;
        public byte[] Buffer = new byte[MaxPacketBufferSize];

        public void Value(ref Int16 value) {
            if (Write) {
                Array.Copy(BitConverter.GetBytes(value), 0, Buffer, offset, sizeof(Int16));
            } else {
                value = BitConverter.ToInt16(Buffer, offset);
            }
            offset += sizeof(Int16);
        }
        public void Value(ref UInt16 value) {
            if (Write) {
                Array.Copy(BitConverter.GetBytes(value), 0, Buffer, offset, sizeof(UInt16));
            } else {
                value = BitConverter.ToUInt16(Buffer, offset);
            }
            offset += sizeof(UInt16);
        }
        public void Value(ref Int32 value) {
            if (Write) {
                Array.Copy(BitConverter.GetBytes(value), 0, Buffer, offset, sizeof(Int32));
            } else {
                value = BitConverter.ToInt32(Buffer, offset);
            }
            offset += sizeof(Int32);
        }
        public void Value(ref UInt32 value) {
            if (Write) {
                Array.Copy(BitConverter.GetBytes(value), 0, Buffer, offset, sizeof(UInt32));
            } else {
                value = BitConverter.ToUInt32(Buffer, offset);
            }
            offset += sizeof(UInt32);
        }
        public void Value(ref Int64 value) {
            if (Write) {
                Array.Copy(BitConverter.GetBytes(value), 0, Buffer, offset, sizeof(Int64));
            } else {
                value = BitConverter.ToInt64(Buffer, offset);
            }
            offset += sizeof(Int64);
        }
        public void Value(ref UInt64 value) {
            if (Write) {
                Array.Copy(BitConverter.GetBytes(value), 0, Buffer, offset, sizeof(UInt64));
            } else {
                value = BitConverter.ToUInt64(Buffer, offset);
            }
            offset += sizeof(UInt64);
        }
        public void Value(ref Single value) {
            if (Write) {
                Array.Copy(BitConverter.GetBytes(value), 0, Buffer, offset, sizeof(Single));
            } else {
                value = BitConverter.ToSingle(Buffer, offset);
            }
            offset += sizeof(Single);
        }
        public void Value(ref Double value) {
            if (Write) {
                Array.Copy(BitConverter.GetBytes(value), 0, Buffer, offset, sizeof(Double));
            } else {
                value = BitConverter.ToDouble(Buffer, offset);
            }
            offset += sizeof(Double);
        }
        public void Value(ref char[] Data, int length) {
            if (Write) {
                Array.Copy(Data, 0, Buffer, offset, length);
            } else {
                Array.Copy(Buffer, offset, Data, 0, length);
            }
            offset += length;
        }
        public void Value(ref string Data, int length) {
            if (Write) {
                Array.Copy(Encoding.UTF8.GetBytes(Data), 0, Buffer, offset, length);
            } else {
                Data = Encoding.UTF8.GetString(Buffer, offset, length);
            }
            offset += length;
        }
    }
}
