using MySql.Data.MySqlClient;
using Org.BouncyCastle.Bcpg;
using Org.BouncyCastle.Crypto.Engines;
using PL_Common;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;
using System.Transactions;

namespace PL_Server_v2_Db
{
    public class Result
    {
        public bool Success { get; set; }
        public Exception? Error { get; set; }

        public static Result Ok() => new Result { Success = true };
        public static Result Fail(Exception ex) => new Result { Success = false, Error = ex };
    }

    public partial class WorldDbConnection {
        string database = "Server=localhost;Port=3306;Database=pl_world;Uid=user_pl;Pwd=pass_pl";

        public WorldDbConnection() { }

        public void QueryTest() {
            Console.WriteLine($"QueryCharacterCreate run: {QueryCharacterCreate(1, 1, "name01")}");
            Console.WriteLine($"QueryCharacterLoad run: {QueryCharacterLoad(1, out CharacterDbInfo charData)}");
            Console.WriteLine($"QueryCharacterItemLoad run: {QueryCharacterItemLoad(1, out List<ItemDbInfo> items)}");
        }

        public bool CharacterCreate(Int64 uid, Int64 cid, string name) {
            QueryCharacterCreate(uid, cid, name);
            return true;
        }

        public bool ItemCreate(Int64 cid, List<ItemDbInfo> items) {
            using MySqlConnection conn = new MySqlConnection(database);
            MySqlTransaction? transaction = null;
            try {
                conn.Open();
                transaction = conn.BeginTransaction();

                Result res = QueryItemCreate(conn, cid, items);
                if (res.Success) {
                    transaction.Commit();
                    return true;
                } else {
                    Console.WriteLine($"ItemDelete fail: {res.Error?.Message}");
                    transaction.Rollback();
                }
            } catch (Exception ex) {
                Console.WriteLine($"ItemDelete fail: {ex.Message}");
                transaction?.Rollback();
            }
            return true;
        }

        public bool ItemRemove(Int64 cid, List<Int64> itemSnList) {
            using MySqlConnection conn = new MySqlConnection(database);
            MySqlTransaction? transaction = null;
            try {
                conn.Open();
                transaction = conn.BeginTransaction();

                Result res = QueryItemRemove(conn, cid, itemSnList);
                if (res.Success) {
                    transaction.Commit();
                    return true;
                } else {
                    Console.WriteLine($"ItemDelete fail: {res.Error?.Message}");
                    transaction.Rollback();
                }
            } catch (Exception ex) {
                Console.WriteLine($"ItemDelete fail: {ex.Message}");
                transaction?.Rollback();
            }
            return true;
        }
    }
}
