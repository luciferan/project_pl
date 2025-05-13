namespace Server
{
    internal class MainApp
    {
        static NetworkService network = new();

        static void Main(string[] args) {
            Console.Write("MainApp Start");

            network.Start();
            while (true) {

            }
        }
    }
}
