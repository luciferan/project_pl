using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net;
using System.Net.Sockets;
using System.ComponentModel.Design;
using PL_Network;

namespace Server
{
    public class PlayerSession
    {
        Socket? _socket = null;
        int _disconnected = 0;

        SocketAsyncEventArgs _recvArgs = new();

        object _sendLock = new();
        SocketAsyncEventArgs _sendArgs = new();
        Queue<ArraySegment<byte>> _sendQueue = new();
        List<ArraySegment<byte>> _sendPendingList = new();

        PacketParser _packetParser = new();

        public PlayerSession(Socket socket) {
            _socket = socket;

            _recvArgs.SetBuffer(new byte[1024 * 10], 0, 1024 * 10);
            _recvArgs.Completed += new EventHandler<SocketAsyncEventArgs>(OnRecvCompleted);
            _recvArgs.UserToken = this;

            _sendArgs.Completed += new EventHandler<SocketAsyncEventArgs>(OnSendCompleted);
            _sendArgs.UserToken = this;

            DoRecv();
        }

        void DoRecv() {
            Console.WriteLine($"PlayerSession DoRecv");
            if (null != _socket && false == _socket.ReceiveAsync(_recvArgs)) {
                OnRecvCompleted(null, _recvArgs);
            }
        }

        void OnRecvCompleted(object? sender, SocketAsyncEventArgs args) {
            if (SocketError.Success == args.SocketError && 0 < args.BytesTransferred && null != args.Buffer) {
                Console.WriteLine($"PlayerSession OnRecvCompleted. recvSize:{args.BytesTransferred}");
                _packetParser.DataParsing(this, args.Buffer, args.Offset, args.BytesTransferred);
            } else {
                Console.WriteLine($"Error: {args.SocketError}, BytesTransferred: {args.BytesTransferred}");
                Disconnect();
            }

            DoRecv();
        }

        public void Send(byte[] sendBuffer) {
            lock (_sendLock) {
                _sendQueue.Enqueue(sendBuffer);
            }
            DoSend();
        }

        public void Send(PacketBase sendPacket) {
            lock (_sendLock) {
                Serializer ser = Serializer.PacketSerializer(sendPacket);
                _sendQueue.Enqueue(ser.Buffer);
            }
            DoSend();
        }

        void DoSend() {
            if (null == _socket || !_socket.Connected) {
                return;
            }
            if (0 != _sendPendingList.Count) {
                return;
            }

            Console.WriteLine($"PlayerSession DoSend");
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
                Console.WriteLine($"PlayerSession OnSendCompleted. sendSize:{args.BytesTransferred}");
                try {
                    lock (_sendLock) {
                        _sendArgs.BufferList = null;
                        _sendPendingList.Clear();

                        if (0 < _sendQueue.Count) {
                            DoSend();
                        }
                    }
                } catch (Exception e) {
                    Console.WriteLine($"Error: {e.Message}, BytesTransferred: {args.BytesTransferred}");
                }
            } else {
                Console.WriteLine($"Error: {args.SocketError}");
                Disconnect();
            }
        }

        void Disconnect() {
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
    }
}
