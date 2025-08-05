using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net;
using System.Net.Sockets;
using PL_Common;
using PL_Network_v2;

namespace PL_Server_v2_Db
{
    public class DbServerApp
    {
        private static readonly Lazy<DbServerApp> _instance = new(() => new());
        public static DbServerApp Instance => _instance.Value;

        Config? config = null;
        NetService netService = new();
        public Config? GetConfig() => config;
        public Int32 GetServerId() => config?.DbId ?? 0;

        bool _running = false;
        Thread? _processThread = null;

        Dictionary<Int32, AuthSession> authSessions = new();
        Dictionary<Int32, WorldSession> worldSessions = new();

        private DbServerApp() { }

        public void AppStart(Config config) {
            this.config = config;

            netService.ListenStart(new List<(IPEndPoint, Func<Socket, NetSession>)> {
                            (new IPEndPoint(config.AuthListener.GetIp(), config.AuthListener.GetPort()), sock => new AuthSession(sock, config.AuthListener.MaxBufferSize)),
                            (new IPEndPoint(config.WorldListener.GetIp(), config.WorldListener.GetPort()), sock => new WorldSession(sock, config.WorldListener.MaxBufferSize)),
                        });

            StartProcess();
        }

        public AuthSession? GetAuthSession(Int32 id = 0) {
            if (authSessions.TryGetValue(id, out var session)) {
                if (null != session) {
                    return session;
                }
            }
            return null;
        }
        public WorldSession? GetWorldSession(Int32 id = 0) {
            if (worldSessions.TryGetValue(id, out var session)) {
                if (null != session) {
                    return session;
                }
            }
            return null;
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
    }
}
