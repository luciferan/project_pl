using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using MySql.Data.MySqlClient;
using PL_Common;

namespace PL_Server_v2_Db
{
    public partial class WorldDbConnection
    {
        public Result QueryItemCreate(MySqlConnection conn, Int64 cid, List<ItemDbInfo> Items) {
            try {
                string query = string.Empty;
                foreach (ItemDbInfo item in Items) {
                    query = "INSERT INTO items "
                        + "(item_sn,item_type,item_count,owner_char_id,bind,loc_type,loc_index)"
                        + "VALUES "
                        + $"({item.ItemSN},{item.ItemType},{item.Count},{cid},{item.Bind},{item.LocType},{item.LocIndex})";

                    using MySqlCommand command = new MySqlCommand(query, conn);
                    if (0 < command.ExecuteNonQuery()) {
                        Console.WriteLine($"Insert Item: charId({cid}) Item:{item.ItemSN},{item.ItemType},{item.Count},{item.Bind},{item.LocType},{item.LocIndex}");
                    } else {
                        Console.WriteLine($"Insert Item fail: charId({cid}) Item:{item.ItemSN},{item.ItemType}");
                        return Result.Fail($"Insert Item fail: charId({cid}) Item:{item.ItemSN},{item.ItemType}");
                    }
                }
            } catch (Exception ex) {
                Console.WriteLine($"Insert Item fail: {ex.Message}");
                return Result.Fail(ex);
            }

            return Result.Ok();
        }

        public Result QueryItemRemove(MySqlConnection conn, Int64 cid, List<Int64> itemSNList) {
            try {
                string query = string.Empty;

                foreach (Int64 itemSn in itemSNList) {
                    query = $"DELETE FROM items WHERE item_sn = {itemSn}";

                    using MySqlCommand command = new MySqlCommand(query, conn);
                    if (0 < command.ExecuteNonQuery()) {
                        Console.WriteLine($"Removed Item: SN{itemSn}");
                    } else {
                        Console.WriteLine($"Removed Item fail: SN{itemSn}");
                        return Result.Fail($"Removed Item fail: SN{itemSn}");
                    }
                }
            } catch (Exception ex) {
                Console.WriteLine($"Removed Item fail: {ex.Message}");
                return Result.Fail(ex);
            }

            return Result.Ok();
        }

        public Result QueryCharacterItemLoad(MySqlConnection conn, Int64 cid, ref List<ItemDbInfo> items) {
            try {
                string query = "select item_sn,item_type,item_count,bind,loc_type,loc_index "
                    + "from items "
                    + $"where owner_char_id = {cid}";
                using MySqlCommand command = new(query, conn);

                using MySqlDataReader rows = command.ExecuteReader();
                while (rows.Read()) {
                    items.Add(new ItemDbInfo(rows.GetInt64("item_sn"), rows.GetInt32("item_type"), rows.GetInt32("item_count"), rows.GetInt16("bind"), rows.GetInt16("loc_type"), rows.GetInt16("loc_index")));
                }
            } catch (Exception ex) {
                Console.WriteLine($"QueryCharacterItemLoad fail. Exception {ex.Message}");
                return Result.Fail(ex);
            }

            return Result.Ok();
        }
    }
}
