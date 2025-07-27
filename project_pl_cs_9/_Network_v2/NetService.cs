using PL_Common;
using System.Net;
using System.Net.Sockets;

namespace PL_Network_v2
{
    public class NetService
    {
        string _listenIp = "";
        int _listenPort = 0;
        Socket? _listenSocket = null;
        IPEndPoint? _listenPoint = null;
        int _listenWaitCount = 10;

        private List<Socket> _listenSockets = new();
        private Dictionary<Socket, Func<Socket, NetSession>> _acceptHandlers = new();

        public NetService() {

        }
        public NetService(string configPath) {
            Init(new Config(configPath));
        }
        public NetService(Config config) {
            Init(config);
        }
        public void Init(Config config) {
            if (config.ListenMode) {
                _listenIp = config.ListenIp;
                _listenPort = config.ListenPort;
                _listenWaitCount = config.ListenWaitCount;
            }
        }
        public void Connect() {
        }
        public void Disconnect() { }

        public void ListenStart() {
            try {
                _listenPoint = new IPEndPoint((_listenIp == "" ? IPAddress.Any : IPAddress.Parse(_listenIp)), _listenPort);
                if (null == _listenPoint) {
                    Console.WriteLine("NetService::ListenStart fail. Invalid ListenPoint.");
                    return;
                }
                Console.WriteLine($"NetService ListenStart. endPoint: {_listenPoint.Address}:{_listenPoint.Port}");

                _listenSocket = new(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
                _listenSocket.Bind(_listenPoint);
                _listenSocket.Listen(_listenWaitCount);

                for (int idx = 0; idx < _listenWaitCount; ++idx) {
                    SocketAsyncEventArgs args = new();
                    args.Completed += new EventHandler<SocketAsyncEventArgs>(OnAcceptCompleted);
                    AcceptRegist(args);
                }
            } catch (Exception e) {
                Console.WriteLine($"NetService::ListenStart Error: {e.Message}");
            }
        }

        void AcceptRegist(SocketAsyncEventArgs args) {
            if (null == _listenSocket) {
                return;
            }

            args.AcceptSocket = null;
            bool pending = _listenSocket.AcceptAsync(args);
            if (false == pending) {
                OnAcceptCompleted(null, args);
            }
        }

        void OnAcceptCompleted(object? sender, SocketAsyncEventArgs args) {
            if (null != args.AcceptSocket && SocketError.Success == args.SocketError) {
                Console.WriteLine($"NetworkService OnAcceptCompleted. clientSocket:{args.AcceptSocket}");
                NetSession session = new(args.AcceptSocket);
            } else {
                Console.WriteLine(args.SocketError.ToString());
            }

            AcceptRegist(args);
        }

        public void ListenStart(List<(IPEndPoint endPoint, Func<Socket, NetSession> sessionFactory)> listenConfigs) {
            foreach (var (endPoint, sessionFactory) in listenConfigs) {
                var listenSocket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
                listenSocket.Bind(endPoint);
                listenSocket.Listen(10);
                _listenSockets.Add(listenSocket);
                _acceptHandlers[listenSocket] = sessionFactory;

                var args = new SocketAsyncEventArgs();
                args.Completed += OnAcceptCompletedFromFactory;
                args.UserToken = listenSocket;
                AcceptRegist(args, listenSocket);
            }
        }

        void AcceptRegist(SocketAsyncEventArgs args, Socket listenSocket) {
            if (null == _listenSocket) {
                return;
            }

            args.AcceptSocket = null;
            bool pending = listenSocket.AcceptAsync(args);
            if (false == pending) {
                OnAcceptCompleted(listenSocket, args);
            }
        }

        void OnAcceptCompletedFromFactory(object? sender, SocketAsyncEventArgs args) {
            var listenSocket = (Socket)args.UserToken!;
            if (null != args.AcceptSocket && SocketError.Success == args.SocketError) {
                if (_acceptHandlers.TryGetValue(listenSocket, out var sessionFactory)) {
                    NetSession session = sessionFactory(args.AcceptSocket);
                }
            } else {
                Console.WriteLine($"OnAcceptCompletedFromFactory fail. {args.SocketError.ToString()}");
            }

            AcceptRegist(args, listenSocket);
        }
    }
}
