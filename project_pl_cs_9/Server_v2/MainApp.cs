using PL_Common;
using PL_Network_v2;

namespace PL_Server_v2
{
    internal class MainApp
    {
        static NetService? netService = null;

        static void Main(string[] args) {
            //Console.WriteLine("Hello, World!");
            Console.WriteLine("server start.");

            string configPath = "server_config_v2.json";

            netService = new(configPath);
            
            netService.ListenStart();

            while( true) {
                string? cmd = Console.ReadLine();
                if( cmd == null) {
                    continue;
                }
                if( cmd.Equals("/exit")) {
                    Console.WriteLine("server exit.");
                    break;
                }
            }

        }
    }
}
