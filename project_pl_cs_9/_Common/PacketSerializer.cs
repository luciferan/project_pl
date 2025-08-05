using System;
using System.Collections.Generic;
using System.Linq;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;

namespace PL_Common
{
    public class Serializer
    {
        static readonly int MaxPacketBufferSize = 1024 * 10;

        bool Write = true;
        int offset = 0;
        byte[] buffer;
        public byte[] Buffer { get { return buffer; } }

        public Serializer() {
            buffer = new byte[MaxPacketBufferSize];
        }
        public Serializer(int length) {
            buffer = new byte[length];
        }

        public Serializer(byte[] Buffer, int length) {
            Write = false;
            this.buffer = new byte[length];
            Array.Copy(Buffer, 0, this.Buffer, 0, length);
        }

        public Int32 GetSize() { return offset; }

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
        public void Value(ref char[] Data, Int32 length) {
            if (Write) {
                Value(ref length);
                Array.Copy(Data, 0, Buffer, offset, length);
            } else {
                Value(ref length);
                Array.Copy(Buffer, offset, Data, 0, length);
            }
            offset += length;
        }
        public void Value(ref byte[] Data, Int32 length) {
            if (Write) {
                Value(ref length);
                Array.Copy(Data, 0, Buffer, offset, length);
            } else {
                Value(ref length);
                Array.Copy(Buffer, offset, Data, 0, length);
            }
            offset += length;
        }
        public void Value(ref string Data, ref Int16 length) {
            if (Write) {
                Value(ref length);
                Array.Copy(Encoding.UTF8.GetBytes(Data), 0, Buffer, offset, length);
            } else {
                Value(ref length);
                Data = Encoding.UTF8.GetString(Buffer, offset, length);
            }
            offset += length;
        }

        //
        static public Serializer PacketSerializer(PacketBase body) {
            int PacketLength = PacketHead.Length + body.GetSize() + PacketTail.Length;
            Serializer ser = new Serializer(PacketLength);

            PacketHead head = new PacketHead();
            head.PacketLength = PacketLength;
            head.Serialize(ser);

            body.SerializeType(ser);
            body.SerializeBody(ser);

            PacketTail tail = new PacketTail();
            tail.Serialize(ser);

            return ser;
        }

        static public Packet PacketDeserializer(byte[] buffer, int bufferLength) {
            return new Packet(buffer, bufferLength);
        }
    }
}
