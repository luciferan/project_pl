using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace PL_Common
{
    public enum PacketTypeSC
    {
        Invalid = 0,
        auth_result,
        enter,
        enter_player,
        leave,
        move,
        interaction,
        heartbeat,
        echo,
        Max,
    }

    public class PacketSC_AuthResult : PacketBase
    {
        Int32 id = 0;
        Int64 token = 0;
        public Int32 Id { get { return Id; } set { Id = value; } }
        public Int64 Token { get { return token; } set { token = value; } }

        public PacketSC_AuthResult() : base((UInt32)PacketTypeSC.auth_result) { }
        public override void Serialize(Serializer ser) {
            ser.Value(ref id);
            ser.Value(ref token);
        }
        public override int GetSize() {
            return GetTypeSize() + sizeof(Int32) + sizeof(Int64);
        }
    }

    public class PacketSC_Enter : PacketBase
    {
        Int64 token = 0;
        Int32 x = 0, y = 0;
        public Int64 Token { get { return Token; } set { Token = value; } }
        public Int32 X { get { return x; } set { x = value; } }
        public Int32 Y { get { return y; } set { y = value; } }

        public PacketSC_Enter() : base((UInt32)PacketTypeSC.enter) { }
        public override void Serialize(Serializer ser) {
            ser.Value(ref token);
            ser.Value(ref x);
            ser.Value(ref y);
        }
        public override int GetSize() {
            return GetTypeSize() + sizeof(Int64) + sizeof(Int32) * 2;
        }
    }

    internal struct CharacterDbData
    {
    }
    public class PacketSC_EnterCharacter : PacketBase
    {
        Int32 count = 0;
        List<CharacterDbData> List = new List<CharacterDbData>();

        public Int32 Count { get { return count; } set { count = value; } }
        //public List<CharacterDbData> CharacterList { get; } = new List<CharacterDbData>();

        public PacketSC_EnterCharacter() : base((uint)PacketTypeSC.enter_player) { }
        public override void Serialize(Serializer ser) {
            ser.Value(ref count);
        }
        public override int GetSize() {
            return GetTypeSize() + sizeof(Int32);
        }
    }

    public class PacketSC_Leave : PacketBase
    {
        Int64 token = 0;
        public Int64 Token { get { return token; } set { token = value; } }

        public PacketSC_Leave() : base((UInt32)PacketTypeSC.leave) { }
        public override void Serialize(Serializer ser) {
            ser.Value(ref token);
        }
        public override int GetSize() {
            return GetTypeSize() + sizeof(Int64);
        }
    }

    public class PacketSC_Move : PacketBase
    {
        Int64 token = 0;
        Int32 x = 0, y = 0;
        public Int64 Token { get { return token; } set { token = value; } }
        public Int32 X { get { return x; } set { x = value; } }
        public Int32 Y { get { return y; } set { y = value; } }

        public PacketSC_Move() : base((UInt32)PacketTypeSC.move) { }
        public override void Serialize(Serializer ser) {
            ser.Value(ref token);
            ser.Value(ref x);
            ser.Value(ref y);
        }
        public override int GetSize() {
            return GetTypeSize() + sizeof(Int64) + sizeof(Int32) * 2;
        }
    }

    public class PacketSC_Interaction : PacketBase
    {
        Int64 token = 0;
        Int64 targetToken = 0;
        Int32 interactionType = 0;
        public Int64 Token { get { return token; } set { token = value; } }
        public Int64 TargetToken { get { return targetToken; } set { targetToken = value; } }
        public Int32 InteractionType { get { return interactionType; } set { interactionType = value; } }

        public PacketSC_Interaction() : base((UInt32)PacketTypeSC.interaction) { }
        public override void Serialize(Serializer ser) {
            ser.Value(ref token);
            ser.Value(ref targetToken);
            ser.Value(ref interactionType);
        }
        public override int GetSize() {
            return GetTypeSize() + sizeof(Int64) * 2 + sizeof(Int32);
        }
    }

    public class PacketSC_HeartBeat : PacketBase
    {
        public PacketSC_HeartBeat() : base((UInt32)PacketTypeSC.heartbeat) { }
        public override void Serialize(Serializer ser) {
        }
        public override int GetSize() {
            return GetTypeSize();
        }
    }

    public class PacketSC_Echo : PacketBase
    {
        Int64 token = 0;
        Int32 echoMsgLength = 0;
        string echoMsg = string.Empty;
        public Int64 Token { get { return token; } set { token = value; } }
        public Int32 EchoMsgLength { get { return echoMsgLength; } set { echoMsgLength = value; } }
        public string EchoMsg { get { return echoMsg; } set { echoMsg = value; } }

        public PacketSC_Echo() : base((UInt32)PacketTypeSC.echo) { }
        public void SetData(Int64 token, string echoMsg) {
            this.token = token;
            this.echoMsg = echoMsg;
            this.echoMsgLength = echoMsg.Length;
        }
        public override void Serialize(Serializer ser) {
            ser.Value(ref token);
            ser.Value(ref echoMsgLength);
            ser.Value(ref echoMsg, echoMsgLength);
        }
        public override int GetSize() {
            return GetTypeSize() + sizeof(Int64) + sizeof(Int32) + echoMsgLength;
        }
    }
}
