namespace Server
{
    internal class MainApp
    {
        static NetworkService Network = new();
        static DbConnector mysqlConnector = new();

        static void Main(string[] args) {
            Console.WriteLine("MainApp Start");

            //mysqlConnector.QueryTest();
            Network.Start();

            while (true) { }
        }
    }
}
