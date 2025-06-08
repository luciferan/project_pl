using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Network
{
    public enum PacketTypeCS
    {
        Invalid = 0,
        auth,
        enter,
        leave,
        move,
        interaction,
        heartbeat,
        echo,
        Max,
    }

    public class PacketCS_Auth : PacketBase
    {
        public Int32 id = 0;
        public Int32 Id { get { return id; } set { id = value; } }

        public PacketCS_Auth() : base((UInt32)PacketTypeCS.auth) { }
        public void Serialize(Serializer ser) {
            ser.Value(ref id);
        }
    }

    public class PacketCS_Eneter : PacketBase
    {
        public Int64 token = 0;
        public Int64 Token { get { return token; } set { token = value; } }

        public PacketCS_Eneter() : base((UInt32)PacketTypeCS.enter) { }
        public void Serialize(Serializer ser) {
            ser.Value(ref token);
        }
    }

    public class PacketCS_Leave : PacketBase
    {
        public Int64 token = 0;
        public Int64 Token { get { return token; } set { token = value; } }

        public PacketCS_Leave() : base((UInt32)PacketTypeCS.leave) { }
        public void Serialize(Serializer ser) {
            ser.Value(ref token);
        }
    }

    public class PacketCS_Move : PacketBase
    {
        public Int32 x = 0, y = 0;
        public Int64 X { get { return X; } set { X = value; } }
        public Int64 Y { get { return Y; } set { Y = value; } }

        public PacketCS_Move() : base((UInt32)PacketTypeCS.move) { }
        public void Serialize(Serializer ser) {
            ser.Value(ref x);
            ser.Value(ref y);
        }
    }

    public class PacketCS_Interaction : PacketBase
    {
        public Int64 targetToken = 0;
        public Int32 interactionType = 0;
        public Int64 TargetToken { get { return targetToken; } set { targetToken = value; } }
        public Int32 InteractionType { get { return interactionType; } set { interactionType = value; } }

        public PacketCS_Interaction() : base((UInt32)PacketTypeCS.interaction) { }
        public void Serialize(Serializer ser) {
            ser.Value(ref targetToken);
            ser.Value(ref interactionType);
        }
    }

    public class PacketCS_Heaetbeat : PacketBase
    {
        public PacketCS_Heaetbeat() : base((UInt32)PacketTypeCS.heartbeat) { }
        public void Serialize(Serializer ser) {
        }
    }

    public class PacketCS_Echo : PacketBase
    {
        public Int32 echoMsgLength = 0;
        public string echoMsg = string.Empty;
        public Int32 EchoMsgLength { get { return echoMsgLength; } set { echoMsgLength = value; } }
        public string EchoMsg { get { return echoMsg; } set { echoMsg = value; } }

        public PacketCS_Echo() : base((UInt32)PacketTypeCS.echo) { }
        public void Serialize(Serializer ser) {
            ser.Value(ref echoMsgLength);
            ser.Value(ref echoMsg, echoMsgLength);
        }
    }
}
