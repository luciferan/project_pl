using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace PL_Common
{
    public enum PacketType_AD
    {
        invalid = 0,
        join_request,
        heartbeat,
        player_auth,
        character_create,
        character_delete,
        character_list,
        max,
    }

    //
    public class PacketAD_JoinRequest : PacketBase
    {
        Int32 auth_id = 0;
        public Int32 WorldId { get { return auth_id; } set { auth_id = value; } }

        //
        public PacketAD_JoinRequest() : base((Int32)PacketTypeWD.heartbeat) { }
        public PacketAD_JoinRequest(byte[] buffer) : this() => SerializeBody(new Serializer(buffer, buffer.Length));
        public override void Serialize(Serializer ser) {
            ser.Value(ref auth_id);
        }
        public override int GetSize() {
            return GetTypeSize() + sizeof(Int32);
        }
    }
    public class PacketAD_Heartbeat : PacketBase
    {
        public PacketAD_Heartbeat() : base((Int32)PacketTypeWD.heartbeat) { }
        public PacketAD_Heartbeat(byte[] buffer) : this() => SerializeBody(new Serializer(buffer, buffer.Length));
        public override void Serialize(Serializer ser) {
        }
        public override int GetSize() {
            return GetTypeSize();
        }
    }
}
