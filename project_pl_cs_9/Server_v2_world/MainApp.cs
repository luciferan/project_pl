using System.Net;
using System.Net.Sockets;
using PL_Common;
using PL_Network_v2;

namespace PL_Server_v2_World
{
    public class MainApp
    {
        static NetService netService = new();

        bool _running = false;
        Thread? _processThread;

        static Config config = new();
        static Dictionary<Socket, NetSession> _sessions = new();

        static void Main(string[] args) {
            Console.WriteLine("server start.");

            string configPath = "world_server_config_v2.json";
            if (false == config.LoadConfig(configPath)) {
                Console.WriteLine("server_config_v2.json 파일을 읽어오지 못했습니다. 기본 설정 파일을 생성합니다.");
                config.DefaultConfig(configPath);
                return;
            }

            //
            Socket socket = netService.Connect(new IPEndPoint(config.DbServerConnect.GetIp(), config.DbServerConnect.GetPort()));
            DbSession session = new DbSession(socket, config.MaxBufferSize);
            _sessions[socket] = session;

            //
            var listenConfigs = new List<(IPEndPoint, Func<Socket, NetSession>)> {
                            (new IPEndPoint(config.ClientListener.GetIp(), config.ClientListener.GetPort()), sock => new PlayerSession(sock, config.MaxBufferSize)),
                        };
            netService.ListenStart(listenConfigs);

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
        }

        public Config GetConfig() { return config; }

        public void StartProcess() {
            _running = true;
            _processThread = new Thread(ProcessThread);
            _processThread.IsBackground = true;
            _processThread.Start();
        }

        public void StopProcess() {
            _running = false;
            _processThread?.Join();
        }

        public void ProcessThread() {
            while (_running) {
                Thread.Sleep(1);
            }
        }

        public void SessionChecker() {
            foreach (var (sock, session) in _sessions) {

            }
        }
    }
}
