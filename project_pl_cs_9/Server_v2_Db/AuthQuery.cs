using System;
using System.Collections.Generic;
using System.Data;
using System.Linq;
using System.Runtime.Loader;
using System.Text;
using System.Threading.Tasks;
using MySql.Data.MySqlClient;
using Org.BouncyCastle.Asn1.TeleTrust;
using PL_Common;

namespace PL_Server_v2_Db
{
    public class AuthDbConnection
    {
        string database = "Server=localhost;Port=3306;Database=pl_auth;Uid=user_pl;Pwd=pass_pl";

        public AuthDbConnection() { }

        public class AccountInfo {
            public string Name { get; set; } = "";
            public string Password { get; set; } = "";

            public AccountInfo() { }
            public override string ToString() {
                return String.Format($"Name: {Name}, Password: {Password}");
            }
        }

        public void QueryTest() {
            using MySqlConnection conn = new (database);

            try {
                conn.Open();

                Console.WriteLine($"QueryAccountCount run: {QueryAccountCount(conn, out Int32 count)}: {count}");
                Console.WriteLine($"QueryAccountFirst run: {QueryAccountList(conn, out AccountInfo info)}: {info}");
            } catch (Exception ex) {
                Console.WriteLine($"QueryTest fail. Exception {ex.Message}");
            }
        }

        #region 테스트용 쿼리
        public Result QueryAccountCount(MySqlConnection conn, out int count) {
            count = 0;

            try {
                string query = "select count(*) from accounts";
                using MySqlCommand cmd = new (query, conn);

                //object? result = cmd.ExecuteScalar();
                //count = Convert.ToInt32(result);
                using MySqlDataReader rows = cmd.ExecuteReader();
                if( rows.Read()) {
                    count = rows.GetInt32(0);
                }
            } catch( Exception ex) {
                Console.WriteLine($"QueryAccountCount fail. Exception {ex.Message}");
                return Result.Fail(ex);
            }

            return Result.Ok();
        }

        public Result QueryAccountList(MySqlConnection conn, out AccountInfo info) {
            info = new();
            try {
                string query = "select id,username,password,inserted_at,deleted_at,updated_at from accounts";
                using MySqlCommand cmd = new (query, conn);

                using MySqlDataReader rows = cmd.ExecuteReader();
                if (rows.Read()) {
                    info.Name = rows.GetString("name");
                    info.Password = rows.GetString("pass");
                }
            } catch (Exception ex) {
                Console.WriteLine($"QueryAccountCount fail. Exception {ex.Message}");
                return Result.Fail(ex);
            }

            return Result.Ok();
        }
        #endregion

        public Result QueryLogin(MySqlConnection conn, string username, string password, out Int64 uid) {
            uid = 0;
            try {
                string query = $"select id from accounts where username = '{username}' and password = '{password}'";
                using MySqlCommand cmd = new MySqlCommand(query, conn);

                using MySqlDataReader rows = cmd.ExecuteReader();
                if (rows.Read()) {
                    uid = rows.GetInt64("id");
                }
            } catch (Exception ex) {
                Console.WriteLine($"QueryLogin fail. Exceptoin {ex.Message}");
                return Result.Fail(ex);
            }

            return Result.Ok();
        }
    }
}
