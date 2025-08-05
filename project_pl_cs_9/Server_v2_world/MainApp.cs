using System.Net;
using System.Net.Sockets;
using PL_Common;
using PL_Network_v2;

namespace PL_Server_v2_World
{
    public class MainApp
    {
        static void Main(string[] args) {
            Console.WriteLine("app start.");
            //
            string configPath = "world_server_config_v2.json";

            Config config = new();
            if (false == config.LoadConfig(configPath)) {
                return;
            }
            WorldServerApp.Instance.AppStart(config);

            //
            while (true) {
                string? cmd = Console.ReadLine();
                if (cmd == null) {
                    continue;
                }
                if (cmd.Equals("/exit")) {
                    Console.WriteLine("server exit.");
                    break;
                }
            }

            Console.WriteLine("app end.");
        }
    }
}
