using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace PL_Common
{
    public enum PacketTypeWD
    {
        invalid = 0,
        join_request,
        heartbeat,
        player_auth,
        character_create,
        character_delete,
        character_list,
        character_load,
        max,
    }

    //
    public class PacketWD_JoinRequest : PacketBase
    {
        Int32 world_id = 0;
        public Int32 WorldId { get { return world_id; } set { world_id = value; } }

        //
        public PacketWD_JoinRequest() : base((Int32)PacketTypeWD.join_request) { }
        public PacketWD_JoinRequest(byte[] buffer) : this() => SerializeBody(new Serializer(buffer, buffer.Length));
        public override void Serialize(Serializer ser) {
            ser.Value(ref world_id);
        }
        public override int GetSize() {
            return GetTypeSize() + sizeof(Int32);
        }
    }
    public class PacketWD_Heartbeat : PacketBase
    {
        public PacketWD_Heartbeat() : base((Int32)PacketTypeWD.heartbeat) { }
        public PacketWD_Heartbeat(byte[] buffer) : this() => SerializeBody(new Serializer(buffer, buffer.Length));
        public override void Serialize(Serializer ser) {
        }
        public override int GetSize() {
            return GetTypeSize();
        }
    }
    public class PacketWD_AuthRequest : PacketBase
    {
        Int16 userIdLen = 0;
        string userId = string.Empty;
        Int16 passwordLen = 0;
        string password = string.Empty;

        public string UserId { get { return userId; } set { userId = value; userIdLen = (Int16)userId.Length; } }
        public string Password { get { return password; } set { password = value; passwordLen = (Int16)password.Length; } }

        //
        public PacketWD_AuthRequest() : base((Int32)PacketTypeWD.player_auth) { }
        public PacketWD_AuthRequest(byte[] buffer) : this() => SerializeBody(new Serializer(buffer, buffer.Length));
        public override void Serialize(Serializer ser) {
            ser.Value(ref userId, ref userIdLen);
            ser.Value(ref password, ref passwordLen);
        }
        public override int GetSize() {
            return GetTypeSize() + sizeof(Int16) * 2 + userIdLen + passwordLen;
        }
    }
    public class PacketWD_CharacterLoad : PacketBase
    {
        Int64 uid = 0;
        Int64 cid = 0;

        public Int64 UserId { get { return uid; } set { uid = value; } }
        public Int64 CharacterId { get { return cid; } set { cid = value; } }

        public PacketWD_CharacterLoad() : base((Int32)PacketTypeWD.character_load) { }
        public PacketWD_CharacterLoad(byte[] buffer) : this() => SerializeBody(new Serializer(buffer, buffer.Length));
        public override void Serialize(Serializer ser) {
            ser.Value(ref uid);
            ser.Value(ref cid);
        }
        public override int GetSize() {
            return GetTypeSize() + sizeof(Int64) * 2;
        }
    }

}
