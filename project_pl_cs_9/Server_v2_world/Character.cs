using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.Json;
using System.Text.Json.Serialization;
using System.Threading.Tasks;
using PL_Common;

namespace PL_Server_v2_World
{
    public class Character : CharacterBase
    {
        [JsonIgnore] public Int32 _index { get; set; } = 0;

        public Character(Int64 uid = 0, Int64 cid = 0) : base(uid, cid) { }
        public void Dump() {
            string json = JsonSerializer.Serialize(this, new JsonSerializerOptions { WriteIndented = true });
            File.WriteAllText($"character_{_index}_dump.json", json);
        }
    }
}
