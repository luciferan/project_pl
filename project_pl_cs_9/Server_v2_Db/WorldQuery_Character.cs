using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using MySql.Data.MySqlClient;
using PL_Common;

namespace PL_Server_v2_Db
{
    public partial class WorldDbConnection
    {
        public Result QueryCharacterCreate(MySqlConnection conn, Int64 uid, Int64 cid, string name) {
            try {
                (int x, int y, int z) = CharacterCreateDefault.pos.GetPos();
                int zone = CharacterCreateDefault.pos.GetZone();

                string query = "";
                query = "INSERT INTO characters "
                    + "(user_id,id,name,zone,pos_x,pos_y,pos_z,inserted_at,last_update_at)"
                    + $"VALUE ({uid},{cid},\"{name}\",{zone},{x},{y},{z},NOW(),NOW())";
                using MySqlCommand create_char = new MySqlCommand(query, conn);

                if (0 < create_char.ExecuteNonQuery()) {
                    Console.WriteLine($"character insert: {uid},{cid},{name}");
                } else {
                    Console.WriteLine($"character insert fail: {uid},{cid},{name}");
                    throw new Exception($"character insert fail");
                }
            } catch (Exception ex) {
                Console.WriteLine($"QueryCharacterCreate fail. Exception {ex.Message}");
                return Result.Fail(ex);
            }

            return Result.Ok();
        }
        public void QueryCharacterDelete() { }
        public void QueryCharacterList() { }

        public Result QueryCharacterLoad(MySqlConnection conn, Int64 cid, ref CharacterDbInfo charData) {
            try {
                string query = "select name,zone,pos_x,pos_y,pos_z "
                    + "from characters "
                    + $"where id = {cid} limit 1";
                using MySqlCommand command = new(query, conn);

                using MySqlDataReader rows = command.ExecuteReader();
                if (rows.Read()) {
                    charData.name = rows.GetString("name");
                    charData.pos.SetPos(rows.GetInt32("zone"), rows.GetInt32("pos_x"), rows.GetInt32("pos_y"), rows.GetInt32("pos_z"));
                }
            } catch (Exception ex) {
                Console.WriteLine($"QueryCharacterLoad fail. Exception {ex.Message}");
                return Result.Fail(ex);
            }

            return Result.Ok();
        }
    }
}
