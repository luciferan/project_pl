namespace Server
{
    internal class MainApp
    {
        static NetworkService network = new();

        static void Main(string[] args) {
            network.Start();

            while (true) {

            }
        }
    }
}
