using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net.Sockets;

namespace PL_Network_v2
{
    public class NetSession
    {
        protected Socket? _socket = null;
        protected int _disconnected = 0;

        protected int _maxBufferSize = 1024 * 10;

        protected SocketAsyncEventArgs _recvArgs = new();
        protected byte[]? _recvBuffer = null;
        protected int _recvLength = 0;
        protected int _currPos = 0;
        protected int _readPos = 0;

        protected SocketAsyncEventArgs _sendArgs = new();
        protected object _sendLock = new();
        protected Queue<ArraySegment<byte>> _sendQueue = new();
        protected List<ArraySegment<byte>> _sendPendingList = new();

        public NetSession(Socket socket, int recvBufferSize = 1024 * 10) {
            _socket = socket;

            _maxBufferSize = recvBufferSize;
            _recvBuffer = new byte[_maxBufferSize];

            _recvArgs.SetBuffer(new byte[_maxBufferSize], 0, _maxBufferSize);
            _recvArgs.Completed += new EventHandler<SocketAsyncEventArgs>(OnRecvCompleted);
            _recvArgs.UserToken = this;

            _sendArgs.Completed += new EventHandler<SocketAsyncEventArgs>(OnSendCompleted);
            _sendArgs.UserToken = this;

            DoRecv();
        }

        void DoRecv() {
            if (null == _socket) {
                return;
            }

            Console.WriteLine($"[{_socket}] DoRecv");
            if (false == _socket.ReceiveAsync(_recvArgs)) {
                OnRecvCompleted(null, _recvArgs);
            }
        }

        void OnRecvCompleted(object? sender, SocketAsyncEventArgs args) {
            if (SocketError.Success == args.SocketError) {
                if (0 < args.BytesTransferred && null != args.Buffer) {
                    Console.WriteLine($"[{_socket}] OnRecvCompleted. recvSize:{args.BytesTransferred}");
                    DataParsing(this, args.Buffer, args.Offset, args.BytesTransferred);
                } else {
                    Console.Write($"[{_socket}] OnRecvCompleted BytesTransferred < 0");
                    Disconnect();
                }
            } else {
                Console.WriteLine($"[{_socket}] Error: OnRecvCompleted. {args.SocketError}, BytesTransferred: {args.BytesTransferred}");
                Disconnect();
            }

            DoRecv();
        }
        protected virtual bool DataParsing(NetSession session, byte[] buffer, int offset, int transferred) => true;

        public void Send(byte[] sendBuffer) {
            lock (_sendLock) {
                _sendQueue.Enqueue(sendBuffer);
            }
            DoSend();
        }

        protected void DoSend() {
            if (null == _socket || !_socket.Connected) {
                return;
            }
            if (0 != _sendPendingList.Count) {
                return;
            }
            Console.WriteLine($"[{_socket}] DoSend");
            lock (_sendLock) {
                for (int cnt = 0; cnt < _sendQueue.Count && cnt < 10; ++cnt) {
                    var buffer = _sendQueue.Dequeue();
                    _sendPendingList.Add(buffer);
                }
            }

            _sendArgs.BufferList = _sendPendingList;
            bool pending = _socket.SendAsync(_sendArgs);
            if (false == pending) {
                OnSendCompleted(null, _sendArgs);
            }
        }

        void OnSendCompleted(object? sender, SocketAsyncEventArgs args) {
            if (SocketError.Success == args.SocketError && 0 < args.BytesTransferred) {
                Console.WriteLine($"[{_socket}] OnSendCompleted. transferred:{args.BytesTransferred}");
                try {
                    lock (_sendLock) {
                        _sendArgs.BufferList = null;
                        _sendPendingList.Clear();

                        if (0 < _sendQueue.Count) {
                            DoSend();
                        }
                    }
                } catch (Exception e) {
                    Console.WriteLine($"[{_socket}] Error: OnSendCompleted. {e.Message}, BytesTransferred: {args.BytesTransferred}");
                }
            } else {
                Console.WriteLine($"[{_socket}] Error: OnSendCompleted. {args.SocketError}");
                Disconnect();
            }
        }

        protected void Disconnect() {
            if (1 == Interlocked.Exchange(ref _disconnected, 1)) {
                return;
            }

            try {
                if (null != _socket && _socket.Connected) {
                    Console.WriteLine($"PlayerSession Disconnect: {_socket.RemoteEndPoint}");
                    _socket.Shutdown(SocketShutdown.Both);
                }
            } catch (ObjectDisposedException) {
                Console.WriteLine("Socket already closed");
            } catch (Exception e) {
                Console.WriteLine($"Error: {e.Message}");
            } finally {
                if (null != _socket) {
                    _socket.Close();
                    _socket = null;
                }
            }
        }

        public virtual void CheckHeartbeat() { }
    }
}
