using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.Serialization;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Linq;

namespace PL_Common
{
    public class Position
    {
        Int32 x = 0, y = 0, z = 0, zone = 0;

        public Position() { }
        public Position(Int32 zone, Int32 x, Int32 y, Int32 z) {
            SetPos(zone, x, y, z);
        }
        public void SetPos(Int32 zone, Int32 x, Int32 y, Int32 z) {
            this.x = x;
            this.y = y;
            this.x = z;
            this.zone = zone;
        }
        public (Int32 x, Int32 y, Int32 z) GetPos() {
            return (x, y, z);
        }
        public Int32 GetZone() { return zone; }

        public void Serializer(ref Serializer ser) {
            ser.Value(ref x);
            ser.Value(ref y);
            ser.Value(ref z);
            ser.Value(ref zone);
        }
        public Int32 GetSize() { return sizeof(Int32) * 4; }
    }

    public class CharacterBase
    {
        public Int64 UID { get; set; } = 0;
        public Int64 CID { get; set; } = 0;
        public string Name { get; set; } = "";
        public Position pos = new();
        public Int32 Hp { get; set; } = 0;

        public CharacterBase(Int64 uid, Int64 cid) { }
    }

    public class CharacterDataSimple
    {
        Int16 order = 0;
        Int64 cid = 0;
        Int16 nameLen = 0;
        string name = string.Empty;

        public Int16 Order { get { return order; } set { order = value; } }
        public Int64 CID { get { return cid; } set { cid = value; } }
        public string Name { get { return name; } set { name = value; nameLen = (Int16)name.Length; } }

        public void Serializer(ref Serializer ser) {
            ser.Value(ref order);
            ser.Value(ref cid);
            ser.Value(ref name, ref nameLen);
        }
        public Int32 GetSize() {
            return sizeof(Int16) + sizeof(Int64) + sizeof(Int16) + nameLen;
        }
    }

    public class CharacterInfo
    {
        public Int64 cid;
        public Int16 nameLen;
        public string name = string.Empty;
        public Position pos = new();
        public Int32 hp = 0;
        public Int32 hpMax = 0;
        public Int32 atk = 0;
        public Int32 def = 0;
        public Int32 moveSpeed = 0;

        public void Serializer(ref Serializer ser) {
            ser.Value(ref cid);
            ser.Value(ref name, ref nameLen);
            pos.Serializer(ref ser);
            ser.Value(ref hp);
            ser.Value(ref hpMax);
            ser.Value(ref atk);
            ser.Value(ref def);
            ser.Value(ref moveSpeed);
        }
        public Int32 GetSize() {
            return sizeof(Int64) + sizeof(Int16) + nameLen + pos.GetSize() + sizeof(Int32) * 5;
        }
    }

    public enum ItemLocationType {
        Equip = 0,
        Inventory,
        Bank,
        Restrict = 250,
        Max,
    }

    public enum ItemBindType {
        None = 0,
        Character,
        Account,
        Unbind,
        Max,
    }

    public struct CharacterDbInfo
    {
        private Int64 _uid= 0;
        private Int64 _cid= 0;
        private Int16 _nameLen = 0;
        private string _name = string.Empty;

        public Int64 uid { get => _uid; set => _uid = value; }
        public Int64 cid { get => _cid; set => _cid = value; }
        public string name {
            get { return _name; }
            set { _name = value; _nameLen = (Int16)_name.Length; }
        }
        public Position pos = new();

        public CharacterDbInfo() { }

        public void Serializer(ref Serializer ser) {
            ser.Value(ref _uid);
            ser.Value(ref _cid);
            ser.Value(ref _name, ref _nameLen);
        }
        public Int32 GetSize() {
            return sizeof(Int64) * 2 + sizeof(Int16) + _nameLen + pos.GetSize();
        }
    }

    public struct ItemDbInfo {
        private Int64 _itemSN = 0;
        private Int32 _itemType = 0;
        private Int32 _count = 0;
        private Int16 _bind = 0;
        private Int16 _locType = 0;
        private Int16 _locIndex = 0;

        public Int64 ItemSN { get => _itemSN; set => _itemSN = value; }
        public Int32 ItemType { get => _itemType; set => _itemType = value; }
        public Int32 Count { get => _count; set => _count = value; }
        public Int16 Bind { get => _bind; set => _bind = value; }
        public Int16 LocType { get =>_locType; set => _locType = value; }
        public Int16 LocIndex {  get => _locIndex; set => _locIndex = value; }

        public ItemDbInfo(Int64 itemSN, Int32 itemType, Int32 count, Int16 bind, Int16 locType, Int16 locIndex) {
            ItemSN = itemSN;
            ItemType = itemType;
            Count = count;
            Bind = bind;
            LocType = locType;
            LocIndex = locIndex;
        }

        public void Serializer(ref Serializer ser) {
            ser.Value(ref _itemSN);
            ser.Value(ref _itemType);
            ser.Value(ref _count);
            ser.Value(ref _bind);
            ser.Value(ref _locType);
            ser.Value(ref _locIndex);
        }
        public Int32 GetSize() {
            return sizeof(Int64) + sizeof(Int32) * 2 + sizeof(Int16) * 3;
        }
    }

    public class CharacterCreateDefault
    {
        public static readonly Position pos = new(0, 0, 0, 0);

        public static readonly List<ItemDbInfo> items = new List<ItemDbInfo> {
            new ItemDbInfo(0, 101, 1, (Int16)ItemBindType.None, (Int16)ItemLocationType.Equip, 0),
            new ItemDbInfo(0, 501, 10, (Int16)ItemBindType.None, (Int16)ItemLocationType.Inventory, 0)
        };

        public CharacterCreateDefault() { }
    }

    public class GeneratorItemSN
    {
        // 0x0000000000000000
        // 0xffffffff00000000: utc_time_sec
        // 0x00000000ff000000: server_id (0~255)
        // 0x0000000000ffffff: sequance_id (0~16777215)

        private static Int32 _lastTimeSec = 0;
        private static byte _serverId = 0;
        private static Int32 _sequance = 0;

        public static void Init(Int32 serverId = 0) {
            _serverId = (byte)serverId;
        }
        public static Int64 GenerateSN() {
            Int32 currTimeSec = (int)DateTimeOffset.UtcNow.ToUnixTimeSeconds();

            if( currTimeSec != _lastTimeSec) {
                _lastTimeSec = currTimeSec;
                _sequance = 0;
            } else {
                _sequance += 1;
                if (_sequance > 0xFFFFFF) {
                    throw new ArgumentOutOfRangeException(nameof(_sequance), "Sequence must be <= 0xFFFFFF");
                }
            }

            return ((Int64)_lastTimeSec << 32) | ((Int64)_serverId << 24) | ((Int64)_sequance & 0xFFFFFF);
        }
    }
}
