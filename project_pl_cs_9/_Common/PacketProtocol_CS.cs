using System;
using System.Collections.Generic;
using System.Linq;
using System.Security;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Linq;
using System.Xml.Schema;

namespace PL_Common
{
    public enum PacketTypeCS
    {
        Invalid = 0,
        auth,
        character_create,
        character_delete,
        enter_world,
        leave_world,
        move,
        interaction,
        chat,
        heartbeat,
        echo,
        Max,
    }

    public class PacketCS_Auth : PacketBase
    {
        Int16 userIdLen = 0;
        string userId = string.Empty;
        Int16 passwordLen = 0;
        string password = string.Empty;

        public string UserId { get { return userId; } set { userId = value; userIdLen = (Int16)userId.Length; } }
        public string Password { get { return password; } set { password = value; passwordLen = (Int16)password.Length; } }

        //
        public PacketCS_Auth() : base((Int32)PacketTypeCS.auth) { }
        public PacketCS_Auth(byte[] buffer) : this() => SerializeBody(new Serializer(buffer, buffer.Length));
        public override void Serialize(Serializer ser) {
            ser.Value(ref userId, ref userIdLen);
            ser.Value(ref password, ref passwordLen);
        }
        public override int GetSize() {
            return GetTypeSize() + sizeof(Int16) * 2 + userIdLen + passwordLen;
        }
    }

    public class PacketCS_CharacterList : PacketBase
    {
        public PacketCS_CharacterList() : base((Int32)PacketTypeCS.character_create) { }
        public PacketCS_CharacterList(byte[] buffer) : this() => SerializeBody(new Serializer(buffer, buffer.Length));
        public override void Serialize(Serializer ser) {
        }
        public override int GetSize() {
            return GetTypeSize();
        }
    }

    public class PacketCS_CharacterCreate : PacketBase
    {
        Int16 nameLen = 0;
        string name = string.Empty;

        public string CharacterName { get { return name; } set { name = value; nameLen = (Int16)name.Length; } }

        //
        public PacketCS_CharacterCreate() : base((Int32)PacketTypeCS.character_create) { }
        public PacketCS_CharacterCreate(byte[] buffer) : this() => SerializeBody(new Serializer(buffer, buffer.Length));
        public override void Serialize(Serializer ser) {
            ser.Value(ref name, ref nameLen);
        }
        public override int GetSize() {
            return GetTypeSize() + sizeof(Int16) + nameLen;
        }
    }

    public class PacketCS_CharacterDelete : PacketBase
    {
        Int64 cid = 0;

        //
        public PacketCS_CharacterDelete() : base((Int32)PacketTypeCS.character_delete) { }
        public PacketCS_CharacterDelete(byte[] buffer) : this() => SerializeBody(new Serializer(buffer, buffer.Length));
        public override void Serialize(Serializer ser) {
            ser.Value(ref cid);
        }
        public override int GetSize() {
            return GetTypeSize() + sizeof(Int64);
        }
    }

    public class PacketCS_EnterWorld : PacketBase
    {
        Int64 token = 0;
        Int64 cid = 0;
        public Int64 EnterCharacterCid { get { return cid; } set { cid = value; } }

        public PacketCS_EnterWorld() : base((Int32)PacketTypeCS.enter_world) { }
        public PacketCS_EnterWorld(byte[] buffer) : this() => SerializeBody(new Serializer(buffer, buffer.Length));
        public override void Serialize(Serializer ser) {
            ser.Value(ref token);
            ser.Value(ref cid);
        }
        public override int GetSize() {
            return GetTypeSize() + sizeof(Int64) * 2;
        }
    }

    public class PacketCS_LeaveWorld : PacketBase
    {
        public PacketCS_LeaveWorld() : base((Int32)PacketTypeCS.leave_world) { }
        public PacketCS_LeaveWorld(byte[] buffer) : this() => SerializeBody(new Serializer(buffer, buffer.Length));
        public override void Serialize(Serializer ser) {
        }
        public override int GetSize() {
            return GetTypeSize() + sizeof(Int64);
        }
    }

    public class PacketCS_Move : PacketBase
    {
        Int32 x = 0, y = 0;
        public Int32 X { get { return X; } set { X = value; } }
        public Int32 Y { get { return Y; } set { Y = value; } }

        public PacketCS_Move() : base((Int32)PacketTypeCS.move) { }
        public PacketCS_Move(byte[] buffer) : this() => SerializeBody(new Serializer(buffer, buffer.Length));
        public override void Serialize(Serializer ser) {
            ser.Value(ref x);
            ser.Value(ref y);
        }
        public override int GetSize() {
            return GetTypeSize() + sizeof(Int32) * 2;
        }
    }

    public class PacketCS_Interaction : PacketBase
    {
        Int64 targetCid = 0;
        Int32 interactionType = 0;
        public Int64 TargetToken { get { return targetCid; } set { targetCid = value; } }
        public Int32 InteractionType { get { return interactionType; } set { interactionType = value; } }

        public PacketCS_Interaction() : base((Int32)PacketTypeCS.interaction) { }
        public PacketCS_Interaction(byte[] buffer) : this() => SerializeBody(new Serializer(buffer, buffer.Length));
        public override void Serialize(Serializer ser) {
            ser.Value(ref targetCid);
            ser.Value(ref interactionType);
        }
        public override int GetSize() {
            return GetTypeSize() + sizeof(Int64) + sizeof(Int32);
        }
    }
    public class PacketCS_Chat : PacketBase
    {
        Int16 chatType = 0;
        Int16 chatMsgLength = 0;
        string chatMsg = string.Empty;
        public string ChatMsg { get { return chatMsg; } set { chatMsg = value; chatMsgLength = (Int16)chatMsg.Length; } }

        public PacketCS_Chat() : base((Int32)PacketTypeCS.chat) { }
        public PacketCS_Chat(byte[] buffer) : this() => SerializeBody(new Serializer(buffer, buffer.Length));
        public override void Serialize(Serializer ser) {
            ser.Value(ref chatType);
            ser.Value(ref chatMsg, ref chatMsgLength);
        }
        public override int GetSize() {
            return GetTypeSize() + sizeof(Int16) * 2 + chatMsgLength;
        }
    }

    public class PacketCS_Heaetbeat : PacketBase
    {
        public PacketCS_Heaetbeat() : base((Int32)PacketTypeCS.heartbeat) { }
        public PacketCS_Heaetbeat(byte[] buffer) : this() => SerializeBody(new Serializer(buffer, buffer.Length));
        public override void Serialize(Serializer ser) {
        }
        public override int GetSize() {
            return GetTypeSize();
        }
    }

    public class PacketCS_Echo : PacketBase
    {
        Int16 msgLength = 0;
        string msg = string.Empty;
        public string EchoMsg { get { return msg; } set { msg = value; msgLength = (Int16)msg.Length; } }

        public PacketCS_Echo() : base((Int32)PacketTypeCS.echo) { }
        public PacketCS_Echo(byte[] buffer) : this() => SerializeBody(new Serializer(buffer, buffer.Length));
        public override void Serialize(Serializer ser) {
            ser.Value(ref msg, ref msgLength);
        }
        public override int GetSize() {
            return GetTypeSize() + sizeof(Int16) + msgLength;
        }
    }
}
