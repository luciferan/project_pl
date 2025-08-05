using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace PL_Common
{
    public enum PacketTypeDW
    {
        invalid = 0,
        join_result,
        player_auth_result,
        character_created,
        character_deleted,
        character_list,
        character_data,
        heartbeat,
        max,
    }

    //
    public class PacketDW_JoinResult : PacketBase
    {
        Int32 result = 0;
        Int32 dbId = 0;
        public Int32 Result { get { return result; } set { result = value; } }
        public Int32 DbId { get { return dbId; } set { dbId = value; } }

        public PacketDW_JoinResult() : base((Int32)PacketTypeDW.heartbeat) { }
        public PacketDW_JoinResult(byte[] buffer) : this() => SerializeBody(new Serializer(buffer, buffer.Length));
        public override void Serialize(Serializer ser) {
            ser.Value(ref result);
            ser.Value(ref dbId);
        }
        public override int GetSize() {
            return GetTypeSize() + sizeof(Int32) * 2;
        }
    }
    public class PacketDW_Heartbeat : PacketBase
    {
        public PacketDW_Heartbeat() : base((Int32)PacketTypeDW.heartbeat) { }
        public PacketDW_Heartbeat(byte[] buffer) : this() => SerializeBody(new Serializer(buffer, buffer.Length));
        public override void Serialize(Serializer ser) {
        }
        public override int GetSize() {
            return GetTypeSize();
        }
    }
}
