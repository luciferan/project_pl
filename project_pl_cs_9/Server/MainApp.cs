using MySql.Data.MySqlClient;

namespace Server
{
    internal class MainApp
    {
        static NetworkService Network = new();
        static DbConnector mysqlConnector = new();

        static Config config = new();

        static void Main(string[] args) {
            Console.WriteLine("MainApp Start");

            config.LoadConfig("ServerConfig.json");

            //mysqlConnector.QueryTest();
            Network.Start();

            while (true) { }
        }
    }
}
