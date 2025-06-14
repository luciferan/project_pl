using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace PL_Network
{
    public enum PacketTypeCS
    {
        Invalid = 0,
        auth,
        enter,
        leave,
        move,
        interaction,
        chat,
        heartbeat,
        echo,
        Max,
    }

    public class PacketCS_Auth : PacketBase
    {
        Int32 id = 0;
        public Int32 Id { get { return id; } set { id = value; } }

        public PacketCS_Auth() : base((UInt32)PacketTypeCS.auth) { }
        public PacketCS_Auth(byte[] Buffer) : this() {
            SerializeBody(new Serializer(Buffer, Buffer.Length));
        }

        public override void Serialize(Serializer ser) {
            ser.Value(ref id);
        }
        public override int GetSize() {
            return GetTypeSize() + sizeof(Int32);
        }
    }

    public class PacketCS_Enter : PacketBase
    {
        Int64 token = 0;
        public Int64 Token { get { return token; } set { token = value; } }

        public PacketCS_Enter() : base((UInt32)PacketTypeCS.enter) { }
        public PacketCS_Enter(byte[] Buffer) : this() {
            SerializeBody(new Serializer(Buffer, Buffer.Length));
        }

        public override void Serialize(Serializer ser) {
            ser.Value(ref token);
        }
        public override int GetSize() {
            return GetTypeSize() + sizeof(Int64);
        }
    }

    public class PacketCS_Leave : PacketBase
    {
        Int64 token = 0;
        public Int64 Token { get { return token; } set { token = value; } }

        public PacketCS_Leave() : base((UInt32)PacketTypeCS.leave) { }
        public PacketCS_Leave(byte[] Buffer) : this() {
            SerializeBody(new Serializer(Buffer, Buffer.Length));
        }

        public override void Serialize(Serializer ser) {
            ser.Value(ref token);
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

        public PacketCS_Move() : base((UInt32)PacketTypeCS.move) { }
        public PacketCS_Move(byte[] Buffer) : this() {
            SerializeBody(new Serializer(Buffer, Buffer.Length));
        }

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
        Int64 targetToken = 0;
        Int32 interactionType = 0;
        public Int64 TargetToken { get { return targetToken; } set { targetToken = value; } }
        public Int32 InteractionType { get { return interactionType; } set { interactionType = value; } }

        public PacketCS_Interaction() : base((UInt32)PacketTypeCS.interaction) { }
        public PacketCS_Interaction(byte[] Buffer) : this() {
            SerializeBody(new Serializer(Buffer, Buffer.Length));
        }

        public override void Serialize(Serializer ser) {
            ser.Value(ref targetToken);
            ser.Value(ref interactionType);
        }
        public override int GetSize() {
            return GetTypeSize() + sizeof(Int64) + sizeof(Int32);
        }
    }
    public class PacketCS_Chat : PacketBase
    {
        Int16 chatType = 0;
        Int32 chatMsgLength = 0;
        string chatMsg = string.Empty;
        public Int32 ChatMsgLength { get { return chatMsgLength; } set { chatMsgLength = value; } }
        public string ChatMsg { get { return chatMsg; } set { chatMsg = value; chatMsgLength = chatMsg.Length; } }

        public PacketCS_Chat() : base((UInt32)PacketTypeCS.chat) { }
        public PacketCS_Chat(byte[] Buffer) : this() {
            SerializeBody(new Serializer(Buffer, Buffer.Length));
        }

        public override void Serialize(Serializer ser) {
            ser.Value(ref chatType);
            ser.Value(ref chatMsgLength);
            ser.Value(ref chatMsg, chatMsgLength);
        }
        public override int GetSize() {
            return GetTypeSize() + sizeof(Int32) + chatMsgLength;
        }
    }

    public class PacketCS_Heaetbeat : PacketBase
    {
        public PacketCS_Heaetbeat() : base((UInt32)PacketTypeCS.heartbeat) { }
        public PacketCS_Heaetbeat(byte[] Buffer) : this() {
            SerializeBody(new Serializer(Buffer, Buffer.Length));
        }

        public override void Serialize(Serializer ser) {
        }
        public override int GetSize() {
            return GetTypeSize();
        }
    }

    public class PacketCS_Echo : PacketBase
    {
        Int32 echoMsgLength = 0;
        string echoMsg = string.Empty;
        public Int32 EchoMsgLength { get { return echoMsgLength; } set { echoMsgLength = value; } }
        public string EchoMsg { get { return echoMsg; } set { echoMsg = value; echoMsgLength = echoMsg.Length; } }

        public PacketCS_Echo() : base((UInt32)PacketTypeCS.echo) { }
        public PacketCS_Echo(byte[] Buffer) : this() {
            SerializeBody(new Serializer(Buffer, Buffer.Length));
        }

        public override void Serialize(Serializer ser) {
            ser.Value(ref echoMsgLength);
            ser.Value(ref echoMsg, echoMsgLength);
        }
        public override int GetSize() {
            return GetTypeSize() + sizeof(Int32) + echoMsgLength;
        }
    }
}
