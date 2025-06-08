using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net;
using System.Net.Sockets;

namespace Server
{
    internal class NetworkService
    {
        Socket _listenSocket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
        IPEndPoint _listenPoint = new IPEndPoint(IPAddress.Any, 16101);
        int _listenWaitCount = 10;

        Stack<PlayerSession> _playerSession = new();

        public void Start() {
            try {
                ListenStart();
            } catch (Exception e) {
                Console.WriteLine($"Error: {e.Message}");
            }
        }

        public void ListenStart() {
            Console.WriteLine($"NetworkService ListenStart. endPoint:{_listenPoint.Address},{_listenPoint.Port}, acceptWaitCount:{_listenWaitCount}");

            try {
                _listenSocket.Bind(_listenPoint);
                _listenSocket.Listen(_listenWaitCount);

                for (int idx = 0; idx < _listenWaitCount; ++idx) {
                    SocketAsyncEventArgs args = new();
                    args.Completed += new EventHandler<SocketAsyncEventArgs>(OnAcceptCompleted);
                    AcceptRegist(args);
                }
            } catch (Exception e) {
                Console.WriteLine($"Error: {e.Message}");
            }
        }

        void AcceptRegist(SocketAsyncEventArgs args) {
            args.AcceptSocket = null;

            bool pending = _listenSocket.AcceptAsync(args);
            if (false == pending) {
                OnAcceptCompleted(null, args);
            }
        }

        void OnAcceptCompleted(object? sender, SocketAsyncEventArgs args) {
            if (null != args.AcceptSocket && SocketError.Success == args.SocketError) {
                Console.WriteLine($"NetworkService OnAcceptCompleted. clientSocket:{args.AcceptSocket}");
                PlayerSession session = new(args.AcceptSocket);
            } else {
                Console.WriteLine(args.SocketError.ToString());
            }

            AcceptRegist(args);
        }
    }
}
