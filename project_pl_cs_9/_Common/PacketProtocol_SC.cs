using System;
using System.Collections.Generic;
using System.ComponentModel.DataAnnotations;
using System.Data;
using System.Linq;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Linq;

namespace PL_Common
{
    public enum PacketTypeSC
    {
        Invalid = 0,
        auth_result,
        character_create,
        character_delete,
        character_list,
        enter_world,
        leave_world,
        enter_other_character,
        leave_other_character,
        move,
        interaction,
        heartbeat,
        echo,
        Max,
    }

    public class PacketSC_AuthResult : PacketBase
    {
        Int32 result = 0;
        Int64 token = 0;
        public Int32 Result { get { return result; } set { result = value; } }
        public Int64 Token { get { return token; } set { token = value; } }

        public PacketSC_AuthResult() : base((Int32)PacketTypeSC.auth_result) { }
        public PacketSC_AuthResult(byte[] buffer) : this() => SerializeBody(new Serializer(buffer, buffer.Length));
        public override void Serialize(Serializer ser) {
            ser.Value(ref result);
            ser.Value(ref token);
        }
        public override int GetSize() {
            return GetTypeSize() + sizeof(Int32) + sizeof(Int64);
        }
    }

    public class PacketSC_CharacterList : PacketBase
    {
        static Int32 MAX_CHARACTER_COUNT = 4;
        Int16 characterCount = 0;
        CharacterDataSimple[] characterList = new CharacterDataSimple[MAX_CHARACTER_COUNT];

        public PacketSC_CharacterList() : base((Int32)PacketTypeSC.character_list) { }
        public PacketSC_CharacterList(byte[] buffer) : this() => SerializeBody(new Serializer(buffer, buffer.Length));
        public override void Serialize(Serializer ser) {
            ser.Value(ref characterCount);
            for (int idx = 0; idx < characterCount; ++idx) {
                characterList[idx].Serializer(ref ser);
            }
        }
        public override int GetSize() {
            return GetTypeSize() + sizeof(Int16) + characterList[0].GetSize() * characterCount;
        }
    }

    public class PacketSC_CharacterCreate : PacketBase
    {
        Int32 result = 0;
        Int64 cid = 0;
        Int16 nameLen = 0;
        string name = string.Empty;
        public Int32 Result { get { return result; } set { result = value; } }
        public Int64 CharacterId { get { return cid; } set { cid = value; } }
        public string CharacterName { get { return name; } set { name = value; nameLen = (Int16)name.Length; } }

        //
        public PacketSC_CharacterCreate() : base((Int32)PacketTypeSC.character_create) { }
        public PacketSC_CharacterCreate(byte[] buffer) : this() => SerializeBody(new Serializer(buffer, buffer.Length));
        public override void Serialize(Serializer ser) {
            ser.Value(ref result);
            ser.Value(ref cid);
            ser.Value(ref name, ref nameLen);
        }
        public override int GetSize() {
            return GetTypeSize() + sizeof(Int32) + sizeof(Int64) + sizeof(Int16) + nameLen;
        }
    }

    public class PacketSC_CharacterDelete : PacketBase
    {
        Int64 result = 0;
        Int64 deletedCid = 0;
        public Int64 Result { get { return result; } set { result = value; } }
        public Int64 deleteCid { get { return result; } set { result = value; } }

        //
        public PacketSC_CharacterDelete() : base((Int32)PacketTypeSC.character_delete) { }
        public PacketSC_CharacterDelete(byte[] buffer) : this() => SerializeBody(new Serializer(buffer, buffer.Length));
        public override void Serialize(Serializer ser) {
            ser.Value(ref result);
            ser.Value(ref deletedCid);
        }
        public override int GetSize() {
            return GetTypeSize() + sizeof(Int32) + sizeof(Int64);
        }
    }

    public class PacketSC_EnterWorld : PacketBase
    {
        Int64 token = 0;
        CharacterInfo info = new();

        public Int64 Token { get { return Token; } set { Token = value; } }
        public CharacterInfo CharacterInfo { get { return info; } set { info = value; } }

        public PacketSC_EnterWorld() : base((Int32)PacketTypeSC.enter_world) { }
        public PacketSC_EnterWorld(byte[] buffer) : this() => SerializeBody(new Serializer(buffer, buffer.Length));

        public override void Serialize(Serializer ser) {
            ser.Value(ref token);
            info.Serializer(ref ser);
        }
        public override int GetSize() {
            return GetTypeSize() + sizeof(Int64) + sizeof(Int32) * 2;
        }
    }

    public class PacketSC_Leave : PacketBase
    {
        Int64 token = 0;
        public Int64 Token { get { return token; } set { token = value; } }

        public PacketSC_Leave() : base((Int32)PacketTypeSC.leave_world) { }
        public PacketSC_Leave(byte[] buffer) : this() => SerializeBody(new Serializer(buffer, buffer.Length));
        public override void Serialize(Serializer ser) {
            ser.Value(ref token);
        }
        public override int GetSize() {
            return GetTypeSize() + sizeof(Int64);
        }
    }

