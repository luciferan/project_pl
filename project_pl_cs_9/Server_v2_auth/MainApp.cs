using System.Net;
using System.Net.Sockets;
using PL_Common;
using PL_Network_v2;

namespace PL_Server_v2_auth
{
    internal class MainApp
    {
        static void Main(string[] args) {
            Console.WriteLine("auth server start.");

            string configPath = "auth_server_config_v2.json";
            Config config = new();
            if (false == config.LoadConfig(configPath)) {
                return;
            }

            //AuthServerApp app = new(config);
            //app.Start();

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

            Console.WriteLine("auth server end.");
        }
    }
}
