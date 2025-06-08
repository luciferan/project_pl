using MySql.Data.MySqlClient;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;

namespace Server
{
    internal class DbConnector
    {

        string database = "Server=192.168.35.120;Port=3306;Database=pl_auth;Uid=user_pl;Pwd=pass_pl";

        public void QueryTest() {
            QuerySelectTest();
            QueryInsertTest("a06", "p01");
            QueryUpdateTest("a06", "p06");
            QueryDeleteTest("a06");
        }

        public void QuerySelectTest() {
            using MySqlConnection conn = new MySqlConnection(database);

            try {
                conn.Open();
                //string user_id = "a01";
                //string password = "p01";

                string query = "select id, name, password from accounts";

                MySqlCommand command = new MySqlCommand(query, conn);
                MySqlDataReader rows = command.ExecuteReader();

                while (rows.Read()) {
                    Console.WriteLine($"{rows["id"]}, {rows["name"]}, {rows["password"]}");
                }
            } catch (Exception ex) {
                Console.WriteLine(ex.Message);
            }
        }

        public void QueryInsertTest(string user_id, string password) {
            using MySqlConnection conn = new MySqlConnection(database);
            try {
                conn.Open();

                string query = $"insert into accounts (name, password) values ('{user_id}', '{password}')";

                MySqlCommand command = new MySqlCommand(query, conn);
                if (command.ExecuteNonQuery() == 1) {
                    Console.WriteLine($"insert query succ. query: {query}");
                } else {
                    Console.WriteLine($"insert query fail. query: {query}");
                }
            } catch (Exception ex) {
                Console.WriteLine(ex.Message);
            }
        }

        public void QueryUpdateTest(string user_id, string password) {
            using MySqlConnection conn = new MySqlConnection(database);
            try {
                conn.Open();

                string query = $"update accounts set password = '{password}' where name = '{user_id}'";

                MySqlCommand command = new MySqlCommand(query, conn);
                if (command.ExecuteNonQuery() > 0) {
                    Console.WriteLine($"update query succ. query: {query}");
                } else {
                    Console.WriteLine($"update query fail. query: {query}");
                }
            } catch (Exception ex) {
                Console.WriteLine(ex.Message);
            }
        }

        public void QueryDeleteTest(string user_id) {
            using MySqlConnection conn = new MySqlConnection(database);
            try {
                conn.Open();

                string query = $"delete from accounts where name = '{user_id}'";

                MySqlCommand command = new MySqlCommand(query, conn);
                if (command.ExecuteNonQuery() > 0) {
                    Console.WriteLine($"update query succ. query: {query}");
                } else {
                    Console.WriteLine($"update query fail. query: {query}");
                }
            } catch (Exception ex) {
                Console.WriteLine(ex.Message);
            }
        }
    }
}
