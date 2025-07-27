using System.Net;
using System.Net.Sockets;
using PL_Common;
using PL_Network_v2;

namespace PL_Server_v2_Db
{
    public class MainApp
    {
        static NetService netService = new();

        static void Main(string[] args) {
            Console.WriteLine("server start.");

            string configPath = "db_server_config_v2.json";
            Config config = new();
            if (false == config.LoadConfig(configPath)) {
                Console.WriteLine("db_server_config_v2.json 파일을 읽어오지 못했습니다. 기본 설정 파일을 생성합니다.");
                config.DefaultConfig(configPath);
                return;
            }

            var listenConfigs = new List<(IPEndPoint, Func<Socket, NetSession>)> {
                            (new IPEndPoint(config.AuthListener.GetIp(), config.AuthListener.GetPort()), sock => new AuthSession(sock, config.MaxBufferSize)),
                            (new IPEndPoint(config.WorldListener.GetIp(), config.WorldListener.GetPort()), sock => new WorldSession(sock, config.MaxBufferSize)),
                        };
            netService.ListenStart(listenConfigs);

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
        }
    }
}
