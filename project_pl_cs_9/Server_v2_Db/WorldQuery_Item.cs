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

                    MySqlCommand command = new MySqlCommand(query, conn);
                    if (0 < command.ExecuteNonQuery()) {
                        Console.WriteLine($"Insert Item: charId({cid}) Item:{item.ItemSN},{item.ItemType},{item.Count},{item.Bind},{item.LocType},{item.LocIndex}");
                    } else {
                        Console.WriteLine($"Insert Item fail: charId({cid}) Item:{item.ItemSN},{item.ItemType}");
                    }
                }
                return Result.Ok();
            } catch (Exception ex) {
                Console.WriteLine($"Insert Item fail: {ex.Message}");
                return Result.Fail(ex);
            }
        }

        public Result QueryItemRemove(MySqlConnection conn, Int64 cid, List<Int64> itemSNList) {
            try {
                string query = string.Empty;

                foreach (Int64 itemSn in itemSNList) {
                    query = $"DELETE FROM items WHERE item_sn = {itemSn}";

                    MySqlCommand command = new MySqlCommand(query, conn);
                    if (0 < command.ExecuteNonQuery()) {
                        Console.WriteLine($"Removed Item: SN{itemSn}");
                    } else {
                        Console.WriteLine($"Removed Item fail: SN{itemSn}");
                    }
                }
                return Result.Ok();
            } catch (Exception ex) {
                Console.WriteLine($"Removed Item fail: {ex.Message}");
                return Result.Fail(ex);
            }
        }
    }
}
