﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net;
using System.Net.Sockets;
using System.ComponentModel.Design;

namespace Server
{
    internal class PlayerSession
    {
        Socket? _socket = null;
        int _disconnected = 0;

        SocketAsyncEventArgs _recvArgs = new();
        byte[] _recvBuffer = new byte[1024];

        object _sendLock = new();
        SocketAsyncEventArgs _sendArgs = new();
        Queue<ArraySegment<byte>> _sendQueue = new();
        List<ArraySegment<byte>> _sendPendingList = new();

        PacketParser _packetParser = new();

        public PlayerSession(Socket socket) {
            _socket = socket;

            _recvArgs.SetBuffer(new byte[1024 * 10]);
            _recvArgs.Completed += new EventHandler<SocketAsyncEventArgs>(OnRecvCompleted);
            _recvArgs.UserToken = this;

            _sendArgs.Completed += new EventHandler<SocketAsyncEventArgs>(OnSendCompleted);
            _sendArgs.UserToken = this;

            DoRecv();
        }

        void DoRecv() {
            if (null != _socket && false != _socket.ReceiveAsync(_recvArgs)) {
                OnRecvCompleted(null, _recvArgs);
            }
        }

        void OnRecvCompleted(object? sender, SocketAsyncEventArgs args) {
            if (SocketError.Success == args.SocketError && 0 < args.BytesTransferred && null != args.Buffer) {
                _packetParser.DataParsing(args.Buffer, args.Offset, args.BytesTransferred, PacketHandler);
            } else {
                Console.WriteLine($"Error: {args.SocketError}, BytesTransferred: {args.BytesTransferred}");
                Disconnect();
            }

            DoRecv();
        }

        public void Send(byte[] sendBuffer) {
            lock (_sendLock) {
                _sendQueue.Enqueue(new ArraySegment<byte>(sendBuffer, 0, sendBuffer.Length));
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
                    Console.WriteLine($"Disconnect: {_socket.RemoteEndPoint}");
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

        void PacketHandler(Packet packet) {
            switch (packet.GetPacketType()) {
                case ProtocolCS.HEARTBEAT:
                case ProtocolCS.ECHO:
                    Console.WriteLine($"RecvPacket {packet.GetPacketType()}");
                    break;
                default:
                    break;
            }
        }
    }
}
