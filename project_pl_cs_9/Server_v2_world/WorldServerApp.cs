using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net;
using System.Net.Sockets;
using PL_Common;
using PL_Network_v2;
using System.Runtime.CompilerServices;

namespace PL_Server_v2_World
{
    public class WorldServerApp
    {
        private static readonly Lazy<WorldServerApp> _instance = new(() => new());
        public static WorldServerApp Instance => _instance.Value;

        NetService netService = new();
        Config? config = null;
        public Config? GetConfig() => config;

        bool _running = false;
        Thread? _processThread = null;

        Dictionary<Int32, DbSession> dbSessions = new();
        Dictionary<Socket, NetSession> playerSessions = new();

        private WorldServerApp() { }

        public void AppStart(Config config) {
            this.config = config;

            do {
                Thread.Sleep(1000);
                Console.WriteLine("DbServer 연결 시도.");
            } while (false == DbConnect());

            //
            netService.ListenStart(new List<(IPEndPoint, Func<Socket, NetSession>)> {
                            (new IPEndPoint(config.ClientListener.GetIp(), config.ClientListener.GetPort()), sock => new PlayerSession(sock, config.ClientListener.MaxBufferSize)),
                        });

            //
            StartProcess();
        }

        public DbSession? GetDbSession(Int32 dbId = 1) {
            if (dbSessions.TryGetValue(dbId, out var session)) {
                if (null != session) {
                    return session;
                }
            }
            return null;
        }

        public void SetDbSession(Int32 id, DbSession session) => dbSessions[id] = session;

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

                DbSession session = new DbSession(socket, config.DbServerConnect.MaxBufferSize);
                dbSessions[config.DbServerConnect.Id] = session;

                PacketWD_JoinRequest sendPacket = new();
                sendPacket.WorldId = config.WorldId;
                session.Send(sendPacket);

                Console.WriteLine($"DbServer 연결 요청. Host:{config.DbServerConnect.GetIp()}:{config.DbServerConnect.GetPort()}");
            } catch (Exception ex) {
                Console.WriteLine($"DbServer 연결 실패. Host:{config.DbServerConnect.GetIp()}:{config.DbServerConnect.GetPort()}. Exception:{ex.Message}");
            }

            return true;
        }

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
            Console.WriteLine("WorldServerApp::ProcessThread 시작");
            while (_running) {
                Thread.Sleep(1);
            }

            Console.WriteLine("WorldServerApp::ProcessThread 종료");
        }

        public void SessionChecker() {
        }
    }
}
