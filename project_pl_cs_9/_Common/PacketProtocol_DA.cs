using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace PL_Common
{
    public enum PacketTypeDA
    {
        invalid = 0,
        join_result,
        heartbeat,
        max,
    }

    //
    public class PacketDA_JoinResult : PacketBase
    {
        Int32 result = 0;
        public Int32 Result { get { return result; } set { result = value; } }

        //
        public PacketDA_JoinResult() : base((Int32)PacketTypeDW.heartbeat) { }
        public PacketDA_JoinResult(byte[] buffer) : this() => SerializeBody(new Serializer(buffer, buffer.Length));
        public override void Serialize(Serializer ser) {
            ser.Value(ref result);
        }
        public override int GetSize() {
            return GetTypeSize() + sizeof(Int32);
        }
    }
    public class PacketDA_Heartbeat : PacketBase
    {
        public PacketDA_Heartbeat() : base((Int32)PacketTypeDW.heartbeat) { }
        public PacketDA_Heartbeat(byte[] buffer) : this() => SerializeBody(new Serializer(buffer, buffer.Length));
        public override void Serialize(Serializer ser) {
        }
        public override int GetSize() {
            return GetTypeSize();
        }
    }
}
