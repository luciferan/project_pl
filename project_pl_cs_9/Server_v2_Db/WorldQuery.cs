using MySql.Data.MySqlClient;
using Org.BouncyCastle.Bcpg;
using Org.BouncyCastle.Crypto.Engines;
using PL_Common;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;
using System.Transactions;

namespace PL_Server_v2_Db
{
    public class WorldDbConnection {
        string database = "Server=localhost;Port=3306;Database=pl_world;Uid=user_pl;Pwd=pass_pl";

        public WorldDbConnection() { }

        public void QueryTest() {
            Console.WriteLine($"QueryCharacterCreate run: {QueryCharacterCreate(1, 1, "name01")}");
            Console.WriteLine($"QueryCharacterLoad run: {QueryCharacterLoad(1, out CharacterDbInfo charData)}");
            Console.WriteLine($"QueryCharacterItemLoad run: {QueryCharacterItemLoad(1, out List<ItemDbInfo> items)}");
        }

        public bool QueryCharacterCreate(Int64 uid, Int64 cid, string name) {
            using MySqlConnection conn = new MySqlConnection(database);
            MySqlTransaction? transaction = null;

            try {
                conn.Open();
                transaction = conn.BeginTransaction();

                (int x, int y, int z) = CharacterCreateDefault.pos.GetPos();
                int zone = CharacterCreateDefault.pos.GetZone();

                string query = "";
                query = "INSERT INTO characters "
                    + "(user_id,id,name,zone,pos_x,pos_y,pos_z,inserted_at,last_update_at)"
                    + $"VALUE ({uid},{cid},\"{name}\",{zone},{x},{y},{z},NOW(),NOW())";
                MySqlCommand create_char = new MySqlCommand(query, conn);
                create_char.ExecuteNonQuery();

                foreach( ItemDbInfo item in CharacterCreateDefault.items) {
                    query = "INSERT INTO items "
                        + "(item_sn,item_type,item_count,owner_char_id,bind,loc_type,loc_index)"
                        + "VALUES "
                        + $"({GeneratorItemSN.GenerateSN()},{item.ItemType},{item.Count},{cid},{item.Bind},{item.LocType},{item.LocIndex})";

                    MySqlCommand craete_item = new MySqlCommand(query, conn);
                    craete_item.ExecuteNonQuery();
                }

                transaction.Commit();
                return true;
            } catch (Exception ex) {
                Console.WriteLine(ex.Message);
                transaction?.Rollback();
                return false;
            }
        }
        public void QueryCharacterDelete() { }
        public void QueryCharacterList() { }
        public bool QueryCharacterLoad(Int64 cid, out CharacterDbInfo charData) {
            charData = new();

            using MySqlConnection conn = new MySqlConnection(database);
            try {
                conn.Open();
                string query = "select name,zone,pos_x,pos_y,pos_z "
                    + "from characters " 
                    + $"where id = {cid} limit 1";
                MySqlCommand command = new MySqlCommand(query, conn);

                MySqlDataReader rows = command.ExecuteReader();
                if( rows.Read()) {
                    charData.name = rows.GetString("name");
                    charData.pos.SetPos(rows.GetInt32("zone"), rows.GetInt32("pos_x"), rows.GetInt32("pos_y"), rows.GetInt32("pos_z"));
                }

                return true;
            } catch (Exception ex) {
                Console.WriteLine(ex.Message);
                return false;
            }
        }

        public bool QueryCharacterItemLoad(Int64 cid, out List<ItemDbInfo> items) {
            items = new List<ItemDbInfo>();

            using MySqlConnection conn = new MySqlConnection(database);
            try {
                conn.Open();
                string query = "select item_sn,item_type,item_count,bind,loc_type,loc_index " 
                    + "from items " 
                    + $"where owner_char_id = {cid}";
                MySqlCommand command = new MySqlCommand(query, conn);

                MySqlDataReader rows = command.ExecuteReader();
                while (rows.Read()) {
                    items.Add(new ItemDbInfo(rows.GetInt64("item_sn"), rows.GetInt32("item_type"), rows.GetInt32("item_count"), rows.GetInt16("bind"), rows.GetInt16("loc_type"), rows.GetInt16("loc_index")));
                }

                return true;
            } catch (Exception ex) {
                Console.WriteLine(ex.Message);
                return false;
            }

        }
    }
}
