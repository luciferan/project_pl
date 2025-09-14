using MySql.Data.MySqlClient;
using Org.BouncyCastle.Asn1.TeleTrust;
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
        public String Reason { get; set; } = "";

        public static Result Ok() => new Result { Success = true };
        public static Result Fail(Exception ex) => new Result { Success = false, Error = ex };
        public static Result Fail(String reason) => new Result { Success = false, Reason = reason };

        public override string ToString() {
            if (Success) {
                return "Success";
            } else {
                return String.Format($"Failure: reason: {Reason?.ToString()}, exception: {Error?.ToString()}");
            }
        }
    }

    public partial class WorldDbConnection
    {
        string database = "Server=localhost;Port=3306;Database=pl_world;Uid=user_pl;Pwd=pass_pl";

        public WorldDbConnection() { }

        public void QueryTest() {
            using MySqlConnection conn = new MySqlConnection(database);
            MySqlTransaction? transaction = null;
            Result res = Result.Ok();

            CharacterDbInfo charData = new();
            List<ItemDbInfo> items = new();

            try {
                conn.Open();
                transaction = conn.BeginTransaction();

                //
                Console.WriteLine($"QueryCharacterCreate run: {QueryCharacterCreate(conn, 1, 1, "name01")}");
                Console.WriteLine($"QueryCharacterLoad run: {QueryCharacterLoad(conn, 1, ref charData)}");
                Console.WriteLine($"QueryCharacterItemLoad run: {QueryCharacterItemLoad(conn, 1, ref items)}");

                transaction.Commit();
            } catch (Exception ex) {
                Console.WriteLine(ex.Message);
                transaction?.Rollback();
            }
        }

        public bool CharacterCreate(Int64 uid, Int64 cid, string name) {
            using MySqlConnection conn = new(database);
            MySqlTransaction? transaction = null;
            Result res = Result.Ok();

            try {
                conn.Open();
                transaction = conn.BeginTransaction();

                //
                res = QueryCharacterCreate(conn, uid, cid, name);
                if (false == res.Success) {
                    Console.WriteLine($"CharacterCreate fail: {res.Error?.Message}");
                    transaction?.Rollback();
                    return false;
                }

                //
                List<ItemDbInfo> ItemList = new();
                foreach (ItemDbInfo item in CharacterCreateDefault.items) {
                    ItemList.Add(new ItemDbInfo(GeneratorItemSN.GenerateSN(), item.ItemType, item.Count, item.Bind, item.LocType, item.LocIndex));
                }

                res = QueryItemCreate(conn, cid, ItemList);
                if (false == res.Success) {
                    Console.WriteLine($"CharacterCreate fail: {res.Error?.Message}");
                    transaction?.Rollback();
                    return false;
                }

                //
                if (res.Success) {
                    transaction.Commit();
                } else {
                    transaction.Rollback();
                }
            } catch (Exception ex) {
                Console.WriteLine($"CharacterCreate fail: {ex.Message}");
                transaction?.Rollback();
            }

            return true;
        }

        public bool CharacterLoad(Int64 cid, ref CharacterDbInfo charData) {
            using MySqlConnection conn = new(database);
            Result res = Result.Ok();

            try {
                conn.Open();
                res = QueryCharacterLoad(conn, cid, ref charData);
                if (false == res.Success) {
                    Console.WriteLine($"QueryCharacterLoad fail");
                    return false;
                }
            } catch (Exception ex) {
                Console.WriteLine($"CharacterLoad fail: {ex.Message}");
                return false;
            }

            return true;
        }

        public bool ItemCreate(Int64 cid, List<ItemDbInfo> items) {
            using MySqlConnection conn = new MySqlConnection(database);
            MySqlTransaction? transaction = null;
            Result res = Result.Ok();

            try {
                conn.Open();
                transaction = conn.BeginTransaction();

                res = QueryItemCreate(conn, cid, items);
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
            Result res = Result.Ok();

            try {
                conn.Open();
                transaction = conn.BeginTransaction();

                res = QueryItemRemove(conn, cid, itemSnList);
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

        public bool ItemLoad(Int64 cid, ref List<ItemDbInfo> items) {
            using MySqlConnection conn = new(database);
            Result res = Result.Ok();

            try {
                conn.Open();

                res = QueryCharacterItemLoad(conn, cid, ref items);
                if (!res.Success) {
                    Console.WriteLine($"ItemLoad fail: ");
                    return false;
                }
            } catch (Exception ex) {
                Console.WriteLine($"ItemLoad fail: {ex.Message}");
                return false;
            }

            return true;
        }
    }
}