    public class PacketSC_EnterOtherCharacter : PacketBase
    {
        static readonly Int32 MaxCharacterCount = 20;
        Int32 count = 0;
        CharacterInfo[] characters = new CharacterInfo[MaxCharacterCount];

        public Int32 SetData(CharacterInfo info) {
            if (count < MaxCharacterCount) {
                characters[count] = info;
                count += 1;
            }
            return count;
        }
        public Int32 Count { get; }
        public CharacterInfo GetCharacterData(Int32 idx) => characters[idx];

        public PacketSC_EnterOtherCharacter() : base((Int32)PacketTypeSC.enter_other_character) { }
        public PacketSC_EnterOtherCharacter(byte[] Buffer) : this() => SerializeBody(new Serializer(Buffer, Buffer.Length));
        public override void Serialize(Serializer ser) {
            ser.Value(ref count);
            for (int idx = 0; idx < count; ++idx) {
                characters[idx].Serializer(ref ser);
            }
        }
        public override int GetSize() {
            return GetTypeSize() + sizeof(Int16) + characters[0].GetSize() * count;
        }
    }
    public class PacketSC_LeaveOtherCharacter : PacketBase
    {
        static readonly Int32 MaxCharacterCount = 20;
        Int32 count = 0;
        Int64[] leaveCharacterCid = new Int64[MaxCharacterCount];

        public Int32 SetData(Int64 cid) {
            leaveCharacterCid[count] = cid;
            count += 1;
            return count;
        }
        public Int32 Count { get; }
        public Int64 GetCharacterData(Int32 idx) => leaveCharacterCid[idx];

        public PacketSC_LeaveOtherCharacter() : base((Int32)PacketTypeSC.enter_other_character) { }
        public PacketSC_LeaveOtherCharacter(byte[] Buffer) : this() => SerializeBody(new Serializer(Buffer, Buffer.Length));
        public override void Serialize(Serializer ser) {
            ser.Value(ref count);
            for (int idx = 0; idx < count; ++idx) {
                ser.Value(ref leaveCharacterCid[idx]);
            }
        }
        public override int GetSize() {
            return GetTypeSize() + sizeof(Int32) + sizeof(Int64) * count;
        }
    }

    public class PacketSC_Move : PacketBase
    {
        Int64 cid = 0;
        Position pos = new();
        public Int64 CharacterId { get { return cid; } set { cid = value; } }

        public PacketSC_Move() : base((Int32)PacketTypeSC.move) { }
        public PacketSC_Move(byte[] Buffer) : this() => SerializeBody(new Serializer(Buffer, Buffer.Length));
        public override void Serialize(Serializer ser) {
            ser.Value(ref cid);
            pos.Serializer(ref ser);
        }
        public override int GetSize() {
            return GetTypeSize() + sizeof(Int64) + pos.GetSize();
        }
    }

    public class PacketSC_Interaction : PacketBase
    {
        Int64 cid = 0;
        Int64 targetCid = 0;
        Int32 interactionType = 0;
        public Int64 CharId { get { return cid; } set { cid = value; } }
        public Int64 TargetCid { get { return targetCid; } set { targetCid = value; } }
        public Int32 InteractionType { get { return interactionType; } set { interactionType = value; } }

        public PacketSC_Interaction() : base((Int32)PacketTypeSC.interaction) { }
        public PacketSC_Interaction(byte[] Buffer) : this() {
            SerializeBody(new Serializer(Buffer, Buffer.Length));
        }
        public override void Serialize(Serializer ser) {
            ser.Value(ref cid);
            ser.Value(ref targetCid);
            ser.Value(ref interactionType);
        }
        public override int GetSize() {
            return GetTypeSize() + sizeof(Int64) * 2 + sizeof(Int32);
        }
    }

    public class PacketSC_Heartbeat : PacketBase
    {
        public PacketSC_Heartbeat() : base((Int32)PacketTypeSC.heartbeat) { }
        public PacketSC_Heartbeat(byte[] Buffer) : this() => SerializeBody(new Serializer(Buffer, Buffer.Length));
        public override void Serialize(Serializer ser) {
        }
        public override int GetSize() {
            return GetTypeSize();
        }
    }

    public class PacketSC_Echo : PacketBase
    {
        Int64 cid = 0;
        Int16 msgLength = 0;
        string msg = string.Empty;
        public Int64 CharId { get { return cid; } set { cid = value; } }
        public string EchoMsg { get { return msg; } set { msg = value; msgLength = (Int16)msg.Length; } }

        public PacketSC_Echo() : base((Int32)PacketTypeSC.echo) { }
        public PacketSC_Echo(byte[] Buffer) : this() => SerializeBody(new Serializer(Buffer, Buffer.Length));

        public override void Serialize(Serializer ser) {
            ser.Value(ref cid);
            ser.Value(ref msg, ref msgLength);
        }
        public override int GetSize() {
            return GetTypeSize() + sizeof(Int64) + sizeof(Int16) + msgLength;
        }
    }
}
