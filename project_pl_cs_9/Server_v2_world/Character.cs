using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.Json;
using System.Text.Json.Serialization;
using System.Threading.Tasks;

namespace PL_Server_v2_World
{
    public class Position {
        public Int32 Zone { get; set; } = 0;
        public Int32 X { get; set; } = 0;
        public Int32 Y { get; set; } = 0;
        public Int32 Z { get; set; } = 0;

        public Position() { }
        public Position(Int32 zone, Int32 x, Int32 y, Int32 z) {
            Zone = zone;
            X = x;
            Y = Y;
            Z = z;
        }
        public void SetPos(Int32 zone, Int32 x, Int32 y, Int32 z) {
            Zone = zone;
            X = x;
            Y = Y;
            Z = z;
        }
        public (Int32 x, Int32 y, Int32 z) GetPos() {
            return (X, Y, Z);
        }
    }

    public class Character {
        [JsonIgnore] public Int32 _index { get; set; } = 0;

        public Int64 CharId { get; set; } = 0;
        public string Name { get; set; } = "";
        public Position Pos { get; set; } = new();

        public Character() { }
        public void Dump() {
            string json = JsonSerializer.Serialize(this, new JsonSerializerOptions { WriteIndented = true });
            File.WriteAllText($"character_{_index}_dump.json", json);
        }
    }
}
