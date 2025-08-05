using PL_Common;
using PL_Network_v2;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;

namespace PL_Server_v2_Auth
{
    public class AuthServerApp
    {
        private static readonly Lazy<AuthServerApp> _instance = new(() => new());
        public static AuthServerApp Instance => _instance.Value;

        NetService netService = new();
        Config? config { get; set; } = null;

        bool running = false;
        Thread? processThread;

        Dictionary<int, DbSession> _dbSessions = new();
        Dictionary<Socket, NetSession> _sessions = new();

        private AuthServerApp() { }
        public void SetConfig(Config config) => this.config = config;

        public void Start(Config config) {
            this.config = config;

            do {
                Thread.Sleep(1000);
                Console.WriteLine("DbServer 연결 시도.");
            } while (DbConnect());

            netService.ListenStart(new List<(IPEndPoint, Func<Socket, NetSession>)> {
                            (new IPEndPoint(config.ClientListener.GetIp(), config.ClientListener.GetPort()), sock => new PlayerSession(sock, config.ClientListener.MaxBufferSize)),
                        });

            //
            StartProcess();
        }
        public bool DbConnect() {
            if (null == config) {
                return false;
            }

            try {
                Socket socket = netService.Connect(new IPEndPoint(config.DbServerConnect.GetIp(), config.DbServerConnect.GetPort()));
                if (null == socket || !socket.Connected) {
                    Console.WriteLine($"DbServer 연결 실패. Host:{config.DbServerConnect.GetIp()}:{config.DbServerConnect.GetPort()}");
                    return false;
                }
                DbSession session = new DbSession(socket, config.ClientListener.MaxBufferSize);
                _dbSessions[config.DbServerConnect.Id] = session;
                Console.WriteLine($"DbServer 연결 성공. Host:{config.DbServerConnect.GetIp()}:{config.DbServerConnect.GetPort()}");
            } catch (Exception ex) {
                Console.WriteLine($"DbServer 연결 실패. Host:{config.DbServerConnect.GetIp()}:{config.DbServerConnect.GetPort()}. Exception:{ex.Message}");
            }

            return true;
        }

        public void StartProcess() {
            running = true;
            processThread = new Thread(ProcessThread);
            processThread.IsBackground = true;
            processThread.Start();
        }

        public void StopProcess() {
            running = false;
            processThread?.Join();
        }

        public void ProcessThread() {
            Console.WriteLine("WorldServerApp::ProcessThread 시작");
            while (running) {
                Thread.Sleep(1);
            }

            Console.WriteLine("WorldServerApp::ProcessThread 종료");
        }
    }
}
