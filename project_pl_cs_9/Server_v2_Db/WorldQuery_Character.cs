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
                if( 0 < create_char.ExecuteNonQuery()) {
                    Console.WriteLine($"character insert: {uid},{cid},{name}");
                } else {
                    Console.WriteLine($"character insert fail: {uid},{cid},{name}");
                    throw new Exception($"character insert fail");
                }

                //
                List<ItemDbInfo> createItems = new();
                foreach (ItemDbInfo item in CharacterCreateDefault.items) {
                    Int64 itemSn = GeneratorItemSN.GenerateSN();
                    query = "INSERT INTO items "
                        + "(item_sn,item_type,item_count,owner_char_id,bind,loc_type,loc_index)"
                        + "VALUES "
                        + $"({itemSn},{item.ItemType},{item.Count},{cid},{item.Bind},{item.LocType},{item.LocIndex})";

                    MySqlCommand craete_item = new MySqlCommand(query, conn);
                    if (0 < craete_item.ExecuteNonQuery()) {
                        Console.WriteLine($"character item insert: {itemSn},{item.ItemType},{item.Count}");
                        createItems.Add(new ItemDbInfo(itemSn, item.ItemType, item.Count, item.Bind, item.LocType, item.LocIndex));
                    } else {
                        Console.WriteLine($"character item insert fail: {itemSn},{item.ItemType},{item.Count}");
                        throw new Exception($"character item insert fail");
                    }
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
                if (rows.Read()) {
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
